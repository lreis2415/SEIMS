#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base configuration of Parameters Sensitivity Analysis.
    @author   : Liangjun Zhu
    @changelog: 17-12-22  lj - initial implementation.\n
                18-1-11   lj - integration of screening method and variant-based method.\n
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


def get_psa_config():
    """Parse arguments.
    Returns:
        cf: ConfigParse object of *.ini file
        mtd: Parameters sensitivity method name, currently, 'morris' and 'fast' are supported.
    """
    # define input arguments
    parser = argparse.ArgumentParser(description="Execute parameters sensitivity analysis.")
    parser.add_argument('-ini', type=str, help="Full path of configuration file")
    # add mutually group
    psa_group = parser.add_mutually_exclusive_group()
    psa_group.add_argument('-morris', action='store_true', help='Run Morris Screening method')
    psa_group.add_argument('-fast', action='store_true', help='Run FAST variant-based method')
    # parse arguments
    args = parser.parse_args()
    ini_file = args.ini
    psa_mtd = 'morris'  # Default
    if args.fast:
        psa_mtd = 'fast'
    elif args.morris:
        psa_mtd = 'morris'
    if not FileClass.is_file_exists(ini_file):
        raise ImportError('Configuration file is not existed: %s' % ini_file)
    cf = ConfigParser()
    cf.read(ini_file)
    return cf, psa_mtd


class MorrisConfig(object):
    """Configuration for Morris screening method."""

    def __init__(self, cf):
        """Get parameters from ConfigParser object."""
        self.param_range_def = 'morris_param_rng.def'
        self.N = 100
        self.num_levels = 10
        self.grid_jump = 2
        self.optimal_t = None
        self.local_opt = True
        section_name = 'Morris_Method'
        if section_name in cf.sections():
            self.param_range_def = cf.get(section_name, 'paramrngdef')
            if cf.has_option(section_name, 'n'):
                self.N = cf.getint(section_name, 'n')
            if cf.has_option(section_name, 'num_levels'):
                self.num_levels = cf.getint(section_name, 'num_levels')
            if cf.has_option(section_name, 'grid_jump'):
                self.grid_jump = cf.getint(section_name, 'grid_jump')
            if cf.has_option(section_name, 'optimal_trajectories'):
                tmp_opt_t = cf.get(section_name, 'optimal_trajectories')
                if not StringClass.string_match(tmp_opt_t, 'none'):
                    self.optimal_t = cf.getint(section_name, 'optimal_trajectories')
                    if self.optimal_t > self.N or self.optimal_t < 2:
                        self.optimal_t = None
            if cf.has_option(section_name, 'local_optimization'):
                self.local_opt = cf.getboolean(section_name, 'local_optimization')
        else:
            raise ValueError('[%s] section MUST be existed in *.ini file.' % section_name)


class FASTConfig(object):
    """Configuration for FAST variant-based method."""

    def __init__(self, cf):
        """Get parameters from ConfigParser object."""
        self.param_range_def = 'fast_param_rng.def'
        self.N = 64
        self.M = 4
        section_name = 'FAST_Method'
        if section_name in cf.sections():
            self.param_range_def = cf.get(section_name, 'paramrngdef')
            if cf.has_option(section_name, 'n'):
                self.N = cf.getint(section_name, 'n')
            if cf.has_option(section_name, 'm'):
                self.M = cf.getint(section_name, 'm')
        else:
            raise ValueError('[%s] section MUST be existed in *.ini file.' % section_name)
        if self.N <= 4 * self.M ** 2:
            raise ValueError('Sample size N > 4M^2 is required for FAST method. M=4 by default.')


class PSAOutputs(object):
    """Predefined output files for parameters sensitivity analysis."""
    def __init__(self, wp):
        """Initialization."""
        self.param_defs_json = wp + os.path.sep + 'param_defs.json'
        self.param_values_txt = wp + os.path.sep + 'param_values.txt'
        self.output_values_dir = wp + os.path.sep + 'temp_output_values'
        self.output_values_txt = wp + os.path.sep + 'output_values.txt'
        self.psa_si_json = wp + os.path.sep + 'psa_si.json'
        self.psa_si_sort_txt = wp + os.path.sep + 'psa_si_sorted.csv'
        UtilClass.mkdir(self.output_values_dir)


