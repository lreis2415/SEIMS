#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Visualization of Scenarios, which is relative independent with SA.

    @author   : Liangjun Zhu, Huiran Gao

    @changelog:
    - 16-09-12  hr - initial implementation.
    - 17-08-18  lj - reorganization.
    - 18-02-09  lj - compatible with Python3.
    - 18-08-24  lj - ReDesign pareto graph and hypervolume graph.
    - 18-10-31  lj - Add type hints based on typing package.
"""
from __future__ import absolute_import, unicode_literals
from future.utils import viewitems

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
from matplotlib import cm
import numpy
import itertools
from pygeoc.utils import StringClass
import re

from typing import List, Optional, Union, Dict, AnyStr, Any
from utility import save_png_eps, get_optimal_bounds

LFs = ['\r', '\n', '\r\n']


def plot_2d_scatter(xlist,  # type: List[float] # X coordinates
                    ylist,  # type: List[float] # Y coordinates
                    title,  # type: AnyStr # Main title of the figure
                    xlabel,  # type: AnyStr # X-axis label
                    ylabel,  # type: AnyStr # Y-axis label
                    ws,  # type: AnyStr # Full path of the destination directory
                    filename,  # type: AnyStr # File name without suffix (e.g., jpg, eps)
                    subtitle='',  # type: AnyStr # Subtitle
                    cn=False,  # type: bool # Use Chinese or not
                    xmin=None,  # type: Optional[float] # Left min X value
                    xmax=None,  # type: Optional[float] # Right max X value
                    xstep=None,  # type: Optional[float] # X interval
                    ymin=None,  # type: Optional[float] # Bottom min Y value
                    ymax=None,  # type: Optional[float] # Up max Y value
                    ystep=None  # type: Optional[float] # Y interval
                    ):
    # type: (...) -> None
    """Scatter plot of 2D points.

    Todo: The size of the point may be vary with the number of points.
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


