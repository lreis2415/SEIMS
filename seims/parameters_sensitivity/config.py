#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base configuration of Parameters Scesitivity Analysis.
    @author   : Liangjun Zhu
    @changelog: 17-12-22  lj - initial implementation.\n
"""
import os
import sys

from pygeoc.utils import FileClass, StringClass, UtilClass, get_config_parser

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.append(os.path.abspath(os.path.join(sys.path[0], '..')))


class PSAConfig(object):
    """Parse parameters sensitivity analysis configuration of SEIMS project."""

    def __init__(self, cf):
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
        self.seims_bin = ''
        self.model_dir = ''
        self.seims_nthread = 1
        self.seims_lyrmethod = 0
        if 'SEIMS_Model' in cf.sections():
            self.seims_bin = cf.get('SEIMS_Model', 'bin_dir')
            self.model_dir = cf.get('SEIMS_Model', 'model_dir')
            self.seims_nthread = cf.getint('SEIMS_Model', 'threadsnum')
            self.seims_lyrmethod = cf.getint('SEIMS_Model', 'layeringmethod')
        else:
            raise ValueError("[SEIMS_Model] section MUST be existed in *.ini file.")
        if not (FileClass.is_dir_exists(self.model_dir)
                and FileClass.is_dir_exists(self.seims_bin)):
            raise IOError('Please Check Directories defined in [PATH]. '
                          'BIN_DIR and MODEL_DIR are required!')

        # 3. Parameters Screening
        self.param_range_def = 'screen_param_rng.def'
        self.N = 100
        self.num_levels = 10
        self.grid_jump = 2
        self.optimal_t = None
        self.local_opt = True
        if 'Parameter_Screening' in cf.sections():
            self.param_range_def = cf.get('Parameter_Screening', 'paramrngdef')
            if cf.has_option('Parameter_Screening', 'n'):
                self.N = cf.getint('Parameter_Screening', 'n')
            if cf.has_option('Parameter_Screening', 'num_levels'):
                self.num_levels = cf.getint('Parameter_Screening', 'num_levels')
            if cf.has_option('Parameter_Screening', 'grid_jump'):
                self.grid_jump = cf.getint('Parameter_Screening', 'grid_jump')
            if cf.has_option('Parameter_Screening', 'optimal_trajectories'):
                tmp_opt_t = cf.get('Parameter_Screening', 'optimal_trajectories')
                if not StringClass.string_match(tmp_opt_t, 'none'):
                    self.optimal_t = cf.getint('Parameter_Screening', 'optimal_trajectories')
                    if self.optimal_t > self.N or self.optimal_t < 2:
                        self.optimal_t = None
            if cf.has_option('Parameter_Screening', 'local_optimization'):
                self.local_opt = cf.getboolean('Parameter_Screening', 'local_optimization')
        else:
            raise ValueError("[Parameter_Screening] section MUST be existed in *.ini file.")
        self.param_range_def = self.model_dir + os.sep + self.param_range_def
        if not FileClass.is_file_exists(self.param_range_def):
            raise IOError('Parameters range definition MUST be provided!')
        # 4. others
        self.psa_outpath = '%s/OUTPUT-PSA-N%dL%d' % (self.model_dir, self.N, self.num_levels)
        UtilClass.rmmkdir(self.psa_outpath)


if __name__ == '__main__':
    cf = get_config_parser()
    cfg = PSAConfig(cf)

    print (cfg.param_range_def)
