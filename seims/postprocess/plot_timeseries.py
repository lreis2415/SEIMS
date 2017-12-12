# -*- coding: utf-8 -*-

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.append(os.path.abspath(os.path.join(sys.path[0], '..')))

from collections import OrderedDict

import matplotlib as mpl

if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    mpl.use('Agg')
import matplotlib.dates as mdates
import matplotlib.pyplot as plt
from pygeoc.utils import FileClass, StringClass, MathClass

from preprocess.db_mongodb import ConnectMongoDB
from preprocess.db_mongodb import MongoQuery
from preprocess.text import FieldNames, DBTableNames, DataValueFields, DataType, StationFields
from preprocess.text import ModelCfgFields, SubbsnStatsName
from preprocess.utility import read_data_items_from_txt


class TimeSeriesPlots(object):
    """Plot time series data, e.g., flow charge, sediment charge, etc.
    """

    def __init__(self, cfg):
        """EMPTY"""
        self.ws = cfg.model_dir
        self.plot_vars = cfg.plt_vars
        self.lang_cn = cfg.lang_cn
        client = ConnectMongoDB(cfg.hostname, cfg.port)
        conn = client.get_conn()
        self.maindb = conn[cfg.spatial_db]
        self.climatedb = conn[cfg.climate_db]
        self.mode = ''
        self.interval = -1
        # UTCTIME
        self.stime = cfg.time_start
        self.etime = cfg.time_end
        self.subbsnID = cfg.plt_subbsnid
        # define data
        self.outletid = -1
        self.pcp_date_value = list()
        self.plot_vars_existed = list()
        self.sim_data_dict = OrderedDict()
        self.sim_data_value = list()
        self.sim_obs_dict = dict()

    def read_model_setting(self):
        """Read the mode and interval from maindb[FILE_IN] and [FILE_OUT]."""
        filein_tab = self.maindb[DBTableNames.main_filein]
        self.mode = filein_tab.find_one({ModelCfgFields.tag: FieldNames.mode})[ModelCfgFields.value]
        if isinstance(self.mode, unicode):
            self.mode = self.mode.encode().upper()

        findinterval = filein_tab.find_one({ModelCfgFields.tag: ModelCfgFields.interval})
        self.interval = int(findinterval[ModelCfgFields.value])
        self.outletid = int(MongoQuery.get_init_parameter_value(self.maindb,
                                                                SubbsnStatsName.outlet))
        # read start time and end time from FILE_OUT
        st = filein_tab.find_one({ModelCfgFields.tag: ModelCfgFields.stime})[ModelCfgFields.value]
        et = filein_tab.find_one({ModelCfgFields.tag: ModelCfgFields.etime})[ModelCfgFields.value]
        st = StringClass.get_datetime(st)
        et = StringClass.get_datetime(et)
        if st > self.stime:
            self.stime = st
        if et < self.etime:
            self.etime = et
        if st > self.etime > self.stime:
            self.stime = st
            self.etime = et

    def read_precipitation(self):
        """
        The precipitation is read according to the subbasin ID.
            Especially when plot a specific subbasin (such as ID 3).
            For the whole basin, the subbasin ID is 0.
        TODO: Extend the selected subbasin ID to a list.
        Returns:
            Precipitation data list with the first element as datetime.
            [[Datetime1, value1], [Datetime2, value2], ..., [Datetimen, valuen]]
        """
        sitelist_tab = self.maindb[DBTableNames.main_sitelist]
        findsites = sitelist_tab.find_one({FieldNames.subbasin_id: self.subbsnID,
                                           FieldNames.mode: self.mode})
        if findsites is not None:
            site_liststr = findsites[FieldNames.site_p]
        else:
            raise RuntimeError('Cannot find precipitation site for subbasin %d.' % self.subbsnID)
        site_list = StringClass.extract_numeric_values_from_string(site_liststr)
        site_list = [int(v) for v in site_list]
        if len(site_list) == 0:
            raise RuntimeError('Cannot find precipitation site for subbasin %d.' % self.subbsnID)

        pcp_dict = OrderedDict()

        for pdata in self.climatedb[DBTableNames.data_values].find(
                {DataValueFields.utc: {"$gte": self.stime, '$lte': self.etime},
                 DataValueFields.type: DataType.p,
                 DataValueFields.id: {"$in": site_list}}).sort([(DataValueFields.utc, 1)]):
            curt = pdata[DataValueFields.utc]
            curv = pdata[DataValueFields.value]
            if curt not in pcp_dict:
                pcp_dict[curt] = 0.
            pcp_dict[curt] += curv
        # average
        if len(site_list) > 1:
            for t in pcp_dict:
                pcp_dict[t] /= len(site_list)
        for t, v in pcp_dict.iteritems():
            # print str(t), v
            self.pcp_date_value.append([t, v])
        print ('Read precipitation done.')

    def read_simulation_from_txt(self):
        """
        Read simulation data from text file according to subbasin ID.
        Returns:
            Simulation data list of all plotted variables, with UTCDATETIME.
            [[Datetime1, var1, var2,...], ...]
        """
        for i, v in enumerate(self.plot_vars):
            txtfile = self.ws + os.sep + v + '.txt'
            if not FileClass.is_file_exists(txtfile):
                print ('WARNING: Simulation variable file: %s is not existed!' % txtfile)
                continue
            data_items = read_data_items_from_txt(txtfile)
            found = False
            data_available = False
            for item in data_items:
                item_vs = StringClass.split_string(item[0], ' ', elim_empty=True)
                if len(item_vs) == 2:
                    if int(item_vs[1]) == self.subbsnID and not found:
                        found = True
                    elif int(item_vs[1]) != self.subbsnID and found:
                        break
                if not found:
                    continue
                if len(item_vs) != 3:
                    continue
                date_str = '%s %s' % (item_vs[0], item_vs[1])
                sim_datetime = StringClass.get_datetime(date_str, "%Y-%m-%d %H:%M:%S")

                if self.stime <= sim_datetime <= self.etime:
                    if sim_datetime not in self.sim_data_dict:
                        self.sim_data_dict[sim_datetime] = list()
                    self.sim_data_dict[sim_datetime].append(float(item_vs[2]))
                    data_available = True
            if data_available:
                self.plot_vars_existed.append(v)
        for d, vs in self.sim_data_dict.items():
            self.sim_data_value.append([d] + vs[:])

        # reset start time and end time
        if len(self.sim_data_value) == 0:
            raise RuntimeError('No available simulate data, please check the start and end time!')
        self.stime = self.sim_data_value[0][0]
        self.etime = self.sim_data_value[-1][0]

        # print (self.sim_data_value)
        print ('Read simulation done.')

    def match_simulation_observation(self):
        """Match the simulation and observation data by UTCDATETIME.

        Returns:
            The dict with the format:
            {VarName: {'UTCDATETIME': [t1, t2, ..., tn],
                       'Obs': [o1, o2, ..., on],
                       'Sim': [s1, s2, ..., sn]},
            ...
            }
        """
        coll_list = self.climatedb.collection_names()
        if DBTableNames.observes not in coll_list:
            return
        isoutlet = 0
        if self.subbsnID == self.outletid:
            isoutlet = 1

        def get_observed_parameter(name):
            if '_' in name:
                name = name.split('_')[1]
            return name

        def get_base_variable_name(name):
            name = get_observed_parameter(name)
            if 'Conc' in name:
                name = name.split('Conc')[0]
            return name

        for i, param_name in enumerate(self.plot_vars_existed):
            # TODO: During preprocess, the observation site should have the
            # TODO:    corresponding subbasinID field.
            site_items = self.climatedb[DBTableNames.sites].find_one(
                    {StationFields.type: get_base_variable_name(param_name),
                     StationFields.outlet: isoutlet})

            if site_items is None:
                continue

            site_id = site_items[StationFields.id]
            for obs in self.climatedb[DBTableNames.observes].find(
                    {DataValueFields.utc: {"$gte": self.stime, '$lte': self.etime},
                     DataValueFields.type: get_observed_parameter(param_name),
                     DataValueFields.id: site_id}).sort([(DataValueFields.utc, 1)]):
                if param_name not in self.sim_obs_dict:
                    self.sim_obs_dict[param_name] = {DataValueFields.utc: list(),
                                                     'Obs': list(),
                                                     'Sim': list()}
                curt = obs[DataValueFields.utc]
                curv = obs[DataValueFields.value]
                if curt in self.sim_data_dict:
                    cursv = self.sim_data_dict[curt][i]
                    self.sim_obs_dict[param_name][DataValueFields.utc].append(curt)
                    self.sim_obs_dict[param_name]['Obs'].append(curv)
                    self.sim_obs_dict[param_name]['Sim'].append(cursv)

        # for param, values in self.sim_obs_dict.iteritems():
        #     print ('Observation-Simulation of %s' % param)
        #     for d, o, s in zip(values[DataValueFields.utc], values['Obs'], values['Sim']):
        #         print str(d), o, s
        print ('Match observation and simulation done.')

    def calculate_statistics(self):
        """NSE, R-square, RMSE, PBIAS.
        Returns:
            The dict with the format:
            {VarName: {'UTCDATETIME': [t1, t2, ..., tn],
                       'Obs': [o1, o2, ..., on],
                       'Sim': [s1, s2, ..., sn]},
                       'NSE': nse_value,
                       'R-square': r2_value,
                       'RMSE': rmse_value,
                       'PBIAS': pbias_value}
            ...
            }
        """
        for param, values in self.sim_obs_dict.iteritems():
            obsl = values['Obs'][:]
            siml = values['Sim'][:]

            # print (len(obsl), len(siml))
            obslnew = list()
            simlnew = list()
            for idx, v in enumerate(obsl):
                if v < 0.001 or siml[idx] < 0.001:
                    continue
                if StringClass.string_match(param, 'SED') and v < 1000:
                    continue
                obslnew.append(v)
                simlnew.append(siml[idx])
            obsl = obslnew[:]
            siml = simlnew[:]

            # print (param, obsl, siml)
            nse_value = MathClass.nashcoef(obsl, siml)
            r2_value = MathClass.rsquare(obsl, siml)
            rmse_value = MathClass.rmse(obsl, siml)
            pbias_value = MathClass.pbias(obsl, siml)
            rsr_value = MathClass.rsr(obsl, siml)
            values['NSE'] = nse_value
            values['R-square'] = r2_value
            values['RMSE'] = rmse_value
            values['PBIAS'] = pbias_value
            values['RSR'] = rsr_value

            print ('Statistics for %s, NSE: %.3f, R2: %.3f, PBIAS: %.3f, RSR: %.3f' %
                   (param, nse_value, r2_value, pbias_value, rsr_value))

    def generate_plots(self):
        """Generate hydrographs of discharge, sediment, nutrient (amount or concentrate), etc."""
        # set ticks direction, in or out
        plt.rcParams['xtick.direction'] = 'out'
        plt.rcParams['ytick.direction'] = 'out'
        plt.rcParams['font.family'] = 'Times New Roman'
        sim_date = self.sim_data_dict.keys()
        for i, param in enumerate(self.plot_vars_existed):
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
                ymax = max(max(sim_list), max(obs_values)) * 1.5
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
                    plt.title('\nNSE: %.2f, R$^2$: %.2f, PBIAS: %.2f%%, RSR: %.2f' %
                              (nse, r2, pbias, rsr), color='red', loc='right')
                except ValueError or Exception:
                    pass
                plt.tight_layout()
            leg.get_frame().set_alpha(0.5)
            # plt.title(param, color='#aa0903')
            timerange = '%s-%s' % (self.stime.strftime('%Y-%m-%d'),
                                   self.etime.strftime('%Y-%m-%d'))
            fpath = self.ws + os.sep + param + '-' + timerange + '.png'
            plt.savefig(fpath, dpi=300)
            print ('Plot %s done, saved as: %s' % (param, fpath))
            # plt.show()

    def workflow(self):
        """"Workflow"""
        # read model in settings from MongoDB
        self.read_model_setting()
        # read precipitation from MongoDB
        self.read_precipitation()
        # read simulation from text files
        self.read_simulation_from_txt()
        # find observation data from MongoDB
        self.match_simulation_observation()
        # calculate statistics indexes
        self.calculate_statistics()
        # create plots
        self.generate_plots()