def plot_3d_scatter(xlist,  # type: List[float] # X coordinates
                    ylist,  # type: List[float] # Y coordinates
                    zlist,  # type: List[float] # Z coordinates
                    title,  # type: AnyStr # Main title of the figure
                    xlabel,  # type: AnyStr # X-axis label
                    ylabel,  # type: AnyStr # Y-axis label
                    zlabel,  # type: AnyStr # Z-axis label
                    ws,  # type: AnyStr # Full path of the destination directory
                    filename,  # type: AnyStr # File name without suffix (e.g., jpg, eps)
                    subtitle='',  # type: AnyStr # Subtitle
                    cn=False,  # type: bool # Use Chinese or not
                    xmin=None,  # type: Optional[float] # Left min X value
                    xmax=None,  # type: Optional[float] # Right max X value
                    ymin=None,  # type: Optional[float] # Bottom min Y value
                    ymax=None,  # type: Optional[float] # Up max Y value
                    zmin=None,  # type: Optional[float] # Min Z value
                    zmax=None,  # type: Optional[float] # Max Z value
                    xstep=None,  # type: Optional[float] # X interval
                    ystep=None,  # type: Optional[float] # Y interval
                    zstep=None  # type: Optional[float] # Z interval
                    ):
    # type: (...) -> None
    """Scatter plot of 3D points.
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


def plot_pareto_front_single(data,
                             # type: Union[numpy.ndarray, List[List[float]]] # [nrows * ncols] array
                             labels,
                             # type: List[AnyStr] # Labels (axis names) with length of ncols
                             ws,  # type: AnyStr # Full path of the destination directory
                             gen_id,  # type: Union[int, AnyStr] # Generation ID
                             title,  # type: AnyStr # Main title of the figure
                             lowers=None,
                             # type: Optional[numpy.ndarray, List[float]] # Lower values of each axis
                             uppers=None,
                             # type: Optional[numpy.ndarray, List[float]] # Higher values of each axis
                             steps=None,
                             # type: Optional[numpy.ndarray, List[float]] # Intervals of each axis
                             cn=False  # type: bool # Use Chinese or not
                             ):
    # type: (...) -> bool
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
        return False
    if len(labels) != axis_size:
        print('Error: The size of fitness values and labels are not consistent!')
        return False
    if lowers is not None and len(lowers) != axis_size:
        print('Warning: The size of fitness values and lowers are not consistent!')
        lowers = None
    if uppers is not None and len(uppers) != axis_size:
        print('Warning: The size of fitness values and uppers are not consistent!')
        uppers = None
    if steps is not None and len(steps) != axis_size:
        print('Warning: The size of fitness values and steps are not consistent!')
        steps = None

    if isinstance(gen_id, int):
        subtitle = '\nGeneration: %d, Population: %d' % (gen_id, pop_size)
        if cn:
            subtitle = u'\n代数: %d, 个体数: %d' % (gen_id, pop_size)
    else:
        subtitle = '\nAll generations, Population: %d' % pop_size
        if cn:
            subtitle = u'\n所有进化代数, 个体数: %d' % pop_size
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
        filename = 'Pareto_Gen_%s_Pop_%d' % (str(gen_id), pop_size)
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
            filename = 'Pareto_Gen_%s_Pop_%d' % (str(gen_id), pop_size)
            if cn:
                filename += '_cn'
            plot_3d_scatter(data[:, x_idx], data[:, y_idx], data[:, z_idx], maintitle,
                            labels[x_idx], labels[y_idx], labels[z_idx],
                            tmpws, filename, subtitle, cn=cn,
                            xmin=x_min, xmax=x_max, ymin=y_min, ymax=y_max, zmin=z_min, zmax=z_max,
                            xstep=x_step, ystep=y_step, zstep=z_step)
    return True


def plot_pareto_fronts_multigenerations(data,
                                        # type: Dict[Union[AnyStr, int], Union[List[List[float]], numpy.ndarray]]
                                        labels,
                                        # type: List[AnyStr] # Labels (axis names) with length of ncols
                                        ws,  # type: AnyStr # Full path of the destination directory
                                        gen_ids,  # type: List[int] # Selected generation IDs
                                        title,  # type: AnyStr # Main title of the figure
                                        lowers=None,
                                        # type: Optional[numpy.ndarray, List[float]] # Lower values of each axis
                                        uppers=None,
                                        # type: Optional[numpy.ndarray, List[float]] # Higher values of each axis
                                        steps=None,
                                        # type: Optional[numpy.ndarray, List[float]] # Intervals of each axis
                                        cn=False  # type: bool # Use Chinese or not
                                        ):
    # type: (...) -> None
    """Plot Pareto fronts of selected generations."""
    filename = 'Pareto_Generations_%s' % ('-'.join(repr(i) for i in gen_ids))
    if cn:
        filename += '_cn'
        plt.rcParams['font.family'] = 'SimSun'  # 宋体
    fig, ax = plt.subplots(figsize=(9, 8))
    # ColorMaps: https://matplotlib.org/tutorials/colors/colormaps.html
    cmap = cm.get_cmap('gist_heat')  # one of the sequential colormaps
    for idx, gen in enumerate(gen_ids):
        if gen not in data:
            continue
        xdata = numpy.array(data[gen])[:, 0]  # first column
        ydata = numpy.array(data[gen])[:, 1]  # second column
        plt.scatter(xdata, ydata, marker='.', s=100,
                    color=cmap(0.8 * (len(gen_ids) - idx) / len(gen_ids)),
                    label='Generation %d' % gen)
    xaxis = plt.gca().xaxis
    yaxis = plt.gca().yaxis
    for xlebal in xaxis.get_ticklabels():
        xlebal.set_fontsize(20)
    for ylebal in yaxis.get_ticklabels():
        ylebal.set_fontsize(20)
    plt.xlabel(labels[0], fontsize=20)
    plt.ylabel(labels[1], fontsize=20)
    # set xy axis limit
    if lowers is not None:
        ax.set_xlim(left=lowers[0])
        ax.set_ylim(bottom=lowers[1])
    if uppers is not None:
        ax.set_xlim(right=uppers[0])
        ax.set_ylim(top=uppers[1])
    if steps is not None:
        xmin, xmax = plt.xlim()
        plt.xticks(numpy.arange(xmin, xmax + steps[0] * 0.99, step=steps[0]))
        ymin, ymax = plt.ylim()
        plt.yticks(numpy.arange(ymin, ymax + steps[1] * 0.99, step=steps[1]))

    plt.legend(fontsize=16, loc=2)  # loc 2: upper left, 4: lower right, 0: best
    plt.tight_layout()
    save_png_eps(plt, ws, filename)

    # close current plot in case of 'figure.max_open_warning'
    plt.cla()
    plt.clf()
    plt.close()


def read_pareto_points_from_txt(txt_file, sce_name, headers, labels=None):
    # type: (AnyStr, AnyStr, List[AnyStr], Optional[List[AnyStr]]) -> (Dict[int, Union[List, numpy.ndarray]], Dict[int, int])
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
    # type: (AnyStr, AnyStr) -> (List[int], List[int])
    """Read the population size of each generations."""
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


def plot_pareto_fronts_multiple(method_paths,  # type: Dict[AnyStr, AnyStr]
                                sce_name,  # type: AnyStr
                                xname,
                                # type: List[AnyStr, AnyStr, Optional[float], Optional[float]]
                                yname,
                                # type: List[AnyStr, AnyStr, Optional[float], Optional[float]]
                                gens,  # type: List[int]
                                ws  # type: AnyStr
                                ):
    # type: (...) -> None
    """
    Plot Pareto fronts of different methods at a same generation for comparision.

    Args:
        method_paths(OrderedDict): key is method name (which also displayed in legend), value is file path.
        sce_name(str): Scenario ID field name.
        xname(list): the first is x field name in log file, and the second is for plot,
                     the third and forth values are low and high limits (optional).
        yname(list): see xname
        gens(list): generation to be plotted
        ws: workspace for output files
    """
    pareto_data = OrderedDict()  # type: OrderedDict[int, Union[List, numpy.ndarray]]
    acc_pop_size = OrderedDict()  # type: Dict[int, int]
    for k, v in viewitems(method_paths):
        v = v + os.path.sep + 'runtime.log'
        pareto_data[k], acc_pop_size[k] = read_pareto_points_from_txt(v, sce_name, xname)
    # print(pareto_data)
    plot_pareto_fronts(pareto_data, xname[1:], yname[1:], gens, ws)


def plot_pareto_fronts(pareto_data,
                       # type: Dict[AnyStr, Dict[Union[AnyStr, int], Union[List[List[float]], numpy.ndarray]]]
                       xname,  # type: List[AnyStr, Optional[float], Optional[float]]
                       yname,  # type: List[AnyStr, Optional[float], Optional[float]]
                       gens,  # type: List[int]
                       ws  # type: AnyStr
                       ):
    # type: (...) -> None
    """
    Plot Pareto fronts of different methods at a same generation for comparision.

    Args:
        pareto_data(OrderedDict)
        xname(list): the first is x-axis name of plot,
                     the second and third values are low and high limits (optional).
        yname(list): see xname
        gens(list): generation to be plotted
        ws: workspace for output files
    """
    if len(xname) < 1 or len(yname) < 1:
        xname = ['x-axis']
        yname = ['y-axis']
    ylabel_str = yname[0]
    xlabel_str = xname[0]
    file_name = '-'.join(list(pareto_data.keys()))

    plt.rcParams['xtick.direction'] = 'out'
    plt.rcParams['ytick.direction'] = 'out'
    plt.rcParams['font.family'] = 'Palatino Linotype'  # 'Times New Roman'

    # Check if xname or yname contains Chinese characters
    zhPattern = re.compile(u'[\u4e00-\u9fa5]+')
    if zhPattern.search(xname[0]) or zhPattern.search(yname[0]):
        plt.rcParams['font.family'] = 'SimSun'  # 宋体

    markers = ['.', '*', '+', 'x', 'd', 'h', 's', '<', '>']
    colors = ['r', 'b', 'g', 'c', 'm', 'y', 'k', 'k', 'k']
    # linestyles = ['-', '--', '-.', ':']
    # # plot accumulate pop size
    # fig, ax = plt.subplots(figsize=(9, 8))
    # mark_idx = 0
    # for method, gen_popsize in acc_pop_size.items():
    #     xdata = gen_popsize[0]
    #     ydata = gen_popsize[1]
    #     print(ydata)
    #     print('Evaluated pop size: %s - %d' % (method, ydata[-1]))
    #     plt.plot(xdata, ydata, linestyle=linestyles[mark_idx], color='black',
    #              label=method, linewidth=2)
    #     mark_idx += 1
    # plt.legend(fontsize=24, loc=2)
    # xaxis = plt.gca().xaxis
    # yaxis = plt.gca().yaxis
    # for xlebal in xaxis.get_ticklabels():
    #     xlebal.set_fontsize(20)
    # for ylebal in yaxis.get_ticklabels():
    #     ylebal.set_fontsize(20)
    # plt.xlabel('Generation count', fontsize=20)
    # plt.ylabel('Total number of simulated individuals', fontsize=20)
    # ax.set_xlim(left=0, right=ax.get_xlim()[1] + 2)
    # plt.tight_layout()
    # fpath = ws + os.path.sep + file_name + '_popsize'
    # plt.savefig(fpath + '.png', dpi=300)
    # plt.savefig(fpath + '.eps', dpi=300)
    # print('%s saved!' % fpath)
    # # close current plot in case of 'figure.max_open_warning'
    # plt.cla()
    # plt.clf()
    # plt.close()

    # plot Pareto points of all generations
    # mark_idx = 0
    # for method, gen_popsize in pareto_data.items():
    #     fig, ax = plt.subplots(figsize=(9, 8))
    #     xdata = list()
    #     ydata = list()
    #     for gen, gendata in gen_popsize.items():
    #         xdata += gen_popsize[gen][xname[0]]
    #         ydata += gen_popsize[gen][yname[0]]
    #     plt.scatter(xdata, ydata, marker=markers[mark_idx], s=20,
    #                 color=colors[mark_idx], label=method)
    #     mark_idx += 1
    #     xaxis = plt.gca().xaxis
    #     yaxis = plt.gca().yaxis
    #     for xlebal in xaxis.get_ticklabels():
    #         xlebal.set_fontsize(20)
    #     for ylebal in yaxis.get_ticklabels():
    #         ylebal.set_fontsize(20)
    #     plt.xlabel(xlabel_str, fontsize=20)
    #     plt.ylabel(ylabel_str, fontsize=20)
    #     # set xy axis limit
    #     curxlim = ax.get_xlim()
    #     if len(xname) >= 3:
    #         if curxlim[0] < xname[2]:
    #             ax.set_xlim(left=xname[2])
    #         if len(xname) >= 4 and curxlim[1] > xname[3]:
    #             ax.set_xlim(right=xname[3])
    #     curylim = ax.get_ylim()
    #     if len(yname) >= 3:
    #         if curylim[0] < yname[2]:
    #             ax.set_ylim(bottom=yname[2])
    #         if len(yname) >= 4 and curylim[1] > yname[3]:
    #             ax.set_ylim(top=yname[3])
    #     plt.tight_layout()
    #     fpath = ws + os.path.sep + method + '-Pareto'
    #     plt.savefig(fpath + '.png', dpi=300)
    #     plt.savefig(fpath + '.eps', dpi=300)
    #     print('%s saved!' % fpath)
    #     # close current plot in case of 'figure.max_open_warning'
    #     plt.cla()
    #     plt.clf()
    #     plt.close()

    # plot comparision of Pareto fronts

    # Get max. and mix. values
    max_x = None
    min_x = None
    max_y = None
    min_y = None
    for method, cur_pareto_data in viewitems(pareto_data):
        if 'min' in cur_pareto_data:
            if min_x is None or min_x > cur_pareto_data['min'][0]:
                min_x = cur_pareto_data['min'][0]
            if min_y is None or min_y > cur_pareto_data['min'][1]:
                min_y = cur_pareto_data['min'][1]
        if 'max' in cur_pareto_data:
            if max_x is None or max_x < cur_pareto_data['max'][0]:
                max_x = cur_pareto_data['max'][0]
            if max_y is None or max_y < cur_pareto_data['max'][1]:
                max_y = cur_pareto_data['max'][1]
    newxname = xname[:]
    newyname = yname[:]
    if min_x is not None and max_x is not None and len(newxname) < 2:
        newxname += get_optimal_bounds(min_x, max_x)
    if min_y is not None and max_y is not None and len(newyname) < 2:
        newyname += get_optimal_bounds(min_y, max_y)

    for gen in gens:
        fig, ax = plt.subplots(figsize=(9, 8))
        mark_idx = 0
        gen_existed = True
        for method, gen_data in viewitems(pareto_data):
            if gen not in gen_data:
                gen_existed = False
                break
            xdata = numpy.array(gen_data[gen])[:, 0]  # first column
            ydata = numpy.array(gen_data[gen])[:, 1]  # second column
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
        if len(newxname) >= 3:
            ax.set_xlim(left=newxname[1])
            ax.set_xlim(right=newxname[2])

        curylim = ax.get_ylim()
        if len(newyname) >= 3:
            ax.set_ylim(bottom=newyname[1])
            ax.set_ylim(top=newyname[2])

        plt.legend(fontsize=16, loc=4)  # loc 2: upper left, 4: lower right
        plt.tight_layout()
        save_png_eps(plt, ws, 'gen%d' % gen)

        # close current plot in case of 'figure.max_open_warning'
        plt.cla()
        plt.clf()
        plt.close()


def read_hypervolume(hypervlog):
    # type: (AnyStr) -> (List[int], List[float], List[float])
    """Read hypervolume data from file."""
    if not os.path.exists(hypervlog):
        print('Error: The hypervolume log file %s is not existed!' % hypervlog)
        return None, None, None
    x = list()  # Generation No.
    nmodel = list()  # Newly executed models count
    hyperv = list()  # Hypervolume value
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
    # type: (AnyStr, Optional[AnyStr], bool) -> bool
    """Plot hypervolume and the newly executed models of each generation.

    Args:
        hypervlog: Full path of the hypervolume log.
        ws: (Optional) Full path of the destination directory
        cn: (Optional) Use Chinese
    """
    x, hyperv, nmodel = read_hypervolume(hypervlog)
    if not x or not hyperv:
        print('Error: No available hypervolume data loaded!')
        return False

    plt.rcParams['xtick.direction'] = 'out'
    plt.rcParams['ytick.direction'] = 'out'
    plt.rcParams['font.family'] = 'Palatino Linotype'  # 'Times New Roman'
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
    if ws is None:
        ws = os.path.dirname(hypervlog)
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
    ax.legend(legends, legends_label, fontsize=16, loc='center right')

    plt.tight_layout()
    save_png_eps(plt, ws, 'hypervolume_modelruns')
    # close current plot in case of 'figure.max_open_warning'
    plt.cla()
    plt.clf()
    plt.close()

    return True


def plot_hypervolume_multiple(method_paths, ws, cn=False):
    # type: (Dict[AnyStr, AnyStr], AnyStr, bool) -> bool
    """Plot hypervolume of multiple optimization methods

    Args:
        method_paths: Dict, key is method name and value is full path of the directory
        ws: Full path of the destination directory
        cn: (Optional) Use Chinese
    """
    hyperv = OrderedDict()  # type: Dict[AnyStr, List[List[int], List[float]]]
    for k, v in viewitems(method_paths):
        v = v + os.path.sep + 'hypervolume.txt'
        genids, nmodels, hv = read_hypervolume(v)
        hyperv[k] = [genids[:], hv[:]]
    return plot_hypervolumes(hyperv, ws, cn)


def plot_hypervolumes(hyperv, ws, cn=False):
    # type: (Dict[AnyStr, Optional[int, float, List[List[int], List[float]]]], AnyStr, bool) -> bool
    """Plot hypervolume of multiple optimization methods

    Args:
        hyperv: Dict, key is method name and value is generation IDs list and hypervolumes list
                      Optionally, key-values of 'bottom' and 'top' are allowed.
        ws: Full path of the destination directory
        cn: (Optional) Use Chinese
    """
    plt.rcParams['xtick.direction'] = 'out'
    plt.rcParams['ytick.direction'] = 'out'
    plt.rcParams['font.family'] = 'Palatino Linotype'  # 'Times New Roman'
    generation_str = 'Generation'
    hyperv_str = 'Hypervolume index'
    if cn:
        plt.rcParams['font.family'] = 'SimSun'  # 宋体
        generation_str = u'进化代数'
        hyperv_str = u'Hypervolume 指数'
    # Line styles: https://matplotlib.org/gallery/lines_bars_and_markers/line_styles_reference.html
    linestyles = ['-', '--', '-.', ':']
    # plot accumulate pop size
    fig, ax = plt.subplots(figsize=(9, 8))
    mark_idx = 0
    for method, gen_hyperv in viewitems(hyperv):
        if not isinstance(gen_hyperv, list):
            continue
        xdata = gen_hyperv[0]
        ydata = gen_hyperv[1]
        plt.plot(xdata, ydata, linestyle=linestyles[mark_idx], color='black',
                 label=method, linewidth=2)
        mark_idx += 1
    plt.legend(fontsize=16, loc=4)
    xaxis = plt.gca().xaxis
    yaxis = plt.gca().yaxis
    for xlebal in xaxis.get_ticklabels():
        xlebal.set_fontsize(20)
    for ylebal in yaxis.get_ticklabels():
        ylebal.set_fontsize(20)
    plt.xlabel(generation_str, fontsize=20)
    plt.ylabel(hyperv_str, fontsize=20)
    ax.set_xlim(left=0, right=ax.get_xlim()[1] + 2)
    if 'bottom' in hyperv:
        ax.set_ylim(bottom=hyperv['bottom'])
    if 'top' in hyperv:
        ax.set_ylim(top=hyperv['top'])
    plt.tight_layout()
    save_png_eps(plt, ws, 'hypervolume')
    # close current plot in case of 'figure.max_open_warning'
    plt.cla()
    plt.clf()
    plt.close()

    return True
