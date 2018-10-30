#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Visualization of Scenarios, which is relative independent with SA.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-09-12  hr - initial implementation.\n
                17-08-18  lj - reorganization.\n
                18-02-09  lj - compatible with Python3.\n
                18-08-24  lj - ReDesign pareto graph and hypervolume graph.\n
"""
from __future__ import absolute_import, unicode_literals

from collections import OrderedDict
from io import open
import os
import sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import matplotlib
from mpl_toolkits.mplot3d import Axes3D  # Do not delete this import

if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy
import itertools
from pygeoc.utils import StringClass
import re

from utility import save_png_eps

LFs = ['\r', '\n', '\r\n']


def plot_2d_scatter(xlist, ylist, title, xlabel, ylabel, ws, filename, subtitle='', cn=False,
                    xmin=None, xmax=None, xstep=None, ymin=None, ymax=None, ystep=None):
    """
    Todo: The size of the point may be vary with the number of points.

    Args:
        xlist: X coordinate list
        ylist: Y coordinate list
        title: Main title of the figure
        xlabel: X-axis label
        ylabel: Y-axis label
        ws: Full path of the destination directory
        filename: File name without suffix (e.g., jpg, eps)
        subtitle: (Optional) Subtitle
        cn: (Optional) Use Chinese
    """
    if cn:
        plt.rcParams['font.family'] = 'SimSun'  # 宋体
    plt.figure()
    plt.title('%s\n' % title, color='red')
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.scatter(xlist, ylist, c='r', alpha=0.8, s=12)
    if xmax is not None:
        plt.xlim(xmax=xmax)
    if xmin is not None:
        plt.xlim(xmin=xmin)
    if xstep is not None:
        xmin, xmax = plt.xlim()
        plt.xticks(numpy.arange(xmin, xmax + xstep * 0.99, step=xstep))
    if ymax is not None:
        plt.ylim(ymax=ymax)
    if ymin is not None:
        plt.ylim(ymin=ymin)
    if ystep is not None:
        ymin, ymax = plt.ylim()
        plt.yticks(numpy.arange(ymin, ymax + ystep * 0.99, step=ystep))

    if subtitle != '':
        plt.title(subtitle, color='green', fontsize=9, loc='right')
    plt.tight_layout()
    save_png_eps(plt, ws, filename)
    # close current plot in case of 'figure.max_open_warning'
    plt.cla()
    plt.clf()
    plt.close()


def plot_3d_scatter(xlist, ylist, zlist, title, xlabel, ylabel, zlabel,
                    ws, filename, subtitle='', cn=False,
                    xmin=None, xmax=None, ymin=None, ymax=None, zmin=None, zmax=None,
                    xstep=None, ystep=None, zstep=None):
    """

    Args:
        xlist: X coordinate list
        ylist: Y coordinate list
        zlist: Z coordinate list
        title: Main title of the figure
        xlabel: X-axis label
        ylabel: Y-axis label
        zlabel: Z-axis label
        ws: Full path of the destination directory
        filename: File name without suffix (e.g., jpg, eps)
        subtitle: (Optional) Subtitle
        cn: (Optional) Use Chinese
    """
    if cn:
        plt.rcParams['font.family'] = 'SimSun'  # 宋体
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    plt.suptitle('%s\n' % title, color='red')
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_zlabel(zlabel)
    ax.scatter(xlist, ylist, zlist, c='r', s=12)
    if xmax is not None:
        ax.set_xlim(right=xmax)
    if xmin is not None:
        ax.set_xlim(left=xmin)
    if xstep is not None:
        xmin, xmax = ax.get_xlim()
        ax.set_xticks(numpy.arange(xmin, xmax + xstep * 0.99, step=xstep))
    if ymax is not None:
        ax.set_ylim(top=ymax)
    if ymin is not None:
        ax.set_ylim(bottom=ymin)
    if ystep is not None:
        ymin, ymax = ax.get_ylim()
        ax.set_yticks(numpy.arange(ymin, ymax + ystep * 0.99, step=ystep))
    if zmax is not None:
        ax.set_zlim3d(top=zmax)
    if zmin is not None:
        ax.set_zlim3d(bottom=zmin)
    if zstep is not None:
        zmin, zmax = ax.get_zlim()
        ax.set_zticks(numpy.arange(zmin, zmax + zstep * 0.99, step=zstep))

    if subtitle != '':
        plt.title(subtitle, color='green', fontsize=9, loc='right')
    plt.tight_layout()
    save_png_eps(plt, ws, filename)
    # close current plot in case of 'figure.max_open_warning'
    plt.cla()
    plt.clf()
    plt.close()


def plot_pareto_front(data, labels, ws, gen_id, title,
                      lowers=None, uppers=None, steps=None, cn=False):
    """
    Plot 2D or 3D pareto front graphs.
    Args:
        data: 2-dimension array, nrows * ncols
        labels: Labels (axis names) list, the length should be equal to ncols
        ws: Workspace path
        gen_id: Generation ID
        title: Title
        lowers: (Optional) Lower values of each axis. Default is None.
        uppers: (Optional) Upper values of each axis. Default is None.
        steps: (Optional) Major ticks of each axis. Default is None.
        cn: (Optional) Use Chinese
    """
    if not isinstance(data, numpy.ndarray):
        data = numpy.array(data)
    pop_size, axis_size = data.shape
    if axis_size <= 1:
        print('Error: The size of fitness values MUST >= 2 to plot 2D graphs!')
        return
    if len(labels) != axis_size:
        print('Error: The size of fitness values and labels are not consistent!')
        return
    if lowers is not None and len(lowers) != axis_size:
        print('Warning: The size of fitness values and lowers are not consistent!')
        lowers = None
    if uppers is not None and len(uppers) != axis_size:
        print('Warning: The size of fitness values and uppers are not consistent!')
        uppers = None
    if steps is not None and len(steps) != axis_size:
        print('Warning: The size of fitness values and steps are not consistent!')
        steps = None

    subtitle = '\nGeneration: %d, Population: %d' % (gen_id, pop_size)
    if cn:
        subtitle = u'\n代数: %d, 个体数: %d' % (gen_id, pop_size)
    # 2D plot
    comb_2d = list(itertools.combinations(range(axis_size), 2))
    for comb in comb_2d:
        x_idx = comb[0]
        y_idx = comb[1]
        x_min = None
        x_max = None
        x_step = None
        y_min = None
        y_max = None
        y_step = None
        if lowers is not None:
            x_min = lowers[x_idx]
            y_min = lowers[y_idx]
        if uppers is not None:
            x_max = uppers[x_idx]
            y_max = uppers[y_idx]
        if steps is not None:
            x_step = steps[x_idx]
            y_step = steps[y_idx]
        maintitle = '%s of (%s, %s)' % (title, labels[x_idx], labels[y_idx])
        dirname = 'Pareto_%s-%s' % (labels[x_idx], labels[y_idx])
        tmpws = ws + os.sep + dirname
        if not os.path.exists(tmpws):
            os.mkdir(tmpws)
        filename = 'Pareto_Gen_%d_Pop_%d_%s-%s' % (gen_id, pop_size, labels[x_idx], labels[y_idx])
        if cn:
            filename += '_cn'
        plot_2d_scatter(data[:, x_idx], data[:, y_idx], maintitle,
                        labels[x_idx], labels[y_idx], tmpws, filename, subtitle, cn=cn,
                        xmin=x_min, xmax=x_max, ymin=y_min, ymax=y_max,
                        xstep=x_step, ystep=y_step)
    if axis_size >= 3:
        # 3D plot
        comb_3d = list(itertools.combinations(range(axis_size), 3))
        for comb in comb_3d:
            x_idx = comb[0]
            y_idx = comb[1]
            z_idx = comb[2]
            x_min = None
            x_max = None
            x_step = None
            y_min = None
            y_max = None
            y_step = None
            z_min = None
            z_max = None
            z_step = None
            if lowers is not None:
                x_min = lowers[x_idx]
                y_min = lowers[y_idx]
                z_min = lowers[z_idx]
            if uppers is not None:
                x_max = uppers[x_idx]
                y_max = uppers[y_idx]
                z_max = uppers[z_idx]
            if steps is not None:
                x_step = steps[x_idx]
                y_step = steps[y_idx]
                z_step = steps[z_idx]
            maintitle = '%s of (%s, %s, %s)' % (title, labels[x_idx], labels[y_idx], labels[z_idx])
            dirname = 'Pareto_%s-%s-%s' % (labels[x_idx], labels[y_idx], labels[z_idx])
            tmpws = ws + os.sep + dirname
            if not os.path.exists(tmpws):
                os.mkdir(tmpws)
            filename = 'Pareto_Gen_%d_Pop_%d_%s-%s-%s' % (gen_id, pop_size, labels[x_idx],
                                                          labels[y_idx], labels[z_idx])
            if cn:
                filename += '_cn'
            plot_3d_scatter(data[:, x_idx], data[:, y_idx], data[:, z_idx], maintitle,
                            labels[x_idx], labels[y_idx], labels[z_idx],
                            tmpws, filename, subtitle, cn=cn,
                            xmin=x_min, xmax=x_max, ymin=y_min, ymax=y_max, zmin=z_min, zmax=z_max,
                            xstep=x_step, ystep=y_step, zstep=z_step)


def read_pareto_points_from_txt(txt_file, sce_name, headers, labels=None):
    """Read Pareto points from `runtime.log` file.

    Args:
        txt_file: Full file path of `runtime.log` output by NSGA2 algorithm.
        sce_name: Field name followed by `generation`, e.g., 'calibrationID', 'scenarioID', etc.
        headers: Filed names in header for each dimension of Pareto front
        labels: (Optional) Labels corresponding to `headers` for Pareto graphs

    Returns:
        pareto_points: `OrderedDict`, key is generation ID, value is Pareto front array
        pareto_popnum: `OrderedDict`, key is generation ID, value is newly model runs number
    """
    with open(txt_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    pareto_points = OrderedDict()
    pareto_popnum = OrderedDict()
    found = False
    cur_gen = -1
    iden_idx = -1

    new_headers = headers[:]
    for i, hd in enumerate(new_headers):
        new_headers[i] = hd.upper()
    if labels is None:
        labels = headers[:]

    headers_idx = list()
    new_labels = list()

    for lno, line in enumerate(lines):
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
            # e.g., ###### Generation: 23 ######
            gen = int(values[0])
            found = True
            cur_gen = gen
            pareto_popnum[cur_gen] = list()
            pareto_points[cur_gen] = list()
            continue
        if not found:  # If the first "###### Generation: 1 ######" has not been found.
            continue
        line_list = StringClass.split_string(str_line.upper(), ['\t'])
        if values is None:  # means header line
            if headers_idx and new_labels:
                continue
            for idx, v in enumerate(line_list):
                if sce_name.upper() in v.upper():
                    iden_idx = idx
                    break
            for fldno, fld in enumerate(new_headers):
                if fld in line_list:
                    tmpidx = line_list.index(fld)
                    headers_idx.append(tmpidx)
                    new_labels.append(labels[fldno])
            continue
        if iden_idx < 0:
            continue
        # now append the real Pareto front point data
        tmpvalues = list()
        for tmpidx in headers_idx:
            tmpvalues.append(StringClass.extract_numeric_values_from_string(line_list[tmpidx])[0])
        pareto_points[cur_gen].append(tmpvalues[:])
        iden_str = line_list[iden_idx]  # e.g., 1-44
        iden_strs = iden_str.split('-')
        if len(iden_strs) == 1:
            pareto_popnum[cur_gen].append(int(iden_strs[0]))
        if len(iden_strs) == 2:
            pareto_popnum.setdefault(int(iden_strs[0]), list())
            pareto_popnum[int(iden_strs[0])].append(int(iden_strs[1]))

    return pareto_points, pareto_popnum


def read_pareto_popsize_from_txt(txt_file, sce_name='scenario'):
    with open(txt_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    pareto_popnum = OrderedDict()
    found = False
    cur_gen = -1
    iden_idx = -1
    for line in lines:
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
            found = True
            cur_gen = gen
            pareto_popnum[cur_gen] = list()
            continue
        if not found:
            continue
        if values is None:  # means header line
            line_list = StringClass.split_string(str_line, ['\t'])
            for idx, v in enumerate(line_list):
                if StringClass.string_match(v, sce_name):
                    iden_idx = idx
                    break
            continue
        if iden_idx < 0:
            continue
        # now append the real Pareto front point data
        pareto_popnum[cur_gen].append(int(values[iden_idx]))

    all_sceids = list()
    acc_num = list()
    genids = sorted(list(pareto_popnum.keys()))
    for idx, genid in enumerate(genids):
        for _id in pareto_popnum[genid]:
            if _id not in all_sceids:
                all_sceids.append(_id)
        acc_num.append(len(all_sceids))
    return genids, acc_num


def plot_pareto_fronts_fromfile(method_paths, sce_name, xname, yname, gens, ws):
    """
    Plot Pareto fronts of different method at a same generation for comparision.
    Args:
        method_paths(OrderedDict): key is method name (which also displayed in legend), value is file path.
        sce_name(str): Scenario ID field name.
        xname(list): the first is x field name in log file, and the second is for plot,
                     the third and forth values are low and high limit (optional).
        yname(list): see xname
        gens(list): generation to be plotted
        ws: workspace for output files
    """
    pareto_data = OrderedDict()
    acc_pop_size = OrderedDict()
    for k, v in method_paths.items():
        v = v + os.path.sep + 'runtime.log'
        pareto_data[k], acc_pop_size[k] = read_pareto_points_from_txt(v, sce_name, xname, yname)
    # print(pareto_data)
    ylabel_str = yname[1]
    xlabel_str = xname[1]
    file_name = '-'.join(list(pareto_data.keys()))

    plt.rcParams['xtick.direction'] = 'out'
    plt.rcParams['ytick.direction'] = 'out'
    plt.rcParams['font.family'] = 'Times New Roman'

    # Check if xname or yname contains Chinese characters
    zhPattern = re.compile(u'[\u4e00-\u9fa5]+')
    if zhPattern.search(xname[1]) or zhPattern.search(yname[1]):
        plt.rcParams['font.family'] = 'SimSun'  # 宋体

    markers = ['.', '+', '*', 'x', 'd', 'h', 's', '<', '>']
    colors = ['r', 'b', 'g', 'c', 'm', 'y', 'k', 'k', 'k']
    linestyles = ['-', '--', '-.', ':']
    # plot accumulate pop size
    fig, ax = plt.subplots(figsize=(9, 8))
    mark_idx = 0
    for method, gen_popsize in acc_pop_size.items():
        xdata = gen_popsize[0]
        ydata = gen_popsize[1]
        print(ydata)
        print('Evaluated pop size: %s - %d' % (method, ydata[-1]))
        plt.plot(xdata, ydata, linestyle=linestyles[mark_idx], color='black',
                 label=method, linewidth=2)
        mark_idx += 1
    plt.legend(fontsize=24, loc=2)
    xaxis = plt.gca().xaxis
    yaxis = plt.gca().yaxis
    for xlebal in xaxis.get_ticklabels():
        xlebal.set_fontsize(20)
    for ylebal in yaxis.get_ticklabels():
        ylebal.set_fontsize(20)
    plt.xlabel('Generation count', fontsize=20)
    plt.ylabel('Total number of simulated individuals', fontsize=20)
    ax.set_xlim(left=0, right=ax.get_xlim()[1] + 2)
    plt.tight_layout()
    fpath = ws + os.path.sep + file_name + '_popsize'
    plt.savefig(fpath + '.png', dpi=300)
    plt.savefig(fpath + '.eps', dpi=300)
    print('%s saved!' % fpath)
    # close current plot in case of 'figure.max_open_warning'
    plt.cla()
    plt.clf()
    plt.close()

    # plot Pareto points of all generations
    mark_idx = 0
    for method, gen_popsize in pareto_data.items():
        fig, ax = plt.subplots(figsize=(9, 8))
        xdata = list()
        ydata = list()
        for gen, gendata in gen_popsize.items():
            xdata += gen_popsize[gen][xname[0]]
            ydata += gen_popsize[gen][yname[0]]
        plt.scatter(xdata, ydata, marker=markers[mark_idx], s=20,
                    color=colors[mark_idx], label=method)
        mark_idx += 1
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
        plt.tight_layout()
        fpath = ws + os.path.sep + method + '-Pareto'
        plt.savefig(fpath + '.png', dpi=300)
        plt.savefig(fpath + '.eps', dpi=300)
        print('%s saved!' % fpath)
        # close current plot in case of 'figure.max_open_warning'
        plt.cla()
        plt.clf()
        plt.close()

    # plot comparision of Pareto fronts
    for gen in gens:
        fig, ax = plt.subplots(figsize=(9, 8))
        mark_idx = 0
        gen_existed = True
        for method, gen_popsize in pareto_data.items():
            if gen not in gen_popsize:
                gen_existed = False
                break
            xdata = gen_popsize[gen][xname[0]]
            ydata = gen_popsize[gen][yname[0]]
            plt.scatter(xdata, ydata, marker=markers[mark_idx], s=100,
                        color=colors[mark_idx], label=method)
            mark_idx += 1
        if not gen_existed:
            plt.cla()
            plt.clf()
            plt.close()
            continue

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
            if len(xname) >= 4:  # and curxlim[1] > xname[3]:
                ax.set_xlim(right=xname[3])
        curylim = ax.get_ylim()
        if len(yname) >= 3:
            if curylim[0] < yname[2]:
                ax.set_ylim(bottom=yname[2])
            if len(yname) >= 4:  # and curylim[1] > yname[3]:
                ax.set_ylim(top=yname[3])

        plt.legend(fontsize=24, loc=4)  # loc 2: upper left, 4: lower right
        plt.tight_layout()
        fpath = ws + os.path.sep + file_name + '-gen' + str(gen)
        plt.savefig(fpath + '.png', dpi=300)
        plt.savefig(fpath + '.eps', dpi=300)
        print('%s saved!' % fpath)
        # close current plot in case of 'figure.max_open_warning'
        plt.cla()
        plt.clf()
        plt.close()


def read_hypervolume(hypervlog):
    if not os.path.exists(hypervlog):
        return None, None, None
    x = list()
    nmodel = list()
    hyperv = list()
    with open(hypervlog, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    for line in lines:
        values = StringClass.extract_numeric_values_from_string(line)
        if values is None:
            continue
        if len(values) < 2:
            continue
        x.append(int(values[0]))
        hyperv.append(values[-1])
        if len(values) >= 3:
            nmodel.append(int(values[1]))
    return x, hyperv, nmodel


def plot_hypervolume_single(hypervlog, ws=None, cn=False):
    """Plot hypervolume and the newly executed models of each generation.

    Args:
        hypervlog: Full path of the hypervolume log.
        ws: (Optional) Full path of the destination directory
        cn: (Optional) Use Chinese
    """
    x, hyperv, nmodel = read_hypervolume(hypervlog)

    plt.rcParams['xtick.direction'] = 'out'
    plt.rcParams['ytick.direction'] = 'out'
    plt.rcParams['font.family'] = 'Times New Roman'
    generation_str = 'Generation'
    hyperv_str = 'Hypervolume index'
    nmodel_str = 'New model evaluations'
    if cn:
        plt.rcParams['font.family'] = 'SimSun'  # 宋体
        generation_str = u'进化代数'
        hyperv_str = u'Hypervolume 指数'
        nmodel_str = u'新运行模型次数'
    linestyles = ['-', '--', '-.', ':']
    markers = ['o', 's', 'v', '*']
    fig, ax = plt.subplots(figsize=(10, 6))
    mark_idx = 0
    p1 = ax.plot(x, hyperv, linestyle=linestyles[0], marker=markers[mark_idx],
                 color='black', label=hyperv_str, linewidth=2, markersize=4)
    mark_idx += 1
    plt.xlabel(generation_str)
    plt.ylabel(hyperv_str)
    ax.set_xlim(left=0, right=ax.get_xlim()[1])
    legends = p1

    plt.tight_layout()
    save_png_eps(plt, ws, 'hypervolume')

    if nmodel:
        # Add right Y-axis
        ax2 = ax.twinx()
        ax.tick_params(axis='x', which='both', bottom='on', top='off')
        ax2.tick_params(axis='y', length=5, width=2, which='major')
        ax2.set_ylabel(nmodel_str)
        p2 = ax2.plot(x, nmodel, linestyle=linestyles[0], marker=markers[mark_idx],
                      color='black', label=nmodel_str, linewidth=2, markersize=4)
        legends += p2

    legends_label = [l.get_label() for l in legends]
    ax.legend(legends, legends_label, loc='center right')

    plt.tight_layout()
    save_png_eps(plt, ws, 'hypervolume_modelruns')
    # close current plot in case of 'figure.max_open_warning'
    plt.cla()
    plt.clf()
    plt.close()


def plot_hypervolume(method_paths, ws, cn=False):
    """Plot hypervolume of multiple optimization methods

    Args:
        method_paths: Dict, key is method name and value is full path of the directory
        ws: Full path of the destination directory
        cn: (Optional) Use Chinese
    """
    hyperv = OrderedDict()
    for k, v in list(method_paths.items()):
        v = v + os.path.sep + 'hypervolume.txt'
        x = list()
        y = list()
        with open(v, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        for line in lines:
            values = StringClass.extract_numeric_values_from_string(line)
            if values is None:
                continue
            if len(values) < 2:
                continue
            x.append(int(values[0]))
            y.append(values[-1])

        if len(x) == len(y) > 0:
            hyperv[k] = [x[:], y[:]]
    plt.rcParams['xtick.direction'] = 'out'
    plt.rcParams['ytick.direction'] = 'out'
    plt.rcParams['font.family'] = 'Times New Roman'
    generation_str = 'Generation'
    hyperv_str = 'Hypervolume index'
    if cn:
        plt.rcParams['font.family'] = 'SimSun'  # 宋体
        generation_str = u'进化代数'
        hyperv_str = u'Hypervolume 指数'
    linestyles = ['-', '--', '-.', ':']
    # plot accumulate pop size
    fig, ax = plt.subplots(figsize=(10, 8))
    mark_idx = 0
    for method, gen_hyperv in hyperv.items():
        xdata = gen_hyperv[0]
        ydata = gen_hyperv[1]
        plt.plot(xdata, ydata, linestyle=linestyles[mark_idx], color='black',
                 label=method, linewidth=2)
        mark_idx += 1
    plt.legend(fontsize=24, loc=2)
    xaxis = plt.gca().xaxis
    yaxis = plt.gca().yaxis
    for xlebal in xaxis.get_ticklabels():
        xlebal.set_fontsize(20)
    for ylebal in yaxis.get_ticklabels():
        ylebal.set_fontsize(20)
    plt.xlabel(generation_str, fontsize=20)
    plt.ylabel(hyperv_str, fontsize=20)
    ax.set_xlim(left=0, right=ax.get_xlim()[1] + 2)
    plt.tight_layout()
    save_png_eps(plt, ws, 'hypervolume')
    # close current plot in case of 'figure.max_open_warning'
    plt.cla()
    plt.clf()
    plt.close()
