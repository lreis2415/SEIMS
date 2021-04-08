"""Plot figures based on matplotlib for parameters sensitivity analysis.

    @author   : Liangjun Zhu

    @changelog:
    - 18-01-15  - lj - initial implementation.
    - 18-02-09  - lj - compatible with Python3.
    - 19-01-07  - lj - incorporated with PlotConfig
"""
from __future__ import absolute_import, unicode_literals

import os
import sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import math
import numpy
import matplotlib

if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    matplotlib.use('Agg', warn=False)
import matplotlib.pyplot as plt
from matplotlib.ticker import LinearLocator

from utility import save_png_eps, PlotConfig

plt.rcParams['font.family'] = ['Times New Roman']
plt.rcParams['axes.titlesize'] = 'small'
plt.rcParams['ytick.labelsize'] = 'x-small'
plt.rcParams['ytick.direction'] = 'out'
plt.rcParams['xtick.labelsize'] = 'x-small'
plt.rcParams['xtick.direction'] = 'out'


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


def sample_histograms(input_sample, names, levels, outpath, outname, param_dict,
                      plot_cfg=None  # type: PlotConfig
                      ):
    """Plot histograms as subplot.

    Args:
        input_sample:
        names:
        levels:
        outpath:
        outname:
        param_dict:
        plot_cfg:

    Returns:
        subplot list.
    """
    fig = plt.figure()
    if plot_cfg is not None:
        plt.rcParams['font.family'] = plot_cfg.font_name
    num_vars = len(names)
    row, col = cal_row_col_num(num_vars)
    for var_idx in range(num_vars):
        ax = fig.add_subplot(row, col, var_idx + 1)
        print('%s: %.3f - %.3f, mean: %.3f' % (names[var_idx], min(input_sample[:, var_idx]),
                                               max(input_sample[:, var_idx]),
                                               numpy.average(input_sample[:, var_idx])))
        ax.hist(input_sample[:, var_idx], bins=levels, density=False, label=None, **param_dict)
        ax.get_yaxis().set_major_locator(LinearLocator(numticks=5))
        ax.get_xaxis().set_major_locator(LinearLocator(numticks=5))
        ax.set_title('%s' % (names[var_idx]), fontsize=plot_cfg.title_fsize)
        ax.tick_params(axis='x',  # changes apply to the x-axis
                       which='both',  # both major and minor ticks are affected
                       bottom=True,  # ticks along the bottom edge are off
                       top=False,  # ticks along the top edge are off
                       labelbottom=True  # labels along the bottom edge are off
                       )
        ax.tick_params(axis='y',  # changes apply to the y-axis
                       which='major',  # both major and minor ticks are affected
                       length=3,
                       right=False)
        if var_idx % col:  # labels along the left edge are off
            ax.tick_params(axis='y', labelleft=False)
    plt.tight_layout()
    save_png_eps(plt, outpath, outname, plot_cfg)
    # close current plot in case of 'figure.max_open_warning'
    plt.cla()
    plt.clf()
    plt.close()


def empirical_cdf(out_values, subsections, input_sample, names, levels,
                  outpath, outname, param_dict, plot_cfg=None):
    """Visualize the empirical cumulative distribution function(CDF)
    of the given variable (x) and subsections of y.

    """
    # prepare data
    if not isinstance(out_values, numpy.ndarray):
        out_values = numpy.array(out_values)
    out_max = numpy.max(out_values)
    out_min = numpy.min(out_values)
    if isinstance(subsections, int):
        if subsections <= 0:
            raise ValueError('subsections MUST be a integer greater than 0, or list.')
        step = (out_max - out_min) / subsections
        subsections = numpy.arange(out_min, out_max + step, step)
    if isinstance(subsections, list) and len(subsections) == 1:  # e.g., [0]
        section_pt = subsections[0]
        if out_min < section_pt < out_max:
            subsections = [out_min, section_pt, out_max]
        else:
            subsections = [out_min, out_max]
    labels = list()
    new_input_sample = list()
    for i in range(1, len(subsections)):
        decimal1 = 0 if int(subsections[i - 1]) == float(subsections[i - 1]) else 2
        decimal2 = 0 if int(subsections[i]) == float(subsections[i]) else 2
        if out_max == subsections[i] and out_min == subsections[i - 1]:
            labels.append('%s=<y<=%s' % ('{0:.{1}f}'.format(subsections[i - 1], decimal1),
                                         '{0:.{1}f}'.format(subsections[i], decimal2)))
            zone = numpy.where((subsections[i - 1] <= out_values) & (out_values <= subsections[i]))
        elif out_max == subsections[i]:
            labels.append('y>=%s' % '{0:.{1}f}'.format(subsections[i - 1], decimal1))
            zone = numpy.where(subsections[i - 1] <= out_values)
        elif out_min == subsections[i - 1]:
            labels.append('y<%s' % ('{0:.{1}f}'.format(subsections[i], decimal2)))
            zone = numpy.where(out_values < subsections[i])
        else:
            labels.append('%s=<y<%s' % ('{0:.{1}f}'.format(subsections[i - 1], decimal1),
                                        '{0:.{1}f}'.format(subsections[i], decimal2)))
            zone = numpy.where((subsections[i - 1] <= out_values) & (out_values < subsections[i]))
        new_input_sample.append(input_sample[zone, :][0])

    if plot_cfg is None:
        plot_cfg = PlotConfig()
    plt.rcParams['font.family'] = plot_cfg.font_name
    fig = plt.figure()

    num_vars = len(names)
    row, col = cal_row_col_num(num_vars)
    for var_idx in range(num_vars):
        ax = fig.add_subplot(row, col, var_idx + 1)
        for ii in range(len(labels) - 1, -1, -1):
            ax.hist(new_input_sample[ii][:, var_idx], bins=levels, density=True,
                    cumulative=True, label=labels[ii], **param_dict)
        ax.get_yaxis().set_major_locator(LinearLocator(numticks=5))
        ax.set_ylim(0, 1)
        ax.set_title('%s' % (names[var_idx]), fontsize=plot_cfg.title_fsize)
        ax.get_xaxis().set_major_locator(LinearLocator(numticks=3))
        ax.tick_params(axis='x',  # changes apply to the x-axis
                       which='both',  # both major and minor ticks are affected
                       bottom=True,  # ticks along the bottom edge are off
                       top=False,  # ticks along the top edge are off
                       labelbottom=True  # labels along the bottom edge are off
                       )
        ax.tick_params(axis='y',  # changes apply to the y-axis
                       which='major',  # both major and minor ticks are affected
                       length=3,
                       right=False)
        if var_idx % col:  # labels along the left edge are off
            ax.tick_params(axis='y', labelleft=False)
        if var_idx == 0:
            ax.legend(loc='lower right', fontsize=plot_cfg.legend_fsize, framealpha=0.8,
                      bbox_to_anchor=(1, 0),
                      borderaxespad=0.2, fancybox=True)
    plt.tight_layout()
    save_png_eps(plt, outpath, outname, plot_cfg)
    # close current plot in case of 'figure.max_open_warning'
    plt.cla()
    plt.clf()
    plt.close()
