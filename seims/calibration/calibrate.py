#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base class of calibration.
    @author   : Liangjun Zhu
    @changelog: 18-01-22  lj - design and implement.\n
                18-01-25  lj - redesign the individual class, add 95PPU, etc.\n
                18-02-09  lj - compatible with Python3.\n
"""
from __future__ import absolute_import

import time
import shutil
from collections import OrderedDict
import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.append(os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import FileClass

from postprocess.utility import read_simulation_from_txt, match_simulation_observation, \
    calculate_statistics
from preprocess.db_mongodb import ConnectMongoDB
from preprocess.text import DBTableNames
from preprocess.utility import read_data_items_from_txt
from run_seims import MainSEIMS
from calibration.config import CaliConfig, get_cali_config
from calibration.sample_lhs import lhs


class observationData(object):
    def __init__(self):
        self.vars = list()
        self.data = OrderedDict()


class simulationData(object):
    def __init__(self):
        self.vars = list()
        self.data = OrderedDict()
        self.sim_obs_data = OrderedDict()
        self.valid = False

    def efficiency_values(self, varname):
        if varname not in self.vars:
            return []
        if varname not in self.sim_obs_data:
            return []
        try:
            return [self.sim_obs_data[varname]['NSE'],
                    self.sim_obs_data[varname]['RSR'],
                    abs(self.sim_obs_data[varname]['PBIAS']) / 100.,
                    self.sim_obs_data[varname]['R-square']]
        except Exception:
            return []

    def output_header(self, varname, prefix=''):
        if varname not in self.vars:
            return ''
        return '%sNSE-%s\t%sRSR-%s\t%sPBIAS-%s\t%sR2-%s\t' % (prefix, varname,
                                                              prefix, varname,
                                                              prefix, varname,
                                                              prefix, varname,)

    def output_efficiency(self, varname):
        if varname not in self.vars:
            return ''
        if varname not in self.sim_obs_data:
            return ''
        try:
            return '%.3f\t%.3f\t%.3f\t%.3f\t' % (self.sim_obs_data[varname]['NSE'],
                                                 self.sim_obs_data[varname]['RSR'],
                                                 self.sim_obs_data[varname]['PBIAS'],
                                                 self.sim_obs_data[varname]['R-square'])
        except Exception:
            return ''


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
                print('WARNING: parameter %s is not existed!' % item[0])
                continue
            num_vars += 1
            names.append(item[0])
            bounds.append([float(item[1]), float(item[2])])
        self.param_defs = {'names': names, 'bounds': bounds, 'num_vars': num_vars}
        return self.param_defs

    def reset_simulation_timerange(self):
        """Update simulation time range in MongoDB [FILE_IN]."""
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

    def initialize(self, n=1):
        """Initialize parameters samples by Latin-Hypercube sampling method.

        Returns:
            A list contains parameter value at each gene location.
        """
        param_num = self.ParamDefs['num_vars']
        lhs_samples = lhs(param_num, n)
        all = list()
        for idx in range(n):
            gene_values = list()
            for i, param_bound in enumerate(self.ParamDefs['bounds']):
                gene_values.append(lhs_samples[idx][i] * (param_bound[1] - param_bound[0]) +
                                   param_bound[0])
            all.append(gene_values)
        return all


def initialize_calibrations(cf):
    """Initial individual of population.
    """
    cali = Calibration(cf)
    return cali.initialize()


def calibration_objectives(cali_obj, ind):
    """Evaluate the objectives of given individual. ##, cali_period, vali_period=''
    """
    cali_obj.ID = ind.id
    model_obj = MainSEIMS(cali_obj.cfg.bin_dir, cali_obj.cfg.model_dir,
                          nthread=cali_obj.cfg.nthread, lyrmtd=cali_obj.cfg.lyrmethod,
                          ip=cali_obj.cfg.hostname, port=cali_obj.cfg.port,
                          sceid=cali_obj.cfg.sceid, caliid=ind.id)
    run_flag = model_obj.run()
    if not run_flag:
        return ind
    # Sleep 0.5 second
    time.sleep(0.5)
    # read simulation data
    # dates = cali_period.split(',')
    # stime = StringClass.get_datetime(dates[0], '%Y-%m-%d %H:%M:%S')
    # etime = StringClass.get_datetime(dates[1], '%Y-%m-%d %H:%M:%S')
    ind.cali.vars, ind.cali.data = read_simulation_from_txt(model_obj.output_dir,
                                                            ind.obs.vars, model_obj.outlet_id,
                                                            cali_obj.cfg.cali_stime,
                                                            cali_obj.cfg.cali_etime)
    # Match with observation data
    ind.cali.sim_obs_data = match_simulation_observation(ind.cali.vars, ind.cali.data,
                                                         ind.obs.vars, ind.obs.data,
                                                         cali_obj.cfg.cali_stime,
                                                         cali_obj.cfg.cali_etime)
    # Calculate NSE, R2, RMSE, PBIAS, and RSR
    ind.cali.valid = calculate_statistics(ind.cali.sim_obs_data)
    # if calculate validation period
    if cali_obj.cfg.calc_validation:
        ind.vali.vars, ind.vali.data = read_simulation_from_txt(model_obj.output_dir,
                                                                ind.obs.vars, model_obj.outlet_id,
                                                                cali_obj.cfg.vali_stime,
                                                                cali_obj.cfg.vali_etime)
        # Match with observation data
        ind.vali.sim_obs_data = match_simulation_observation(ind.vali.vars, ind.vali.data,
                                                             ind.obs.vars, ind.obs.data,
                                                             cali_obj.cfg.vali_stime,
                                                             cali_obj.cfg.vali_etime)
        # Calculate NSE, R2, RMSE, PBIAS, and RSR
        ind.vali.valid = calculate_statistics(ind.vali.sim_obs_data)
    # delete model output directory for saving storage
    shutil.rmtree(model_obj.output_dir)
    return ind


if __name__ == '__main__':
    cf, method = get_cali_config()
    cfg = CaliConfig(cf, method=method)

    caliobj = Calibration(cfg)

    # test the picklable of Scenario class.
    import pickle

    s = pickle.dumps(caliobj)
    # print(s)
    new_cali = pickle.loads(s)
    print(new_cali.bin_dir)
