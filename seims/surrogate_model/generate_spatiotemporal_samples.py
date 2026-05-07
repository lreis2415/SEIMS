"""Generate spatiotemporal samples for surrogate model training.

Encoding scheme:
- Each spatial unit has 2 dimensions: BMP type (1-4) and implementation time (1-5)
- Type: One-Hot encoding (4 dimensions)
- Time: Ordinal encoding (1 dimension, 0=no BMP, 1-5=periods)
- Total features: 105 units × 5 dimensions = 525 features
"""
import os
import sys
import random
import numpy as np
import pandas as pd
from datetime import datetime

# Set random seed for reproducibility
random.seed(42)
np.random.seed(42)


def stratified_sampling(n_samples=5000, n_units=105):
    """Generate stratified samples based on BMP density.

    Args:
        n_samples: Total number of samples
        n_units: Number of spatial units (default 105 for SLPPOS)

    Returns:
        List of sample dictionaries
    """
    samples = []

    # Stratification: (min_bmps, max_bmps, n_samples)
    strata = [
        (0, 0, 50),        # Baseline scenarios
        (1, 20, 1000),     # Low density
        (21, 50, 2000),    # Medium density
        (51, 80, 1500),    # High density
        (81, 105, 450)     # Full coverage
    ]

    # Time period weights (early, mid, late implementation)
    time_weights = [0.15, 0.15, 0.20, 0.20, 0.30]  # periods 1-5

    sample_id = 0

    for min_bmps, max_bmps, n in strata:
        print(f"Generating {n} samples with {min_bmps}-{max_bmps} BMPs...")

        for _ in range(n):
            # Random number of BMPs in this stratum
            if min_bmps == max_bmps:
                n_bmps = min_bmps
            else:
                n_bmps = random.randint(min_bmps, max_bmps)

            # Initialize genes (all zeros = no BMP)
            genes_type = np.zeros(n_units, dtype=int)
            genes_time = np.zeros(n_units, dtype=int)

            if n_bmps > 0:
                # Randomly select spatial units for BMP implementation
                units = random.sample(range(n_units), n_bmps)

                for unit in units:
                    # Random BMP type (1-4)
                    genes_type[unit] = random.randint(1, 4)

                    # Random implementation time (1-5) with weighted distribution
                    genes_time[unit] = random.choices(
                        [1, 2, 3, 4, 5],
                        weights=time_weights
                    )[0]

            # Create sample record
            sample = {'sample_id': sample_id}

            # Add gene values
            for i in range(n_units):
                sample[f'gene_{i}_type'] = genes_type[i]
                sample[f'gene_{i}_time'] = genes_time[i]

            samples.append(sample)
            sample_id += 1

    return samples


def save_samples(samples, output_file):
    """Save samples to CSV file."""
    df = pd.DataFrame(samples)
    df.to_csv(output_file, index=False)
    print(f"\nSaved {len(samples)} samples to {output_file}")

    # Print statistics
    print("\n" + "="*60)
    print("Sample Statistics")
    print("="*60)

    # Calculate BMP density for each sample
    n_units = 105
    densities = []

    for _, row in df.iterrows():
        n_bmps = sum(1 for i in range(n_units) if row[f'gene_{i}_type'] > 0)
        densities.append(n_bmps)

    densities = np.array(densities)

    print(f"Total samples: {len(samples)}")
    print(f"BMP density range: {densities.min()} - {densities.max()}")
    print(f"Mean BMP density: {densities.mean():.2f}")
    print(f"Median BMP density: {np.median(densities):.0f}")

    # Distribution by density
    print("\nDensity distribution:")
    print(f"  0 BMPs: {np.sum(densities == 0)}")
    print(f"  1-20 BMPs: {np.sum((densities >= 1) & (densities <= 20))}")
    print(f"  21-50 BMPs: {np.sum((densities >= 21) & (densities <= 50))}")
    print(f"  51-80 BMPs: {np.sum((densities >= 51) & (densities <= 80))}")
    print(f"  81-105 BMPs: {np.sum(densities >= 81)}")

    # Time period distribution
    print("\nTime period distribution:")
    for period in range(1, 6):
        count = 0
        for _, row in df.iterrows():
            for i in range(n_units):
                if row[f'gene_{i}_time'] == period:
                    count += 1
        print(f"  Period {period}: {count} BMPs")


if __name__ == '__main__':
    print("="*60)
    print("Spatiotemporal Sample Generation")
    print("="*60)
    print(f"Start time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

    # Generate samples
    samples = stratified_sampling(n_samples=5000, n_units=105)

    # Save to CSV
    output_dir = "D:/EGC/SEIMS-dev/data/youwuzhen10m"
    output_file = os.path.join(output_dir, "samples_spatiotemporal_5000.csv")
    save_samples(samples, output_file)

    print(f"\nEnd time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print("="*60)
