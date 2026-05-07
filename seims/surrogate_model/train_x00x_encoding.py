"""Train surrogate model using merged gene encoding (type*1000 + time).

This encoding avoids the issue where time=0 was being learned as a separate
pattern for samples with no BMP.
"""
import os
import pickle
import json
import numpy as np
import pandas as pd
from sklearn.preprocessing import StandardScaler, OneHotEncoder
from sklearn.model_selection import train_test_split
from sklearn.metrics import r2_score, mean_squared_error
import joblib
import warnings
warnings.filterwarnings('ignore')

# Paths
DATA_DIR = 'D:/EGC/SEIMS-dev/data/youwuzhen10m'
OUTPUT_DIR = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/surrogate_x00x'

os.makedirs(OUTPUT_DIR, exist_ok=True)

print("=" * 60)
print("Training Surrogate Model with x00x Encoding")
print("=" * 60)
print()

# 1. Load data
print("[1] Loading data...")
df_samples = pd.read_csv(f'{DATA_DIR}/samples_spatiotemporal_hillslp_5000.csv')
df_results = pd.read_csv(f'{DATA_DIR}/samples_spatiotemporal_hillslp_5000_results.csv')
print(f"  samples: {len(df_samples)}, results: {len(df_results)}")

# 2. Create merged gene values (type*1000 + time)
print("\n[2] Creating merged gene encoding...")
type_cols = [c for c in df_samples.columns if 'type' in c]
time_cols = [c for c in df_samples.columns if 'time' in c]

n_units = 105
gene_cols = []
for i in range(n_units):
    gene_cols.append(f'gene_{i}')

# Create gene matrix with merged encoding
gene_matrix = np.zeros((len(df_samples), n_units), dtype=int)
for i in range(n_units):
    gene_matrix[:, i] = df_samples[f'gene_{i}_type'].values * 1000 + df_samples[f'gene_{i}_time'].values

gene_df = pd.DataFrame(gene_matrix, columns=gene_cols)
print(f"  gene matrix shape: {gene_matrix.shape}")
print(f"  unique gene values: {np.unique(gene_matrix)}")

# 3. One-hot encode each position
print("\n[3] One-hot encoding each gene position...")
# For each position, one-hot encode the 21 possible values (0, 1001-4005)
# Total features: 105 * 21 = 2205

# Flatten for one-hot encoding across all positions
# Actually, we need to encode each position independently
# Each position has 21 possible values

# Stack all positions for fitting the encoder
all_genes_flat = gene_matrix.flatten().reshape(-1, 1)  # (5000*105, 1)
print(f"  Flattened shape: {all_genes_flat.shape}")

# Fit OneHotEncoder on all possible values
all_possible_values = np.unique(gene_matrix.flatten())
print(f"  All possible values: {all_possible_values}")

# Create encoder that handles all 21 values
encoder = OneHotEncoder(categories=[all_possible_values.tolist()]*n_units, sparse_output=False, handle_unknown='ignore')

# Fit on all samples
encoder.fit(gene_matrix)
print(f"  encoder n_features_in_: {encoder.n_features_in_}")

# Transform
X_onehot = encoder.transform(gene_matrix)
print(f"  X_onehot shape: {X_onehot.shape}")

# Target
y = df_results['sediment'].values
print(f"  y shape: {y.shape}")
print(f"  sediment range: {y.min():,.0f} - {y.max():,.0f}, mean: {y.mean():,.0f}")

# 4. Train/val/test split with stratification on n_bmps
print("\n[4] Splitting data...")
n_bmps = (gene_matrix > 0).sum(axis=1)

# Stratified split based on n_bmps
from sklearn.model_selection import StratifiedShuffleSplit

# Create bins for n_bmps
n_bmps_bins = pd.cut(n_bmps, bins=[-1, 10, 30, 50, 70, 200], labels=[0, 1, 2, 3, 4])
n_bmps_bins = n_bmps_bins.astype(int)  # Convert to int

sss = StratifiedShuffleSplit(n_splits=1, test_size=0.2, random_state=42)
train_idx, test_idx = next(sss.split(X_onehot, n_bmps_bins))

X_train = X_onehot[train_idx]
X_test = X_onehot[test_idx]
y_train = y[train_idx]
y_test = y[test_idx]

# Further split train into train/val
sss2 = StratifiedShuffleSplit(n_splits=1, test_size=0.125, random_state=42)  # 0.125 of 0.8 = 0.1 of total
train_bins = n_bmps_bins[train_idx]
train_idx2, val_idx = next(sss2.split(X_train, train_bins))

X_tr = X_train[train_idx2]
X_val = X_train[val_idx]
y_tr = y_train[train_idx2]
y_val = y_train[val_idx]

print(f"  train: {len(X_tr)}, val: {len(X_val)}, test: {len(X_test)}")

