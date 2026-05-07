"""Step 1: Generate 2000 samples - Simplified version without SEIMS config."""
import sys
import os
import json
import random
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Set seed
random.seed(42)

# Configuration
output_dir = "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training_2000_optimized"
os.makedirs(output_dir, exist_ok=True)
os.makedirs(os.path.join(output_dir, 'samples'), exist_ok=True)

print("=" * 80)
print("Step 1: Generate 2000 Samples with Optimized Strategy")
print("=" * 80)

# Generate samples directly
n_samples = 2000
n_genes = 105  # SLPPOS units
bmp_types = [0, 1, 2, 3, 4]  # 0=no BMP, 1-4=BMP types

samples = []
for i in range(n_samples):
    # Uniform coverage rate [0, 1]
    cr = random.random()

    # Generate gene values
    gene_values = []
    for j in range(n_genes):
        if random.random() < cr:
            # Select BMP type (1-4)
            gene_values.append(random.choice([1, 2, 3, 4]))
        else:
            # No BMP
            gene_values.append(0)

    samples.append(gene_values)

    if (i + 1) % 500 == 0:
        print(f"  Generated {i + 1}/{n_samples} scenarios")

# Save samples
df = pd.DataFrame(samples, columns=[f'gene_{i}' for i in range(n_genes)])
df.insert(0, 'sample_id', range(len(samples)))
samples_file = os.path.join(output_dir, 'samples', 'samples.csv')
df.to_csv(samples_file, index=False)
print(f"  Saved {len(samples)} samples to {samples_file}")

# Analyze distribution
X = np.array(samples)
bmp_counts = (X > 0).sum(axis=1)
bmp_ratios = bmp_counts / n_genes

print("\n" + "=" * 80)
print("Distribution Analysis")
print("=" * 80)
print(f"\nTotal samples: {len(X)}")
print(f"Total genes: {n_genes}")
print(f"\nBMP Coverage Statistics:")
print(f"  Min: {bmp_counts.min()} ({bmp_ratios.min()*100:.1f}%)")
print(f"  Max: {bmp_counts.max()} ({bmp_ratios.max()*100:.1f}%)")
print(f"  Mean: {bmp_counts.mean():.1f} ({bmp_ratios.mean()*100:.1f}%)")
print(f"  Median: {np.median(bmp_counts):.0f} ({np.median(bmp_ratios)*100:.1f}%)")
print(f"  Std: {bmp_counts.std():.1f}")

# Distribution by bins
bins = [0, 21, 42, 63, 84, 105]
labels = ['0-20%', '21-40%', '41-60%', '61-80%', '81-100%']
hist, _ = np.histogram(bmp_counts, bins=bins)

print(f"\nDistribution by coverage:")
for i, label in enumerate(labels):
    print(f"  {label}: {hist[i]} samples ({hist[i]/len(X)*100:.1f}%)")

# Check uniformity
low = (bmp_ratios < 0.2).sum()
mid_low = ((bmp_ratios >= 0.2) & (bmp_ratios < 0.4)).sum()
mid = ((bmp_ratios >= 0.4) & (bmp_ratios < 0.6)).sum()
mid_high = ((bmp_ratios >= 0.6) & (bmp_ratios < 0.8)).sum()
high = (bmp_ratios >= 0.8).sum()

print(f"\nUniformity check:")
print(f"  <20%: {low} ({low/len(X)*100:.1f}%)")
print(f"  20-40%: {mid_low} ({mid_low/len(X)*100:.1f}%)")
print(f"  40-60%: {mid} ({mid/len(X)*100:.1f}%)")
print(f"  60-80%: {mid_high} ({mid_high/len(X)*100:.1f}%)")
print(f"  >=80%: {high} ({high/len(X)*100:.1f}%)")

# Plot histogram
plt.figure(figsize=(10, 6))
plt.hist(bmp_ratios * 100, bins=20, edgecolor='black', alpha=0.7)
plt.xlabel('BMP Coverage (%)')
plt.ylabel('Number of Samples')
plt.title('Distribution of BMP Coverage (2000 Samples)')
plt.grid(True, alpha=0.3)
plt.tight_layout()
plot_file = os.path.join(output_dir, 'samples', 'distribution.png')
plt.savefig(plot_file, dpi=150)
print(f"\n  Saved distribution plot to {plot_file}")

print("\n" + "=" * 80)
print("Conclusion:")
if low > 100 and high > 100 and abs(low - high) < 300:
    print("  [OK] Distribution is UNIFORM - Ready for simulation")
else:
    print("  [WARNING] Distribution may not be uniform - Review results")
print("=" * 80)
