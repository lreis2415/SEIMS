#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base class of calibration.
    @author   : Liangjun Zhu
    @changelog: 18-01-22  lj - design and implement.\n
"""
import shutil

from pygeoc.utils import FileClass

from config import CaliConfig, get_cali_config
from postprocess.utility import read_simulation_from_txt, match_simulation_observation, \
    calculate_statistics
from preprocess.db_mongodb import ConnectMongoDB
from preprocess.text import DBTableNames
from preprocess.utility import read_data_items_from_txt
from run_seims import MainSEIMS
from sample_lhs import lhs


class Calibration(object):
    """Base class of automatic calibration.

    Attributes:
        ID(integer): Calibration ID in current generation, range from 0 to N-1(individuals).
        modelrun(boolean): Has SEIMS model run successfully?
    """

    def __init__(self, cali_cfg, id=-1):
        """Initialize."""
        self.cfg = cali_cfg
        self.ID = id
        self.param_defs = dict()
        # run seims related
        self.modelrun = False
        self.reset_simulation_timerange()

    @property
    def ParamDefs(self):
        """Read cali_param_rng.def file

           name,lower_bound,upper_bound

            e.g.,
             Param1,0,1
             Param2,0.5,1.2
             Param3,-1.0,1.0

        Returns:
            a dictionary containing:
            - names - the names of the parameters
            - bounds - a list of lists of lower and upper bounds
            - num_vars - a scalar indicating the number of variables
                         (the length of names)
        """
        # read param_defs.json if already existed
        if self.param_defs:
            return self.param_defs
        # read param_range_def file and output to json file
        client = ConnectMongoDB(self.cfg.hostname, self.cfg.port)
        conn = client.get_conn()
        db = conn[self.cfg.spatial_db]
        collection = db['PARAMETERS']

        names = list()
        bounds = list()
        num_vars = 0
        if not FileClass.is_file_exists(self.cfg.param_range_def):
            raise ValueError('Parameters definition file: %s is not'
                             ' existed!' % self.cfg.param_range_def)
        items = read_data_items_from_txt(self.cfg.param_range_def)
        for item in items:
            if len(item) < 3:
                continue
            # find parameter name, print warning message if not existed
            cursor = collection.find({'NAME': item[0]}, no_cursor_timeout=True)
            if not cursor.count():
                print ('WARNING: parameter %s is not existed!' % item[0])
                continue
            num_vars += 1
            names.append(item[0])
            bounds.append([float(item[1]), float(item[2])])
        self.param_defs = {'names': names, 'bounds': bounds, 'num_vars': num_vars}
        return self.param_defs

    def reset_simulation_timerange(self):
        """Read simulation time range from MongoDB."""
        client = ConnectMongoDB(self.cfg.hostname, self.cfg.port)
        conn = client.get_conn()
        db = conn[self.cfg.spatial_db]
        stime_str = self.cfg.time_start.strftime('%Y-%m-%d %H:%M:%S')
        etime_str = self.cfg.time_end.strftime('%Y-%m-%d %H:%M:%S')
        db[DBTableNames.main_filein].find_one_and_update({'TAG': 'STARTTIME'},
                                                         {'$set': {'VALUE': stime_str}})
        db[DBTableNames.main_filein].find_one_and_update({'TAG': 'ENDTIME'},
                                                         {'$set': {'VALUE': etime_str}})
        client.close()

    def initialize(self):
        """Initialize parameters samples by Latin-Hypercube sampling method.

        Returns:
            A list contains parameter value at each gene location.
        """
        param_num = self.ParamDefs['num_vars']
        lhs_samples = lhs(param_num, 1)[0]
        gene_values = list()
        for i, param_bound in enumerate(self.ParamDefs['bounds']):
            gene_values.append(lhs_samples[i] * (param_bound[1] - param_bound[0]) + param_bound[0])
        return gene_values


def initialize_calibrations(cf):
    """Initial individual of population.
    """
    cali = Calibration(cf)
    return cali.initialize()


def calibration_objectives(cali_obj, ind):
    """Evaluate the objectives of given individual.
    """
    cali_obj.ID = ind.id
    model_obj = MainSEIMS(cali_obj.cfg.bin_dir, cali_obj.cfg.model_dir,
                          nthread=cali_obj.cfg.nthread, lyrmtd=cali_obj.cfg.lyrmethod,
                          ip=cali_obj.cfg.hostname, port=cali_obj.cfg.port,
                          sceid=cali_obj.cfg.sceid, caliid=ind.id)
    run_flag = model_obj.run()
    if not run_flag:  # return all outputs to be -9999.
        return [-9999.] * 10
    output_variables = list()
    obs_vars = ['Q', 'SED']
    # 1. read observation data from MongoDB
    obs_vars, obs_data_dict = model_obj.ReadOutletObservations(obs_vars)
    # 2. read simulation data
    sim_vars, sim_data_dict = read_simulation_from_txt(model_obj.output_dir,
                                                       obs_vars, model_obj.outlet_id,
                                                       model_obj.start_time,
                                                       model_obj.end_time)
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

    # delete model output directory for saving storage
    shutil.rmtree(model_obj.output_dir)
    return output_variables


if __name__ == '__main__':
    cf, method = get_cali_config()
    cfg = CaliConfig(cf, method=method)

    caliobj = Calibration(cfg)

    # test the picklable of Scenario class.
    import pickle

    s = pickle.dumps(caliobj)
    # print (s)
    new_cali = pickle.loads(s)
    print (new_cali.bin_dir)
