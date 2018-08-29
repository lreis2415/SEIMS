#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Utility functions and classes for postprocess.
    @author   : Liangjun Zhu
    @changelog: 18-01-04  lj - initial implementation.\n
                18-02-09  lj - compatible with Python3.\n
"""
from __future__ import absolute_import

import os
import sys
import bisect
from collections import OrderedDict

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import StringClass, FileClass, MathClass, UtilClass
from preprocess.utility import read_data_items_from_txt
from preprocess.text import DataValueFields


def save_png_eps(plot, wp, name):
    """Save figures, both png and eps formats"""
    eps_dir = wp + os.path.sep + 'eps'
    UtilClass.mkdir(eps_dir)
    for figpath in [wp + os.path.sep + name + '.png', eps_dir + os.path.sep + name + '.eps']:
        plot.savefig(figpath, dpi=300)


def read_simulation_from_txt(ws, plot_vars, subbsnID, stime, etime):
    """
    Read simulation data from text file according to subbasin ID.
    Returns:
        1. Matched variable names, [var1, var2, ...]
        2. Simulation data dict of all plotted variables, with UTCDATETIME.
           {Datetime: [value_of_var1, value_of_var2, ...], ...}
    """
    plot_vars_existed = list()
    sim_data_dict = OrderedDict()
    for i, v in enumerate(plot_vars):
        txtfile = ws + os.path.sep + v + '.txt'
        if not FileClass.is_file_exists(txtfile):
            print('WARNING: Simulation variable file: %s is not existed!' % txtfile)
            continue
        data_items = read_data_items_from_txt(txtfile)
        found = False
        data_available = False
        for item in data_items:
            item_vs = StringClass.split_string(item[0], ' ', elim_empty=True)
            if len(item_vs) == 2:
                if int(item_vs[1]) == subbsnID and not found:
                    found = True
                elif int(item_vs[1]) != subbsnID and found:
                    break
            if not found:
                continue
            if len(item_vs) != 3:
                continue
            date_str = '%s %s' % (item_vs[0], item_vs[1])
            sim_datetime = StringClass.get_datetime(date_str, "%Y-%m-%d %H:%M:%S")

            if stime <= sim_datetime <= etime:
                if sim_datetime not in sim_data_dict:
                    sim_data_dict[sim_datetime] = list()
                sim_data_dict[sim_datetime].append(float(item_vs[2]))
                data_available = True
        if data_available:
            plot_vars_existed.append(v)

    print('Read simulation from %s to %s done.' % (stime.strftime('%c'),
                                                   etime.strftime('%c')))
    return plot_vars_existed, sim_data_dict


def match_simulation_observation(sim_vars, sim_dict, obs_vars, obs_dict,
                                 start_time=None, end_time=None):
    """Match the simulation and observation data by UTCDATETIME for each variable.

    Args:
        sim_vars: Simulated variable list, e.g., ['Q', 'SED']
        sim_dict: {Datetime: [value_of_var1, value_of_var2, ...], ...}
        obs_vars: Observed variable list, which may be None or [], e.g., ['Q']
        obs_dict: same format with sim_dict
        start_time: Start time, by default equals to the start of simulation data
        end_time: End time, see start_time
    Returns:
        The dict with the format:
        {VarName: {'UTCDATETIME': [t1, t2, ..., tn],
                   'Obs': [o1, o2, ..., on],
                   'Sim': [s1, s2, ..., sn]},
        ...
        }
    """
    if start_time is None:
        start_time = list(sim_dict.keys())[0]
    if end_time is None:
        end_time = list(sim_dict.keys())[-1]
    sim_obs_dict = dict()
    if not obs_vars:  # obs_vars is None or []
        return None
    sim_to_obs = OrderedDict()
    for i, param_name in enumerate(sim_vars):
        if param_name not in obs_vars:
            continue
        sim_to_obs[i] = obs_vars.index(param_name)
        sim_obs_dict[param_name] = {DataValueFields.utc: list(),
                                    'Obs': list(), 'Sim': list()}
    for sim_date, sim_values in sim_dict.items():
        if sim_date not in obs_dict or not start_time <= sim_date <= end_time:
            continue
        for sim_i, obs_i in sim_to_obs.items():
            param_name = sim_vars[sim_i]
            obs_values = obs_dict.get(sim_date)
            if obs_i > len(obs_values) or obs_values[obs_i] is None:
                continue
            sim_obs_dict[param_name][DataValueFields.utc].append(sim_date)
            sim_obs_dict[param_name]['Obs'].append(obs_values[obs_i])
            sim_obs_dict[param_name]['Sim'].append(sim_values[sim_i])

    # for param, values in self.sim_obs_dict.items():
    #     print('Observation-Simulation of %s' % param)
    #     for d, o, s in zip(values[DataValueFields.utc], values['Obs'], values['Sim']):
    #         print(str(d), o, s)
    print('Match observation and simulation done.')
    return sim_obs_dict


def calculate_statistics(sim_obs_dict, stime=None, etime=None):
    """Calculate NSE, R-square, RMSE, PBIAS, and RSR.
    Args:
        sim_obs_dict: {VarName: {'UTCDATETIME': [t1, t2, ..., tn],
                                 'Obs': [o1, o2, ..., on],
                                 'Sim': [s1, s2, ..., sn]
                                 },
                       ...
                       }
        stime: Start time for statistics calculation.
        etime: End time for statistics calculation.
    Returns:
        The dict with the format:
        {VarName: {'UTCDATETIME': [t1, t2, ..., tn],
                   'Obs': [o1, o2, ..., on],
                   'Sim': [s1, s2, ..., sn]},
                   'NSE': nse_value,
                   'R-square': r2_value,
                   'RMSE': rmse_value,
                   'PBIAS': pbias_value,
                   'lnNSE': lnnse_value,
                   'NSE1': nse1_value,
                   'NSE3': nse3_value
                   },
        ...
        }
    """
    if not sim_obs_dict:  # if None or dict()
        return None, None
    for param, values in sim_obs_dict.items():
        if stime is None and etime is None:
            sidx = 0
            eidx = len(values['UTCDATETIME'])
        else:
            sidx = bisect.bisect_left(values['UTCDATETIME'], stime)
            eidx = bisect.bisect_right(values['UTCDATETIME'], etime)
        obsl = values['Obs'][sidx:eidx]
        siml = values['Sim'][sidx:eidx]

        nse_value = MathClass.nashcoef(obsl, siml)
        r2_value = MathClass.rsquare(obsl, siml)
        rmse_value = MathClass.rmse(obsl, siml)
        pbias_value = MathClass.pbias(obsl, siml)
        rsr_value = MathClass.rsr(obsl, siml)
        lnnse_value = MathClass.nashcoef(obsl, siml, log=True)
        nse1_value = MathClass.nashcoef(obsl, siml, expon=1)
        nse3_value = MathClass.nashcoef(obsl, siml, expon=3)

        values['NSE'] = nse_value
        values['R-square'] = r2_value
        values['RMSE'] = rmse_value
        values['PBIAS'] = pbias_value
        values['RSR'] = rsr_value
        values['lnNSE'] = lnnse_value
        values['NSE1'] = nse1_value
        values['NSE3'] = nse3_value

        # print('Statistics for %s, NSE: %.3f, R2: %.3f, RMSE: %.3f, PBIAS: %.3f, RSR: %.3f,'
        #       ' lnNSE: %.3f, NSE1: %.3f, NSE3: %.3f' %
        #       (param, nse_value, r2_value, rmse_value, pbias_value, rsr_value,
        #        lnnse_value, nse1_value, nse3_value))
    return ['NSE', 'R-square', 'RMSE', 'PBIAS', 'RSR', 'lnNSE', 'NSE1', 'NSE3']
