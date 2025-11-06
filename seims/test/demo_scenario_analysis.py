"""BMP scenarios analysis based on SEIMS-based watershed model and NSGA-II algorithm.

The invoking format is:

  `python demo_scenario_analysis.py -name <demo_watershed_name>`

    @author   : Liangjun Zhu

    @changelog:
    - 19-01-08 - lj - initial implementation.
    - 25-09-09 - lj - redesign model folder's structure
"""
from __future__ import absolute_import, unicode_literals

import os
import sys
import time
from io import open

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import UtilClass

from utility.scoop_func import scoop_log
from scenario_analysis import BMPS_CFG_UNITS
from scenario_analysis.config import SAConfig
from scenario_analysis.spatialunits.config import SASlpPosConfig,\
    SAConnFieldConfig, SACommUnitConfig
from scenario_analysis.spatialunits.scenario import SUScenario
from scenario_analysis.spatialunits import main_nsga2 as sa_nsga2
from test.demo_config import ModelPaths, write_single_scenario_analysis_config_file
from test.demo_config import DEMO_MODELS, get_watershed_name_info


def main():
    wtsd_name, model_name, config_name, bin_dir = get_watershed_name_info()
    if wtsd_name not in list(DEMO_MODELS.keys()):
        print('%s is not one of the available demo watershed: %s' %
              (wtsd_name, ','.join(list(DEMO_MODELS.keys()))))
        exit(-1)
    model_dict = DEMO_MODELS[wtsd_name]
    if model_name not in list(model_dict.keys()):
        print('%s is not one of the available demo watershed models: %s' %
              (model_name, ','.join(list(model_dict.keys()))))
        exit(-1)
    if 'confignames' not in list(model_dict[model_name].keys()):
        print('The key confignames MUST be specified for each model!')
        exit(-1)
    if config_name not in model_dict[model_name]['confignames']:
        print('The configName %s of model %s is not defined!' % (config_name, model_name))
        exit(-1)

    cur_path = UtilClass.current_path(lambda: 0)
    SEIMS_path = os.path.abspath(cur_path + '../../..')

    model_paths = ModelPaths(SEIMS_path, wtsd_name, model_name, bin_dir)
    org_file_name = 'scenario_analysis.ini'
    org_cfg_file = '%s/%s' % (model_paths.cfg_dir, org_file_name)
    sce_cfg_file = '%s/%s' % (model_paths.workspace, org_file_name)
    if config_name != '':
        org_cfg_file = '%s/%s/%s' % (model_paths.cfg_dir, config_name, org_file_name)
        sce_cfg_file = '%s/%s/%s' % (model_paths.workspace, config_name, org_file_name)
        cf = write_single_scenario_analysis_config_file(model_paths, org_cfg_file,
                                                        sce_cfg_file)
    else:
        cf = write_single_scenario_analysis_config_file(model_paths, org_cfg_file,
                                                        sce_cfg_file)

    base_cfg = SAConfig(cf)  # type: SAConfig
    if base_cfg.bmps_cfg_unit == BMPS_CFG_UNITS[3]:  # SLPPOS
        cfg = SASlpPosConfig(cf)
    elif base_cfg.bmps_cfg_unit == BMPS_CFG_UNITS[2]:  # CONNFIELD
        cfg = SAConnFieldConfig(cf)
    else:  # Common spatial units, e.g., HRU and EXPLICITHRU
        cfg = SACommUnitConfig(cf)
    cfg.construct_indexes_units_gene()

    sce = SUScenario(cfg)

    scoop_log('### START TO SCENARIOS OPTIMIZING ###')
    start_t = time.time()

    fpop, fstats = sa_nsga2.main(sce)
    fpop.sort(key=lambda x: x.fitness.values)
    scoop_log(fstats)
    with open(cfg.opt.logbookfile, 'w', encoding='utf-8') as f:
        # In case of 'TypeError: write() argument 1 must be unicode, not str' in Python2.7
        #   when using unicode_literals, please use '%s' to concatenate string!
        f.write('%s' % fstats.__str__())

    end_t = time.time()
    scoop_log('Running time: %.2fs' % (end_t - start_t))


if __name__ == "__main__":
    main()
