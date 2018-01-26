#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base configuration of Parameter Calibration.
    @author   : Liangjun Zhu
    @changelog: 18-1-20  lj - initial implementation.\n
"""
import os
import sys
import argparse

try:
    from ConfigParser import ConfigParser  # py2
except ImportError:
    from configparser import ConfigParser  # py3

from pygeoc.utils import FileClass, StringClass, UtilClass

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.append(os.path.abspath(os.path.join(sys.path[0], '..')))


def get_cali_config():
    """Parse arguments.
    Returns:
        cf: ConfigParse object of *.ini file
        mtd: Calibration method name, currently, 'nsgaii' is supported.
    """
    # define input arguments
    parser = argparse.ArgumentParser(description="Execute parameters calibration.")
    parser.add_argument('-ini', type=str, help="Full path of configuration file")
    # add mutually group
    psa_group = parser.add_mutually_exclusive_group()
    psa_group.add_argument('-nsgaii', action='store_true', help='Run NSGA-II method')
    # parse arguments
    args = parser.parse_args()
    ini_file = args.ini
    psa_mtd = 'nsgaii'  # Default
    if args.nsgaii:
        psa_mtd = 'nsgaii'
    if not FileClass.is_file_exists(ini_file):
        raise ImportError('Configuration file is not existed: %s' % ini_file)
    cf = ConfigParser()
    cf.read(ini_file)
    return cf, psa_mtd


class nsgaiiConfig(object):
    """NSGA-II related parameters"""

    def __init__(self, cf, wp):
        self.ngens = 1
        self.npop = 4
        self.rcross = 0.75
        self.rmut = 0.1
        self.rsel = 0.8
        if 'NSGAII' in cf.sections():
            self.ngens = cf.getint('NSGAII', 'generationsnum')
            self.npop = cf.getint('NSGAII', 'populationsize')
            self.rcross = cf.getfloat('NSGAII', 'crossoverrate')
            self.rmut = cf.getfloat('NSGAII', 'mutaterate')
            self.rsel = cf.getfloat('NSGAII', 'selectrate')
        else:
            raise ValueError('[NSGAII] section MUST be existed in *.ini file.')
        if self.npop % 4 != 0:
            raise ValueError('PopulationSize must be a multiple of 4.')
        self.dirname = 'Cali_NSGAII_Gen_%d_Pop_%d' % (self.ngens, self.npop)

        self.out_dir = wp + os.sep + self.dirname
        UtilClass.rmmkdir(self.out_dir)
        self.hypervlog = self.out_dir + os.sep + 'hypervolume.txt'
        self.logfile = self.out_dir + os.sep + 'runtime.log'
        self.logbookfile = self.out_dir + os.sep + 'logbook.txt'
        self.simdata_dir = self.out_dir + os.sep + 'simulated_data'
        UtilClass.rmmkdir(self.simdata_dir)
        self.ppu_dir = self.out_dir + os.sep + '95ppu'
        UtilClass.rmmkdir(self.ppu_dir)


class CaliConfig(object):
    """Parse parameters calibration configuration of SEIMS project."""

    def __init__(self, cf, method='nsgaii'):
        """Initialization."""
        # 1. MongoDB
        self.hostname = '127.0.0.1'  # localhost by default
        self.port = 27017
        self.spatial_db = ''
        if 'MONGODB' in cf.sections():
            self.hostname = cf.get('MONGODB', 'hostname')
            self.port = cf.getint('MONGODB', 'port')
            self.spatial_db = cf.get('MONGODB', 'spatialdbname')
        else:
            raise ValueError('[MONGODB] section MUST be existed in *.ini file.')
        if not StringClass.is_valid_ip_addr(self.hostname):
            raise ValueError('HOSTNAME illegal defined in [MONGODB]!')

        # 2. SEIMS_Model
        self.model_dir = ''
        self.bin_dir = ''
        self.nthread = 1
        self.lyrmethod = 0
        self.sceid = 0
        self.param_range_def = 'cali_param_rng.def'
        if 'SEIMS_Model' not in cf.sections():
            raise ValueError("[SEIMS_Model] section MUST be existed in *.ini file.")
        self.model_dir = cf.get('SEIMS_Model', 'model_dir')
        self.bin_dir = cf.get('SEIMS_Model', 'bin_dir')
        self.nthread = cf.getint('SEIMS_Model', 'threadsnum')
        self.lyrmethod = cf.getint('SEIMS_Model', 'layeringmethod')
        if cf.has_option('SEIMS_Model', 'scenarioid'):
            self.sceid = cf.getint('SEIMS_Model', 'scenarioid')
        self.param_range_def = cf.get('SEIMS_Model', 'paramrngdef')
        self.param_range_def = self.model_dir + os.sep + self.param_range_def
        tstart = cf.get('SEIMS_Model', 'time_start')
        tend = cf.get('SEIMS_Model', 'time_end')
        try:  # UTCTIME
            self.time_start = StringClass.get_datetime(tstart)
            self.time_end = StringClass.get_datetime(tend)
        except ValueError:
            raise ValueError('The time format MUST be "YYYY-MM-DD" or "YYYY-MM-DD HH:MM:SS".')
        if self.time_start >= self.time_end:
            raise ValueError("Wrong time setted in [OPTIONAL_PARAMETERS]!")

        if not (FileClass.is_dir_exists(self.model_dir)
                and FileClass.is_dir_exists(self.bin_dir)):
            raise IOError('Please Check Directories defined in [PATH]. '
                          'BIN_DIR and MODEL_DIR are required!')
        # 3. optimization algorithm
        self.opt_mtd = method
        self.opt = None
        if self.opt_mtd == 'nsgaii':
            self.opt = nsgaiiConfig(cf, self.model_dir)


if __name__ == '__main__':
    cf, method = get_cali_config()
    cfg = CaliConfig(cf, method=method)

    print (cfg)

    # test the picklable of SAConfig class.
    import pickle

    s = pickle.dumps(cfg)
    # print (s)
    new_cfg = pickle.loads(s)
    print (new_cfg.model_dir)
