"""Run SEIMS simulation for 5000 samples in parallel."""
import os
import sys
import pandas as pd
import numpy as np
from datetime import datetime
import time
from multiprocessing import Pool, Manager, cpu_count

seims_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if seims_root not in sys.path:
    sys.path.insert(0, seims_root)

from pygeoc.utils import get_config_parser
from scenario_analysis.spatialunits.config import SASlpPosConfig
from scenario_analysis.spatialunits.scenario import SUScenario

def run_single_sample(args):
    """Run simulation for a single sample."""
    idx, row, config_file = args

    # Initialize config for this process
    sys.argv = ['run.py', '-ini', config_file]
    cf = get_config_parser()
    cf.read(config_file)
    sa_cfg = SASlpPosConfig(cf)
    sa_cfg.construct_indexes_units_gene()
    sa_cfg.key_bmps = {}

    # Convert to gene values
    gene_values = []
    for i in range(sa_cfg.units_num):
        bmp_type = int(row[f'gene_{i}_type'])
        time_period = int(row[f'gene_{i}_time'])
        if bmp_type == 0:
            gene_values.append(0)
        else:
            gene_values.append(bmp_type * 1000 + time_period)

    n_bmps = sum(1 for g in gene_values if g > 0)

    try:
        sce = SUScenario(sa_cfg)
        sce.gene_values = gene_values
        sce.decoding_with_bmps_order()
        sce.export_to_mongodb()

        satisfied, [costs, maintains, incomes] = sce.satisfy_investment_constraints

        sce.execute_seims_model()
        sce.calculate_economy_bmps_order(costs, maintains, incomes)
        sce.calculate_environment_bmps_order()

        return {
            'sample_id': idx,
            'scenario_id': sce.ID,
            'n_bmps': n_bmps,
            'economy': sce.economy,
            'environment': sce.environment,
            'sediment': sce.sed_sum,
            'satisfied': satisfied
        }
    except Exception as e:
        print(f'[ERROR] Sample {idx} failed: {e}')
        return {
            'sample_id': idx,
            'n_bmps': n_bmps,
            'economy': np.nan,
            'environment': np.nan,
            'sediment': np.nan,
            'satisfied': False
        }

def run_parallel(config_file, sample_file, output_file, n_workers=4, start_idx=0):
    """Run samples in parallel."""

    print('='*70)
    print(f'Running 5000 Samples in Parallel ({n_workers} workers)')
    print('='*70)
    print(f'Config: {config_file}')
    print(f'Samples: {sample_file}')
    print(f'Output: {output_file}')
    print(f'Workers: {n_workers}')
    print(f'Start time: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}')
    print('='*70)

    # Load samples
    df = pd.read_csv(sample_file)
    n_samples = len(df)

    # Load existing results
    results = []
    if start_idx > 0 and os.path.exists(output_file):
        existing_df = pd.read_csv(output_file)
        results = existing_df.to_dict('records')
        print(f'Loaded {len(results)} existing results')

    # Prepare tasks
    tasks = [(idx, df.iloc[idx], config_file) for idx in range(start_idx, n_samples)]

    start_time = time.time()
    completed = start_idx

    # Run in parallel
    with Pool(processes=n_workers) as pool:
        for result in pool.imap_unordered(run_single_sample, tasks):
            results.append(result)
            completed += 1

            if completed % 10 == 0:
                elapsed = time.time() - start_time
                avg_time = elapsed / (completed - start_idx)
                remaining = (n_samples - completed) * avg_time
                print(f'[{completed}/{n_samples}] Elapsed: {elapsed/3600:.2f}h, Avg: {avg_time:.1f}s, Remaining: {remaining/3600:.2f}h')

            if completed % 100 == 0:
                temp_df = pd.DataFrame(results)
                temp_df.to_csv(output_file, index=False)
                print(f'  Checkpoint saved: {len(results)} results')

    # Final save
    final_df = pd.DataFrame(results)
    final_df.to_csv(output_file, index=False)

    total_time = time.time() - start_time
    print('\n' + '='*70)
    print('Completed!')
    print('='*70)
    print(f'Total samples: {n_samples}')
    print(f'Successful: {final_df["economy"].notna().sum()}')
    print(f'Failed: {final_df["economy"].isna().sum()}')
    print(f'Total time: {total_time/3600:.2f} hours')
    print(f'Average time: {total_time/n_samples:.1f} seconds/sample')
    print(f'Speedup: {n_workers}x (theoretical)')
    print(f'Output: {output_file}')
    print('='*70)

if __name__ == '__main__':
    from multiprocessing import freeze_support
    freeze_support()  # Required for Windows

    config_file = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/workspace/scenario_analysis_s_t_constrained.ini'
    sample_file = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/samples_spatiotemporal_hillslp_5000.csv'
    output_file = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/samples_spatiotemporal_hillslp_5000_results.csv'

    # Number of parallel workers (adjust based on CPU cores)
    n_workers = min(4, cpu_count())  # Use 4 workers or CPU count, whichever is smaller

    # Check if resuming
    start_idx = 0
    if os.path.exists(output_file):
        existing_df = pd.read_csv(output_file)
        start_idx = len(existing_df)
        print(f'Resuming from sample {start_idx}')

    run_parallel(config_file, sample_file, output_file, n_workers, start_idx)
