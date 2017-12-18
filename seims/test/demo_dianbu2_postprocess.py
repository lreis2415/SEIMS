#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
  Postprecessing for SEIMS. Demo watershed named dianbu2.
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

from postprocess.plot_timeseries import TimeSeriesPlots
from demo_config import ModelPaths, write_postprocess_config_file


def main():
    cur_path = UtilClass.current_path()
    SEIMS_path = os.path.abspath(cur_path + '../../..')
    model_paths = ModelPaths(SEIMS_path, 'dianbu2', 'model_dianbu2_30m_demo')

    # hydrograph, e.g. discharge
    scenario_id = 0
    post_cfg = write_postprocess_config_file(model_paths, 'postprocess_30m_omp.ini', scenario_id)
    TimeSeriesPlots(post_cfg).workflow()


if __name__ == "__main__":
    main()
