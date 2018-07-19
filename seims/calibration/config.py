#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base configuration of Parameter Calibration.
    @author   : Liangjun Zhu
    @changelog: 18-1-20  lj - initial implementation.\n
                18-02-09  lj - compatible with Python3.\n
"""
from __future__ import absolute_import

import os
import sys
import argparse

try:
    from ConfigParser import ConfigParser  # py2
except ImportError:
    from configparser import ConfigParser  # py3

from pygeoc.utils import FileClass, StringClass, UtilClass

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from run_seims import ParseSEIMSConfig


def get_cali_config():
    """Parse arguments.
    Returns:
        cf: ConfigParse object of *.ini file
        mtd: Calibration method name, currently, 'nsga2' is supported.
    """
    # define input arguments
    parser = argparse.ArgumentParser(description="Execute parameters calibration.")
    parser.add_argument('-ini', type=str, help="Full path of configuration file")
    # add mutually group
    psa_group = parser.add_mutually_exclusive_group()
    psa_group.add_argument('-nsga2', action='store_true', help='Run NSGA-II method')
    # parse arguments
    args = parser.parse_args()
    ini_file = args.ini
    psa_mtd = 'nsga2'  # Default
    if args.nsga2:
        psa_mtd = 'nsga2'
    if not FileClass.is_file_exists(ini_file):
        raise ImportError('Configuration file is not existed: %s' % ini_file)
    cf = ConfigParser()
    cf.read(ini_file)
    return cf, psa_mtd


class ParseNSGA2Config(object):
    """NSGA-II related parameters"""

    def __init__(self, cf, wp):
        self.ngens = 1
        self.npop = 4
        self.rsel = 0.8
        self.rcross = 0.75
        self.rmut = 0.1
        if 'NSGA2' not in cf.sections():
            raise ValueError('[NSGA2] section MUST be existed in *.ini file.')

        self.ngens = cf.getint('NSGA2', 'generationsnum')
        self.npop = cf.getint('NSGA2', 'populationsize')
        self.rsel = cf.getfloat('NSGA2', 'selectrate')
        self.rcross = cf.getfloat('NSGA2', 'crossoverrate')
        self.rmut = cf.getfloat('NSGA2', 'mutaterate')

        if self.npop % 4 != 0:
            raise ValueError('PopulationSize must be a multiple of 4.')
        self.dirname = 'Cali_NSGA2_Gen_%d_Pop_%d' % (self.ngens, self.npop)

        self.out_dir = wp + os.path.sep + self.dirname
        UtilClass.rmmkdir(self.out_dir)
        self.hypervlog = self.out_dir + os.path.sep + 'hypervolume.txt'
        self.logfile = self.out_dir + os.path.sep + 'runtime.log'
        self.logbookfile = self.out_dir + os.path.sep + 'logbook.txt'
        self.simdata_dir = self.out_dir + os.path.sep + 'simulated_data'
        UtilClass.rmmkdir(self.simdata_dir)


class CaliConfig(object):
    """Parse parameters calibration configuration of SEIMS project."""

    def __init__(self, cf, method='nsga2'):
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

        if not (cf.has_option('CALI_Settings', 'cali_time_start') and
                cf.has_option('CALI_Settings', 'cali_time_end')):
            raise ValueError("Start and end time of Calibration "
                             "MUST be specified in [CALI_Settings].")
        try:  # UTCTIME
            tstart = cf.get('CALI_Settings', 'cali_time_start')
            tend = cf.get('CALI_Settings', 'cali_time_end')
            self.cali_stime = StringClass.get_datetime(tstart)
            self.cali_etime = StringClass.get_datetime(tend)
            self.calc_validation = False
            if cf.has_option('CALI_Settings', 'vali_time_start') and \
                cf.has_option('CALI_Settings', 'vali_time_end'):
                tstart = cf.get('CALI_Settings', 'vali_time_start')
                tend = cf.get('CALI_Settings', 'vali_time_end')
                self.vali_stime = StringClass.get_datetime(tstart)
                self.vali_etime = StringClass.get_datetime(tend)
                self.calc_validation = True
        except ValueError:
            raise ValueError('The time format MUST be "YYYY-MM-DD" or "YYYY-MM-DD HH:MM:SS".')
        if self.cali_stime >= self.cali_etime or (self.calc_validation and
                                                  self.vali_stime >= self.vali_etime):
            raise ValueError("Wrong time setted in [CALI_Settings]!")

        # 3. Parameters settings for specific optimization algorithm
        self.opt_mtd = method
        self.opt = None
        if self.opt_mtd == 'nsga2':
            self.opt = ParseNSGA2Config(cf, self.model.model_dir)


if __name__ == '__main__':
    cf, method = get_cali_config()
    cfg = CaliConfig(cf, method=method)

    print(cfg)

    # test the picklable of SAConfig class.
    import pickle

    s = pickle.dumps(cfg)
    # print(s)
    new_cfg = pickle.loads(s)
    print(new_cfg.model.model_dir)
