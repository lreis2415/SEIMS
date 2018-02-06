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
from postprocess.utility import save_png_eps
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
    cali_sim_obs_data = list()
    cali_sim_data = list()
    with open(outdir + os.sep + 'gen%d_caliObsData.json' % gen_num, 'w') as f:
        for ind in pops:
            ind.cali.sim_obs_data['Gen'] = ind.gen
            ind.cali.sim_obs_data['ID'] = ind.id
            ind.cali.sim_obs_data['var_name'] = ind.cali.vars
            cali_sim_obs_data.append(ind.cali.sim_obs_data)
        json_data = json.dumps(cali_sim_obs_data, indent=4, cls=SpecialJsonEncoder)
        f.write(json_data)
    with open(outdir + os.sep + 'gen%d_caliSimData.pickle' % gen_num, 'w') as f:
        for ind in pops:
            cali_sim_data.append(ind.cali.data)
        pickle.dump(cali_sim_data, f)
    vali_sim_obs_data = list()
    vali_sim_data = list()
    if pops[0].vali.valid:
        with open(outdir + os.sep + 'gen%d_valiObsData.json' % gen_num, 'w') as f:
            for ind in pops:
                ind.vali.sim_obs_data['Gen'] = ind.gen
                ind.vali.sim_obs_data['ID'] = ind.id
                ind.vali.sim_obs_data['var_name'] = ind.vali.vars
                vali_sim_obs_data.append(ind.vali.sim_obs_data)
            json_data = json.dumps(vali_sim_obs_data, indent=4, cls=SpecialJsonEncoder)
            f.write(json_data)
        with open(outdir + os.sep + 'gen%d_valiSimData.pickle' % gen_num, 'w') as f:
            for ind in pops:
                vali_sim_data.append(ind.vali.data)
            pickle.dump(vali_sim_data, f)
    try:
        # Calculate 95PPU for current generation, and plot the desired variables, e.g., Q and SED
        calculate_95ppu(cali_sim_obs_data, cali_sim_data, outdir, gen_num,
                        vali_sim_obs_data, vali_sim_data)
    except Exception:
        pass


