#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Generate configuration files of SEIMS-based watershed model.

The invoking format is:

  `python demo_config.py -name <demo_watershed_name>`

    @author   : Liangjun Zhu

    @changelog:
    - 18-02-09 - lj - compatible with Python3.
    - 19-01-07 - lj - add configuration settings of sensitivity analysis, calibration,
                        and scenario analysis
"""
from __future__ import absolute_import, unicode_literals

import argparse
from configparser import ConfigParser
import os
import sys
from io import open

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from typing import AnyStr
from pygeoc.utils import UtilClass

from preprocess.config import PreprocessConfig
from run_seims import ParseSEIMSConfig
from postprocess.config import PostConfig
from parameters_sensitivity.config import PSAConfig
from calibration.config import CaliConfig

DEMO_MODELS = {'youwuzhen': 'demo_youwuzhen30m_longterm_model'}


def get_watershed_name(desc='Specify watershed name to run this script.'):
    # type: (AnyStr) -> (ConfigParser, AnyStr)
    """Parse arguments.
    Returns:
        name: Watershed name, 'youwuzhen' by default.
    """
    # define input arguments
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument('-name', type=str, help='Name of demo watershed')
    # parse arguments
    args = parser.parse_args()
    if args.name is None:
        return 'youwuzhen'  # default
    return args.name


class ModelPaths(object):
    """Paths required for SEIMS model setting.

    Args:
        bpath: Base path of SEIMS.
        data_dir_name: e.g., youwuzhen
        model_dir_name: e.g., demo_youwuzhen30m_longterm_model
    """

    def __init__(self, bpath, data_dir_name, model_dir_name):
        self.mpi_bin = None
        self.bin_dir = bpath + os.path.sep + 'bin'
        self.prescript_dir = bpath + os.path.sep + 'seims' + os.path.sep + 'preprocess'
        self.base_dir = bpath + os.path.sep + 'data' + os.path.sep + data_dir_name
        self.cfg_dir = self.base_dir + os.path.sep + 'model_configs'
        self.model_dir = self.base_dir + os.path.sep + model_dir_name
        self.data_dir = self.base_dir + os.path.sep + 'data_prepare'
        self.clim_dir = self.data_dir + os.path.sep + 'climate'
        self.spatial_dir = self.data_dir + os.path.sep + 'spatial'
        self.observe_dir = self.data_dir + os.path.sep + 'observed'
        self.scenario_dir = self.data_dir + os.path.sep + 'scenario'
        self.lookup_dir = self.data_dir + os.path.sep + 'lookup'
        self.workspace = self.base_dir + os.path.sep + 'workspace'
        UtilClass.mkdir(self.workspace)
        print('SEIMS binary location: %s' % self.bin_dir)
        print('Demo data location: %s' % self.base_dir)
        print('Data preprocessing location: %s' % self.workspace)


def write_preprocess_config_file(mpaths, org_file_name):
    org_cfg_file = mpaths.cfg_dir + os.path.sep + org_file_name
    pre_cfg_file = mpaths.workspace + os.path.sep + org_file_name
    cfg_items = list()
    with open(org_cfg_file, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            cfg_items.append(line.strip())
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

    with open(pre_cfg_file, 'w', encoding='utf-8') as f:
        for item in cfg_items:
            f.write('%s\n' % item)

    cf = ConfigParser()
    cf.read(pre_cfg_file)
    return PreprocessConfig(cf)


def write_runmodel_config_file(mpaths, org_file_name):
    org_cfg_file = mpaths.cfg_dir + os.path.sep + org_file_name
    runmodel_cfg_file = mpaths.workspace + os.path.sep + org_file_name
    cfg_items = list()
    with open(org_cfg_file, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            cfg_items.append(line.strip())
    cfg_items.append('MODEL_DIR = %s' % mpaths.model_dir)
    cfg_items.append('BIN_DIR = %s' % mpaths.bin_dir)

    with open(runmodel_cfg_file, 'w', encoding='utf-8') as f:
        for item in cfg_items:
            f.write('%s\n' % item)

    cf = ConfigParser()
    cf.read(runmodel_cfg_file)
    return ParseSEIMSConfig(cf)


def write_postprocess_config_file(mpaths, org_file_name, sceid=0, caliid=-1):
    org_cfg_file = mpaths.cfg_dir + os.path.sep + org_file_name
    post_cfg_file = mpaths.workspace + os.path.sep + org_file_name
    cfg_items = list()
    with open(org_cfg_file, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            cfg_items.append(line.strip())
    cfg_items.append('MODEL_DIR = %s' % mpaths.model_dir)
    cfg_items.append('BIN_DIR = %s' % mpaths.bin_dir)
    cfg_items.append('scenarioID = %d' % sceid)
    cfg_items.append('calibrationID = %d' % caliid)

    with open(post_cfg_file, 'w', encoding='utf-8') as f:
        for item in cfg_items:
            f.write('%s\n' % item)

    cf = ConfigParser()
    cf.read(post_cfg_file)
    return PostConfig(cf)


def write_sensitivity_config_file(mpaths, org_file_name):
    org_cfg_file = mpaths.cfg_dir + os.path.sep + org_file_name
    psa_cfg_file = mpaths.workspace + os.path.sep + org_file_name
    cfg_items = list()
    with open(org_cfg_file, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            cfg_items.append(line.strip())
    cfg_items.append('MODEL_DIR = %s' % mpaths.model_dir)
    cfg_items.append('BIN_DIR = %s' % mpaths.bin_dir)

    with open(psa_cfg_file, 'w', encoding='utf-8') as f:
        for item in cfg_items:
            f.write('%s\n' % item)

    cf = ConfigParser()
    cf.read(psa_cfg_file)
    return PSAConfig(cf)


def write_calibration_config_file(mpaths, org_file_name):
    org_cfg_file = mpaths.cfg_dir + os.path.sep + org_file_name
    cali_cfg_file = mpaths.workspace + os.path.sep + org_file_name
    cfg_items = list()
    with open(org_cfg_file, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            cfg_items.append(line.strip())
    cfg_items.append('MODEL_DIR = %s' % mpaths.model_dir)
    cfg_items.append('BIN_DIR = %s' % mpaths.bin_dir)

    with open(cali_cfg_file, 'w', encoding='utf-8') as f:
        for item in cfg_items:
            f.write('%s\n' % item)

    cf = ConfigParser()
    cf.read(cali_cfg_file)
    return CaliConfig(cf)


def write_scenario_analysis_config_file(mpaths, org_file_name):
    org_cfg_file = mpaths.cfg_dir + os.path.sep + org_file_name
    sa_cfg_file = mpaths.workspace + os.path.sep + org_file_name
    cfg_items = list()
    with open(org_cfg_file, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            cfg_items.append(line.strip())
    cfg_items.append('MODEL_DIR = %s' % mpaths.model_dir)
    cfg_items.append('BIN_DIR = %s' % mpaths.bin_dir)

    with open(sa_cfg_file, 'w', encoding='utf-8') as f:
        for item in cfg_items:
            f.write('%s\n' % item)

    cf = ConfigParser()
    cf.read(sa_cfg_file)
    return cf


def main():
    """FUNCTION TESTS"""
    cur_path = UtilClass.current_path(lambda: 0)
    SEIMS_path = os.path.abspath(cur_path + '../../..')
    # More demo data could be added in the future.
    for wtsd_name, model_name in list(DEMO_MODELS.items()):
        model_paths = ModelPaths(SEIMS_path, wtsd_name, model_name)
        prep_cfg = write_preprocess_config_file(model_paths, 'preprocess.ini')
        runmodel_cfg = write_runmodel_config_file(model_paths, 'runmodel.ini')
        postp_cfg = write_postprocess_config_file(model_paths, 'postprocess.ini')
        psa_cfg = write_sensitivity_config_file(model_paths, 'sensitivity_analysis.ini')
        cali_cfg = write_calibration_config_file(model_paths, 'calibration.ini')


if __name__ == "__main__":
    main()
