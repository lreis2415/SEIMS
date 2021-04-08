"""Data preprocessing of SEIMS-based watershed model.

The invoking format is:

  `python demo_preprocess.py -name <demo_watershed_name>`

    @author   : Liangjun Zhu

    @changelog:
    - 18-02-09 - lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import UtilClass

from preprocess.db_build_mongodb import ImportMongodbClass
from preprocess.sd_delineation import SpatialDelineation
from test.demo_config import ModelPaths, write_preprocess_config_file
from test.demo_config import DEMO_MODELS, get_watershed_name


def main():
    wtsd_name = get_watershed_name('Specify watershed name to run preprocess.')
    if wtsd_name not in list(DEMO_MODELS.keys()):
        print('%s is not one of the available demo watershed: %s' %
              (wtsd_name, ','.join(list(DEMO_MODELS.keys()))))
        exit(-1)
    cur_path = UtilClass.current_path(lambda: 0)
    SEIMS_path = os.path.abspath(cur_path + '../../..')

    model_paths = ModelPaths(SEIMS_path, wtsd_name, DEMO_MODELS[wtsd_name])
    seims_cfg = write_preprocess_config_file(model_paths, 'preprocess.ini')

    SpatialDelineation.workflow(seims_cfg)  # Spatial delineation by TauDEM
    ImportMongodbClass.workflow(seims_cfg)  # Import to MongoDB database


if __name__ == "__main__":
    main()
