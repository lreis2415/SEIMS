#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Common used functions for plotting based on matplotlib.
    @author   : Liangjun Zhu
    @changelog: 18-10-29 - lj - Extract from other packages.
"""
from __future__ import absolute_import

import os
import sys
import matplotlib as mpl
if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    mpl.use('Agg', warn=False)
import matplotlib.pyplot as plt

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import StringClass, FileClass, MathClass, UtilClass


def save_png_eps(plot, wp, name):
    # type: (plt, str, str) -> None
    """Save figures, both png and eps formats"""
    eps_dir = wp + os.path.sep + 'eps'
    pdf_dir = wp + os.path.sep + 'pdf'
    UtilClass.mkdir(eps_dir)
    UtilClass.mkdir(pdf_dir)
    for figpath in [wp + os.path.sep + name + '.png',
                    eps_dir + os.path.sep + name + '.eps',
                    pdf_dir + os.path.sep + name + '.pdf']:
        plot.savefig(figpath, dpi=300)


