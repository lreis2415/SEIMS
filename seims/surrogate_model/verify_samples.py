"""Verify sample data by re-running SEIMS on random samples."""
import os
import sys
import json
import random
import pandas as pd

seims_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
if seims_root not in sys.path:
    sys.path.insert(0, seims_root)

from scenario_analysis.unified_config import UnifiedConfig


def verify_samples(csv_file, model_dir, n_samples=5):
    """Verify random samples by re-running SEIMS."""

    print("=" * 80)
    print("Sample Data Verification")
    print("=" * 80)

    # Load CSV
    df = pd.read_csv(csv_file)
    print(f"\nTotal samples in CSV: {len(df)}")

    # Random sample
    sample_indices = random.sample(range(len(df)), min(n_samples, len(df)))
    print(f"Randomly selected {len(sample_indices)} samples for verification\n")

    # Create test config
    test_config = {
        "mode": "spatial",
        "model": {
            "model_dir": model_dir,
            "bin_dir": "D:/EGC/SEIMS-dev/build/bin",
            "model_name": os.path.basename(model_dir),
            "host": "127.0.0.1",
            "port": 27017,
            "scenario_db": "demo_youwuzhen30m_Scenario"
        },
        "spatial": {
            "unit": "SLPPOS",
            "config_method": "HILLSLP"
        },
        "algorithm": {
            "type": "NSGA2",
            "GenerationsNum": 1,
            "PopulationSize": 4
        },
        "budget": {
            "enable_investment_quota": False
        },
        "surrogate": {
            "use_surrogate": False
        },
        "output": {
            "output_dir": "D:/EGC/SEIMS-dev/test_results/sample_verification",
            "export_scenario_txt": True,
            "export_scenario_tif": False
        },
        "evaluation": {
            "eval_stime": "2013-01-01 00:00:00",
            "eval_etime": "2017-12-31 23:59:59",
            "worst_economy": 300.0,
            "worst_environment": 0.0
        }
    }

    cfg = UnifiedConfig(test_config)

    # Convert to legacy config and wrap in ConfigParser
    from configparser import ConfigParser
    from scenario_analysis.spatialunits.config import SASlpPosConfig

    legacy_config = cfg.to_legacy_config()
    cf = ConfigParser()
    for section, values in legacy_config.items():
        cf.add_section(section)
        for key, value in values.items():
            cf.set(section, key, str(value))

    sa_cfg = SASlpPosConfig(cf)
    sa_cfg.construct_indexes_units_gene()

    # Import scenario module and DEAP
    from scenario_analysis.spatialunits.scenario import scenario_effectiveness
    from deap import base, creator

    # Create DEAP types if not already created
    if not hasattr(creator, "FitnessMin"):
        creator.create("FitnessMin", base.Fitness, weights=(-1.0, -1.0))
    if not hasattr(creator, "Individual"):
        creator.create("Individual", list, fitness=creator.FitnessMin)

    results = []

    for idx in sample_indices:
        row = df.iloc[idx]
        sample_id = row['sample_id']
        sed_csv = row['SED']

        # Extract gene values
        gene_cols = [f'gene_{i}' for i in range(105)]
        genes = [int(row[col]) for col in gene_cols]

        print(f"\n{'='*60}")
        print(f"Sample {sample_id}")
        print(f"{'='*60}")
        print(f"CSV SED: {sed_csv:,.2f}")
        print(f"Genes: {genes[:10]}... (showing first 10)")

        # Create DEAP individual
        ind = creator.Individual(genes)

        # Run SEIMS using scenario_effectiveness
        scenario_effectiveness(sa_cfg, ind)

        # Extract results from individual
        sed_seims = ind.sed_sum

        diff = sed_seims - sed_csv
        pct_diff = (diff / sed_csv) * 100

        print(f"SEIMS SED: {sed_seims:,.2f}")
        print(f"Difference: {diff:,.2f} ({pct_diff:+.2f}%)")

        results.append({
            'sample_id': sample_id,
            'sed_csv': sed_csv,
            'sed_seims': sed_seims,
            'diff': diff,
            'pct_diff': pct_diff
        })

    # Summary
    print(f"\n{'='*80}")
    print("Verification Summary")
    print(f"{'='*80}")

    df_results = pd.DataFrame(results)
    print(f"\nMean difference: {df_results['diff'].mean():,.2f}")
    print(f"Mean % difference: {df_results['pct_diff'].mean():+.2f}%")
    print(f"Std % difference: {df_results['pct_diff'].std():.2f}%")
    print(f"\nMin % diff: {df_results['pct_diff'].min():+.2f}%")
    print(f"Max % diff: {df_results['pct_diff'].max():+.2f}%")

    # Save results
    output_file = "sample_verification_results.csv"
    df_results.to_csv(output_file, index=False)
    print(f"\nResults saved to {output_file}")

    return df_results


if __name__ == '__main__':
    csv_file = "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training_2000_optimized/simulation_results/collected_data.csv"
    model_dir = "D:/EGC/SEIMS-dev/data/youwuzhen/demo_youwuzhen30m_longterm_model"

    verify_samples(csv_file, model_dir, n_samples=5)
