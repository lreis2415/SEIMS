#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""User defined evaluate_models variables for parameters sensitivity analysis.
    @author   : Liangjun Zhu
    @changelog: 17-12-22  lj - initial implementation.\n
                18-02-09  lj - compatible with Python3.\n
"""
from __future__ import absolute_import

import shutil
import os
import datetime
import time

import numpy
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import StringClass

from postprocess.utility import read_simulation_from_txt, match_simulation_observation, \
    calculate_statistics
from run_seims import MainSEIMS


def get_evaluate_output_name_unit():
    """User defined names and outputs of evaluated outputs"""
    output_name = ['meanQ', 'meanSED',
                   'NSE-Q', 'R2-Q', 'RMSE-Q', 'PBIAS-Q', 'RSR-Q', 'lnNSE-Q', 'NSE1-Q', 'NSE3-Q',
                   'NSE-SED', 'R2-SED', 'RMSE-SED', 'PBIAS-SED', 'RSR-SED',
                   'lnNSE-SED', 'NSE1-SED', 'NSE3-SED',
                   'PDIFF-Q', 'PDIFF-SED',
                   'meanSOER']
    output_unit = [' ($m^3/s$)', ' (kg)',
                   '', '', '', '', '', '', '', '',
                   '', '', '', '', '', '', '', '',
                   '', '', ' (kg)']
    return output_name, output_unit


def evaluate_model_response(modelcfg_dict, cali_idx, period):
    """Run SEIMS model, calculate and return the desired output variables.
    See Also:
        get_evaluate_output_name_unit
    Args:
        model_obj: MainSEIMS object
    """
    model_obj = MainSEIMS(modelcfg_dict['bin_dir'], modelcfg_dict['model_dir'],
                          nthread=modelcfg_dict['nthread'], lyrmtd=modelcfg_dict['lyrmethod'],
                          ip=modelcfg_dict['hostname'], port=modelcfg_dict['port'],
                          sceid=modelcfg_dict['scenario_id'], caliid=cali_idx)
    run_flag = model_obj.run()
    if not run_flag:  # return all outputs to be -9999.
        return [-9999.] * 21
    time.sleep(0.5)
    output_variables = list()
    obs_vars = ['Q', 'SED']
    tif_name = 'SOER_SUM.tif'
    # 1. read observation data from MongoDB
    obs_vars, obs_data_dict = model_obj.ReadOutletObservations(obs_vars)
    # 2. read simulation data
    dates = period.split(',')
    stime = StringClass.get_datetime(dates[0], '%Y-%m-%d %H:%M:%S')
    etime = StringClass.get_datetime(dates[1], '%Y-%m-%d %H:%M:%S')
    sim_vars, sim_data_dict = read_simulation_from_txt(model_obj.output_dir,
                                                       obs_vars, model_obj.outlet_id,
                                                       stime, etime)
    # calculate simulated mean Q and SED
    sim_data = numpy.array(sim_data_dict.values())
    output_variables.append(numpy.average(sim_data[:, 0]))  # meanQ
    output_variables.append(numpy.average(sim_data[:, 1]))  # meanSED
    # 3. Match with observation data
    sim_obs_dict = match_simulation_observation(sim_vars, sim_data_dict,
                                                obs_vars, obs_data_dict)
    # 4. Calculate NSE, R2, RMSE, PBIAS, RSR, lnNSE, NSE1, and NSE3
    calculate_statistics(sim_obs_dict)
    output_variables.append(sim_obs_dict['Q']['NSE'])
    output_variables.append(sim_obs_dict['Q']['R-square'])
    output_variables.append(sim_obs_dict['Q']['RMSE'])
    output_variables.append(sim_obs_dict['Q']['PBIAS'])
    output_variables.append(sim_obs_dict['Q']['RSR'])
    output_variables.append(sim_obs_dict['Q']['lnNSE'])
    output_variables.append(sim_obs_dict['Q']['NSE1'])
    output_variables.append(sim_obs_dict['Q']['NSE3'])
    output_variables.append(sim_obs_dict['SED']['NSE'])
    output_variables.append(sim_obs_dict['SED']['R-square'])
    output_variables.append(sim_obs_dict['SED']['RMSE'])
    output_variables.append(sim_obs_dict['SED']['PBIAS'])
    output_variables.append(sim_obs_dict['SED']['RSR'])
    output_variables.append(sim_obs_dict['SED']['lnNSE'])
    output_variables.append(sim_obs_dict['SED']['NSE1'])
    output_variables.append(sim_obs_dict['SED']['NSE3'])
    # Added 2018-1-27 calculate PDIFF = (ObsPeak - SimPeak)/ObsPeak
    obsmax = max(sim_obs_dict['Q']['Obs'])
    simmax = max(sim_obs_dict['Q']['Sim'])
    output_variables.append((obsmax - simmax) / obsmax)
    obsmax = max(sim_obs_dict['SED']['Obs'])
    simmax = max(sim_obs_dict['SED']['Sim'])
    output_variables.append((obsmax - simmax) / obsmax)
    # 5. Calculate average soil erosion (kg/day)
    rfile = model_obj.output_dir + os.sep + tif_name
    rr = RasterUtilClass.read_raster(rfile)
    timespan_days = (etime + datetime.timedelta(seconds=1) - stime).days
    ave_soer = rr.get_sum() / timespan_days  # unit: day
    output_variables.append(ave_soer)

    # delete model output directory for saving storage
    shutil.rmtree(model_obj.output_dir)
    return output_variables


def main():
    """MAIN FUNCTION."""
    bindir = r'D:\compile\bin\seims'
    modeldir = r'C:\z_data\ChangTing\seims_models_phd\youwuzhen10m_longterm_model'
    model_cfg_dict = {'bin_dir': bindir, 'model_dir': modeldir,
                      'nthread': 4, 'lyrmethod': 1,
                      'hostname': '127.0.0.1', 'port': 27017,
                      'scenario_id': 0}
    print(evaluate_model_response(model_cfg_dict, 0, '2012-12-01,2013-02-11'))


if __name__ == '__main__':
    main()
