"""Generate spatiotemporal samples using HILLSLP strategy.

This script reuses the initialization method from spatio-temporal optimization
to generate 5000 samples with proper BMP configuration strategy.
"""
import os
import sys
import random
import pandas as pd
from datetime import datetime

# Add SEIMS root to path
seims_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if seims_root not in sys.path:
    sys.path.insert(0, seims_root)

from pygeoc.utils import get_config_parser
from scenario_analysis.spatialunits.config import SASlpPosConfig
from scenario_analysis.spatialunits.scenario import initialize_scenario_s_t

# Set random seed for reproducibility
random.seed(42)

def generate_samples_with_hillslp(config_file, n_samples=5000):
    """Generate samples using HILLSLP initialization strategy.

    Args:
        config_file: Path to scenario analysis config file
        n_samples: Number of samples to generate

    Returns:
        List of sample dictionaries
    """
    print('='*70)
    print('Generating Spatiotemporal Samples with HILLSLP Strategy')
    print('='*70)
    print(f'Config file: {config_file}')
    print(f'Target samples: {n_samples}')
    print(f'Start time: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}')

    # Set sys.argv for config parser
    sys.argv = ['generate_samples_with_hillslp.py', '-ini', config_file]

    # Parse configuration
    cf = get_config_parser()
    cf.read(config_file)
    sa_cfg = SASlpPosConfig(cf)

    # Adjust time period weights: 30% for period 1, 17.5% for periods 2-5
    # This is done by modifying the random selection in initialize_s_t
    # We'll generate samples and post-process time distribution

    samples = []
    n_units = sa_cfg.units_num

    print(f'\nGenerating {n_samples} samples...')
    for sample_id in range(n_samples):
        if (sample_id + 1) % 500 == 0:
            print(f'  Generated {sample_id + 1}/{n_samples} samples...')

        # Initialize using HILLSLP strategy
        gene_values = initialize_scenario_s_t(sa_cfg)

        # Adjust time distribution: 30% period 1, 17.5% each for periods 2-5
        time_weights = [0.30, 0.175, 0.175, 0.175, 0.175]
        for i in range(len(gene_values)):
            if gene_values[i] > 0:
                bmp_type = gene_values[i] // 1000
                # Reassign time period with new weights
                time_period = random.choices([1, 2, 3, 4, 5], weights=time_weights)[0]
                gene_values[i] = bmp_type * 1000 + time_period

        # Convert to type and time columns
        sample = {'sample_id': sample_id}
        for i in range(n_units):
            if gene_values[i] == 0:
                sample[f'gene_{i}_type'] = 0
                sample[f'gene_{i}_time'] = 0
            else:
                sample[f'gene_{i}_type'] = gene_values[i] // 1000
                sample[f'gene_{i}_time'] = gene_values[i] % 1000

        samples.append(sample)

    print(f'\nGenerated {len(samples)} samples successfully!')
    return samples


def save_samples(samples, output_file):
    """Save samples to CSV file."""
    df = pd.DataFrame(samples)
    df.to_csv(output_file, index=False)
    print(f'\nSaved to: {output_file}')

    # Print statistics
    print('\n' + '='*70)
    print('Sample Statistics')
    print('='*70)

    n_units = 105
    n_samples = len(samples)

    # BMP density
    densities = []
    for _, row in df.iterrows():
        n_bmps = sum(1 for i in range(n_units) if row[f'gene_{i}_type'] > 0)
        densities.append(n_bmps)

    import numpy as np
    densities = np.array(densities)

    print(f'\n[BMP Density]')
    print(f'  Total samples: {n_samples}')
    print(f'  Density range: {densities.min()} - {densities.max()}')
    print(f'  Mean: {densities.mean():.2f}')
    print(f'  Median: {np.median(densities):.0f}')

    # Type distribution
    print(f'\n[BMP Type Distribution]')
    type_counts = {1: 0, 2: 0, 3: 0, 4: 0}
    for _, row in df.iterrows():
        for i in range(n_units):
            bmp_type = row[f'gene_{i}_type']
            if bmp_type > 0:
                type_counts[bmp_type] += 1

    total_bmps = sum(type_counts.values())
    for bmp_type in [1, 2, 3, 4]:
        count = type_counts[bmp_type]
        pct = count / total_bmps * 100 if total_bmps > 0 else 0
        print(f'  Type {bmp_type}: {count:6d} ({pct:5.1f}%)')

    # Time distribution
    print(f'\n[Time Period Distribution]')
    time_counts = {1: 0, 2: 0, 3: 0, 4: 0, 5: 0}
    for _, row in df.iterrows():
        for i in range(n_units):
            time_period = row[f'gene_{i}_time']
            if time_period > 0:
                time_counts[time_period] += 1

    for period in [1, 2, 3, 4, 5]:
        count = time_counts[period]
        pct = count / total_bmps * 100 if total_bmps > 0 else 0
        print(f'  Period {period}: {count:6d} ({pct:5.1f}%)')

    print('\n' + '='*70)


if __name__ == '__main__':
    # Configuration file for youwuzhen10m
    config_file = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/workspace/scenario_analysis_s_t_constrained.ini'

    # Generate samples
    samples = generate_samples_with_hillslp(config_file, n_samples=5000)

    # Save to CSV
    output_file = 'D:/EGC/SEIMS-dev/data/youwuzhen10m/samples_spatiotemporal_hillslp_5000.csv'
    save_samples(samples, output_file)

    print(f'\nEnd time: {datetime.now().strftime("%Y-%m-%d %H:%M:%S")}')
    print('='*70)
