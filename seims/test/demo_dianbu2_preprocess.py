#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
  Preprecessing for SEIMS. Demo watershed named dianbu2.
                18-02-09  lj - compatible with Python3.\n
"""
from __future__ import absolute_import

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

try:
    from ConfigParser import ConfigParser  # py2
except ImportError:
    from configparser import ConfigParser  # py3
from pygeoc.utils import UtilClass

from preprocess.db_build_mongodb import ImportMongodbClass
from preprocess.sd_delineation import SpatialDelineation
from test.demo_config import ModelPaths, write_preprocess_config_file


def main():
    cur_path = UtilClass.current_path(lambda: 0)
    SEIMS_path = os.path.abspath(cur_path + '../../..')
    model_paths = ModelPaths(SEIMS_path, 'dianbu2', 'demo_dianbu2_model')
    seims_cfg = write_preprocess_config_file(model_paths, 'preprocess.ini')

    SpatialDelineation.workflow(seims_cfg)  # Spatial delineation by TauDEM
    ImportMongodbClass.workflow(seims_cfg)  # Import to MongoDB database


if __name__ == "__main__":
    main()
