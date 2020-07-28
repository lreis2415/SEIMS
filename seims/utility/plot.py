"""Common used functions for plotting based on matplotlib.

    @author   : Liangjun Zhu

    @changelog:
    - 18-10-29 - lj - Extract from other packages.
    - 18-11-18 - lj - Add getting value bounds related functions.
    = 19-01-07 - lj - Add PlotConfig for basic plot settings for matplotlib
"""
from __future__ import absolute_import, unicode_literals

import os
import sys
import math
from decimal import localcontext, Decimal, ROUND_HALF_UP
import matplotlib as mpl

if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    mpl.use('Agg', warn=False)
import matplotlib.pyplot as plt
from matplotlib import font_manager

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from configparser import ConfigParser
from typing import AnyStr, Union, List, Optional
from pygeoc.utils import UtilClass, StringClass


class PlotConfig(object):
    """Configuration for plots based on matplotlib."""

    def __init__(self, cf=None):
        # type: (Optional[ConfigParser]) -> None
        """Get parameters from ConfigParser object."""
        self.fmts = ['png']
        self.font_name = 'Times New Roman'
        self.plot_cn = False
        self.title_fsize = 18
        self.legend_fsize = 14
        self.tick_fsize = 12
        self.axislabel_fsize = 14
        self.label_fsize = 16
        self.dpi = 300
        section_name = 'OPTIONAL_MATPLOT_SETTINGS'
        if cf is None or not cf.has_section(section_name):
            return
        if cf.has_option(section_name, 'figure_formats'):
            fmts_strings = cf.get(section_name, 'figure_formats')
            fmts_strings = fmts_strings.lower()
            fmts_list = StringClass.split_string(fmts_strings, [',', ';', '-'])
            for fmt in fmts_list:
                if fmt not in ['png', 'tif', 'jpg', 'pdf', 'eps', 'svg', 'ps']:
                    continue
                if fmt not in self.fmts:
                    self.fmts.append(fmt)
        if cf.has_option(section_name, 'font_title'):
            font_name = cf.get(section_name, 'font_title')
            if font_manager.findfont(font_manager.FontProperties(family=font_name)):
                self.font_name = font_name
            else:
                print('Warning: The specified font title %s can not be found!'
                      'Please copy the .ttf font file to the directory of'
                      'Lib/site-packages/matplotlib/mpl-data/fonts/ttf, '
                      'rebuild the font cache by font_manager._rebuild(), '
                      'and rerun this script.' % font_name)
        if cf.has_option(section_name, 'lang_cn'):
            self.plot_cn = cf.getboolean(section_name, 'lang_cn')
        if cf.has_option(section_name, 'title_fontsize'):
            self.title_fsize = cf.getint(section_name, 'title_fontsize')
        if cf.has_option(section_name, 'legend_fontsize'):
            self.legend_fsize = cf.getint(section_name, 'legend_fontsize')
        if cf.has_option(section_name, 'ticklabel_fontsize'):
            self.tick_fsize = cf.getint(section_name, 'ticklabel_fontsize')
        if cf.has_option(section_name, 'axislabel_fontsize'):
            self.axislabel_fsize = cf.getint(section_name, 'axislabel_fontsize')
        if cf.has_option(section_name, 'label_fontsize'):
            self.label_fsize = cf.getint(section_name, 'label_fontsize')
        if cf.has_option(section_name, 'dpi'):
            self.dpi = cf.getint(section_name, 'dpi')


def save_png_eps(plot, wp, name, plot_cfg=None):
    # type: (plt, AnyStr, AnyStr, Optional[PlotConfig]) -> None
    """Save figures, both png and eps formats"""
    # plot.tight_layout()
    if plot_cfg is None:
        plot_cfg = PlotConfig()
    if plot_cfg.plot_cn:
        wp = wp + os.path.sep + 'cn'
        UtilClass.mkdir(wp)
    for fmt in plot_cfg.fmts:
        fmt_dir = wp + os.path.sep + fmt
        UtilClass.mkdir(fmt_dir)
        figpath = fmt_dir + os.path.sep + name + '.' + fmt
        plot.savefig(figpath, dpi=plot_cfg.dpi)


