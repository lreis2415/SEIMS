#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base configuration of Scenario Analysis.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-12-30  hr - initial implementation.\n
                17-08-18  lj - reorganize as basic class.\n
"""
import json
import os
import sys

from pygeoc.utils import FileClass, StringClass, UtilClass, get_config_parser

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.append(os.path.abspath(os.path.join(sys.path[0], '..')))


class SAConfig(object):
    """Parse scenario analysis configuration of SEIMS project."""

    def __init__(self, cf):
        """Initialization."""
        # 1. NSGA-II related parameters
        self.nsga2_ngens = 1
        self.nsga2_npop = 4
        self.nsga2_rcross = 0.75
        self.nsga2_pmut = 0.05
        self.nsga2_rmut = 0.1
        self.nsga2_rsel = 0.8
        if 'NSGAII' in cf.sections():
            self.nsga2_ngens = cf.getint('NSGAII', 'generationsnum')
            self.nsga2_npop = cf.getint('NSGAII', 'populationsize')
            self.nsga2_rcross = cf.getfloat('NSGAII', 'crossoverrate')
            self.nsga2_pmut = cf.getfloat('NSGAII', 'maxmutateperc')
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
        self.model_dir = ''
        self.seims_bin = ''
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

        # 4. Application specific setting section [BMPs]
        self.bmps_info = dict()
        self.bmps_rule = False
        self.rule_method = 1
        self.bmps_retain = dict()
        self.export_sce_txt = False
        self.export_sce_tif = False
        if 'BMPs' in cf.sections():
            bmpsinfostr = cf.get('BMPs', 'bmps_info')
            self.bmps_rule = cf.getboolean('BMPs', 'bmps_rule')
            if cf.has_option('BMPs', 'rule_method'):
                self.rule_method = cf.getint('BMPs', 'rule_method')
            if cf.has_option('BMPs', 'bmps_retain'):
                bmpsretainstr = cf.get('BMPs', 'bmps_retain')
                self.bmps_retain = json.loads(bmpsretainstr)
                self.bmps_retain = UtilClass.decode_strs_in_dict(self.bmps_retain)
            if cf.has_option('BMPs', 'export_scenario_txt'):
                self.export_sce_txt = cf.getboolean('BMPs', 'export_scenario_txt')
            if cf.has_option('BMPs', 'export_scenario_tif'):
                self.export_sce_tif = cf.getboolean('BMPs', 'export_scenario_tif')
        else:
            raise ValueError("[BMPs] section MUST be existed for specific SA.")
        self.bmps_info = json.loads(bmpsinfostr)
        self.bmps_info = UtilClass.decode_strs_in_dict(self.bmps_info)

        # 5. Application specific setting section [Effectiveness]
        self.worst_econ = 0
        self.worst_env = 0
        self.runtime_years = 0
        if 'Effectiveness' in cf.sections():
            self.worst_econ = cf.getfloat('Effectiveness', 'worst_economy')
            self.worst_env = cf.getfloat('Effectiveness', 'worst_environment')
            self.runtime_years = cf.getfloat('Effectiveness', 'runtime_years')
            self.runtime_years = cf.getfloat('Effectiveness', 'runtime_years')

        # 6. define gene_values
        fn = 'Gen_%d_Pop_%d' % (self.nsga2_ngens, self.nsga2_npop)
        fn += '_rule' if self.bmps_rule else '_random'
        self.nsga2_dir = self.model_dir + os.sep + 'NSGAII_OUTPUT' + os.sep + fn
        self.scenario_dir = self.nsga2_dir + os.sep + 'Scenarios'
        UtilClass.rmmkdir(self.nsga2_dir)
        UtilClass.rmmkdir(self.scenario_dir)
        self.hypervlog = self.nsga2_dir + os.sep + 'hypervolume.txt'
        self.scenariolog = self.nsga2_dir + os.sep + 'scenarios_info.txt'
        self.logfile = self.nsga2_dir + os.sep + 'runtime.log'
        self.logbookfile = self.nsga2_dir + os.sep + 'logbook.txt'


if __name__ == '__main__':
    cf = get_config_parser()
    cfg = SAConfig(cf)

    # test the picklable of SAConfig class.
    import pickle

    s = pickle.dumps(cfg)
    # print (s)
    new_cfg = pickle.loads(s)
    print (new_cfg.model_dir)
