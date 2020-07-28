"""Entrance of Postprocess for SEIMS.

    @author   : Liangjun Zhu, Huiran Gao

    @changelog:
    - 17-08-17  - lj - redesign and rewrite the plotting program.
    - 18-02-09  - lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from postprocess.config import parse_ini_configuration
from postprocess.plot_timeseries import TimeSeriesPlots


def main():
    """Main workflow."""
    cfg = parse_ini_configuration()

    TimeSeriesPlots(cfg).generate_plots()


if __name__ == "__main__":
    main()
