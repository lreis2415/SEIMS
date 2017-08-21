#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Configuration of Scenario Analysis for SEIMS.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-12-30  hr - initial implementation.\n
                17-08-18  lj - reorganize as basic class.\n
"""
import json
import os

try:
    from ConfigParser import ConfigParser  # py2
except ImportError:
    from configparser import ConfigParser  # py3

from seims.pygeoc.pygeoc.utils.utils import FileClass, StringClass, UtilClass, get_config_file


class SAConfig(object):
    """Parse scenario analysis configuration of SEIMS project."""

    def __init__(self, cf):
        """Initialization."""
        # 1. NSGA-II related parameters
        self.nsga2_ngens = 1
        self.nsga2_npop = 4
        self.nsga2_rcross = 0.75
        self.nsga2_rmut = 0.1
        self.nsga2_rsel = 0.8
        if 'NSGAII' in cf.sections():
            self.nsga2_ngens = cf.getint('NSGAII', 'generationsnum')
            self.nsga2_npop = cf.getint('NSGAII', 'populationsize')
            self.nsga2_rcross = cf.getfloat('NSGAII', 'crossoverrate')
            self.nsga2_rmut = cf.getfloat('NSGAII', 'mutaterate')
            self.nsga2_rsel = cf.getfloat('NSGAII', 'selectrate')
        else:
            raise ValueError('[NSGAII] section MUST be existed in *.ini file.')
        if self.nsga2_npop % 4 != 0:
            raise ValueError('PopulationSize must be a multiple of 4.')
        # 2. MongoDB
        self.hostname = '127.0.0.1'  # localhost by default
        self.port = 27017
        self.spatial_db = ''
        self.bmp_scenario_db = ''
        if 'MONGODB' in cf.sections():
            self.hostname = cf.get('MONGODB', 'hostname')
            self.port = cf.getint('MONGODB', 'port')
            self.spatial_db = cf.get('MONGODB', 'spatialdbname')
            self.bmp_scenario_db = cf.get('MONGODB', 'bmpscenariodbname')
        else:
            raise ValueError('[MONGODB] section MUST be existed in *.ini file.')
        if not StringClass.is_valid_ip_addr(self.hostname):
            raise ValueError('HOSTNAME illegal defined in [MONGODB]!')

        # 3. SEIMS_Model
        self.model_dir = cf.get('SEIMS_Model', 'model_dir')
        self.seims_bin = cf.get('SEIMS_Model', 'bin_dir')
        self.seims_nthread = 1
        self.seims_lyrmethod = 0
        if 'SEIMS_Model' in cf.sections():
            self.model_dir = cf.get('SEIMS_Model', 'model_dir')
            self.seims_bin = cf.get('SEIMS_Model', 'bin_dir')
            self.seims_nthread = cf.getint('SEIMS_Model', 'threadsnum')
            self.seims_lyrmethod = cf.getint('SEIMS_Model', 'layeringmethod')
        else:
            raise ValueError("[SEIMS_Model] section MUST be existed in *.ini file.")
        if not (FileClass.is_dir_exists(self.model_dir)
                and FileClass.is_dir_exists(self.seims_bin)):
            raise IOError('Please Check Directories defined in [PATH]. '
                          'BIN_DIR and MODEL_DIR are required!')
        # 4. define gen_values
        self.nsga2_dir = self.model_dir + os.sep + 'NSGAII_OUTPUT' + os.sep + \
                         'Gen_%d_Pop_%d' % (self.nsga2_ngens, self.nsga2_npop)
        self.scenario_dir = self.nsga2_dir + os.sep + 'Scenarios'
        UtilClass.rmmkdir(self.nsga2_dir)
        UtilClass.rmmkdir(self.scenario_dir)
        self.hypervlog = self.nsga2_dir + os.sep + 'hypervolume.txt'
        self.scenariolog = self.nsga2_dir + os.sep + 'scenarios_info.txt'
        self.logfile = self.nsga2_dir + os.sep + 'runtime.log'


        # 5. Application specific setting section [BMPs]
        self.bmps_info = dict()
        self.bmps_rule = False
        if 'BMPs' in cf.sections():
            bmpsinfostr = cf.get('BMPs', 'bmps_info')
            self.bmps_rule = cf.getboolean('BMPs', 'bmps_rule')
        else:
            raise ValueError("[BMPs] section MUST be existed for specific SA.")
        self.bmps_info = json.loads(bmpsinfostr)
        self.bmps_info = {str(k): (str(v) if isinstance(v, unicode) else v) for k, v in
                          self.bmps_info.items()}


def parse_ini_configuration():
    """Load model configuration from *.ini file"""
    cf = ConfigParser()
    ini_file = get_config_file()
    cf.read(ini_file)
    return SAConfig(cf)


if __name__ == '__main__':
    cfg = parse_ini_configuration()
    print (cfg.model_dir)
