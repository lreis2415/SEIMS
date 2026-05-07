"""Batch SEIMS simulation runner."""
import sys
import os
import time
from typing import List, Dict
import pandas as pd
from multiprocessing import Pool, cpu_count
from configparser import ConfigParser

# Add SEIMS root to path
seims_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
if seims_root not in sys.path:
    sys.path.insert(0, seims_root)

from scenario_analysis.spatialunits.config import SASlpPosConfig, SAConnFieldConfig
from scenario_analysis.spatialunits.scenario import SUScenario
import json


def run_single_scenario(args):
    """Run single scenario simulation (for multiprocessing)."""
    sample_id, gene_values, config_ini, output_vars = args

    try:
        # Reinitialize MongoDB connection for this subprocess
        import global_mongoclient as MongoDBObj
        from preprocess.db_mongodb import ConnectMongoDB
        MongoDBObj.client = ConnectMongoDB(ip=MongoDBObj.host, port=MongoDBObj.port).get_conn()

        # Create scenario
        cf = ConfigParser()
        cf.read(config_ini, encoding='utf-8')

        # Auto-detect spatial unit type
        bmps_cfg_units_str = cf.get('BMPs', 'bmps_cfg_units')
        bmps_cfg_units = json.loads(bmps_cfg_units_str)
        spatial_unit = list(bmps_cfg_units.keys())[0]

        if spatial_unit == 'SLPPOS':
            sa_config = SASlpPosConfig(cf)
        elif spatial_unit == 'CONNFIELD':
            sa_config = SAConnFieldConfig(cf)
        else:
            raise ValueError(f"Unsupported spatial unit: {spatial_unit}")

        sa_config.construct_indexes_units_gene()

        scenario = SUScenario(sa_config)
        scenario.initialize(input_genes=gene_values)

        # Set scenario ID
        scenario.ID = sample_id
        scenario.model.scenario_id = sample_id

        # Update output directory with correct scenario_id
        scenario.model.UpdateScenarioID()

        # Decode gene values to BMP items and export to MongoDB
        scenario.boundary_adjustment()
        scenario.decoding()

        # Debug: check bmp_items before export
        print(f"  [DEBUG] Sample {sample_id}: {len(scenario.bmp_items)} bmp_items to export")

        scenario.export_to_mongodb()

        # Debug: verify export
        from preprocess.db_mongodb import DBTableNames
        conn = MongoDBObj.client
        scenariodb = conn[scenario.scenario_db]
        count = scenariodb[DBTableNames.scenarios].count_documents({'ID': sample_id})
        print(f"  [DEBUG] Sample {sample_id}: {count} documents in MongoDB after export")

        # Run simulation using scenario.execute_seims_model()
        run_success = scenario.execute_seims_model()

        # Check if simulation was successful
        if not run_success:
            print(f"  [ERROR] Sample {sample_id}: SEIMS simulation failed")
            return None

        # Calculate environment effectiveness (SED values)
        scenario.calculate_environment()

        # Collect results - use scenario attributes directly
        results = {'sample_id': sample_id}

        # SED: 泥沙绝对值（吨/年）
        if 'SED' in output_vars:
            results['SED'] = scenario.sed_sum

        # For Q and TN, we need to read from output files or use available attributes
        # Currently only sed_sum is reliably available
        # TODO: Add methods to read Q and TN from output files if needed

        return results
    except Exception as e:
        print(f"  [ERROR] Sample {sample_id} failed: {str(e)}")
        import traceback
        traceback.print_exc()
        return None


class BatchRunner:
    """Batch SEIMS simulation runner."""

    def __init__(self, config_ini: str, output_vars: List[str]):
        """
        Initialize batch runner.

        Args:
            config_ini: Path to SEIMS scenario_analysis.ini file
            output_vars: List of output variables to collect
        """
        self.config_ini = config_ini
        self.output_vars = output_vars

    def run(self, samples: List[List[int]], n_jobs: int = 4) -> pd.DataFrame:
        """
        Run SEIMS simulation for all samples.

        Args:
            samples: List of gene value lists
            n_jobs: Number of parallel jobs

        Returns:
            DataFrame with simulation results
        """
        print(f"\n[2/6] Running SEIMS simulations ({len(samples)} samples, {n_jobs} parallel jobs)...")

        # Prepare arguments
        args_list = [(i, sample, self.config_ini, self.output_vars)
                     for i, sample in enumerate(samples)]

        # Run in parallel
        start_time = time.time()
        if n_jobs > 1:
            with Pool(processes=n_jobs) as pool:
                results = pool.map(run_single_scenario, args_list)
        else:
            results = [run_single_scenario(args) for args in args_list]

        elapsed = time.time() - start_time

        # Filter out failed simulations
        results = [r for r in results if r is not None]
        success_rate = len(results) / len(samples) * 100

        print(f"  Completed: {len(results)}/{len(samples)} ({success_rate:.1f}% success)")
        print(f"  Time: {elapsed/60:.1f} minutes ({elapsed/len(results):.1f}s per sample)")

        # Convert to DataFrame
        df = pd.DataFrame(results)

        # Add gene values
        for i, sample in enumerate(samples):
            if i < len(df):
                for j, val in enumerate(sample):
                    df.loc[i, f'gene_{j}'] = val

        return df

    def save_results(self, df: pd.DataFrame, output_file: str):
        """Save simulation results to CSV."""
        df.to_csv(output_file, index=False)
        print(f"  Saved results to {output_file}")
