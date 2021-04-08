"""Auto-calibration of SEIMS-based watershed model based on NSGA-II algorithm.

The invoking format is:

  `python demo_calibration.py -name <demo_watershed_name>`

    @author   : Liangjun Zhu

    @changelog:
    - 19-01-07  - lj - initial implementation.
"""
from __future__ import absolute_import, unicode_literals

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import UtilClass

from test.demo_config import ModelPaths, write_calibration_config_file
from test.demo_config import DEMO_MODELS, get_watershed_name
from calibration import main_nsga2 as cali_nsga2


def main():
    wtsd_name = get_watershed_name('Specify watershed name to run auto-calibration.')
    if wtsd_name not in list(DEMO_MODELS.keys()):
        print('%s is not one of the available demo watershed: %s' %
              (wtsd_name, ','.join(list(DEMO_MODELS.keys()))))
        exit(-1)

    cur_path = UtilClass.current_path(lambda: 0)
    SEIMS_path = os.path.abspath(cur_path + '../../..')
    model_paths = ModelPaths(SEIMS_path, wtsd_name, DEMO_MODELS[wtsd_name])

    cali_cfg = write_calibration_config_file(model_paths, 'calibration.ini')
    cali_nsga2.main(cali_cfg)


if __name__ == "__main__":
    main()
