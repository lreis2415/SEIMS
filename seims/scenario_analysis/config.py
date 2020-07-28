"""Base configuration of Scenario Analysis.

    @author   : Liangjun Zhu, Huiran Gao

    @changelog:
    - 16-12-30  - hr - initial implementation.
    - 17-08-18  - lj - reorganize as basic class.
    - 18-02-09  - lj - compatible with Python3.
    - 18-10-29  - lj - Redesign the code structure.
"""
from __future__ import absolute_import, unicode_literals

from configparser import ConfigParser
from future.utils import viewitems
from datetime import datetime
import json
import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))
from typing import Union, Optional, Dict, List, AnyStr
from pygeoc.utils import UtilClass, StringClass
from run_seims import ParseSEIMSConfig
from utility import get_optimization_config, parse_datetime_from_ini
from utility import ParseNSGA2Config, PlotConfig
from scenario_analysis import BMPS_CFG_UNITS, BMPS_CFG_METHODS, BMPS_CFG_PAIR


class SAConfig(object):
    """Parse scenario analysis configuration of SEIMS project."""

    def __init__(self, cf, method='nsga2'):
        # type: (ConfigParser, str) -> None
        """Initialization."""
        # 1. SEIMS model related
        self.model = ParseSEIMSConfig(cf)  # type: ParseSEIMSConfig

        # 2. Common settings of BMPs scenario
        self.eval_stime = None  # type: Optional[datetime]
        self.eval_etime = None  # type: Optional[datetime]
        self.worst_econ = 0.
        self.worst_env = 0.
        self.runtime_years = 0.
        self.export_sce_txt = False
        self.export_sce_tif = False
        if 'Scenario_Common' not in cf.sections():
            raise ValueError('[Scenario_Common] section MUST be existed in *.ini file.')
        self.eval_stime = parse_datetime_from_ini(cf, 'Scenario_Common', 'eval_time_start')
        self.eval_etime = parse_datetime_from_ini(cf, 'Scenario_Common', 'eval_time_end')
        self.worst_econ = cf.getfloat('Scenario_Common', 'worst_economy')
        self.worst_env = cf.getfloat('Scenario_Common', 'worst_environment')
        self.runtime_years = cf.getfloat('Scenario_Common', 'runtime_years')
        if cf.has_option('Scenario_Common', 'export_scenario_txt'):
            self.export_sce_txt = cf.getboolean('Scenario_Common', 'export_scenario_txt')
        if cf.has_option('Scenario_Common', 'export_scenario_tif'):
            self.export_sce_tif = cf.getboolean('Scenario_Common', 'export_scenario_tif')

        # 3. Application specific setting section [BMPs]
        # Selected BMPs, the key is BMPID, and value is the BMP information dict
        self.bmps_info = dict()  # type: Dict[int, Dict[AnyStr, Union[int, float, AnyStr, List[Union[int, float, AnyStr]]]]]
        # BMPs to be constant for generated scenarios during optimization, same format with bmps_info
        self.bmps_retain = dict()  # type: Dict[int, Dict[AnyStr, Union[int, float, AnyStr, List[Union[int, float, AnyStr]]]]]
        self.eval_info = dict()  # type: Dict[AnyStr, Union[int, float, AnyStr]]
        self.bmps_cfg_unit = 'CONNFIELD'  # type: AnyStr
        self.bmps_cfg_method = 'RAND'  # type: AnyStr
        if 'BMPs' not in cf.sections():
            raise ValueError('[BMPs] section MUST be existed for specific scenario analysis.')

        bmpsinfostr = cf.get('BMPs', 'bmps_info')
        self.bmps_info = UtilClass.decode_strs_in_dict(json.loads(bmpsinfostr))
        if cf.has_option('BMPs', 'bmps_retain'):
            bmpsretainstr = cf.get('BMPs', 'bmps_retain')
            self.bmps_retain = json.loads(bmpsretainstr)
            self.bmps_retain = UtilClass.decode_strs_in_dict(self.bmps_retain)
        evalinfostr = cf.get('BMPs', 'eval_info')
        self.eval_info = UtilClass.decode_strs_in_dict(json.loads(evalinfostr))
        bmpscfgunitstr = cf.get('BMPs', 'bmps_cfg_units')
        bmpscfgunitdict = UtilClass.decode_strs_in_dict(json.loads(bmpscfgunitstr))
        for unitname, unitcfg in viewitems(bmpscfgunitdict):
            self.bmps_cfg_unit = unitname
            if self.bmps_cfg_unit not in BMPS_CFG_UNITS:
                raise ValueError('BMPs configuration unit MUST be '
                                 'one of %s' % BMPS_CFG_UNITS.__str__())
            if not isinstance(unitcfg, dict):
                raise ValueError('The value of BMPs configuration unit MUST be dict value!')
            for cfgname, cfgvalue in viewitems(unitcfg):
                for bmpid, bmpdict in viewitems(self.bmps_info):
                    if cfgname in bmpdict:
                        continue
                    self.bmps_info[bmpid][cfgname] = cfgvalue
            break

        if cf.has_option('BMPs', 'bmps_cfg_method'):
            self.bmps_cfg_method = cf.get('BMPs', 'bmps_cfg_method')
            if self.bmps_cfg_method not in BMPS_CFG_METHODS:
                print('BMPs configuration method MUST be one of %s' % BMPS_CFG_METHODS.__str__())
                self.bmps_cfg_method = 'RAND'

        # Check the validation of configuration unit and method
        if self.bmps_cfg_method not in BMPS_CFG_PAIR.get(self.bmps_cfg_unit):
            raise ValueError('BMPs configuration method %s '
                             'is not supported on unit %s' % (self.bmps_cfg_method,
                                                              self.bmps_cfg_unit))

        # Optimize boundary of BMP configuration unit
        self.boundary_adaptive = False
        self.boundary_adaptive_threshs = None
        if cf.has_option('BMPs', 'bmps_cfg_units_opt'):
            self.boundary_adaptive = cf.getboolean('BMPs', 'bmps_cfg_units_opt')
        if cf.has_option('BMPs', 'boundary_adaptive_threshold'):
            tstr = cf.get('BMPs', 'boundary_adaptive_threshold')
            self.boundary_adaptive_threshs = StringClass.extract_numeric_values_from_string(tstr)
            if 0 not in self.boundary_adaptive_threshs:
                self.boundary_adaptive_threshs.append(0)  # 0 means no adjustment of boundary
            for tmp_thresh in self.boundary_adaptive_threshs:
                if -1 * tmp_thresh not in self.boundary_adaptive_threshs:
                    self.boundary_adaptive_threshs.append(-1 * tmp_thresh)

        # 4. Parameters settings for specific optimization algorithm
        self.opt_mtd = method
        self.opt = None  # type: Union[ParseNSGA2Config, None]
        if self.opt_mtd == 'nsga2':
            self.opt = ParseNSGA2Config(cf, self.model.model_dir,
                                        'SA_NSGA2_%s_%s' % (self.bmps_cfg_unit,
                                                            self.bmps_cfg_method))
        # Using the existed population derived from previous scenario optimization
        self.initial_byinput = cf.getboolean(self.opt_mtd.upper(), 'inputpopulation') if \
            cf.has_option(self.opt_mtd.upper(), 'inputpopulation') else False
        self.input_pareto_file = None
        self.input_pareto_gen = -1
        if cf.has_option(self.opt_mtd.upper(), 'paretofrontsfile'):
            self.input_pareto_file = cf.get(self.opt_mtd.upper(), 'paretofrontsfile')
        if cf.has_option(self.opt_mtd.upper(), 'generationselected'):
            self.input_pareto_gen = cf.getint(self.opt_mtd.upper(), 'generationselected')

        self.scenario_dir = self.opt.out_dir + os.path.sep + 'Scenarios'
        UtilClass.rmmkdir(self.scenario_dir)

        # 5. (Optional) Plot settings for matplotlib
        self.plot_cfg = PlotConfig(cf)


if __name__ == '__main__':
    cf, method = get_optimization_config('Execute scenario analysis.')
    cfg = SAConfig(cf, method=method)  # type: SAConfig

    # test the picklable of SAConfig class.
    import pickle

    s = pickle.dumps(cfg)
    new_cfg = pickle.loads(s)  # type: SAConfig
    print(new_cfg.model.model_dir)
    print('BMPs configuration unit %s, method %s' % (new_cfg.bmps_cfg_unit,
                                                     new_cfg.bmps_cfg_method))
