"""Configuration parser for surrogate model training."""
import json
import os
from typing import Dict, Any, List


class SurrogateConfig:
    """Parse and validate surrogate model configuration."""

    def __init__(self, config_file: str):
        """Load configuration from JSON file."""
        if not os.path.exists(config_file):
            raise FileNotFoundError(f"Config file not found: {config_file}")

        with open(config_file, 'r') as f:
            self.config = json.load(f)

        self._validate()

    def _validate(self):
        """Validate required fields."""
        required = ['project', 'seims', 'sampling', 'evaluation', 'training']
        for key in required:
            if key not in self.config:
                raise ValueError(f"Missing required section: {key}")

    @property
    def work_dir(self) -> str:
        return self.config['project']['work_dir']

    @property
    def model_dir(self) -> str:
        return self.config['seims']['model_dir']

    @property
    def bin_dir(self) -> str:
        return self.config['seims']['bin_dir']

    @property
    def n_samples(self) -> int:
        return self.config['sampling']['n_samples']

    @property
    def output_vars(self) -> List[str]:
        return self.config['evaluation']['output_vars']

    @property
    def test_size(self) -> float:
        return self.config['training']['test_size']

    @property
    def epochs(self) -> int:
        return self.config['training']['epochs']

    def get(self, section: str, key: str = None, default: Any = None) -> Any:
        """Get configuration value."""
        if key is None:
            return self.config.get(section, default)
        return self.config.get(section, {}).get(key, default)
