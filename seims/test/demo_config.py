#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""
  Running SEIMS of demo watershed, currently, dianbu watershed was used.
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

from preprocess.config import SEIMSConfig
from postprocess.config import PostConfig


class ModelPaths(object):
    """Paths required for SEIMS model setting.

    Args:
        bpath: Base path of SEIMS.
        data_dir_name: e.g., dianbu2
        model_dir_name: e.g., model_dianbu2_30m_demo
    """

    def __init__(self, bpath, data_dir_name, model_dir_name):
        self.mpi_bin = None
        self.bin_dir = bpath + os.sep + 'bin'
        self.prescript_dir = bpath + os.sep + 'seims' + os.sep + 'preprocess'
        self.base_dir = bpath + os.sep + 'data' + os.sep + data_dir_name
        self.cfg_dir = self.base_dir + os.sep + 'model_configs'
        self.model_dir = self.base_dir + os.sep + model_dir_name
        self.data_dir = self.base_dir + os.sep + 'data_prepare'
        self.clim_dir = self.data_dir + os.sep + 'climate'
        self.spatial_dir = self.data_dir + os.sep + 'spatial'
        self.observe_dir = self.data_dir + os.sep + 'observed'
        self.scenario_dir = self.data_dir + os.sep + 'scenario'
        self.lookup_dir = self.data_dir + os.sep + 'lookup'
        self.workspace = self.base_dir + os.sep + 'workspace'
        UtilClass.mkdir(self.workspace)
        print ('SEIMS binary location: %s' % self.bin_dir)
        print ('Demo data location: %s' % self.base_dir)
        print ('Data preprocess location: %s' % self.workspace)


def write_preprocess_config_file(mpaths, org_file_name):
    org_cfg_file = mpaths.cfg_dir + os.sep + org_file_name
    pre_cfg_file = mpaths.workspace + os.sep + org_file_name
    cfg_items = list()
    with open(org_cfg_file, 'r') as f:
        for line in f.readlines():
            cfg_items.append(line.strip())
    # print cfg_items
    cfg_items.append('[PATH]')
    cfg_items.append('PREPROC_SCRIPT_DIR = %s' % mpaths.prescript_dir)
    cfg_items.append('CPP_PROGRAM_DIR = %s' % mpaths.bin_dir)
    cfg_items.append('MPIEXEC_DIR = %s' % mpaths.mpi_bin)
    # Input data
    cfg_items.append('BASE_DATA_DIR = %s' % mpaths.base_dir)
    cfg_items.append('CLIMATE_DATA_DIR = %s' % mpaths.clim_dir)
    cfg_items.append('SPATIAL_DATA_DIR = %s' % mpaths.spatial_dir)
    cfg_items.append('MEASUREMENT_DATA_DIR = %s' % mpaths.observe_dir)
    cfg_items.append('BMP_DATA_DIR = %s' % mpaths.scenario_dir)
    cfg_items.append('MODEL_DIR = %s' % mpaths.model_dir)
    cfg_items.append('TXT_DB_DIR = %s' % mpaths.lookup_dir)
    # Output directory
    cfg_items.append('WORKING_DIR = %s' % mpaths.workspace)

    with open(pre_cfg_file, 'w') as f:
        for item in cfg_items:
            f.write(item + '\n')

    cf = ConfigParser()
    cf.read(pre_cfg_file)
    return SEIMSConfig(cf)


def write_postprocess_config_file(mpaths, org_file_name, sceid=0):
    org_cfg_file = mpaths.cfg_dir + os.sep + org_file_name
    post_cfg_file = mpaths.workspace + os.sep + org_file_name
    cfg_items = list()
    with open(org_cfg_file, 'r') as f:
        for line in f.readlines():
            cfg_items.append(line.strip())
    # print cfg_items
    cfg_items.append('[PATH]')
    cfg_items.append('MODEL_DIR = %s' % mpaths.model_dir)
    cfg_items.append('ScenarioID = %d' % sceid)

    with open(post_cfg_file, 'w') as f:
        for item in cfg_items:
            f.write(item + '\n')

    cf = ConfigParser()
    cf.read(post_cfg_file)
    return PostConfig(cf)


def main():
    """FUNCTION TESTS"""
    cur_path = UtilClass.current_path()
    SEIMS_path = os.path.abspath(cur_path + '../../..')
    model_paths = ModelPaths(SEIMS_path, 'dianbu2', 'model_dianbu2_30m_demo')
    prep_cfg = write_preprocess_config_file(model_paths, 'preprocess_30m_omp.ini')
    postp_cfg = write_postprocess_config_file(model_paths, 'postprocess_30m_omp.ini')


if __name__ == "__main__":
    main()
