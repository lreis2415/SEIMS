"""  Parameters sensitivity analysis of SEIMS-based watershed model.

The invoking format is:

  `python demo_parameters_sensitivity.py -name <demo_watershed_name>`

    @author   : Liangjun Zhu

    @changelog:
    - 19-01-07 - lj - initial implementation.
    - 25-09-09 - lj - redesign model folder's structure
"""
from __future__ import absolute_import, unicode_literals

import os
import sys
import time

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import UtilClass

from parameters_sensitivity.sensitivity import Sensitivity
from test.demo_config import ModelPaths, write_single_sensitivity_config_file
from test.demo_config import DEMO_MODELS, get_watershed_name_info


def main():
    wtsd_name, model_name, config_name = get_watershed_name_info()
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

    model_paths = ModelPaths(SEIMS_path, wtsd_name, model_name)
    org_file_name = 'sensitivity_analysis.ini'
    org_cfg_file = '%s/%s' % (model_paths.cfg_dir, org_file_name)
    psa_cfg_file = '%s/%s' % (model_paths.model_dir, org_file_name)
    if config_name != '':
        org_cfg_file = '%s/%s/%s' % (model_paths.cfg_dir, config_name, org_file_name)
        psa_cfg_file = '%s/%s/%s' % (model_paths.model_dir, config_name, org_file_name)
        psa_cfg = write_single_sensitivity_config_file(model_paths, org_cfg_file,
                                                       psa_cfg_file)
    else:
        psa_cfg = write_single_sensitivity_config_file(model_paths, org_cfg_file,
                                                       psa_cfg_file)

    print('### START TO PARAMETERS SENSITIVITY ANALYSIS ###')
    start_t = time.time()
    saobj = Sensitivity(psa_cfg)
    saobj.run()
    saobj.plot()
    print('### END OF PARAMETERS SENSITIVITY ANALYSIS ###')
    print('Running time: %.2fs' % (time.time() - start_t))


if __name__ == "__main__":
    main()