# 5. Scale
print("\n[5] Scaling features and target...")
scaler_X = StandardScaler()
X_tr_scaled = scaler_X.fit_transform(X_tr)
X_val_scaled = scaler_X.transform(X_val)
X_test_scaled = scaler_X.transform(X_test)

scaler_y = StandardScaler()
y_tr_scaled = scaler_y.fit_transform(y_tr.reshape(-1, 1)).flatten()
y_val_scaled = scaler_y.transform(y_val.reshape(-1, 1)).flatten()
y_test_scaled = scaler_y.transform(y_test.reshape(-1, 1)).flatten()

# 6. Train XGBoost
print("\n[6] Training XGBoost...")
import xgboost as xgb

model = xgb.XGBRegressor(
    n_estimators=500,
    max_depth=7,
    learning_rate=0.05,
    subsample=0.8,
    colsample_bytree=0.8,
    min_child_weight=3,
    random_state=42,
    n_jobs=-1
)

model.fit(
    X_tr_scaled, y_tr_scaled,
    eval_set=[(X_val_scaled, y_val_scaled)],
    verbose=50
)

# 7. Evaluate
print("\n[7] Evaluating model...")
y_tr_pred = model.predict(X_tr_scaled)
y_val_pred = model.predict(X_val_scaled)
y_test_pred = model.predict(X_test_scaled)

# Inverse transform predictions
y_tr_pred_inv = scaler_y.inverse_transform(y_tr_pred.reshape(-1, 1)).flatten()
y_val_pred_inv = scaler_y.inverse_transform(y_val_pred.reshape(-1, 1)).flatten()
y_test_pred_inv = scaler_y.inverse_transform(y_test_pred.reshape(-1, 1)).flatten()

def calc_metrics(y_true, y_pred):
    r2 = r2_score(y_true, y_pred)
    rmse = np.sqrt(mean_squared_error(y_true, y_pred))
    mae = np.mean(np.abs(y_true - y_pred))
    mape = np.mean(np.abs((y_true - y_pred) / y_true)) * 100
    return r2, rmse, mae, mape

r2_tr, rmse_tr, mae_tr, mape_tr = calc_metrics(y_tr, y_tr_pred_inv)
r2_val, rmse_val, mae_val, mape_val = calc_metrics(y_val, y_val_pred_inv)
r2_test, rmse_test, mae_test, mape_test = calc_metrics(y_test, y_test_pred_inv)

print(f"\n  Train: R2={r2_tr:.4f}, RMSE={rmse_tr/1e6:.4f}M, MAE={mae_tr/1e6:.4f}M, MAPE={mape_tr:.2f}%")
print(f"  Val:   R2={r2_val:.4f}, RMSE={rmse_val/1e6:.4f}M, MAE={mae_val/1e6:.4f}M, MAPE={mape_val:.2f}%")
print(f"  Test:  R2={r2_test:.4f}, RMSE={rmse_test/1e6:.4f}M, MAE={mae_test/1e6:.4f}M, MAPE={mape_test:.2f}%")

# 8. Check base scenario prediction
print("\n[8] Checking base scenario prediction...")
# Base scenario: all zeros
base_gene = np.zeros((1, n_units), dtype=int)
base_X = encoder.transform(base_gene)
base_X_scaled = scaler_X.transform(base_X)
base_pred_norm = model.predict(base_X_scaled)[0]
base_pred = scaler_y.inverse_transform([[base_pred_norm]])[0][0]
print(f"  Base scenario prediction: {base_pred:,.2f}")
print(f"  True base (n_bmps=0): 48,181,648")
print(f"  Difference: {(base_pred - 48181648)/1e6:.2f}M")

# 9. Save model
print("\n[9] Saving model...")
joblib.dump(model, f'{OUTPUT_DIR}/surrogate_model.pkl')
joblib.dump(scaler_X, f'{OUTPUT_DIR}/scaler_X.pkl')
joblib.dump(scaler_y, f'{OUTPUT_DIR}/scaler_y.pkl')
joblib.dump(encoder, f'{OUTPUT_DIR}/onehot_encoder.pkl')

# Save training log
with open(f'{OUTPUT_DIR}/train.log', 'w') as f:
    f.write("=" * 60 + "\n")
    f.write("Surrogate Model Training with x00x Encoding\n")
    f.write("=" * 60 + "\n\n")
    f.write(f"[1] Data: {len(df_samples)} samples\n")
    f.write(f"[2] Features: {X_onehot.shape[1]} (105 positions × 21 values)\n")
    f.write(f"[3] Split: train={len(X_tr)}, val={len(X_val)}, test={len(X_test)}\n")
    f.write(f"[4] Train R2={r2_tr:.4f}, Val R2={r2_val:.4f}, Test R2={r2_test:.4f}\n")
    f.write(f"[5] Base scenario: pred={base_pred:.2f}, true=48181648\n")
    f.write("\nMetric units: RMSE, MAE in millions\n")

print("\nDone! Model saved to:", OUTPUT_DIR)