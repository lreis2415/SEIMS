#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""User defined evaluate_models variables for parameters sensitivity analysis.
    @author   : Liangjun Zhu
    @changelog: 17-12-22  lj - initial implementation.\n
"""
import shutil
import os
import datetime

import numpy
from pygeoc.raster import RasterUtilClass

from postprocess.utility import read_simulation_from_txt, match_simulation_observation, \
    calculate_statistics
from run_seims import MainSEIMS


def get_evaluate_output_name_unit():
    """User defined names and outputs of evaluated outputs"""
    output_name = ['meanQ', 'meanSED',
                   'NSE-Q', 'R2-Q', 'RMSE-Q', 'PBIAS-Q', 'RSR-Q',
                   'NSE-SED', 'R2-SED', 'RMSE-SED', 'PBIAS-SED', 'RSR-SED',
                   'meanSOER']
    output_unit = [' ($m^3/s$)', ' (kg)', '', '', '', '', '', '', '', '', '', '', ' (kg)']
    return output_name, output_unit


def evaluate_model_response(modelcfg_dict, cali_idx):
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
    if not run_flag:
        return None
    output_variables = list()
    obs_vars = ['Q', 'SED']
    tif_name = 'SOER_SUM.tif'
    # 1. read observation data from MongoDB
    obs_vars, obs_data_dict = model_obj.ReadOutletObservations(obs_vars)
    # 2. read simulation data
    sim_vars, sim_data_dict = read_simulation_from_txt(model_obj.output_dir,
                                                       obs_vars, model_obj.outlet_id,
                                                       model_obj.start_time,
                                                       model_obj.end_time)
    # calculate simulated mean Q and SED
    sim_data = numpy.array(sim_data_dict.values())
    output_variables.append(numpy.average(sim_data[:, 0]))  # meanQ
    output_variables.append(numpy.average(sim_data[:, 1]))  # meanSED
    # 3. Match with observation data
    sim_obs_dict = match_simulation_observation(sim_vars, sim_data_dict,
                                                obs_vars, obs_data_dict)
    # 4. Calculate NSE, R2, RMSE, PBIAS, and RSR
    calculate_statistics(sim_obs_dict)
    output_variables.append(sim_obs_dict['Q']['NSE'])
    output_variables.append(sim_obs_dict['Q']['R-square'])
    output_variables.append(sim_obs_dict['Q']['RMSE'])
    output_variables.append(sim_obs_dict['Q']['PBIAS'])
    output_variables.append(sim_obs_dict['Q']['RSR'])
    output_variables.append(sim_obs_dict['SED']['NSE'])
    output_variables.append(sim_obs_dict['SED']['R-square'])
    output_variables.append(sim_obs_dict['SED']['RMSE'])
    output_variables.append(sim_obs_dict['SED']['PBIAS'])
    output_variables.append(sim_obs_dict['SED']['RSR'])
    # 5. Calculate average soil erosion (kg/day)
    rfile = model_obj.output_dir + os.sep + tif_name
    rr = RasterUtilClass.read_raster(rfile)
    timespan_days = (model_obj.end_time + datetime.timedelta(seconds=1) - model_obj.start_time).days
    ave_soer = rr.get_sum() / timespan_days  # unit: day
    output_variables.append(ave_soer)

    # delete model output directory for saving storage
    shutil.rmtree(model_obj.output_dir)
    return output_variables


def main():
    """MAIN FUNCTION."""
    from run_seims import MainSEIMS

    bindir = r'D:\compile\bin\seims'
    modeldir = r'C:\z_data\ChangTing\seims_models_phd\youwuzhen10m_longterm_model'
    seimsobj = MainSEIMS(bindir, modeldir,
                         nthread=2, lyrmtd=1,
                         ip='127.0.0.1', port=27017,
                         sceid=0, caliid=0)
    print evaluate_model_response(seimsobj)


if __name__ == '__main__':
    main()
