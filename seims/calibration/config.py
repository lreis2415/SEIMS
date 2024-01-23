# coding:utf-8
"""Base configuration of Parameter Calibration.

    @author   : Liangjun Zhu

    @changelog:
    - 18-01-20  - lj - initial implementation.
    - 18-02-09  - lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import glob
import os
import shutil
import sys
from configparser import ConfigParser
from pathlib import Path

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

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

        self.param_range_defs = 'cali_param_rng.def'
        if cf.has_option('CALI_Settings', 'paramrngdef'):
            self.param_range_defs = cf.get('CALI_Settings', 'paramrngdef')
        suffix, postfix = self.param_range_defs.split('.')
        self.param_range_defs = glob.glob(Path(self.model.model_dir, suffix + '*.' + postfix).as_posix())
        if not self.param_range_defs:
            raise IOError('Files with pattern %s not found! Ranges of parameters MUST be provided!'
                          % (suffix + '*.' + postfix))

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

        for f in self.param_range_defs:
            shutil.copy(f, self.opt.out_dir)
        structure_files = glob.glob(os.path.join(self.model.model_dir, 'structure*.config'))
        for sf in structure_files:
            shutil.copy(sf, self.opt.out_dir)

        # logger.configure_logging(log_file_prefix='calibration')
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
