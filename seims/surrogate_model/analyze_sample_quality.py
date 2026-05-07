"""
样本质量分析：
1. 基因值分布与覆盖度（含重复检测）
2. 抽样验证：每100个样本取1个重跑，对比结果
"""
import os
import sys
import pandas as pd
import numpy as np
from datetime import datetime
import time

# ── 路径设置 ──
SEIMS_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if SEIMS_ROOT not in sys.path:
    sys.path.insert(0, SEIMS_ROOT)

SAMPLE_FILE  = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/samples_spatiotemporal_hillslp_5000.csv'
RESULT_FILE  = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/samples_spatiotemporal_hillslp_5000_results.csv'
VERIFY_OUT   = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/verification_results.csv'
CONFIG_FILE  = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/workspace/scenario_analysis_s_t_constrained.ini'

# ════════════════════════════════════════════════════
# PART 1: 分布与覆盖度分析
# ════════════════════════════════════════════════════
def analyze_distribution(samples_df, results_df):
    print('\n' + '='*70)
    print('PART 1: 基因值分布与覆盖度分析')
    print('='*70)

    # --- 1.1 基本信息 ---
    n = len(samples_df)
    n_genes = sum(1 for c in samples_df.columns if c.endswith('_type'))
    print(f'\n样本总数: {n}')
    print(f'基因位数: {n_genes}（每位有 type + time 两个维度）')

    # --- 1.2 重复检测 ---
    type_cols = [c for c in samples_df.columns if c.endswith('_type')]
    time_cols = [c for c in samples_df.columns if c.endswith('_time')]
    gene_cols = type_cols + time_cols

    dup_full = samples_df.duplicated(subset=gene_cols, keep=False)
    dup_type_only = samples_df.duplicated(subset=type_cols, keep=False)

    print(f'\n【重复检测】')
    print(f'  完整基因向量重复数（type+time）: {dup_full.sum()} 行  ({dup_full.mean()*100:.2f}%)')
    print(f'  仅BMP类型重复数（忽略时间）    : {dup_type_only.sum()} 行  ({dup_type_only.mean()*100:.2f}%)')
    n_unique_full = samples_df[gene_cols].drop_duplicates().shape[0]
    n_unique_type = samples_df[type_cols].drop_duplicates().shape[0]
    print(f'  唯一完整基因组合数: {n_unique_full}')
    print(f'  唯一BMP类型组合数 : {n_unique_type}')

    # --- 1.3 n_bmps 分布 ---
    nb = results_df['n_bmps']
    print(f'\n【BMP数量分布（n_bmps）】')
    print(f'  最小值: {nb.min()},  最大值: {nb.max()}')
    print(f'  均值  : {nb.mean():.2f},  中位数: {nb.median():.0f}')
    print(f'  标准差: {nb.std():.2f}')
    bins = [0, 1, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 106]
    hist, edges = np.histogram(nb, bins=bins)
    print(f'  分布直方图:')
    for i, cnt in enumerate(hist):
        bar = '█' * (cnt // 20)
        print(f'    [{edges[i]:3.0f}-{edges[i+1]:3.0f}): {cnt:4d}  {bar}')

    # --- 1.4 基因type值分布 ---
    all_types = samples_df[type_cols].values.flatten()
    type_counts = pd.Series(all_types).value_counts().sort_index()
    total_slots = len(all_types)
    print(f'\n【基因type值分布（全部 {total_slots} 个基因位）】')
    for val, cnt in type_counts.items():
        print(f'  type={val}: {cnt:8d}  ({cnt/total_slots*100:.2f}%)')

    # --- 1.5 基因time值分布（仅激活位） ---
    # 找到激活的基因（type > 0）对应的time
    all_type_arr = samples_df[type_cols].values  # (5000, 105)
    all_time_arr = samples_df[time_cols].values
    active_mask = all_type_arr > 0
    active_times = all_time_arr[active_mask]
    time_counts = pd.Series(active_times).value_counts().sort_index()
    print(f'\n【基因time值分布（仅激活基因位，共 {active_mask.sum()} 个）】')
    for val, cnt in time_counts.items():
        print(f'  time={val}: {cnt:8d}  ({cnt/active_mask.sum()*100:.2f}%)')

    # --- 1.6 结果统计 ---
    print(f'\n【模拟结果统计】')
    for col in ['economy', 'environment', 'sediment']:
        s = results_df[col]
        print(f'  {col:12s}: min={s.min():.2f}, max={s.max():.2f}, mean={s.mean():.2f}, std={s.std():.2f}')
    sat = results_df['satisfied']
    print(f'  satisfied=True : {sat.sum()}  ({sat.mean()*100:.1f}%)')
    print(f'  satisfied=False: {(~sat).sum()}  ({(~sat).mean()*100:.1f}%)')

    # --- 1.7 异常值检测 ---
    print(f'\n【异常值检测】')
    nan_rows = results_df[results_df[['economy','environment','sediment']].isna().any(axis=1)]
    print(f'  NaN结果行数: {len(nan_rows)}')
    zero_env = results_df[results_df['environment'] == 0]
    print(f'  environment=0 行数: {len(zero_env)}')
    zero_sed = results_df[results_df['sediment'] == 0]
    print(f'  sediment=0 行数   : {len(zero_sed)}')

    # 检查 n_bmps==0 时 economy 是否为0
    zero_bmp = results_df[results_df['n_bmps'] == 0]
    nonzero_econ = zero_bmp[zero_bmp['economy'] != 0.0]
    print(f'  n_bmps=0 但 economy≠0 的行数: {len(nonzero_econ)} (预期=0)')

    return n_unique_full, dup_full.sum()


# ════════════════════════════════════════════════════
# PART 2: 抽样验证（每100个取1个，共50个）
# ════════════════════════════════════════════════════
def verify_samples(samples_df, results_df, verify_indices):
    print('\n' + '='*70)
    print('PART 2: 抽样验证（重跑对比）')
    print('='*70)
    print(f'验证样本索引: {verify_indices[:5]} ... (共{len(verify_indices)}个)')
    print(f'开始时间: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}')

    from pygeoc.utils import get_config_parser
    from scenario_analysis.spatialunits.config import SASlpPosConfig
    from scenario_analysis.spatialunits.scenario import SUScenario

    # 初始化 config
    sys.argv = ['run.py', '-ini', CONFIG_FILE]
    cf = get_config_parser()
    cf.read(CONFIG_FILE)
    sa_cfg = SASlpPosConfig(cf)
    sa_cfg.construct_indexes_units_gene()
    sa_cfg.key_bmps = {}

    type_cols = [c for c in samples_df.columns if c.endswith('_type')]

    verify_records = []
    n_match = 0
    n_fail  = 0
    TOLERANCE = 1e-3  # 相对误差容忍度

    for rank, idx in enumerate(verify_indices):
        row = samples_df.iloc[idx]
        orig = results_df.iloc[idx]

        # 构造 gene_values
        gene_values = []
        for i in range(sa_cfg.units_num):
            bmp_type  = int(row[f'gene_{i}_type'])
            time_period = int(row[f'gene_{i}_time'])
            if bmp_type == 0:
                gene_values.append(0)
            else:
                gene_values.append(bmp_type * 1000 + time_period)

        t0 = time.time()
        try:
            sce = SUScenario(sa_cfg)
            sce.gene_values = gene_values
            sce.decoding_with_bmps_order()
            sce.export_to_mongodb()
            satisfied, [costs, maintains, incomes] = sce.satisfy_investment_constraints
            sce.execute_seims_model()
            sce.calculate_economy_bmps_order(costs, maintains, incomes)
            sce.calculate_environment_bmps_order()

            new_econ  = sce.economy
            new_env   = sce.environment
            new_sed   = sce.sed_sum
            elapsed   = time.time() - t0

            # 对比
            def rel_err(a, b):
                if b == 0:
                    return abs(a - b)
                return abs(a - b) / abs(b)

            econ_ok = rel_err(new_econ, orig['economy'])   < TOLERANCE
            env_ok  = rel_err(new_env,  orig['environment']) < TOLERANCE
            sed_ok  = rel_err(new_sed,  orig['sediment'])   < TOLERANCE
            all_ok  = econ_ok and env_ok and sed_ok
            status  = 'PASS' if all_ok else 'FAIL'
            if all_ok:
                n_match += 1
            else:
                n_fail += 1

            verify_records.append({
                'sample_idx': idx,
                'status': status,
                'orig_economy':     orig['economy'],
                'new_economy':      new_econ,
                'econ_rel_err':     rel_err(new_econ, orig['economy']),
                'orig_environment': orig['environment'],
                'new_environment':  new_env,
                'env_rel_err':      rel_err(new_env, orig['environment']),
                'orig_sediment':    orig['sediment'],
                'new_sediment':     new_sed,
                'sed_rel_err':      rel_err(new_sed, orig['sediment']),
                'elapsed_s':        elapsed,
            })

            print(f'  [{rank+1:2d}/{len(verify_indices)}] idx={idx:4d}  {status}  '
                  f'econ_err={rel_err(new_econ,orig["economy"]):.2e}  '
                  f'env_err={rel_err(new_env,orig["environment"]):.2e}  '
                  f'sed_err={rel_err(new_sed,orig["sediment"]):.2e}  '
                  f'({elapsed:.0f}s)')

        except Exception as e:
            n_fail += 1
            print(f'  [{rank+1:2d}/{len(verify_indices)}] idx={idx:4d}  ERROR: {e}')
            verify_records.append({
                'sample_idx': idx, 'status': 'ERROR',
                'orig_economy': orig['economy'], 'new_economy': np.nan,
                'econ_rel_err': np.nan,
                'orig_environment': orig['environment'], 'new_environment': np.nan,
                'env_rel_err': np.nan,
                'orig_sediment': orig['sediment'], 'new_sediment': np.nan,
                'sed_rel_err': np.nan,
                'elapsed_s': time.time() - t0,
            })

    # 汇总
    vdf = pd.DataFrame(verify_records)
    vdf.to_csv(VERIFY_OUT, index=False)

    print(f'\n【验证汇总】')
    print(f'  通过 (PASS)  : {n_match} / {len(verify_indices)}  ({n_match/len(verify_indices)*100:.1f}%)')
    print(f'  失败 (FAIL)  : {n_fail}  ({n_fail/len(verify_indices)*100:.1f}%)')

    fail_rows = vdf[vdf['status'] == 'FAIL']
    if len(fail_rows) > 0:
        print(f'\n  ⚠️  失败样本详情:')
        for _, r in fail_rows.iterrows():
            print(f'    idx={int(r["sample_idx"])}: '
                  f'econ_err={r["econ_rel_err"]:.3e}, '
                  f'env_err={r["env_rel_err"]:.3e}, '
                  f'sed_err={r["sed_rel_err"]:.3e}')

    elapsed_rows = vdf['elapsed_s'].dropna()
    print(f'\n  平均重跑时间: {elapsed_rows.mean():.1f}s/样本')
    print(f'  验证结果已保存: {VERIFY_OUT}')
    print(f'  结束时间: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}')

    return vdf


# ════════════════════════════════════════════════════
# MAIN
# ════════════════════════════════════════════════════
if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--dist-only', action='store_true', help='只做分布分析，不重跑验证')
    parser.add_argument('--verify-only', action='store_true', help='只做验证')
    args = parser.parse_args()

    print(f'加载数据...')
    samples_df = pd.read_csv(SAMPLE_FILE)
    results_df = pd.read_csv(RESULT_FILE)
    print(f'  样本文件: {len(samples_df)} 行')
    print(f'  结果文件: {len(results_df)} 行')

    if not args.verify_only:
        analyze_distribution(samples_df, results_df)

    if not args.dist_only:
        # 每100个取1个：0, 100, 200, ..., 4900 → 50个样本
        verify_indices = list(range(0, 5000, 100))
        verify_samples(samples_df, results_df, verify_indices)

    print('\nDone.')
