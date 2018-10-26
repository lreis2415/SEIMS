#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Configuration of Postprocess for SEIMS.
    @author   : Liangjun Zhu, Huiran Gao
    @changelog: 17-08-17  lj - reorganize as basic class
                18-02-09  lj - compatible with Python3.
                18-10-23  lj - Use `ParseSEIMSConfig` class.
"""
from __future__ import absolute_import

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

try:
    from ConfigParser import ConfigParser  # py2
except ImportError:
    from configparser import ConfigParser  # py3

from pygeoc.utils import StringClass, get_config_file
from run_seims import ParseSEIMSConfig

class PostConfig(object):
    """Parse postprocess configuration of SEIMS project."""

    def __init__(self, cf):
        """Initialization."""
        # 1. SEIMS model related
        self.model_cfg = ParseSEIMSConfig(cf)
        # 2. Parameters
        self.plt_subbsnid = -1
        self.plt_vars = list()
        if 'PARAMETERS' in cf.sections():
            self.plt_subbsnid = cf.getint('PARAMETERS', 'plot_subbasinid')
            plt_vars_str = cf.get('PARAMETERS', 'plot_variables')
        else:
            raise ValueError("[PARAMETERS] section MUST be existed in *.ini file.")
        if self.plt_subbsnid < 0:
            raise ValueError("PLOT_SUBBASINID must be greater or equal than 0.")
        if plt_vars_str != '':
            self.plt_vars = StringClass.split_string(plt_vars_str)
        else:
            raise ValueError("PLOT_VARIABLES illegal defined in [PARAMETERS]!")

        # 3. Optional_Parameters
        if 'OPTIONAL_PARAMETERS' in cf.sections():
            tstart = cf.get('OPTIONAL_PARAMETERS', 'cali_time_start')
            tend = cf.get('OPTIONAL_PARAMETERS', 'cali_time_end')
        else:
            raise ValueError("[OPTIONAL_PARAMETERS] section MUST be existed in *.ini file.")
        try:
            # UTCTIME
            self.cali_stime = StringClass.get_datetime(tstart)
            self.cali_etime = StringClass.get_datetime(tend)
            if cf.has_option('OPTIONAL_PARAMETERS', 'vali_time_start') and \
                    cf.has_option('OPTIONAL_PARAMETERS', 'vali_time_end'):
                tstart = cf.get('OPTIONAL_PARAMETERS', 'vali_time_start')
                tend = cf.get('OPTIONAL_PARAMETERS', 'vali_time_end')
                self.vali_stime = StringClass.get_datetime(tstart)
                self.vali_etime = StringClass.get_datetime(tend)
            else:
                self.vali_stime = None
                self.vali_etime = None
        except ValueError:
            raise ValueError('The time format MUST be "YYYY-MM-DD" or "YYYY-MM-DD HH:MM:SS".')
        if self.cali_stime >= self.cali_etime:
            raise ValueError("Wrong time settings of calibration in [OPTIONAL_PARAMETERS]!")
        if self.vali_stime and self.vali_etime and self.vali_stime >= self.vali_etime:
            raise ValueError("Wrong time settings of validation in [OPTIONAL_PARAMETERS]!")
        # 4. Switches
        self.lang_cn = False
        if 'SWITCH' in cf.sections():
            self.lang_cn = cf.getboolean('SWITCH', 'lang_cn')


def parse_ini_configuration():
    """Load model configuration from *.ini file"""
    cf = ConfigParser()
    ini_file = get_config_file()
    cf.read(ini_file)
    return PostConfig(cf)


if __name__ == '__main__':
    cfg = parse_ini_configuration()
