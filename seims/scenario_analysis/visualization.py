#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Visualization of Scenarios, which is relative independent with SA.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-09-12  hr - initial implementation.\n
                17-08-18  lj - reorganization.\n
"""
import os

import matplotlib

if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy
from seims.pygeoc.pygeoc.utils.utils import StringClass

LFs = ['\r', '\n', '\r\n']


def plot_pareto_front(pop, ws, pop_size, gen_id):
    front = numpy.array([ind.fitness.values for ind in pop])
    # Plot
    plt.figure(gen_id)
    plt.title('Pareto frontier of Scenarios Optimization\n', color='#aa0903')
    # plt.xlabel('Economic calculate_economy(Million Yuan)')
    plt.xlabel('Economic effectiveness')
    plt.ylabel('Environmental effectiveness')
    # front[:, 0] /= 1000000.
    # front[:, 1] /= 1000.
    plt.scatter(front[:, 0], front[:, 1], c='r', alpha=0.8, s=12)
    plt.title('\nPopulation: %d, Generation: %d' % (pop_size, gen_id), color='green', fontsize=9,
              loc='right')
    img_path = ws + os.sep + 'Pareto_Gen_%d_Pop_%d.png' % (gen_id, pop_size)
    plt.savefig(img_path)
    # plt.show()


def read_pareto_points_from_txt(txt_file, xname, yname, gens):
    f = open(txt_file)
    pareto_points = dict()
    found = False
    cur_gen = -1
    xidx = -1
    yidx = -1
    for line in f:
        str_line = line
        for LF in LFs:
            if LF in line:
                str_line = line.split(LF)[0]
                break
        values = StringClass.extract_numeric_values_from_string(str_line)
        # Check generation
        if str_line[0] == '#' and 'Generation' in str_line:
            if len(values) != 1:
                continue
            gen = int(values[0])
            if gen not in gens:
                found = False
                continue
            found = True
            cur_gen = gen
            pareto_points[cur_gen] = {xname[0]: list(), yname[0]: list()}
            continue
        if not found:
            continue
        if values is None:  # means header line
            line_list = StringClass.split_string(str_line, ['\t'])
            for idx, v in enumerate(line_list):
                if StringClass.string_match(v, xname[0]):
                    xidx = idx
                if StringClass.string_match(v, yname[0]):
                    yidx = idx
            continue
        if xidx < 0 or yidx < 0:
            continue
        # now append the real Pareto front point data
        pareto_points[cur_gen][xname[0]].append(values[xidx])
        pareto_points[cur_gen][yname[0]].append(values[yidx])
    f.close()
    return pareto_points


def plot_pareto_fronts_by_method(method_files, xname, yname, gens, ws):
    """
    Plot Pareto fronts of different method at a same generation for comparision.
    Args:
        method_files(dict): key is method name (which also displayed in legend), value is file path.
        xname(list): the first is x field name in log file, and the second on is for plot,
                     the third and forth values are low and high limit (optional).
        yname(list): see xname
        gens(list): generation to be plotted
        ws: workspace for output files
    """
    pareto_data = dict()
    for k, v in method_files.iteritems():
        pareto_data[k] = read_pareto_points_from_txt(v, xname, yname, gens)
    # print (pareto_data)
    ylabel_str = yname[1]
    xlabel_str = xname[1]
    file_name = '-'.join(pareto_data.keys())

    plt.rcParams['xtick.direction'] = 'out'
    plt.rcParams['ytick.direction'] = 'out'
    plt.rcParams['font.family'] = 'Times New Roman'
    for gen in gens:
        markers = ['.', '+', '*', 'x', 'd', 'h', 's', '<', '>']
        fig, ax = plt.subplots(figsize=(12, 8))
        mark_idx = 0
        for method, valuedict in pareto_data.iteritems():
            if gen not in valuedict:
                continue
            xdata = valuedict[gen][xname[0]]
            ydata = valuedict[gen][yname[0]]
            plt.scatter(xdata, ydata, marker=markers[mark_idx], s=60,
                        color='black', label=method)
            mark_idx += 1
            # plt.plot(xdata, ydata, marker='.',  linestyle='none', alpha=0.9, label=method)
        xaxis = plt.gca().xaxis
        yaxis = plt.gca().yaxis
        for xlebal in xaxis.get_ticklabels():
            xlebal.set_fontsize(20)
        for ylebal in yaxis.get_ticklabels():
            ylebal.set_fontsize(20)
        plt.xlabel(xlabel_str, fontsize=20)
        plt.ylabel(ylabel_str, fontsize=20)
        # set xy axis limit
        curxlim = ax.get_xlim()
        if len(xname) >= 3:
            if curxlim[0] < xname[2]:
                ax.set_xlim(left=xname[2])
            if len(xname) >= 4 and curxlim[1] > xname[3]:
                ax.set_xlim(right=xname[3])
        curylim = ax.get_ylim()
        if len(yname) >= 3:
            if curylim[0] < yname[2]:
                ax.set_ylim(bottom=yname[2])
            if len(yname) >= 4 and curylim[1] > yname[3]:
                ax.set_ylim(top=yname[3])

        plt.legend(fontsize=24, loc=2)
        plt.tight_layout()
        fpath = ws + os.sep + file_name + '-gen' + str(gen) + '.png'
        plt.savefig(fpath, dpi=300)
        print ('%s saved!' % fpath)
        # close current plot in case of 'figure.max_open_warning'
        plt.cla()
        plt.clf()
        plt.close()


def main():
    """Main Entrance."""
    base_dir = r'C:\z_data\ChangTing\seims_models\NSGA2_Output\sedtoch'
    method_pareto = {'Rule based': base_dir + os.sep + 'gen22_pop100_rule/runtime.log',
                     'Random': base_dir + os.sep + 'gen22_pop100_rdm/runtime.log'}
    xaxis = ['economy', 'Cost (10,000 USD$)', 0.]
    yaxis = ['environmental', 'Sediment reduction rate']
    draw_gens = range(1, 22)

    plot_pareto_fronts_by_method(method_pareto, xaxis, yaxis, draw_gens, base_dir)


if __name__ == '__main__':
    main()
