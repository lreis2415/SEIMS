#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Entrance of Postprocess for SEIMS.
    @author   : Liangjun Zhu, Huiran Gao
    @changelog: 17-08-17  lj - redesign and rewrite the plotting program.\n
"""

from config import parse_ini_configuration
from plot_timeseries import TimeSeriesPlots


def main():
    """Main workflow."""
    cfg = parse_ini_configuration()

    TimeSeriesPlots(cfg).workflow()


if __name__ == "__main__":
    main()
