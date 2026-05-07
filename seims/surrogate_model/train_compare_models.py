"""
多模型对比训练脚本
输入特征: gene_*_type (one-hot 525维) + gene_*_time (数值 105维) = 630维
目标y   : sediment (原始物理单位 kg/年)
对比模型: RandomForest / ExtraTrees / XGBoost / GradientBoosting / MLP
"""
import os
import sys
import time
import warnings
import numpy as np
import pandas as pd
import joblib
import json
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from datetime import datetime
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import OneHotEncoder, StandardScaler
from sklearn.metrics import r2_score, mean_squared_error, mean_absolute_error
from sklearn.ensemble import RandomForestRegressor, ExtraTreesRegressor, GradientBoostingRegressor
from sklearn.neural_network import MLPRegressor
import xgboost as xgb

warnings.filterwarnings('ignore')

# ── 路径 ──
BASE_DIR    = 'D:/EGC/SEIMS-dev'
SAMPLE_FILE = f'{BASE_DIR}/data/youwuzhen10m/samples_spatiotemporal_hillslp_5000.csv'
RESULT_FILE = f'{BASE_DIR}/data/youwuzhen10m/samples_spatiotemporal_hillslp_5000_results.csv'
OUT_DIR     = f'{BASE_DIR}/data/youwuzhen10m/surrogate_model_comparison'

# ════════════════════════════════════════════════════
# 1. 特征工程
# ════════════════════════════════════════════════════
def build_features(samples_df):
    """
    type列 one-hot (0-4, 5类) + time列直接数值 (0-5)
    返回: X (ndarray, n x 630), encoder, type_cols, time_cols
    """
    type_cols = [f'gene_{i}_type' for i in range(105)]
    time_cols = [f'gene_{i}_time' for i in range(105)]

    X_type = samples_df[type_cols].values.astype(int)
    X_time = samples_df[time_cols].values.astype(float)

    # one-hot: 每列取值 0-4
    encoder = OneHotEncoder(
        categories=[list(range(5))] * 105,
        sparse_output=False,
        handle_unknown='ignore'
    )
    X_type_oh = encoder.fit_transform(X_type)   # (5000, 525)

    X = np.hstack([X_type_oh, X_time])          # (5000, 630)
    print(f'  feature shape: {X.shape}  '
          f'(type_onehot={X_type_oh.shape[1]}, time_raw={X_time.shape[1]})')
    return X, encoder


def get_target(results_df):
    return results_df['sediment'].values.reshape(-1, 1)


# ════════════════════════════════════════════════════
# 2. 评估指标
# ════════════════════════════════════════════════════
def calc_metrics(y_true, y_pred, label=''):
    y_true = y_true.ravel()
    y_pred = y_pred.ravel()
    r2   = r2_score(y_true, y_pred)
    rmse = np.sqrt(mean_squared_error(y_true, y_pred))
    mae  = mean_absolute_error(y_true, y_pred)
    mape = np.mean(np.abs((y_true - y_pred) / y_true)) * 100
    if label:
        print(f'  {label:6s} | R2={r2:.4f}  RMSE={rmse/1e6:.4f}M  '
              f'MAE={mae/1e6:.4f}M  MAPE={mape:.2f}%')
    return dict(r2=r2, rmse=rmse, mae=mae, mape=mape)


# ════════════════════════════════════════════════════
# 3. 模型定义
# ════════════════════════════════════════════════════
def get_models():
    return {
        'RandomForest': RandomForestRegressor(
            n_estimators=300, max_depth=None,
            min_samples_leaf=2, n_jobs=-1, random_state=42),

        'ExtraTrees': ExtraTreesRegressor(
            n_estimators=300, max_depth=None,
            min_samples_leaf=2, n_jobs=-1, random_state=42),

        'XGBoost': xgb.XGBRegressor(
            n_estimators=500, max_depth=7,
            learning_rate=0.05, subsample=0.8,
            colsample_bytree=0.8, min_child_weight=3,
            tree_method='hist', n_jobs=-1, random_state=42,
            early_stopping_rounds=30, eval_metric='rmse',
            verbosity=0),

        'GradientBoosting': GradientBoostingRegressor(
            n_estimators=300, max_depth=5,
            learning_rate=0.05, subsample=0.8,
            min_samples_leaf=3, random_state=42),

        'MLP': MLPRegressor(
            hidden_layer_sizes=(256, 128, 64, 32),
            activation='relu', solver='adam',
            learning_rate_init=0.001, max_iter=500,
            early_stopping=True, validation_fraction=0.1,
            n_iter_no_change=20, random_state=42, verbose=False),
    }


