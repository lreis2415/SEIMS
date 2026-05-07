"""Generate 5000 spatiotemporal samples using unified optimizer.

This script runs 1 generation with population size 5000 to generate samples,
then extracts the gene values without running SEIMS evaluation.
"""
import os
import sys
import json
import pandas as pd
from datetime import datetime

# Add SEIMS root to path
seims_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if seims_root not in sys.path:
    sys.path.insert(0, seims_root)

from scenario_analysis.unified_config import UnifiedConfig
from scenario_analysis.unified_optimizer_v2 import create_scenario
from scenario_analysis.spatialunits.scenario import initialize_scenario_s_t

def generate_samples_from_optimizer(config_file, n_samples=5000):
    """Generate samples by initializing population without evaluation."""

    print('='*70)
    print('Generating Spatiotemporal Samples via Unified Optimizer')
    print('='*70)
    print(f'Config: {config_file}')
    print(f'Samples: {n_samples}')
    print(f'Start: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}')

    # Load config
    with open(config_file, 'r') as f:
        config_dict = json.load(f)

    # Override pop_size
    config_dict['optimization']['pop_size'] = n_samples
    config_dict['optimization']['max_gen'] = 1

    # Create unified config
    cfg = UnifiedConfig(config_dict)

    # Create scenario object
    sa_cfg = create_scenario(cfg)

    print(f'\nInitializing {n_samples} individuals...')

    samples = []
    n_units = sa_cfg.units_num

    for i in range(n_samples):
        if (i + 1) % 500 == 0:
            print(f'  Generated {i + 1}/{n_samples}...')

        # Initialize using HILLSLP strategy
        gene_values = initialize_scenario_s_t(sa_cfg)

        # Adjust time distribution: 30% period 1, 17.5% each for 2-5
        import random
        time_weights = [0.30, 0.175, 0.175, 0.175, 0.175]
        for j in range(len(gene_values)):
            if gene_values[j] > 0:
                bmp_type = gene_values[j] // 1000
                time_period = random.choices([1, 2, 3, 4, 5], weights=time_weights)[0]
                gene_values[j] = bmp_type * 1000 + time_period

        # Convert to sample format
        sample = {'sample_id': i}
        for j in range(n_units):
            if gene_values[j] == 0:
                sample[f'gene_{j}_type'] = 0
                sample[f'gene_{j}_time'] = 0
            else:
                sample[f'gene_{j}_type'] = gene_values[j] // 1000
                sample[f'gene_{j}_time'] = gene_values[j] % 1000

        samples.append(sample)

    print(f'\nGenerated {len(samples)} samples!')
    return samples


def save_and_analyze(samples, output_file):
    """Save samples and print statistics."""
    df = pd.DataFrame(samples)
    df.to_csv(output_file, index=False)
    print(f'\nSaved to: {output_file}')

    # Statistics
    import numpy as np
    n_units = 105
    n_samples = len(samples)

    print('\n' + '='*70)
    print('Sample Statistics')
    print('='*70)

    # Density
    densities = []
    for _, row in df.iterrows():
        n_bmps = sum(1 for i in range(n_units) if row[f'gene_{i}_type'] > 0)
        densities.append(n_bmps)
    densities = np.array(densities)

    print(f'\n[BMP Density]')
    print(f'  Samples: {n_samples}')
    print(f'  Range: {densities.min()} - {densities.max()}')
    print(f'  Mean: {densities.mean():.2f}')
    print(f'  Median: {np.median(densities):.0f}')

    # Type distribution
    print(f'\n[Type Distribution]')
    type_counts = {1: 0, 2: 0, 3: 0, 4: 0}
    for _, row in df.iterrows():
        for i in range(n_units):
            t = row[f'gene_{i}_type']
            if t > 0:
                type_counts[t] += 1

    total = sum(type_counts.values())
    for t in [1, 2, 3, 4]:
        pct = type_counts[t] / total * 100 if total > 0 else 0
        print(f'  Type {t}: {type_counts[t]:6d} ({pct:5.1f}%)')

    # Time distribution
    print(f'\n[Time Distribution]')
    time_counts = {1: 0, 2: 0, 3: 0, 4: 0, 5: 0}
    for _, row in df.iterrows():
        for i in range(n_units):
            p = row[f'gene_{i}_time']
            if p > 0:
                time_counts[p] += 1

    for p in [1, 2, 3, 4, 5]:
        pct = time_counts[p] / total * 100 if total > 0 else 0
        print(f'  Period {p}: {time_counts[p]:6d} ({pct:5.1f}%)')

    print('\n' + '='*70)


if __name__ == '__main__':
    config_file = 'D:/EGC/SEIMS-dev/seims/surrogate_model/config_generate_samples.json'
    output_file = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/samples_spatiotemporal_hillslp_5000.csv'

    samples = generate_samples_from_optimizer(config_file, n_samples=5000)
    save_and_analyze(samples, output_file)

    print(f'\nEnd: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}')
    print('='*70)
