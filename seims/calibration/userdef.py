#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""User defined functions.
    @author   : Liangjun Zhu
    @changelog: 18-1-22  lj - initial implementation.\n
"""
import json
import pickle
import os
import sys

import numpy

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.append(os.path.abspath(os.path.join(sys.path[0], '..')))
import matplotlib as mpl

if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    mpl.use('Agg', warn=False)
import matplotlib.dates as mdates
import matplotlib.pyplot as plt
from preprocess.db_mongodb import ConnectMongoDB
from parameters_sensitivity.figure import save_png_eps
from parameters_sensitivity.sensitivity import SpecialJsonEncoder
from pygeoc.utils import StringClass


def write_param_values_to_mongodb(hostname, port, spatial_db, param_defs, param_values):
    # update Parameters collection in MongoDB
    client = ConnectMongoDB(hostname, port)
    conn = client.get_conn()
    db = conn[spatial_db]
    collection = db['PARAMETERS']
    collection.update_many({}, {'$unset': {'CALI_VALUES': ''}})
    for idx, pname in enumerate(param_defs['names']):
        v2str = ','.join(str(v) for v in param_values[:, idx])
        collection.find_one_and_update({'NAME': pname}, {'$set': {'CALI_VALUES': v2str}})
    client.close()


def output_population_details(pops, outdir, gen_num):
    """Output population details, i.e., the simulation data, etc."""
    # Save as json, which can be loaded by json.load()
    with open(outdir + os.sep + 'gen%d_simObsData.json' % gen_num, 'w') as f:
        data = list()
        for ind in pops:
            ind.sim.sim_obs_data['Gen'] = ind.gen
            ind.sim.sim_obs_data['ID'] = ind.id
            ind.sim.sim_obs_data['var_name'] = ind.sim.vars
            data.append(ind.sim.sim_obs_data)
        json_data = json.dumps(data, indent=4, cls=SpecialJsonEncoder)
        f.write(json_data)
    with open(outdir + os.sep + 'gen%d_simData.pickle' % gen_num, 'w') as f:
        data = list()
        for ind in pops:
            data.append(ind.sim.data)
        pickle.dump(data, f)


def calculate_95ppu(pops, outdir, gen_num):
    """Calculate 95% prediction uncertainty and plot the hydrographs."""
    plt.rcParams['xtick.direction'] = 'out'
    plt.rcParams['ytick.direction'] = 'out'
    plt.rcParams['font.family'] = 'Times New Roman'
    plt.rcParams['timezone'] = 'UTC'
    if len(pops) < 2:
        return
    var_name = pops[0].sim.vars
    for idx, var in enumerate(var_name):
        ylabel_str = var
        if var in ['Q', 'QI', 'QG', 'QS']:
            ylabel_str += ' (m$^3$/s)'
        elif 'CONC' in var.upper():  # Concentrate
            if 'SED' in var.upper():
                ylabel_str += ' (g/L)'
            else:
                ylabel_str += ' (mg/L)'
        else:  # amount
            ylabel_str += ' (kg)'
        obs_dates = pops[0].sim.sim_obs_data[var]['UTCDATETIME']
        if isinstance(obs_dates[0], str) or isinstance(obs_dates[0], unicode):
            obs_dates = [StringClass.get_datetime(s) for s in obs_dates]
        obs_data = pops[0].sim.sim_obs_data[var]['Obs']

        sim_dates = pops[0].sim.data.keys()
        sim_data = list()
        sim_best = None
        for idx2, ind in enumerate(pops):
            tmp = numpy.array(ind.sim.data.values())
            tmp = tmp[:, idx]
            if idx2 == 0:
                sim_best = tmp.tolist()
            sim_data.append(tmp.tolist())
        sim_data = numpy.array(sim_data)
        ylows = numpy.percentile(sim_data, 2.5, 0, interpolation='nearest')
        yups = numpy.percentile(sim_data, 97.5, 0, interpolation='nearest')

        count = 0
        ylows_obs = list()
        yups_obs = list()
        for oi, ov in enumerate(obs_data):
            try:
                si = sim_dates.index(obs_dates[oi])
                ylows_obs.append(ylows[si])
                yups_obs.append(yups[si])
                if ylows[si] <= ov <= yups[si]:
                    count += 1
            except Exception:
                continue
        p_value = float(count) / len(obs_data)
        ylows_obs = numpy.array(ylows_obs)
        yups_obs = numpy.array(yups_obs)
        r_value = numpy.mean(yups_obs - ylows_obs) / numpy.std(numpy.array(obs_data))
        # concatenate text
        txt = 'P-factor: %.2f, R-factor: %.2f\n' % (p_value, r_value)
        txt += 'Best simulation:\n\tNSE: %.2f, RSR: %.2f, ' \
               'PBIAS: %.2f%%, R$^2$: %.2f' % (pops[0].sim.sim_obs_data[var]['NSE'],
                                               pops[0].sim.sim_obs_data[var]['RSR'],
                                               pops[0].sim.sim_obs_data[var]['PBIAS'],
                                               pops[0].sim.sim_obs_data[var]['R-square'])
        # plot
        fig, ax = plt.subplots(figsize=(12, 4))
        ax.fill_between(sim_dates, ylows.tolist(), yups.tolist(),
                        color=(0.8, 0.8, 0.8), label='95PPU')
        ax.scatter(obs_dates, obs_data, marker='.', s=20,
                   color='g', label='Observed points')
        ax.plot(sim_dates, sim_best, linestyle='-', color='red',
                label='Best simulation', linewidth=2)
        ax.set_ylim(bottom=0.)
        date_fmt = mdates.DateFormatter('%m-%d-%y')
        ax.xaxis.set_major_formatter(date_fmt)
        ax.tick_params(axis='x', bottom='on', top='off', length=5, width=2, which='major')
        ax.tick_params(axis='y', left='on', right='off', length=5, width=2, which='major')
        plt.xlabel('Date', fontsize='small')
        plt.ylabel(ylabel_str, fontsize='small')
        # add legend
        handles, labels = ax.get_legend_handles_labels()
        order = [labels.index('95PPU'), labels.index('Observed points'),
                 labels.index('Best simulation')]
        ax.legend([handles[idx] for idx in order], [labels[idx] for idx in order],
                  fontsize='medium', loc=1, framealpha=0.8)
        # add text
        plt.figtext(0.1, 0.75, txt)
        # fig.autofmt_xdate(rotation=0, ha='center')
        plt.tight_layout()
        save_png_eps(plt, outdir, 'Gen%d_95ppu_%s' % (gen_num, var))
        # close current plot in case of 'figure.max_open_warning'
        plt.cla()
        plt.clf()
        plt.close()
