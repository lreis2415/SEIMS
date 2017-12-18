#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
  Running SEIMS of demo watershed named dianbu2.
"""
import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.append(os.path.abspath(os.path.join(sys.path[0], '..')))

try:
    from ConfigParser import ConfigParser  # py2
except ImportError:
    from configparser import ConfigParser  # py3
from pygeoc.utils import UtilClass

from run_seims import MainSEIMS
from demo_config import ModelPaths


def main():
    cur_path = UtilClass.current_path()
    SEIMS_path = os.path.abspath(cur_path + '../../..')
    model_paths = ModelPaths(SEIMS_path, 'dianbu2', 'model_dianbu2_30m_demo')

    scenario_id = 0
    seims_obj = MainSEIMS(model_paths.bin_dir, model_paths.model_dir, sceid=scenario_id)
    seims_obj.run()


if __name__ == "__main__":
    main()
