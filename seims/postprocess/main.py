#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Entrance of Postprocess for SEIMS.
    @author   : Liangjun Zhu, Huiran Gao
    @changelog: 17-08-17  lj - redesign and rewrite the plotting program.\n
                18-02-09  lj - compatible with Python3.\n
"""
from __future__ import absolute_import

from postprocess.config import parse_ini_configuration
from postprocess.plot_timeseries import TimeSeriesPlots


def main():
    """Main workflow."""
    cfg = parse_ini_configuration()

    TimeSeriesPlots(cfg).generate_plots()


if __name__ == "__main__":
    main()
