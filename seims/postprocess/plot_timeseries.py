#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Plot time-series variables.
    @author   : Liangjun Zhu
    @changelog: 17-08-17  lj - redesign and rewrite the plotting program.\n
                18-01-04  lj - separate load data from MongoDB operations.\n
"""

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.append(os.path.abspath(os.path.join(sys.path[0], '..')))

import matplotlib as mpl

if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    mpl.use('Agg')
import matplotlib.dates as mdates
import matplotlib.pyplot as plt

from preprocess.text import DataValueFields
from load_mongodb import ReadModelData
from utility import read_simulation_from_txt, match_simulation_observation, calculate_statistics


class TimeSeriesPlots(object):
    """Plot time series data, e.g., flow charge, sediment charge, etc.
    """

    def __init__(self, cfg):
        """EMPTY"""
        self.ws = cfg.model_dir
        self.plot_vars = cfg.plt_vars
        self.lang_cn = cfg.lang_cn
        # UTCTIME
        self.stime = cfg.time_start
        self.etime = cfg.time_end
        self.subbsnID = cfg.plt_subbsnid

        # Read model data from MongoDB
        self.readData = ReadModelData(cfg.hostname, cfg.port, cfg.spatial_db)
        # client = ConnectMongoDB(cfg.hostname, cfg.port)
        # conn = client.get_conn()
        # self.maindb = conn[cfg.spatial_db]
        # self.climatedb = conn[cfg.climate_db]
        self.mode = self.readData.Mode
        self.interval = self.readData.Interval
        # check start and end time
        st, et = self.readData.SimulationPeriod
        if st > self.stime:
            self.stime = st
        if et < self.etime:
            self.etime = et
        if st > self.etime > self.stime:
            self.stime = st
            self.etime = et
        self.outletid = self.readData.OutletID
        # read precipitation
        self.pcp_date_value = self.readData.Precipitation(self.subbsnID, self.stime, self.etime)
        # read simulated data and update the available variables
        self.plot_vars, self.sim_data_dict = read_simulation_from_txt(self.ws, self.plot_vars,
                                                                      self.outletid, self.stime,
                                                                      self.etime)
        self.sim_data_value = list()
        for d, vs in self.sim_data_dict.items():
            self.sim_data_value.append([d] + vs[:])
        # reset start time and end time
        if len(self.sim_data_value) == 0:
            raise RuntimeError('No available simulate data, please check the start and end time!')
        self.stime = self.sim_data_value[0][0]
        self.etime = self.sim_data_value[-1][0]
        # read observation data from MongoDB
        self.obs_vars, self.obs_data_dict = self.readData.Observation(self.subbsnID, self.plot_vars,
                                                                      self.stime, self.etime)

        self.sim_obs_dict = match_simulation_observation(self.plot_vars, self.sim_data_dict,
                                                         self.obs_vars, self.obs_data_dict)
        calculate_statistics(self.sim_obs_dict)

    def generate_plots(self):
        """Generate hydrographs of discharge, sediment, nutrient (amount or concentrate), etc."""
        # set ticks direction, in or out
        plt.rcParams['xtick.direction'] = 'out'
        plt.rcParams['ytick.direction'] = 'out'
        plt.rcParams['font.family'] = 'Times New Roman'
        sim_date = self.sim_data_dict.keys()
        for i, param in enumerate(self.plot_vars):
            # plt.figure(i)
            fig, ax = plt.subplots(figsize=(12, 4))
            ylabel_str = param
            if param in ['Q', 'QI', 'QG', 'QS']:
                ylabel_str += ' (m$^3$/s)'
            elif 'CONC' in param.upper():  # Concentrate
                if 'SED' in param.upper():
                    ylabel_str += ' (g/L)'
                else:
                    ylabel_str += ' (mg/L)'
            else:  # amount
                ylabel_str += ' (kg)'

            obs_dates = None
            obs_values = None
            if param in self.sim_obs_dict:
                obs_dates = self.sim_obs_dict[param][DataValueFields.utc]
                obs_values = self.sim_obs_dict[param]['Obs']
            if obs_values is not None:
                # bar graph
                p1 = ax.bar(obs_dates, obs_values, label='Observation', color='none',
                            edgecolor='black',
                            linewidth=0.5, align='center', hatch='//')
                # # line graph
                # p1, = ax.plot(obs_dates, obs_values, label='Observation', color='black', marker='+',
                #              markersize=2, linewidth=1)
            sim_list = [v[i + 1] for v in self.sim_data_value]
            p2, = ax.plot(sim_date, sim_list, label='Simulation', color='red',
                          marker='+', markersize=2, linewidth=0.8)
            plt.xlabel('Date')
            # format the ticks date axis
            date_fmt = mdates.DateFormatter('%m-%d-%y')
            # autodates = mdates.AutoDateLocator()
            # days = mdates.DayLocator(bymonthday=range(1, 32), interval=4)
            # months = mdates.MonthLocator()
            # ax.xaxis.set_major_locator(months)
            ax.xaxis.set_major_formatter(date_fmt)
            # ax.xaxis.set_minor_locator(days)
            ax.tick_params('both', length=5, width=2, which='major')
            ax.tick_params('both', length=3, width=1, which='minor')
            fig.autofmt_xdate(rotation=0, ha='center')

            plt.ylabel(ylabel_str)
            # plt.legend(bbox_to_anchor = (0.03, 0.85), loc = 2, shadow = True)
            if obs_values is not None:
                ymax = max(max(sim_list), max(obs_values)) * 1.8
                ymin = min(min(sim_list), min(obs_values)) * 0.8
            else:
                ymax = max(sim_list) * 1.8
                ymin = min(sim_list) * 0.8
            ax.set_ylim(float(ymin), float(ymax))
            ax2 = ax.twinx()
            ax.tick_params(axis='x', which='both', bottom='on', top='off')
            ax2.tick_params(axis='y', length=5, width=2, which='major')
            ax2.set_ylabel(r'Precipitation (mm)')

            pcp_date = [v[0] for v in self.pcp_date_value]
            preci = [v[1] for v in self.pcp_date_value]
            p3 = ax2.bar(pcp_date, preci, label='Rainfall', color='blue', linewidth=0,
                         align='center')
            ax2.set_ylim(float(max(preci)) * 1.8, float(min(preci)) * 0.8)
            if obs_values is None or len(obs_values) < 2:
                leg = ax.legend([p3, p2], ['Rainfall', 'Simulation'], ncol=2,
                                bbox_to_anchor=(0., 1.02, 1., 0.102),
                                borderaxespad=0.2,
                                loc='lower left', fancybox=True)
                plt.tight_layout(rect=(0, 0, 1, 0.93))
            else:
                leg = ax.legend([p3, p1, p2], ['Rainfall', 'Observation', 'Simulation'],
                                bbox_to_anchor=(0., 1.02, 1., 0.102),
                                borderaxespad=0.,
                                ncol=3, loc='lower left', fancybox=True)
                try:
                    nse = self.sim_obs_dict[param]['NSE']
                    r2 = self.sim_obs_dict[param]['R-square']
                    pbias = self.sim_obs_dict[param]['PBIAS']
                    rsr = self.sim_obs_dict[param]['RSR']
                    plt.title('\nNSE: %.2f, RSR: %.2f, PBIAS: %.2f%%, R$^2$: %.2f' %
                              (nse, rsr, pbias, r2), color='red', loc='right')
                except ValueError or Exception:
                    pass
                plt.tight_layout()
            leg.get_frame().set_alpha(0.5)
            # plt.title(param, color='#aa0903')
            timerange = '%s-%s' % (self.stime.strftime('%Y-%m-%d'),
                                   self.etime.strftime('%Y-%m-%d'))
            fpath = self.ws + os.sep + param + '-' + timerange
            plt.savefig(fpath + '.png', dpi=300)
            plt.savefig(fpath + '.eps', dpi=300)
            print ('Plot %s done, saved as: %s' % (param, fpath))
