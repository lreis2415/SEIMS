#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Plot figures based on matplotlib for parameters sensitivity analysis.
    @author   : Liangjun Zhu
    @changelog: 18-1-15  lj - initial implementation.\n
"""
import math
import os

import matplotlib

if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    matplotlib.use('Agg', warn=False)
import matplotlib.pyplot as plt
from matplotlib.ticker import LinearLocator
from pygeoc.utils import UtilClass


plt.rcParams['font.family'] = ['Times New Roman']
plt.rcParams['axes.titlesize'] = 'small'
plt.rcParams['ytick.labelsize'] = 'x-small'
plt.rcParams['ytick.direction'] = 'out'
plt.rcParams['xtick.labelsize'] = 'x-small'
plt.rcParams['xtick.direction'] = 'out'


def save_png_eps(plot, wp, name):
    """Save figures, both png and eps formats"""
    png_dir = wp + os.sep + 'png'
    eps_dir = wp + os.sep + 'eps'
    UtilClass.mkdir(png_dir)
    UtilClass.mkdir(eps_dir)
    for figpath in [png_dir + os.sep + name + '.png', eps_dir + os.sep + name + '.eps']:
        plot.savefig(figpath, dpi=300)


def cal_row_col_num(tot):
    """determine the appropriate row and col number.
    Cols number decreases from 8 to 5 to figure out the most uniform row and col num.
    """
    col = 8
    if tot < col:
        col = tot
        return 1, col
    row = int(math.ceil(tot / 8.))
    for i in range(8, 4, -1):
        if tot % i == 0:
            return tot / i, i
    for i in range(8, 4, -1):
        divide = tot / i
        if tot % i > i / 2:
            row = divide + 1
            col = i
    return row, col


def sample_histograms(fig, input_sample, names, levels, param_dict):
    """Plot histograms as subplot.

    Args:
        fig:
        input_sample:
        names:
        levels:
        param_dict:

    Returns:
        subplot list.
    """
    num_vars = len(names)
    row, col = cal_row_col_num(num_vars)
    out = list()
    for var_idx in range(num_vars):
        ax = fig.add_subplot(row, col, var_idx + 1)
        out.append(ax.hist(input_sample[:, var_idx],
                           bins=levels,
                           normed=False,
                           label=None,
                           **param_dict))
        ax.get_yaxis().set_major_locator(LinearLocator(numticks=5))
        ax.get_xaxis().set_major_locator(LinearLocator(numticks=5))
        ax.set_title('%s' % (names[var_idx]))
        ax.tick_params(axis='x',  # changes apply to the x-axis
                       which='both',  # both major and minor ticks are affected
                       bottom='off',  # ticks along the bottom edge are off
                       top='off',  # ticks along the top edge are off
                       labelbottom='off')  # labels along the bottom edge are off)
        ax.tick_params(axis='y',  # changes apply to the y-axis
                       which='major',  # both major and minor ticks are affected
                       length=3,
                       right='off')
        if var_idx % col:  # labels along the left edge are off
            ax.tick_params(axis='y', labelleft='off')
    return out
