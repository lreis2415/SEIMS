"""Handle time series data.

    @author   : Liangjun Zhu

    @changelog:
    - 18-10-29 - lj - Extract from other packages.
"""
from __future__ import absolute_import, unicode_literals

import bisect
from collections import OrderedDict
from datetime import datetime

from typing import List, Dict, Optional, Union, AnyStr
from pygeoc.utils import MathClass


def match_simulation_observation(sim_vars,  # type: List[AnyStr]
                                 sim_dict,  # type: Dict[datetime, List[float]]
                                 obs_vars,  # type: Optional[List[AnyStr]]
                                 obs_dict,  # type: Optional[Dict[datetime, List[float]]]
                                 start_time=None,  # type: Optional[datetime]
                                 end_time=None  # type: Optional[datetime]
                                 ):
    # type: (...) -> Optional[Dict[AnyStr, Dict[AnyStr, Union[List[datetime], List[float]]]]]
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
    if not obs_vars:
        return None
    sim_to_obs = OrderedDict()
    for i, param_name in enumerate(sim_vars):
        if param_name not in obs_vars:
            continue
        sim_to_obs[i] = obs_vars.index(param_name)
        sim_obs_dict[param_name] = {'UTCDATETIME': list(), 'Obs': list(), 'Sim': list()}
    for sim_date, sim_values in sim_dict.items():
        if sim_date not in obs_dict or not start_time <= sim_date <= end_time:
            continue
        for sim_i, obs_i in sim_to_obs.items():
            param_name = sim_vars[sim_i]
            obs_values = obs_dict.get(sim_date)
            if obs_i > len(obs_values) or obs_values[obs_i] is None:
                continue
            sim_obs_dict[param_name]['UTCDATETIME'].append(sim_date)
            sim_obs_dict[param_name]['Obs'].append(obs_values[obs_i])
            sim_obs_dict[param_name]['Sim'].append(sim_values[sim_i])

    # for param, values in self.sim_obs_dict.items():
    #     print('Observation-Simulation of %s' % param)
    #     for d, o, s in zip(values[DataValueFields.utc], values['Obs'], values['Sim']):
    #         print(str(d), o, s)
    print('Match observation and simulation done.')
    return sim_obs_dict


def calculate_statistics(sim_obs_dict,  # type: Optional[Dict[AnyStr, Dict[AnyStr, Union[List[datetime], List[float], float]]]]
                         stime=None,  # type: Optional[datetime]
                         etime=None  # type: Optional[datetime]
                         ):
    # type: (...) -> Optional[List[AnyStr]]
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
        Return name list of the calculated statistics
    """
    if not sim_obs_dict:
        return None
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