def round_half_up(value, ndigit=0):
    """Since Python builtin function round() cannot properly round up by half,
     use decimal module instead..

    References:
        https://stackoverflow.com/questions/33019698/how-to-properly-round-up-half-float-numbers-in-python
    """
    with localcontext() as ctx:
        ctx.rounding = ROUND_HALF_UP
        if ndigit == 0:
            return float(Decimal(value).to_integral_value()) + 0.0
        return float(Decimal(value * 10 ** ndigit).to_integral_value()) * 10 ** (-ndigit) + 0.0


def magnitude(value):
    # type: (Union[int, float]) -> int
    """Get the order of magnitude of a numeric value.

    Examples:
        >>> magnitude(-0.0125)
        -2
        >>> magnitude(0.125)
        -1
        >>> magnitude(0.12)
        -1
        >>> magnitude(0.1)
        -1
        >>> magnitude(0.0)
        0
        >>> magnitude(3.5)
        0
        >>> magnitude(11)
        1
        >>> magnitude(111)
        2
    """
    if value == 0:
        return 0
    return int(math.floor(math.log10(abs(value))))


def get_bound(value, up=False):
    # type: (Union[int, float], bool) -> List[Union[int, float]]
    """Calculate the optimal up or low bound.

    Examples:
        >>> get_bound(0.00175)  # order: -3 -> ndigits: [3] + [0.0]
        [0.001, 0.0]
        >>> get_bound(0.00175, up=True)  # order: -3 -> ndigits: [3] + [10^-2]
        [0.002, 0.01]
        >>> get_bound(0.0125)  # order: -2 -> ndigits: [2] + [0.0]
        [0.01, 0.0]
        >>> get_bound(0.0125, up=True)  # order: -2 -> ndigits: [2] + [10^-1]
        [0.02, 0.1]
        >>> get_bound(0.1) # order: -1 -> ndigits: [1] + [0.0]
        [0.1, 0.0]
        >>> get_bound(0.1, up=True)  # order: -1 -> ndigits: [1] + [10^0]
        [0.2, 1.0]
        >>> get_bound(1.5)  # order: 0 -> ndigits: [0] + [0]
        [1.0, 0.0]
        >>> get_bound(1.5, up=True) # order: 0 -> ndigits: [0] + orders: [1]
        [2.0, 10.0]
        >>> get_bound(5.0)
        [5.0, 0.0]
        >>> get_bound(5.0, up=True)
        [6.0, 10.0]
        >>> get_bound(12.5)  # order: 1 ->, ndigits: [0, -1] + [0]
        [12.0, 10.0, 0.0]
        >>> get_bound(12.5, up=True)  # order: 1 ->, ndigits: [0, -1] + orders: [2]
        [13.0, 20.0, 100.0]
        >>> get_bound(125.5)  # order: 2 -> ndigits: [0, -1, -2] + orders: [1] + [0.0]
        [125.0, 120.0, 100.0, 10.0, 0.0]
        >>> get_bound(125.5, up=True)  # order: 2 -> ndigits: [0, -1, -2] + orders: [3]
        [126.0, 130.0, 200.0, 1000.0]
        >>> get_bound(988, up=True)  # order: 2 -> ndigits: [0, -1, -2] + orders: [3]
        [989.0, 990.0, 1000.0, 1000.0]
        >>> get_bound(-125.5)  # equals to -1 * get_bound(125.5, up=True)
        [-126.0, -130.0, -200.0, -1000.0]
        >>> get_bound(-125.5, up=True)
        [-125.0, -120.0, -100.0, -10.0, 0.0]

    Returns:
        List of bounds with the same order and higher (or lower) orders of the input value.
    """
    order = magnitude(value)
    if value < 0:
        return list(0.0 + -1 * v for v in get_bound(-1 * value, not up))
    if order < 0:
        return list(10 ** order * v for v in get_bound(10 ** (-order) * value, up))
    # now order is >= 0
    ndigits = list(range(-1 * order, 1))
    ndigits.reverse()
    if up:
        orders = [order + 1]
    else:
        orders = list(range(1, order))
        orders.reverse()
    appended = list()
    if not up:
        appended.append(0.0)
    bounds = list()
    for digit in ndigits:
        if up:
            cur_up = round_half_up(value + 0.5 * 10 ** (-digit), digit)
            # if magnitude(cur_up) != order:
            #     continue
            bounds.append(cur_up)
        else:
            cur_low = round_half_up(value - 0.5 * 10 ** (-digit), digit)
            # if len(bounds) >= 1 and bounds[-1] == cur_low:
            #     continue
            bounds.append(cur_low)
    bounds += list(1.0 * 10 ** o for o in orders)
    bounds += appended

    return bounds


