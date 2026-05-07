"""Analyze sample representativeness and design sampling strategy."""
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from collections import Counter
import os

def analyze_sample_distribution(csv_file):
    """Analyze the distribution of existing samples."""

    print("=" * 80)
    print("Sample Distribution Analysis")
    print("=" * 80)

    df = pd.read_csv(csv_file)
    print(f"\nTotal samples: {len(df)}")

    # Extract gene columns
    gene_cols = [col for col in df.columns if col.startswith('gene_')]
    X = df[gene_cols].values
    y = df['SED'].values

    print(f"\n1. SED Distribution")
    print(f"   Min:    {y.min():.2f}")
    print(f"   Max:    {y.max():.2f}")
    print(f"   Mean:   {y.mean():.2f}")
    print(f"   Median: {np.median(y):.2f}")
    print(f"   Std:    {y.std():.2f}")

    # Analyze BMP density (number of non-zero genes)
    print(f"\n2. BMP Density Distribution")
    bmp_counts = (X > 0).sum(axis=1)
    print(f"   Min BMPs:    {bmp_counts.min()}")
    print(f"   Max BMPs:    {bmp_counts.max()}")
    print(f"   Mean BMPs:   {bmp_counts.mean():.1f}")
    print(f"   Median BMPs: {np.median(bmp_counts):.1f}")

    # Distribution by density ranges
    density_ranges = [
        (0, 0, "Zero (baseline)"),
        (1, 20, "Sparse (1-20)"),
        (21, 50, "Low (21-50)"),
        (51, 80, "Medium (51-80)"),
        (81, 105, "Dense (81-105)")
    ]

    print(f"\n   Density breakdown:")
    for low, high, label in density_ranges:
        count = ((bmp_counts >= low) & (bmp_counts <= high)).sum()
        pct = count / len(bmp_counts) * 100
        print(f"   {label:20s}: {count:4d} ({pct:5.1f}%)")

    # Analyze BMP type distribution
    print(f"\n3. BMP Type Distribution (excluding 0)")
    all_bmps = X[X > 0]
    bmp_type_counts = Counter(all_bmps.astype(int))
    print(f"   Type 1: {bmp_type_counts.get(1, 0):5d} ({bmp_type_counts.get(1, 0)/len(all_bmps)*100:5.1f}%)")
    print(f"   Type 2: {bmp_type_counts.get(2, 0):5d} ({bmp_type_counts.get(2, 0)/len(all_bmps)*100:5.1f}%)")
    print(f"   Type 3: {bmp_type_counts.get(3, 0):5d} ({bmp_type_counts.get(3, 0)/len(all_bmps)*100:5.1f}%)")
    print(f"   Type 4: {bmp_type_counts.get(4, 0):5d} ({bmp_type_counts.get(4, 0)/len(all_bmps)*100:5.1f}%)")

    # Check for baseline scenario (all zeros)
    all_zero = (X == 0).all(axis=1).sum()
    print(f"\n4. Baseline Scenarios (all zeros): {all_zero}")

    # Check for maximum BMP scenario (all 4s or all non-zero)
    all_nonzero = (X > 0).all(axis=1).sum()
    print(f"   Full coverage (all non-zero): {all_nonzero}")

    # Analyze SED by density
    print(f"\n5. SED by BMP Density")
    for low, high, label in density_ranges:
        mask = (bmp_counts >= low) & (bmp_counts <= high)
        if mask.sum() > 0:
            sed_range = y[mask]
            print(f"   {label:20s}: {sed_range.mean():.0f} ± {sed_range.std():.0f}")

    return {
        'n_samples': len(df),
        'bmp_counts': bmp_counts,
        'sed_values': y,
        'n_baseline': all_zero,
        'n_full': all_nonzero,
        'density_ranges': density_ranges
    }


def recommend_sampling_strategy(analysis_result):
    """Recommend sampling strategy based on analysis."""

    print("\n" + "=" * 80)
    print("Sampling Strategy Recommendation")
    print("=" * 80)

    bmp_counts = analysis_result['bmp_counts']
    n_baseline = analysis_result['n_baseline']
    n_full = analysis_result['n_full']

    recommendations = []

    # Check baseline coverage
    if n_baseline < 10:
        n_needed = 50 - n_baseline
        recommendations.append({
            'type': 'baseline',
            'reason': f'Only {n_baseline} baseline scenarios (all zeros)',
            'n_samples': n_needed,
            'priority': 'HIGH'
        })

    # Check sparse BMP coverage
    n_sparse = ((bmp_counts >= 1) & (bmp_counts <= 20)).sum()
    if n_sparse < 100:
        n_needed = 150 - n_sparse
        recommendations.append({
            'type': 'sparse',
            'reason': f'Only {n_sparse} sparse BMP scenarios (1-20 BMPs)',
            'n_samples': n_needed,
            'priority': 'MEDIUM'
        })

    # Check dense BMP coverage
    n_dense = (bmp_counts >= 81).sum()
    if n_dense < 100:
        n_needed = 150 - n_dense
        recommendations.append({
            'type': 'dense',
            'reason': f'Only {n_dense} dense BMP scenarios (81-105 BMPs)',
            'n_samples': n_needed,
            'priority': 'MEDIUM'
        })

    # Check full coverage
    if n_full < 10:
        n_needed = 50 - n_full
        recommendations.append({
            'type': 'full',
            'reason': f'Only {n_full} full coverage scenarios',
            'n_samples': n_needed,
            'priority': 'LOW'
        })

    if not recommendations:
        print("\n[OK] Sample distribution is adequate. No additional sampling needed.")
        print("\nCurrent coverage:")
        print(f"  - Baseline: {n_baseline} samples")
        print(f"  - Sparse:   {((bmp_counts >= 1) & (bmp_counts <= 20)).sum()} samples")
        print(f"  - Dense:    {(bmp_counts >= 81).sum()} samples")
        return None

    print("\n[WARNING] Sample distribution has gaps. Recommended additional sampling:\n")

    total_needed = 0
    for i, rec in enumerate(recommendations, 1):
        print(f"{i}. {rec['type'].upper()} scenarios [{rec['priority']} priority]")
        print(f"   Reason: {rec['reason']}")
        print(f"   Recommended: {rec['n_samples']} additional samples\n")
        total_needed += rec['n_samples']

    print(f"Total additional samples needed: {total_needed}")
    print(f"New total: {analysis_result['n_samples'] + total_needed}")

    return recommendations


if __name__ == '__main__':
    csv_file = "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training_2000_optimized/simulation_results/collected_data.csv"

    if not os.path.exists(csv_file):
        print(f"Error: File not found: {csv_file}")
        exit(1)

    # Analyze
    result = analyze_sample_distribution(csv_file)

    # Recommend
    recommendations = recommend_sampling_strategy(result)

    print("\n" + "=" * 80)
