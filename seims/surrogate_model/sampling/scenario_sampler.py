"""BMP scenario sampler using existing SEIMS optimization code."""
import sys
import os

# Add SEIMS root to path
seims_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
if seims_root not in sys.path:
    sys.path.insert(0, seims_root)

from scenario_analysis.spatialunits.config import SAConnFieldConfig, SASlpPosConfig
from scenario_analysis.spatialunits.scenario import SUScenario
from configparser import ConfigParser
from typing import List
import pandas as pd
import json


class ScenarioSampler:
    """Generate BMP scenario samples by reusing existing optimization code."""

    def __init__(self, config_ini: str):
        """
        Initialize sampler with SEIMS configuration.

        Args:
            config_ini: Path to SEIMS scenario_analysis.ini file
        """
        cf = ConfigParser()
        cf.read(config_ini, encoding='utf-8')

        # Auto-detect spatial unit type
        bmps_cfg_units_str = cf.get('BMPs', 'bmps_cfg_units')
        bmps_cfg_units = json.loads(bmps_cfg_units_str)
        spatial_unit = list(bmps_cfg_units.keys())[0]

        if spatial_unit == 'SLPPOS':
            self.sa_config = SASlpPosConfig(cf)
        elif spatial_unit == 'CONNFIELD':
            self.sa_config = SAConnFieldConfig(cf)
        else:
            raise ValueError(f"Unsupported spatial unit: {spatial_unit}")

        # Must call this to build unit_to_gene mapping
        self.sa_config.construct_indexes_units_gene()
        self.gene_num = self.sa_config.genes_num

    def sample(self, n_samples: int, seed: int = None) -> List[List[int]]:
        """
        Generate n random BMP scenario samples.

        Args:
            n_samples: Number of samples to generate
            seed: Random seed for reproducibility

        Returns:
            List of gene value lists, shape (n_samples, gene_num)
        """
        if seed is not None:
            import random
            random.seed(seed)

        samples = []
        for i in range(n_samples):
            scenario = SUScenario(self.sa_config)
            gene_values = scenario.initialize()
            samples.append(gene_values[:])

            if (i + 1) % 50 == 0:
                print(f"  Generated {i + 1}/{n_samples} scenarios")

        return samples

    def save_samples(self, samples: List[List[int]], output_file: str):
        """Save samples to CSV file."""
        df = pd.DataFrame(samples, columns=[f'gene_{i}' for i in range(self.gene_num)])
        df.insert(0, 'sample_id', range(len(samples)))
        df.to_csv(output_file, index=False)
        print(f"  Saved {len(samples)} samples to {output_file}")