def get_optimal_bounds(low_value, up_value):
    # type: (Union[int, float], Union[int, float]) -> (Union[int, float], Union[int, float])
    """Calculate the optimal bounds of given lower and upper values for plotting.

    Examples:
        >>> get_optimal_bounds(1.2, 5.5)
        (1.0, 6.0)
        >>> get_optimal_bounds(0.12, 0.55)  # doctest: +ELLIPSIS
        (0.1, 0.6...)
        >>> get_optimal_bounds(5, 158)
        (0.0, 160.0)
        >>> get_optimal_bounds(5, 58)
        (0.0, 60.0)
        >>> get_optimal_bounds(5, 55)
        (0.0, 56.0)
        >>> get_optimal_bounds(5, 89)
        (0.0, 90.0)
        >>> get_optimal_bounds(5, 121)
        (0.0, 130.0)
        >>> get_optimal_bounds(0.5, 58)
        (0.0, 60.0)
        >>> get_optimal_bounds(121, 288)
        (120.0, 290.0)
        >>> get_optimal_bounds(1210, 2880)
        (1200.0, 2900.0)
        >>> get_optimal_bounds(0.025, 0.11)
        (0.0, 0.2)
        >>> get_optimal_bounds(0.0025, 0.11)
        (0.0, 0.2)
        >>> get_optimal_bounds(0.00025, 0.11)
        (0.0, 0.2)
    """
    low_mag = magnitude(low_value)
    up_mag = magnitude(up_value)
    low_bounds = get_bound(low_value)
    up_bounds = get_bound(up_value, up=True)
    # print(low_bounds, up_bounds)
    if not low_bounds or not up_bounds:
        return low_value, up_value
    low = low_bounds[0]
    up = up_bounds[0]
    # Condition 1:
    if low_mag == up_mag:
        if low_mag >= 2:
            return low_bounds[low_mag - 1], up_bounds[up_mag - 1]
        return low, up
    # Condition 2:
    if low_mag <= -1 and up_mag - low_mag >= 1:
        if up_mag > 0:
            return low_bounds[-1], up_bounds[up_mag]
        return low_bounds[-1], up
    # Condition 3:
    if 0 <= low_mag <= 1 and up_mag - low_mag >= 1:
        if up_bounds[up_mag] - up_bounds[up_mag - 1] <= 2 * 10 ** (up_mag - 1):
            return low_bounds[-1], up_bounds[up_mag]
        return low_bounds[-1], up_bounds[up_mag - 1]

    return low, up


if __name__ == '__main__':
    # Run doctest in docstrings of Google code style
    # python -m doctest utils.py (only when doctest.ELLIPSIS is not specified)
    # or python utils.py -v
    # or py.test --doctest-module utils.py
    import doctest

    doctest.testmod()