class PSAConfig(object):
    """Parse parameters sensitivity analysis configuration of SEIMS project."""

    def __init__(self, cf, method='morris'):
        """Initialization."""
        self.method = method
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
        self.seims_bin = ''
        self.model_dir = ''
        self.seims_version = 'OMP'
        self.mpi_bin = None
        self.hosts_opt = None
        self.hostfile = None
        self.seims_nprocess = 1
        self.seims_nthread = 1
        self.seims_lyrmethod = 0
        if 'SEIMS_Model' not in cf.sections():
            raise ValueError("[SEIMS_Model] section MUST be existed in *.ini file.")
        self.seims_bin = cf.get('SEIMS_Model', 'bin_dir')
        self.model_dir = cf.get('SEIMS_Model', 'model_dir')
        self.seims_nthread = cf.getint('SEIMS_Model', 'threadsnum')
        self.seims_lyrmethod = cf.getint('SEIMS_Model', 'layeringmethod')
        if cf.has_option('SEIMS_Model', 'version'):
            self.seims_version = cf.get('SEIMS_Model', 'version')
        if cf.has_option('SEIMS_Model', 'mpi_bin'):
            self.mpi_bin = cf.get('SEIMS_Model', 'mpi_bin')
        if cf.has_option('SEIMS_Model', 'hostopt'):
            self.hosts_opt = cf.get('SEIMS_Model', 'hostopt')
        if cf.has_option('SEIMS_Model', 'hostfile'):
            self.hostfile = cf.get('SEIMS_Model', 'hostfile')
        if cf.has_option('SEIMS_Model', 'processnum'):
            self.seims_nprocess = cf.getint('SEIMS_Model', 'processnum')
        try:
            # UTCTIME
            tstart = cf.get('SEIMS_Model', 'sim_time_start')
            tend = cf.get('SEIMS_Model', 'sim_time_end')
            self.time_start = StringClass.get_datetime(tstart)
            self.time_end = StringClass.get_datetime(tend)
            tstart = cf.get('SEIMS_Model', 'psa_time_start')
            tend = cf.get('SEIMS_Model', 'psa_time_end')
            self.psa_stime = StringClass.get_datetime(tstart)
            self.psa_etime = StringClass.get_datetime(tend)
        except ValueError:
            raise ValueError('The time format MUST be "YYYY-MM-DD" or "YYYY-MM-DD HH:MM:SS".')
        if self.time_start >= self.time_end or self.psa_stime >= self.psa_etime:
            raise ValueError("Wrong time settings in [SEIMS_Model]!")

        if not (FileClass.is_dir_exists(self.model_dir)
                and FileClass.is_dir_exists(self.seims_bin)):
            raise IOError('Please Check Directories defined in [PATH]. '
                          'BIN_DIR and MODEL_DIR are required!')

        self.evaluate_params = list()
        if cf.has_option('SEIMS_Model', 'evaluate_param'):
            eva_str = cf.get('SEIMS_Model', 'evaluate_param')
            self.evaluate_params = StringClass.split_string(eva_str, ',')
        else:
            self.evaluate_params = ['Q']  # Default
        # 3. Parameters settings for sensitivity analysis methods
        self.morris = None
        self.fast = None
        if self.method == 'fast':
            self.fast = FASTConfig(cf)
            self.param_range_def = self.model_dir + os.path.sep + self.fast.param_range_def
            self.psa_outpath = '%s/PSA-FAST-N%dM%d' % (self.model_dir, self.fast.N, self.fast.M)
        elif self.method == 'morris':
            self.morris = MorrisConfig(cf)
            self.param_range_def = self.model_dir + os.path.sep + self.morris.param_range_def
            self.psa_outpath = '%s/PSA-Morris-N%dL%d' % (self.model_dir,
                                                         self.morris.N, self.morris.num_levels)
        if not FileClass.is_file_exists(self.param_range_def):
            raise IOError('Parameters range definition MUST be provided!')
        # Do not remove psa_outpath if already existed
        UtilClass.mkdir(self.psa_outpath)
        self.outfiles = PSAOutputs(self.psa_outpath)


if __name__ == '__main__':
    cf, method = get_psa_config()
    cfg = PSAConfig(cf, method=method)

    print(cfg.param_range_def)
    if cfg.method == 'morris':
        print('Morris sceening method:')
        print('  N: %d, num_levels: %d, grid_jump: %d' % (cfg.morris.N, cfg.morris.num_levels,
                                                           cfg.morris.grid_jump))
    elif cfg.method == 'fast':
        print('FAST variant-based method')
        print('  N: %d, M: %d' % (cfg.fast.N, cfg.fast.M))
