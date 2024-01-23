# coding:utf-8
"""Base class of calibration.

    @author   : Liangjun Zhu

    @changelog:
    - 18-01-22  - lj - design and implement.
    - 18-01-25  - lj - redesign the individual class, add 95PPU, etc.
    - 18-02-09  - lj - compatible with Python3.
    - 20-07-22  - lj - update to use global MongoClient object.
"""
from __future__ import absolute_import, unicode_literals

import logging
import os
import sys
from collections import OrderedDict
from copy import deepcopy

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from typing import Optional
from pymongo import MongoClient

from utility import read_data_items_from_txt_with_subbasin_id
import global_mongoclient as MongoDBObj
from preprocess.text import DBTableNames, ModelParamFields
from run_seims import MainSEIMS
from calibration.config import CaliConfig, get_optimization_config
from calibration.sample_lhs import lhs


class TimeseriesData(object):
    """Time series data, for observation and simulation data."""

    def __init__(self):
        self.vars = list()
        self.data = OrderedDict()


class ObsSimData(object):
    """Paired time series data of observation and simulation, associated with statistics."""

    def __init__(self):
        self.vars = list()
        self.data = OrderedDict()
        self.sim_obs_data = OrderedDict()
        self.objnames = list()
        self.objvalues = list()
        self.valid = False

    def efficiency_values(self, varname, effnames):
        values = list()
        tmpvars = list()
        for name in effnames:
            tmpvar = '%s-%s' % (varname, name)
            if tmpvar not in self.objnames:
                values.append(-9999.)
            else:
                if name.upper() == 'PBIAS':
                    tmpvars.append('%s-abs(PBIAS)' % varname)
                else:
                    tmpvars.append(tmpvar)
                values.append(self.objvalues[self.objnames.index(tmpvar)])
        return values, tmpvars

    def output_header(self, varname, effnames, prefix=''):
        concate = ''
        for name in effnames:
            tmpvar = '%s-%s' % (varname, name)
            if tmpvar not in self.objnames:
                concate += '\t'
            else:
                if name.upper() == 'PBIAS':
                    tmpvar = '%s-abs(PBIAS)' % varname
                if prefix != '':
                    concate += '%s-%s\t' % (prefix, tmpvar)
                else:
                    concate += '%s\t' % tmpvar
        return concate

    def output_efficiency(self, varname, effnames):
        concate = ''
        for name in effnames:
            tmpvar = '%s-%s' % (varname, name)
            if tmpvar not in self.objnames:
                concate += '\t'
            else:
                concate += '%.3f\t' % self.objvalues[self.objnames.index(tmpvar)]
        return concate


class Calibration(object):
    """Base class of automatic calibration.

    Attributes:
        ID(integer): Calibration ID in current generation, range from 0 to N-1(individuals).
        modelrun(boolean): Has SEIMS model run successfully?
    """

    def __init__(self, cali_cfg, id=-1):
        # type: (CaliConfig, Optional[int]) -> None
        """Initialize.
        self.param_defs
        {
            param_name : [
                {
                    subbasins: [1,2,3]
                    bounds: [0,1]
                },
                {
                    subbasins: [0]
                    bounds: [0,1]
                }
            ],
        }
        """
        self.cfg = cali_cfg
        self.model = cali_cfg.model
        self.ID = id
        self.param_defs = dict()
        # run seims related
        self.modelrun = False
        self.reset_simulation_timerange()
        self.init_param_defs()

    def init_param_defs(self):
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
        # read param_range_def file and output to json file
        conn = MongoDBObj.client  # type: MongoClient
        db = conn[self.cfg.model.db_name]
        collection = db['PARAMETERS']

        if not self.cfg.param_range_defs:
            assert ValueError('Parameters definition file does not exist!'
                              ' They should have been read and check when reading cfg!')

        for cali_range_file in self.cfg.param_range_defs:
            subbasin_ids, items = read_data_items_from_txt_with_subbasin_id(cali_range_file)
            for item in items:
                if len(item) < 3:
                    continue
                name = item[0]
                low, up = float(item[1]), float(item[2])
                if collection.count_documents({'NAME': name}) <= 0:
                    logging.warning('parameter %s does not exist!' % name)
                    continue
                if low > up:
                    raise ValueError('Lower bound of parameter %s is larger than upper bound!' % name)
                if name not in self.param_defs:
                    self.param_defs[name] = list()
                dict_tmp = dict()
                dict_tmp[ModelParamFields.subbasins] = subbasin_ids
                dict_tmp[ModelParamFields.bounds] = [low, up]
                self.param_defs[name].append(dict_tmp)

    def reset_simulation_timerange(self):
        """Update simulation time range in MongoDB [FILE_IN]."""
        conn = MongoDBObj.client  # type: MongoClient
        db = conn[self.cfg.model.db_name]
        stime_str = self.cfg.model.simu_stime.strftime('%Y-%m-%d %H:%M:%S')
        etime_str = self.cfg.model.simu_etime.strftime('%Y-%m-%d %H:%M:%S')
        db[DBTableNames.main_filein].find_one_and_update({'TAG': 'STARTTIME'},
                                                         {'$set': {'VALUE': stime_str}})
        db[DBTableNames.main_filein].find_one_and_update({'TAG': 'ENDTIME'},
                                                         {'$set': {'VALUE': etime_str}})

    def init_param_values(self, n=1):
        """Initialize parameters samples by Latin-Hypercube sampling method.

        return: param_values
        """
        param_num = 0
        for name, subbasin_range_list in self.param_defs.items():
            param_num += len(subbasin_range_list)

        lhs_samples = lhs(param_num, n)
        param_values = deepcopy(self.param_defs)
        iter_count = 0
        for name in sorted(param_values.keys()):
            for i, rng_dict in enumerate(param_values[name]):
                low, up = rng_dict['bounds']
                gene_values = [(lhs_samples[iter_count][j] * (up - low) + low) for j in range(len(lhs_samples[iter_count]))]
                param_values[name][i]['values'] = gene_values
            iter_count += 1
            if iter_count >= n:
                break

        return param_values


