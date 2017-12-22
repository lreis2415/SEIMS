#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Evaluate SEIMS model for parameters sensitivity analysis.
    @author   : Liangjun Zhu
    @changelog: 17-12-22  lj - initial implementation.\n
"""
from run_seims import MainSEIMS


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
    model_obj.model_dir
