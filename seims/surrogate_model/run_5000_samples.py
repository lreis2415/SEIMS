"""Run SEIMS simulation for all 5000 HILLSLP samples and save results."""
import os
import sys
import pandas as pd
import numpy as np
from datetime import datetime
import time

seims_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if seims_root not in sys.path:
    sys.path.insert(0, seims_root)

from pygeoc.utils import get_config_parser
from scenario_analysis.spatialunits.config import SASlpPosConfig
from scenario_analysis.spatialunits.scenario import SUScenario

def run_all_samples(config_file, sample_file, output_file, start_idx=0):
    """Run SEIMS for all samples and save results."""

    print('='*70)
    print('Running SEIMS Simulation for 5000 HILLSLP Samples')
    print('='*70)
    print(f'Config: {config_file}')
    print(f'Samples: {sample_file}')
    print(f'Output: {output_file}')
    print(f'Start time: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}')
    print('='*70)

    # Initialize config
    sys.argv = ['run.py', '-ini', config_file]
    cf = get_config_parser()
    cf.read(config_file)
    sa_cfg = SASlpPosConfig(cf)
    sa_cfg.construct_indexes_units_gene()
    sa_cfg.key_bmps = {}

    # Load samples
    df = pd.read_csv(sample_file)
    n_samples = len(df)
    print(f'\nLoaded {n_samples} samples')
    print(f'Starting from sample {start_idx}')

    # Load existing results if resuming
    results = []
    if start_idx > 0 and os.path.exists(output_file):
        existing_df = pd.read_csv(output_file)
        results = existing_df.to_dict('records')
        print(f'Loaded {len(results)} existing results')

    # Process samples
    start_time = time.time()

    for idx in range(start_idx, n_samples):
        row = df.iloc[idx]

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

        # Run simulation
        try:
            sce = SUScenario(sa_cfg)
            sce.gene_values = gene_values
            sce.decoding_with_bmps_order()
            sce.export_to_mongodb()

            satisfied, [costs, maintains, incomes] = sce.satisfy_investment_constraints

            sce.execute_seims_model()
            sce.calculate_economy_bmps_order(costs, maintains, incomes)
            sce.calculate_environment_bmps_order()

            # Save result
            result = {
                'sample_id': idx,
                'n_bmps': n_bmps,
                'economy': sce.economy,
                'environment': sce.environment,
                'sediment': sce.sed_sum,
                'satisfied': satisfied
            }
            results.append(result)

            # Progress report
            elapsed = time.time() - start_time
            avg_time = elapsed / (idx - start_idx + 1)
            remaining = (n_samples - idx - 1) * avg_time

            if (idx + 1) % 10 == 0:
                print(f'[{idx+1}/{n_samples}] BMPs={n_bmps}, Econ={sce.economy:.2f}, Env={sce.environment:.2f}, Sed={sce.sed_sum:.0f}')
                print(f'  Elapsed: {elapsed/3600:.2f}h, Avg: {avg_time:.1f}s/sample, Remaining: {remaining/3600:.2f}h')

            # Save checkpoint every 100 samples
            if (idx + 1) % 100 == 0:
                temp_df = pd.DataFrame(results)
                temp_df.to_csv(output_file, index=False)
                print(f'  Checkpoint saved: {len(results)} results')

        except Exception as e:
            print(f'[ERROR] Sample {idx} failed: {e}')
            result = {
                'sample_id': idx,
                'n_bmps': n_bmps,
                'economy': np.nan,
                'environment': np.nan,
                'sediment': np.nan,
                'satisfied': False
            }
            results.append(result)

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
    print(f'Output: {output_file}')
    print('='*70)

if __name__ == '__main__':
    config_file = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/workspace/scenario_analysis_s_t_constrained.ini'
    sample_file = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/samples_spatiotemporal_hillslp_5000.csv'
    output_file = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/samples_spatiotemporal_hillslp_5000_results.csv'

    # Check if resuming
    start_idx = 0
    if os.path.exists(output_file):
        existing_df = pd.read_csv(output_file)
        start_idx = len(existing_df)
        print(f'Resuming from sample {start_idx}')

    run_all_samples(config_file, sample_file, output_file, start_idx)