def initialize_calibrations(cf):
    """Initial individual of population.
    """
    cali = Calibration(cf)
    return cali.param_defs


def calibration_objectives(pop):
    """Evaluate the objectives of given individual.
    pop -> (cali_obj, ind)
    """
    print(pop[0])
    print(pop[1])
    cali_obj = pop[0]
    ind = pop[1]
    logging.debug('Evaluate the objectives of individual %d' % ind.id)
    cali_obj.ID = ind.id
    model_args = cali_obj.model.ConfigDict
    model_args.setdefault('calibration_id', -1)
    model_args['calibration_id'] = ind.id
    model_obj = MainSEIMS(args_dict=model_args)

    # Set observation data to model_obj, no need to query database
    model_obj.SetOutletObservations(ind.obs.vars, ind.obs.data)

    # Execute model
    model_obj.SetMongoClient()
    model_obj.run()
    # time.sleep(0.1)  # Wait a moment in case of unpredictable file system error

    # read simulation data of the entire simulation period (include calibration and validation)
    if model_obj.ReadTimeseriesSimulations():
        ind.sim.vars = model_obj.sim_vars[:]
        ind.sim.data = deepcopy(model_obj.sim_value)
    else:
        model_obj.clean(calibration_id=ind.id)
        model_obj.UnsetMongoClient()
        return ind
    # Calculate NSE, R2, RMSE, PBIAS, and RSR, etc. of calibration period
    ind.cali.vars, ind.cali.data = model_obj.ExtractSimData(cali_obj.cfg.cali_stime,
                                                            cali_obj.cfg.cali_etime)
    ind.cali.sim_obs_data = model_obj.ExtractSimObsData(cali_obj.cfg.cali_stime,
                                                        cali_obj.cfg.cali_etime)

    ind.cali.objnames, \
        ind.cali.objvalues = model_obj.CalcTimeseriesStatistics(ind.cali.sim_obs_data,
                                                                cali_obj.cfg.cali_stime,
                                                                cali_obj.cfg.cali_etime)
    if ind.cali.objnames and ind.cali.objvalues:
        ind.cali.valid = True
    else:
        logging.error('Calibration period is not valid, please check the data.')

    # Calculate NSE, R2, RMSE, PBIAS, and RSR, etc. of validation period
    if cali_obj.cfg.calc_validation:
        ind.vali.vars, ind.vali.data = model_obj.ExtractSimData(cali_obj.cfg.vali_stime,
                                                                cali_obj.cfg.vali_etime)
        ind.vali.sim_obs_data = model_obj.ExtractSimObsData(cali_obj.cfg.vali_stime,
                                                            cali_obj.cfg.vali_etime)

        ind.vali.objnames, \
            ind.vali.objvalues = model_obj.CalcTimeseriesStatistics(ind.vali.sim_obs_data,
                                                                    cali_obj.cfg.vali_stime,
                                                                    cali_obj.cfg.vali_etime)
        if ind.vali.objnames and ind.vali.objvalues:
            ind.vali.valid = True
        else:
            logging.error('Validation period is not valid, please check the data.')
    # Get timespan
    ind.io_time, ind.comp_time, ind.simu_time, ind.runtime = model_obj.GetTimespan()

    # delete model output directory for saving storage
    model_obj.clean(calibration_id=ind.id)
    model_obj.UnsetMongoClient()
    logging.debug(ind)
    return ind


if __name__ == '__main__':
    cf, method = get_optimization_config()
    cfg = CaliConfig(cf, method=method)

    caliobj = Calibration(cfg)

    # test the picklable of Scenario class.
    import pickle

    s = pickle.dumps(caliobj)
    # print(s)
    new_cali = pickle.loads(s)
    print(new_cali.bin_dir)
