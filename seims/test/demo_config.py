# -*- coding: utf-8 -*-
"""Generate configuration files of SEIMS-based watershed model.

The invoking format is:

  `python demo_config.py -name <demo_watershed_name>`

    @author   : Liangjun Zhu

    @changelog:
    - 18-02-09 - lj - compatible with Python3.
    - 19-01-07 - lj - add configuration settings of sensitivity analysis, calibration,
                        and scenario analysis
    - 25-09-09 - lj - redesign model folder's structure
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
from pygeoc.utils import UtilClass, FileClass

from preprocess.config import PreprocessConfig
from run_seims import ParseSEIMSConfig
from postprocess.config import PostConfig
from parameters_sensitivity.config import PSAConfig
from calibration.config import CaliConfig

DEMO_MODELS = {'youwuzhen':  # data folder name
                   {'demo_youwuzhen30m_model':  # modelname
                        {'preprocessini': 'preprocess.ini',  # preprocess ini, for all sub-models
                         'confignames': [  # each model can have multiple configName, aka sub-model!
                             'daily',
                             'storm'],
                         'bin_dir': None
                         }
                    }
               }


def get_watershed_name_info(desc='Specify watershed name, modelname, and configname'
                                 ' to run this script.'):
    # type: (AnyStr) -> (ConfigParser, AnyStr)
    """Parse arguments.
    Returns:
        name: Watershed name, 'youwuzhen' by default.
        modelname: Watershed model name, 'demo_youwuzhen30m_model' by default.
        configname: Model config name, 'daily' by default.
    """
    # define input arguments
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument('-name', type=str, help='Name of demo watershed')
    parser.add_argument('-model', type=str, help='Model name of demo watershed')
    parser.add_argument('-config', type=str,
                        help='Config model name of demo watershed')
    parser.add_argument('-bin_dir', type=str, help='executable path')
    # parse arguments
    args = parser.parse_args()
    datadirname = args.name
    modelname = args.model
    configname = args.config
    bindir = args.bin_dir
    if datadirname is None:
        datadirname = 'youwuzhen'  # default
    if modelname is None:
        modelname = 'demo_youwuzhen30m_model'
    if configname is None:
        configname = 'daily'
    return datadirname, modelname, configname, bindir


class ModelPaths(object):
    """Paths required for SEIMS model setting.

    Args:
        bpath: Base path of SEIMS.
        data_dir_name: e.g., youwuzhen
        model_dir_name: e.g., demo_youwuzhen30m_model
    """

    def __init__(self, bpath, data_dir_name, model_dir_name, bin_dir=None):
        self.mpi_bin = None
        self.bin_dir = bin_dir
        if bin_dir is None:
            self.bin_dir = bpath + os.path.sep + 'build' + os.path.sep + 'bin'
        self.prescript_dir = bpath + os.path.sep + 'seims' + os.path.sep + 'preprocess'
        self.base_dir = bpath + os.path.sep + 'data' + os.path.sep + data_dir_name
        self.model_dir = self.base_dir + os.path.sep + model_dir_name
        self.data_dir = self.base_dir + os.path.sep + 'data_prepare'
        self.clim_dir = self.data_dir + os.path.sep + 'climate'
        self.spatial_dir = self.data_dir + os.path.sep + 'spatial'
        self.observe_dir = self.data_dir + os.path.sep + 'observed'
        self.scenario_dir = self.data_dir + os.path.sep + 'scenario'
        self.lookup_dir = self.data_dir + os.path.sep + 'lookup'
        self.cfg_dir = self.model_dir + os.path.sep + 'model_configs'
        self.workspace = self.model_dir + os.path.sep + 'workspace'
        UtilClass.mkdir(self.workspace)
        print('SEIMS binary location: %s' % self.bin_dir)
        print('Demo data location: %s' % self.data_dir)
        print('Data preprocessing location: %s' % self.workspace)


def write_preprocess_config_file(mpaths, org_file_name):
    org_cfg_file = '%s/%s' % (mpaths.cfg_dir, org_file_name)
    pre_cfg_file = '%s/%s' % (mpaths.workspace, org_file_name)
    if not FileClass.is_file_exists(org_cfg_file):
        print('%s is not existed!' % org_cfg_file)
        return None
    dst_dir = os.path.dirname(pre_cfg_file)
    if not FileClass.is_dir_exists(dst_dir):
        UtilClass.mkdir(dst_dir)
    cfg_items = list()
    with open(org_cfg_file, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            cfg_items.append(line.strip())
    cfg_items.append('[PATH]')
    cfg_items.append('PREPROC_SCRIPT_DIR = %s' % mpaths.prescript_dir)
    cfg_items.append('CPP_PROGRAM_DIR = %s' % mpaths.bin_dir)
    # cfg_items.append('MPIEXEC_DIR = %s' % mpaths.mpi_bin)
    # Input data
    cfg_items.append('BASE_DATA_DIR = %s' % mpaths.data_dir)
    #cfg_items.append('CLIMATE_DATA_DIR = %s' % mpaths.clim_dir)
    #cfg_items.append('SPATIAL_DATA_DIR = %s' % mpaths.spatial_dir)
    #cfg_items.append('MEASUREMENT_DATA_DIR = %s' % mpaths.observe_dir)
    #cfg_items.append('BMP_DATA_DIR = %s' % mpaths.scenario_dir)
    cfg_items.append('MODEL_DIR = %s' % mpaths.model_dir)
    #cfg_items.append('TXT_DB_DIR = %s' % mpaths.lookup_dir)
    # Output directory
    cfg_items.append('WORKING_DIR = %s' % mpaths.workspace)

    with open(pre_cfg_file, 'w', encoding='utf-8') as f:
        for item in cfg_items:
            f.write('%s\n' % item)

    cf = ConfigParser()
    cf.read(pre_cfg_file)
    return PreprocessConfig(cf)


def write_runmodel_config_file(mpaths, org_file_name, config_name=[]):
    org_cfg_file = '%s/%s' % (mpaths.cfg_dir, org_file_name)
    runmodel_cfg_file = '%s/%s' % (mpaths.workspace, org_file_name)
    if len(config_name) > 0:
        for cname in config_name:
            org_cfg_file = '%s/%s/%s' % (mpaths.cfg_dir, cname, org_file_name)
            runmodel_cfg_file = '%s/%s/%s' % (mpaths.workspace, cname, org_file_name)
            write_single_runmodel_config_file(mpaths, org_cfg_file, runmodel_cfg_file)
    else:
        write_single_runmodel_config_file(mpaths, org_cfg_file, runmodel_cfg_file)


def write_single_runmodel_config_file(mpaths, org_cfg_file, runmodel_cfg_file):
    if not FileClass.is_file_exists(org_cfg_file):
        print('%s is not existed!' % org_cfg_file)
        return None
    dst_dir = os.path.dirname(runmodel_cfg_file)
    if not FileClass.is_dir_exists(dst_dir):
        UtilClass.mkdir(dst_dir)
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


def write_postprocess_config_file(mpaths, org_file_name, config_name=[], sceid=0, caliid=-1):
    org_cfg_file = '%s/%s' % (mpaths.cfg_dir, org_file_name)
    post_cfg_file = '%s/%s' % (mpaths.workspace, org_file_name)
    if len(config_name) > 0:
        for cname in config_name:
            org_cfg_file = '%s/%s/%s' % (mpaths.cfg_dir, cname, org_file_name)
            post_cfg_file = '%s/%s/%s' % (mpaths.workspace, cname, org_file_name)
            write_single_postprocess_config_file(mpaths, org_cfg_file, post_cfg_file, sceid, caliid)
    else:
        write_single_postprocess_config_file(mpaths, org_cfg_file, post_cfg_file, sceid, caliid)


def write_single_postprocess_config_file(mpaths, org_cfg_file, post_cfg_file,
                                         sceid=0, caliid=-1):
    if not FileClass.is_file_exists(org_cfg_file):
        print('%s is not existed!' % org_cfg_file)
        return None
    dst_dir = os.path.dirname(post_cfg_file)
    if not FileClass.is_dir_exists(dst_dir):
        UtilClass.mkdir(dst_dir)
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


def write_sensitivity_config_file(mpaths, org_file_name, config_name=[]):
    org_cfg_file = '%s/%s' % (mpaths.cfg_dir, org_file_name)
    psa_cfg_file = '%s/%s' % (mpaths.workspace, org_file_name)
    if len(config_name) > 0:
        for cname in config_name:
            org_cfg_file = '%s/%s/%s' % (mpaths.cfg_dir, cname, org_file_name)
            psa_cfg_file = '%s/%s/%s' % (mpaths.workspace, cname, org_file_name)
            write_single_sensitivity_config_file(mpaths, org_cfg_file, psa_cfg_file)
    else:
        write_single_sensitivity_config_file(mpaths, org_cfg_file, psa_cfg_file)


def write_single_sensitivity_config_file(mpaths, org_cfg_file, psa_cfg_file):
    if not FileClass.is_file_exists(org_cfg_file):
        print('%s is not existed!' % org_cfg_file)
        return None
    dst_dir = os.path.dirname(psa_cfg_file)
    if not FileClass.is_dir_exists(dst_dir):
        UtilClass.mkdir(dst_dir)
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


def write_calibration_config_file(mpaths, org_file_name, config_name=[]):
    org_cfg_file = '%s/%s' % (mpaths.cfg_dir, org_file_name)
    cali_cfg_file = '%s/%s' % (mpaths.workspace, org_file_name)
    if len(config_name) > 0:
        for cname in config_name:
            org_cfg_file = '%s/%s/%s' % (mpaths.cfg_dir, cname, org_file_name)
            cali_cfg_file = '%s/%s/%s' % (mpaths.workspace, cname, org_file_name)
            write_single_calibration_config_file(mpaths, org_cfg_file, cali_cfg_file)
    else:
        write_single_calibration_config_file(mpaths, org_cfg_file, cali_cfg_file)


def write_single_calibration_config_file(mpaths, org_cfg_file, cali_cfg_file):
    if not FileClass.is_file_exists(org_cfg_file):
        print('%s is not existed!' % org_cfg_file)
        return None
    dst_dir = os.path.dirname(cali_cfg_file)
    if not FileClass.is_dir_exists(dst_dir):
        UtilClass.mkdir(dst_dir)
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


def write_scenario_analysis_config_file(mpaths, org_file_name, config_name=[]):
    org_cfg_file = '%s/%s' % (mpaths.cfg_dir, org_file_name)
    sa_cfg_file = '%s/%s' % (mpaths.workspace, org_file_name)
    if len(config_name) > 0:
        for cname in config_name:
            org_cfg_file = '%s/%s/%s' % (mpaths.cfg_dir, cname, org_file_name)
            sa_cfg_file = '%s/%s/%s' % (mpaths.workspace, cname, org_file_name)
            write_single_scenario_analysis_config_file(mpaths, org_cfg_file, sa_cfg_file)
    else:
        write_single_scenario_analysis_config_file(mpaths, org_cfg_file, sa_cfg_file)


def write_single_scenario_analysis_config_file(mpaths, org_cfg_file, sa_cfg_file):
    if not FileClass.is_file_exists(org_cfg_file):
        print('%s is not existed!' % org_cfg_file)
        return None
    dst_dir = os.path.dirname(sa_cfg_file)
    if not FileClass.is_dir_exists(dst_dir):
        UtilClass.mkdir(dst_dir)
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
    for wtsd_name, model_dict in list(DEMO_MODELS.items()):
        for model_name, model_dict in list(model_dict.items()):
            bin_dir = model_dict.get('bin_dir', None)
            model_paths = ModelPaths(SEIMS_path, wtsd_name, model_name, bin_dir=bin_dir)
            if 'preprocessini' not in list(model_dict.keys()):
                print('The key preprocessini MUST be specified for each model!')
                continue
            if 'confignames' not in list(model_dict.keys()):
                print('The key confignames MUST be specified for each model!')
                continue
            write_preprocess_config_file(model_paths, model_dict['preprocessini'])
            write_runmodel_config_file(model_paths, 'runmodel.ini',
                                       model_dict['confignames'])
            write_postprocess_config_file(model_paths, 'postprocess.ini',
                                          model_dict['confignames'])
            write_sensitivity_config_file(model_paths, 'sensitivity_analysis.ini',
                                          model_dict['confignames'])
            write_calibration_config_file(model_paths, 'calibration.ini',
                                          model_dict['confignames'])
            write_scenario_analysis_config_file(model_paths, 'scenario_analysis.ini',
                                                model_dict['confignames'])


if __name__ == "__main__":
    main()