# ════════════════════════════════════════════════════
# 4. 训练 + 评估单个模型
# ════════════════════════════════════════════════════
def train_one(name, model, X_train, y_train, X_val, y_val, X_test, y_test,
              scaler_y, out_dir):
    print(f'\n[{name}]')
    t0 = time.time()

    if name == 'XGBoost':
        model.fit(X_train, y_train.ravel(),
                  eval_set=[(X_val, y_val.ravel())],
                  verbose=False)
    else:
        model.fit(X_train, y_train.ravel())

    elapsed = time.time() - t0
    print(f'  training time: {elapsed:.1f}s')

    def predict_orig(X):
        p = model.predict(X).reshape(-1, 1)
        return scaler_y.inverse_transform(p)

    y_train_orig = scaler_y.inverse_transform(y_train)
    y_val_orig   = scaler_y.inverse_transform(y_val)
    y_test_orig  = scaler_y.inverse_transform(y_test)

    tr = calc_metrics(y_train_orig, predict_orig(X_train), 'train')
    va = calc_metrics(y_val_orig,   predict_orig(X_val),   'val  ')
    te = calc_metrics(y_test_orig,  predict_orig(X_test),  'test ')

    model_path = os.path.join(out_dir, 'models', f'{name}.pkl')
    os.makedirs(os.path.dirname(model_path), exist_ok=True)
    joblib.dump(model, model_path)

    y_true_plot = y_test_orig.ravel()
    y_pred_plot = predict_orig(X_test).ravel()
    fig, ax = plt.subplots(figsize=(6, 6))
    ax.scatter(y_true_plot / 1e6, y_pred_plot / 1e6, alpha=0.4, s=15, color='steelblue')
    mn = min(y_true_plot.min(), y_pred_plot.min()) / 1e6
    mx = max(y_true_plot.max(), y_pred_plot.max()) / 1e6
    ax.plot([mn, mx], [mn, mx], 'r--', lw=1.5, label='1:1 line')
    ax.set_xlabel('True sediment (x1e6 kg/yr)')
    ax.set_ylabel('Predicted sediment (x1e6 kg/yr)')
    ax.set_title(f'{name}  R2={te["r2"]:.4f}  RMSE={te["rmse"]/1e6:.4f}M')
    ax.legend()
    ax.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.savefig(os.path.join(out_dir, f'scatter_{name}.png'), dpi=150)
    plt.close()

    return dict(name=name, train_time=elapsed, train=tr, val=va, test=te)


# ════════════════════════════════════════════════════
# 5. 汇总对比图
# ════════════════════════════════════════════════════
def plot_comparison(all_results, out_dir):
    names = [r['name'] for r in all_results]
    r2s   = [r['test']['r2']         for r in all_results]
    rmses = [r['test']['rmse'] / 1e6 for r in all_results]
    mapes = [r['test']['mape']       for r in all_results]

    fig, axes = plt.subplots(1, 3, figsize=(15, 5))
    colors = ['#4C72B0', '#55A868', '#C44E52', '#8172B2', '#CCB974']

    for ax, vals, title, ylabel in zip(
        axes,
        [r2s, rmses, mapes],
        ['Test R2', 'Test RMSE (x1e6 kg/yr)', 'Test MAPE (%)'],
        ['R2', 'RMSE (x1e6)', 'MAPE (%)']
    ):
        bars = ax.bar(names, vals, color=colors[:len(names)], edgecolor='white', linewidth=0.8)
        ax.set_title(title, fontsize=13)
        ax.set_ylabel(ylabel)
        ax.tick_params(axis='x', rotation=20)
        for bar, v in zip(bars, vals):
            ax.text(bar.get_x() + bar.get_width() / 2, bar.get_height() * 1.01,
                    f'{v:.4f}', ha='center', va='bottom', fontsize=9)
        ax.grid(axis='y', alpha=0.3)

    plt.suptitle('Surrogate Model Comparison (sediment, test set)', fontsize=14, y=1.02)
    plt.tight_layout()
    plt.savefig(os.path.join(out_dir, 'model_comparison.png'), dpi=150, bbox_inches='tight')
    plt.close()
    print(f'  Saved: {out_dir}/model_comparison.png')


