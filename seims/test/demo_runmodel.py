"""Running SEIMS-based watershed model of demo data.

The invoking format is:

  `python demo_runmodel.py -name <demo_watershed_name>`

    @author   : Liangjun Zhu

    @changelog:
    - 18-02-09 - lj - compatible with Python3.
    - 19-01-09 - lj - redesign to use configuration INI file
"""
from __future__ import absolute_import, unicode_literals

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import UtilClass

from run_seims import MainSEIMS
from test.demo_config import ModelPaths, DEMO_MODELS, get_watershed_name, write_runmodel_config_file


def main():
    wtsd_name = get_watershed_name('Specify watershed name to run SEIMS-based model.')
    if wtsd_name not in list(DEMO_MODELS.keys()):
        print('%s is not one of the available demo watershed: %s' %
              (wtsd_name, ','.join(list(DEMO_MODELS.keys()))))
        exit(-1)
    cur_path = UtilClass.current_path(lambda: 0)
    SEIMS_path = os.path.abspath(cur_path + '../../..')
    model_paths = ModelPaths(SEIMS_path, wtsd_name, DEMO_MODELS[wtsd_name])

    runmodel_cfg = write_runmodel_config_file(model_paths, 'runmodel.ini')

    seims_obj = MainSEIMS(args_dict=runmodel_cfg.ConfigDict)
    seims_obj.run()
    for l in seims_obj.runlogs:
        print(l)


if __name__ == "__main__":
    main()
