#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Hydrology relation functions, e.g. FlowDirectionCode
    author: Liangjun Zhu
    changlog: 17-06-25 lj - check by pylint and reformat by Google style
"""

import math

from ..utils.utils import StringClass

SQ2 = math.sqrt(2.)


class FlowModelConst(object):
    """flow direction constants according to different flow model"""
    # D8 flow directions in TauDEM
    # 4  3  2
    # 5     1
    # 6  7  8
    d8dir_td = [1, 2, 3, 4, 5, 6, 7, 8]
    d8len_td = {1: 1, 3: 1, 5: 1, 7: 1, 2: SQ2, 4: SQ2, 6: SQ2, 8: SQ2}
    d8delta_td = {1: (0, 1),
                  2: (-1, 1),
                  3: (-1, 0),
                  4: (-1, -1),
                  5: (0, -1),
                  6: (1, -1),
                  7: (1, 0),
                  8: (1, 1)}
    d8_inflow_td = [5, 6, 7, 8, 1, 2, 3, 4]

    # delta row and col as the same sequence as d8delta_ag and d8delta_td
    ccw_drow = [0, -1, -1, -1, 0, 1, 1, 1]  # row, not include itself
    ccw_dcol = [1, 1, 0, -1, -1, -1, 0, 1]  # col

    # d8dir_td corresponding to ArcGIS
    # 32 64 128
    # 64     1
    # 8   4  2
    d8dir_ag = [1, 128, 64, 32, 16, 8, 4, 2]
    d8len_ag = {1: 1, 4: 1, 16: 1, 64: 1, 2: SQ2, 8: SQ2, 32: SQ2, 128: SQ2}
    d8delta_ag = {1: (0, 1),
                  128: (-1, 1),
                  64: (-1, 0),
                  32: (-1, -1),
                  16: (0, -1),
                  8: (1, -1),
                  4: (1, 0),
                  2: (1, 1)}
    d8_inflow_ag = [16, 8, 4, 2, 1, 128, 64, 32]

    # CounterClockwise radian from east direction
    #        nw   n   ne
    #           \ | /
    #         w - + - e
    #           / | \
    #        sw   s   se
    e = 0
    ne = math.pi * 0.25
    n = math.pi * 0.5
    nw = math.pi * 0.75
    w = math.pi
    sw = math.pi * 1.25
    s = math.pi * 1.5
    se = math.pi * 1.75
    d8anglelist = [e, ne, n, nw, w, sw, s, se]

    @staticmethod
    def get_cell_length(flow_model):
        """Get flow direction induced cell length dict.
        Args:
            flow_model: Currently, "TauDEM" and "ArcGIS" are supported.
        """
        if StringClass.string_match(flow_model, "TauDEM"):
            return FlowModelConst.d8len_td
        elif StringClass.string_match(flow_model, "ArcGIS"):
            return FlowModelConst.d8len_ag

    @staticmethod
    def get_cell_shift(flow_model):
        """Get flow direction induced cell shift dict.
        Args:
            flow_model: Currently, "TauDEM" and "ArcGIS" are supported.
        """
        if StringClass.string_match(flow_model, "TauDEM"):
            return FlowModelConst.d8delta_td
        elif StringClass.string_match(flow_model, "ArcGIS"):
            return FlowModelConst.d8delta_ag
