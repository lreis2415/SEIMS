#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base configuration of Scenario Analysis.

    @author   : Liangjun Zhu, Huiran Gao

    @changelog:

    - 16-12-30  hr - initial implementation.
    - 17-08-18  lj - reorganize as basic class.
    - 18-02-09  lj - compatible with Python3.
    - 18-10-29  lj - Redesign the code structure.
"""
from __future__ import absolute_import

from configparser import ConfigParser
import json
import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))
from typing import Union
from pygeoc.utils import UtilClass
from run_seims import ParseSEIMSConfig
from utility import get_optimization_config, parse_datetime_from_ini
from utility import ParseNSGA2Config
from scenario_analysis import BMPS_RULE_METHODS


class SAConfig(object):
    """Parse scenario analysis configuration of SEIMS project."""

    def __init__(self, cf, method='nsga2'):
        # type: (ConfigParser, str) -> None
        """Initialization."""
        # 1. SEIMS model related
        self.model = ParseSEIMSConfig(cf)
        # self.spatial_db = self.model.db_name
        # self.bmp_scenario_db

        # 2. Common settings of BMPs scenario
        self.eval_stime = None
        self.eval_etime = None
        self.worst_econ = 0.
        self.worst_env = 0.
        self.runtime_years = 0.
        if 'Scenario_Common' in cf.sections():
            self.eval_stime = parse_datetime_from_ini(cf, 'Scenario_Common', 'eval_time_start')
            self.eval_etime = parse_datetime_from_ini(cf, 'Scenario_Common', 'eval_time_end')
            self.worst_econ = cf.getfloat('Scenario_Common', 'worst_economy')
            self.worst_env = cf.getfloat('Scenario_Common', 'worst_environment')
            self.runtime_years = cf.getfloat('Scenario_Common', 'runtime_years')
        else:
            raise ValueError('[Scenario_Common] section MUST be existed in *.ini file.')

        # 3. Application specific setting section [BMPs]
        self.bmps_info = dict()  # BMP to be evaluated, JSON format
        self.bmps_retain = dict()  # BMPs to be constant, JSON format
        self.export_sce_txt = False
        self.export_sce_tif = False
        self.bmps_rule_method = 'RDM'
        if 'BMPs' in cf.sections():
            bmpsinfostr = cf.get('BMPs', 'bmps_info')
            self.bmps_info = json.loads(bmpsinfostr)
            self.bmps_info = UtilClass.decode_strs_in_dict(self.bmps_info)
            if cf.has_option('BMPs', 'bmps_retain'):
                bmpsretainstr = cf.get('BMPs', 'bmps_retain')
                self.bmps_retain = json.loads(bmpsretainstr)
                self.bmps_retain = UtilClass.decode_strs_in_dict(self.bmps_retain)
            if cf.has_option('BMPs', 'export_scenario_txt'):
                self.export_sce_txt = cf.getboolean('BMPs', 'export_scenario_txt')
            if cf.has_option('BMPs', 'export_scenario_tif'):
                self.export_sce_tif = cf.getboolean('BMPs', 'export_scenario_tif')
            if cf.has_option('BMPs', 'bmps_rule_method'):
                self.bmps_rule_method = cf.get('BMPs', 'bmps_rule_method')
                if self.bmps_rule_method not in BMPS_RULE_METHODS:
                    self.bmps_rule_method = 'RDM'
        else:
            raise ValueError("[BMPs] section MUST be existed for specific SA.")

        # 4. Parameters settings for specific optimization algorithm
        self.opt_mtd = method
        self.opt = None  # type: Union[ParseNSGA2Config]
        if self.opt_mtd == 'nsga2':
            self.opt = ParseNSGA2Config(cf, self.model.model_dir, 'SA_NSGA2_%s' % self.bmps_rule_method)
        self.scenario_dir = self.opt.out_dir + os.path.sep + 'Scenarios'
        UtilClass.rmmkdir(self.scenario_dir)


if __name__ == '__main__':
    cf, method = get_optimization_config("Execute scenario analysis.")
    cfg = SAConfig(cf, method=method)

    # test the picklable of SAConfig class.
    import pickle

    s = pickle.dumps(cfg)
    # print(s)
    new_cfg = pickle.loads(s)
    print(new_cfg.model.model_dir, new_cfg.bmps_rule_method)
