"""Base configuration of Parameter Calibration.

    @author   : Liangjun Zhu

    @changelog:
    - 18-01-20  - lj - initial implementation.
    - 18-02-09  - lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

from configparser import ConfigParser
import os
import sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import FileClass
from run_seims import ParseSEIMSConfig
from utility import get_optimization_config, parse_datetime_from_ini
from utility import ParseNSGA2Config, PlotConfig


class CaliConfig(object):
    """Parse parameters calibration configuration of SEIMS project."""

    def __init__(self, cf, method='nsga2'):
        # type: (ConfigParser, str) -> None
        """Initialization."""
        # 1. SEIMS model related
        self.model = ParseSEIMSConfig(cf)

        # 2. Common settings of auto-calibration
        if 'CALI_Settings' not in cf.sections():
            raise ValueError("[CALI_Settings] section MUST be existed in *.ini file.")
        self.param_range_def = 'cali_param_rng.def'
        if cf.has_option('CALI_Settings', 'paramrngdef'):
            self.param_range_def = cf.get('CALI_Settings', 'paramrngdef')
        self.param_range_def = self.model.model_dir + os.path.sep + self.param_range_def
        if not FileClass.is_file_exists(self.param_range_def):
            raise IOError('Ranges of parameters MUST be provided!')

        # UTCTIME of calibration and validation (optional) periods
        if not (cf.has_option('CALI_Settings', 'cali_time_start') and
                cf.has_option('CALI_Settings', 'cali_time_end')):
            raise ValueError("Start and end time of Calibration "
                             "MUST be specified in [CALI_Settings].")
        self.cali_stime = parse_datetime_from_ini(cf, 'CALI_Settings', 'cali_time_start')
        self.cali_etime = parse_datetime_from_ini(cf, 'CALI_Settings', 'cali_time_end')
        self.vali_stime = parse_datetime_from_ini(cf, 'CALI_Settings', 'vali_time_start')
        self.vali_etime = parse_datetime_from_ini(cf, 'CALI_Settings', 'vali_time_end')
        self.calc_validation = True if self.vali_stime and self.vali_etime else False
        if self.cali_stime >= self.cali_etime or (self.calc_validation and
                                                  self.vali_stime >= self.vali_etime):
            raise ValueError("Wrong time settings in [CALI_Settings]!")

        # 3. Parameters settings for specific optimization algorithm
        self.opt_mtd = method
        self.opt = None
        if self.opt_mtd == 'nsga2':
            self.opt = ParseNSGA2Config(cf, self.model.model_dir, 'CALI_NSGA2_Gen_%d_Pop_%d')

        # 4. (Optional) Plot settings for matplotlib
        self.plot_cfg = PlotConfig(cf)


if __name__ == '__main__':
    cf, method = get_optimization_config("Execute parameters calibration.")
    cfg = CaliConfig(cf, method=method)

    print(cfg)

    # test the picklable of SAConfig class.
    import pickle

    s = pickle.dumps(cfg)
    # print(s)
    new_cfg = pickle.loads(s)
    print(new_cfg.model.model_dir)