# ════════════════════════════════════════════════════
# 6. MAIN
# ════════════════════════════════════════════════════
def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    print('=' * 70)
    print('Multi-Model Surrogate Comparison')
    print(f'Start: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}')
    print('=' * 70)

    print('\n[1] Loading data...')
    samples_df = pd.read_csv(SAMPLE_FILE)
    results_df = pd.read_csv(RESULT_FILE)
    print(f'  samples: {len(samples_df)},  results: {len(results_df)}')

    print('\n[2] Feature engineering...')
    X, encoder = build_features(samples_df)
    y = get_target(results_df)
    print(f'  sediment: min={y.min():.0f}  max={y.max():.0f}  mean={y.mean():.0f}')

    joblib.dump(encoder, os.path.join(OUT_DIR, 'onehot_encoder.pkl'))

    # 70 / 10 / 20 split
    X_tv, X_test, y_tv, y_test = train_test_split(X, y, test_size=0.20, random_state=42)
    X_train, X_val, y_train, y_val = train_test_split(X_tv, y_tv, test_size=0.125, random_state=42)
    print(f'\n[3] Split: train={len(X_train)}  val={len(X_val)}  test={len(X_test)}')

    scaler_y = StandardScaler()
    y_train_sc = scaler_y.fit_transform(y_train)
    y_val_sc   = scaler_y.transform(y_val)
    y_test_sc  = scaler_y.transform(y_test)
    joblib.dump(scaler_y, os.path.join(OUT_DIR, 'scaler_y.pkl'))

    scaler_X = StandardScaler()
    X_train_sc = scaler_X.fit_transform(X_train)
    X_val_sc   = scaler_X.transform(X_val)
    X_test_sc  = scaler_X.transform(X_test)
    joblib.dump(scaler_X, os.path.join(OUT_DIR, 'scaler_X.pkl'))

    print('\n[4] Training models...')
    models = get_models()
    all_results = []

    for name, model in models.items():
        if name == 'MLP':
            Xtr, Xva, Xte = X_train_sc, X_val_sc, X_test_sc
        else:
            Xtr, Xva, Xte = X_train, X_val, X_test
        result = train_one(name, model, Xtr, y_train_sc, Xva, y_val_sc,
                           Xte, y_test_sc, scaler_y, OUT_DIR)
        all_results.append(result)

    print('\n' + '=' * 70)
    print('COMPARISON SUMMARY (test set, original units):')
    print('=' * 70)
    print(f'{"Model":20s} {"R2":>8s} {"RMSE(M)":>10s} {"MAE(M)":>10s} {"MAPE(%)":>10s} {"Time(s)":>9s}')
    print('-' * 70)
    for r in sorted(all_results, key=lambda x: -x['test']['r2']):
        t = r['test']
        print(f'{r["name"]:20s} {t["r2"]:8.4f} {t["rmse"]/1e6:10.4f} '
              f'{t["mae"]/1e6:10.4f} {t["mape"]:10.2f} {r["train_time"]:9.1f}')

    summary = []
    for r in all_results:
        summary.append({
            'model':       r['name'],
            'train_time_s': round(r['train_time'], 1),
            'test_r2':     round(r['test']['r2'],   4),
            'test_rmse':   round(r['test']['rmse'],  1),
            'test_mae':    round(r['test']['mae'],   1),
            'test_mape':   round(r['test']['mape'],  4),
            'val_r2':      round(r['val']['r2'],     4),
        })
    with open(os.path.join(OUT_DIR, 'comparison_results.json'), 'w') as f:
        json.dump(summary, f, indent=2)

    print('\n[5] Plotting...')
    plot_comparison(all_results, OUT_DIR)

    best = max(all_results, key=lambda x: x['test']['r2'])
    print(f'\nBest model: {best["name"]}  (test R2={best["test"]["r2"]:.4f})')
    print(f'End: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}')
    print(f'Output dir: {OUT_DIR}')


if __name__ == '__main__':
    main()