def calculate_95ppu(sim_obs_data, sim_data, outdir, gen_num,
                    vali_sim_obs_data=None, vali_sim_data=None):
    """Calculate 95% prediction uncertainty and plot the hydrographs."""
    plt.rcParams['xtick.direction'] = 'out'
    plt.rcParams['ytick.direction'] = 'out'
    plt.rcParams['font.family'] = 'Times New Roman'
    plt.rcParams['timezone'] = 'UTC'
    plt.rcParams['mathtext.fontset'] = 'custom'
    plt.rcParams['mathtext.it'] = 'STIXGeneral:italic'
    plt.rcParams['mathtext.bf'] = 'STIXGeneral:italic:bold'
    if len(sim_data) < 2:
        return
    var_name = sim_obs_data[0]['var_name']
    for idx, var in enumerate(var_name):
        plot_validation = False
        if vali_sim_obs_data and vali_sim_data and var in vali_sim_obs_data[0]['var_name']:
            plot_validation = True
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
        cali_obs_dates = sim_obs_data[0][var]['UTCDATETIME'][:]
        if isinstance(cali_obs_dates[0], str) or isinstance(cali_obs_dates[0], unicode):
            cali_obs_dates = [StringClass.get_datetime(s) for s in cali_obs_dates]
        obs_dates = cali_obs_dates[:]
        if plot_validation:
            vali_obs_dates = vali_sim_obs_data[0][var]['UTCDATETIME']
            if isinstance(vali_obs_dates[0], str) or isinstance(vali_obs_dates[0], unicode):
                vali_obs_dates = [StringClass.get_datetime(s) for s in vali_obs_dates]
            obs_dates += vali_obs_dates
        obs_data = sim_obs_data[0][var]['Obs'][:]
        if plot_validation:
            obs_data += vali_sim_obs_data[0][var]['Obs']

        sim_dates = sim_data[0].keys()
        if isinstance(sim_dates[0], str) or isinstance(sim_dates[0], unicode):
            sim_dates = [StringClass.get_datetime(s) for s in sim_dates]
        if plot_validation:
            vali_sim_dates = vali_sim_data[0].keys()
            if isinstance(vali_sim_dates[0], str) or isinstance(vali_sim_dates[0], unicode):
                vali_sim_dates = [StringClass.get_datetime(s) for s in vali_sim_dates]
            sim_dates += vali_sim_dates
        sim_data_list = list()
        caliBestIdx = -1
        caliBestNSE = -9999.
        for idx2, ind in enumerate(sim_data):
            tmp = numpy.array(ind.values())
            tmp = tmp[:, idx]
            if sim_obs_data[idx2][var]['NSE'] > caliBestNSE:
                caliBestNSE = sim_obs_data[idx2][var]['NSE']
                caliBestIdx = idx2
            tmpsim = tmp.tolist()
            if plot_validation:
                tmpsim += numpy.array(vali_sim_data[idx2].values())[:, idx].tolist()
            sim_data_list.append(tmpsim)

        sim_best = numpy.array(sim_data[caliBestIdx].values())[:, idx]
        sim_best = sim_best.tolist()
        if plot_validation:
            sim_best += numpy.array(vali_sim_data[caliBestIdx].values())[:, idx].tolist()
        sim_data_list = numpy.array(sim_data_list)
        ylows = numpy.percentile(sim_data_list, 2.5, 0, interpolation='nearest')
        yups = numpy.percentile(sim_data_list, 97.5, 0, interpolation='nearest')

        def calculate_95ppu_efficiency(obs_data_list, obs_dates_list, sim_dates_list):
            count = 0
            ylows_obs = list()
            yups_obs = list()
            for oi, ov in enumerate(obs_data_list):
                try:
                    si = sim_dates_list.index(obs_dates_list[oi])
                    ylows_obs.append(ylows[si])
                    yups_obs.append(yups[si])
                    if ylows[si] <= ov <= yups[si]:
                        count += 1
                except Exception:
                    continue
            p = float(count) / len(obs_data_list)
            ylows_obs = numpy.array(ylows_obs)
            yups_obs = numpy.array(yups_obs)
            r = numpy.mean(yups_obs - ylows_obs) / numpy.std(numpy.array(obs_data_list))
            return p, r

        # concatenate text
        p_value, r_value = calculate_95ppu_efficiency(sim_obs_data[0][var]['Obs'],
                                                      cali_obs_dates,
                                                      sim_data[0].keys())
        txt = 'P-factor: %.2f, R-factor: %.2f\n' % (p_value, r_value)
        txt += 'One of the best simulations:\n' \
               '    $\mathit{NSE}$: %.2f\n' \
               '    $\mathit{RSR}$: %.2f\n' \
               '    $\mathit{PBIAS}$: %.2f%%\n' \
               '    $\mathit{R^2}$: %.2f' % (sim_obs_data[caliBestIdx][var]['NSE'],
                                             sim_obs_data[caliBestIdx][var]['RSR'],
                                             sim_obs_data[caliBestIdx][var]['PBIAS'],
                                             sim_obs_data[caliBestIdx][var]['R-square'])
        # concatenate text of validation if needed
        vali_txt = ''
        if plot_validation:
            p_value, r_value = calculate_95ppu_efficiency(vali_sim_obs_data[0][var]['Obs'],
                                                          vali_obs_dates,
                                                          vali_sim_data[0].keys())
            vali_txt = 'P-factor: %.2f, R-factor: %.2f\n\n' % (p_value, r_value)
            vali_txt += '    $\mathit{NSE}$: %.2f\n' \
                        '    $\mathit{RSR}$: %.2f\n' \
                        '    $\mathit{PBIAS}$: %.2f%%\n' \
                        '    $\mathit{R^2}$: %.2f' % (vali_sim_obs_data[caliBestIdx][var]['NSE'],
                                                      vali_sim_obs_data[caliBestIdx][var]['RSR'],
                                                      vali_sim_obs_data[caliBestIdx][var]['PBIAS'],
                                                      vali_sim_obs_data[caliBestIdx][var]['R-square'])
        # plot
        fig, ax = plt.subplots(figsize=(12, 4))
        ax.fill_between(sim_dates, ylows.tolist(), yups.tolist(),
                        color=(0.8, 0.8, 0.8), label='95PPU')
        ax.scatter(obs_dates, obs_data, marker='.', s=20,
                   color='g', label='Observed points')
        ax.plot(sim_dates, sim_best, linestyle='--', color='red',
                label='Best simulation', linewidth=1)
        ax.set_xlim(left=min(sim_dates), right=max(sim_dates))
        ax.set_ylim(bottom=0.)
        date_fmt = mdates.DateFormatter('%m-%d-%y')
        ax.xaxis.set_major_formatter(date_fmt)
        ax.tick_params(axis='x', bottom='on', top='off', length=5, width=2, which='major')
        ax.tick_params(axis='y', left='on', right='off', length=5, width=2, which='major')
        plt.xlabel('Date', fontsize='small')
        plt.ylabel(ylabel_str, fontsize='small')
        # plot separate dash line
        delta_dt = (max(sim_dates) - min(sim_dates)) / 9
        delta_dt2 = (max(sim_dates) - min(sim_dates)) / 35
        sep_time = max(sim_dates) - delta_dt
        ymax, ymin = ax.get_ylim()
        yc = abs(ymax - ymin) * 0.9
        if plot_validation:
            sep_time = vali_sim_dates[0] if vali_sim_dates[0] > sim_dates[0] else sim_dates[0]
            ax.axvline(sep_time, color='black', linestyle='dashed', linewidth=2)
            plt.text(sep_time - delta_dt, yc, 'Calibration',
                     fontdict={'style': 'italic', 'weight': 'bold'}, color='black')
            plt.text(sep_time + delta_dt2, yc, 'Validation',
                     fontdict={'style': 'italic', 'weight': 'bold'}, color='black')

        # add legend
        handles, labels = ax.get_legend_handles_labels()
        order = [labels.index('95PPU'), labels.index('Observed points'),
                 labels.index('Best simulation')]
        ax.legend([handles[idx] for idx in order], [labels[idx] for idx in order],
                  fontsize='medium', loc=2, framealpha=0.8)
        # add text
        plt.text(sep_time - 2 * delta_dt, yc * 0.6, txt, color='red')
        if plot_validation:
            plt.text(sep_time + delta_dt2, yc * 0.6, vali_txt, color='red')
        # fig.autofmt_xdate(rotation=0, ha='center')
        plt.tight_layout()
        save_png_eps(plt, outdir, 'Gen%d_95ppu_%s' % (gen_num, var))
        # close current plot in case of 'figure.max_open_warning'
        plt.cla()
        plt.clf()
        plt.close()


