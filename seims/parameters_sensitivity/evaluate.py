#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Evaluate SEIMS model for parameters sensitivity analysis.
    @author   : Liangjun Zhu
    @changelog: 17-12-22  lj - initial implementation.\n
"""
import os
import shutil

from run_seims import MainSEIMS
from preprocess.utility import sum_outlet_output


def build_seims_model(modelcfg_dict, cali_idx):
    """Build SEIMS model."""
    return MainSEIMS(modelcfg_dict['bin_dir'], modelcfg_dict['model_dir'],
                     nthread=modelcfg_dict['nthread'], lyrmtd=modelcfg_dict['lyrmethod'],
                     ip=modelcfg_dict['hostname'], port=modelcfg_dict['port'],
                     sceid=modelcfg_dict['scenario_id'], caliid=cali_idx)


def evaluate_model_response(model_obj):
    """Run SEIMS model, calculate and return the desired output variables."""
    run_flag = model_obj.run()
    if not run_flag:
        return None
    output_variables = list()
    # 1. Total flow discharge (m3/s)
    qfile = model_obj.output_dir + os.sep + 'Q.txt'
    qsum = sum_outlet_output(qfile)
    output_variables.append(qsum)
    # 2. Total sediment output (t)
    sedfile = model_obj.output_dir + os.sep + 'SED.txt'
    sedsum = sum_outlet_output(sedfile)
    sedsum /= 1000000.
    output_variables.append(sedsum)

    # delete model output directory for saving storage
    shutil.rmtree(model_obj.output_dir)
    return output_variables
