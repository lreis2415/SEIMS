"""Configuration of Postprocess for SEIMS.

    @author   : Liangjun Zhu, Huiran Gao

    @changelog:
    - 17-08-17  - lj - reorganize as basic class
    - 18-02-09  - lj - compatible with Python3.
    - 18-10-23  - lj - Use `ParseSEIMSConfig` class.
"""
from __future__ import absolute_import, unicode_literals

from configparser import ConfigParser
import os
import sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import StringClass, get_config_file

from utility import parse_datetime_from_ini, PlotConfig
from run_seims import ParseSEIMSConfig


class PostConfig(object):
    """Parse postprocess configuration of SEIMS project."""

    def __init__(self, cf):
        """Initialization."""
        # 1. SEIMS model related
        self.model_cfg = ParseSEIMSConfig(cf)
        # 2. Parameters
        self.plt_subbsnid = -1
        self.plot_vars = list()
        if 'PARAMETERS' in cf.sections():
            self.plt_subbsnid = cf.getint('PARAMETERS', 'plot_subbasinid')
            plt_vars_str = cf.get('PARAMETERS', 'plot_variables')
        else:
            raise ValueError("[PARAMETERS] section MUST be existed in *.ini file.")
        if self.plt_subbsnid < 0:
            raise ValueError("PLOT_SUBBASINID must be greater or equal than 0.")
        if plt_vars_str != '':
            self.plot_vars = StringClass.split_string(plt_vars_str, [',', ' '])
        else:
            raise ValueError("PLOT_VARIABLES illegal defined in [PARAMETERS]!")

        # 3. Optional_Parameters
        if 'OPTIONAL_PARAMETERS' not in cf.sections():
            raise ValueError("[OPTIONAL_PARAMETERS] section MUST be existed in *.ini file.")
        # UTCTIME
        self.cali_stime = parse_datetime_from_ini(cf, 'OPTIONAL_PARAMETERS', 'cali_time_start')
        self.cali_etime = parse_datetime_from_ini(cf, 'OPTIONAL_PARAMETERS', 'cali_time_end')
        self.vali_stime = parse_datetime_from_ini(cf, 'OPTIONAL_PARAMETERS', 'vali_time_start')
        self.vali_etime = parse_datetime_from_ini(cf, 'OPTIONAL_PARAMETERS', 'vali_time_end')

        if not self.cali_stime or not self.cali_etime or self.cali_stime >= self.cali_etime:
            raise ValueError("Wrong time settings of calibration in [OPTIONAL_PARAMETERS]!")
        if self.vali_stime and self.vali_etime and self.vali_stime >= self.vali_etime:
            raise ValueError("Wrong time settings of validation in [OPTIONAL_PARAMETERS]!")
        # 4. Plot settings based on matplotlib
        self.plot_cfg = PlotConfig(cf)


def parse_ini_configuration():
    """Load model configuration from *.ini file"""
    cf = ConfigParser()
    ini_file = get_config_file()
    cf.read(ini_file)
    return PostConfig(cf)


if __name__ == '__main__':
    cfg = parse_ini_configuration()