if __name__ == '__main__':
    wp = r'C:\z_data\ChangTing\seims_models_phd\youwuzhen10m_longterm_model\Cali_NSGAII_Gen_3_Pop_4'
    simdir = wp + os.sep + 'simulated_data'
    all_cali_obs_data = list()
    all_cali_data = list()
    gen_cali_id = list()
    with open(simdir + os.sep + 'gen0_caliObsData.json', 'r') as f:
        cali_obs_data = json.load(f)
    with open(simdir + os.sep + 'gen0_caliSimData.pickle', 'r') as f:
        cali_data = pickle.load(f)
    # validation data
    with open(simdir + os.sep + 'gen0_valiObsData.json', 'r') as f:
        vali_obs_data = json.load(f)
    with open(simdir + os.sep + 'gen0_valiSimData.pickle', 'r') as f:
        vali_data = pickle.load(f)

    for a, b in zip(cali_obs_data, cali_data):
        if [a['Gen'], a['ID']] not in gen_cali_id:
            gen_cali_id.append([a['Gen'], a['ID']])
            all_cali_obs_data.append(a)
            all_cali_data.append(b)
    calculate_95ppu(cali_obs_data, cali_data, simdir, 0, vali_obs_data, vali_data)
    print ('Total simulations %d' % len(all_cali_obs_data))
