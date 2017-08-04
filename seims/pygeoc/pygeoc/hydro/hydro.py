#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Hydrology relation functions, e.g. FlowDirectionCode

    author: Liangjun Zhu

    changlog: 17-06-25 lj - check by pylint and reformat by Google style.\n
"""
from ..raster.raster import RasterUtilClass, GDALDataType
from ..utils.utils import FileClass
from ..utils.utils import PI, SQ2


class FlowModelConst(object):
    """flow direction constants according to different flow model"""
    # CounterClockwise radian from east direction
    #        nw   n   ne
    #           \ | /
    #         w - + - e
    #           / | \
    #        sw   s   se
    e = 0.
    ne = PI * 0.25
    n = PI * 0.5
    nw = PI * 0.75
    w = PI
    sw = PI * 1.25
    s = PI * 1.5
    se = PI * 1.75
    d8anglelist = [e, ne, n, nw, w, sw, s, se]

    # 1. D8 flow directions in TauDEM
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

    # 2. D8 flow directions in ArcGIS
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

    # 3. D8 flow directions in Whitebox GAT (Geospatial Analysis Tools)
    # http://www.uoguelph.ca/~hydrogeo/Whitebox/index.html
    # 64 128  1
    # 32      2
    # 16   8  4
    d8dir_wb = [2, 1, 128, 64, 32, 16, 8, 4]
    d8len_wb = {1: 1, 4: 1, 16: 1, 64: 1, 2: SQ2, 8: SQ2, 32: SQ2, 128: SQ2}
    d8delta_wb = {2: (0, 1),
                  1: (-1, 1),
                  128: (-1, 0),
                  64: (-1, -1),
                  32: (0, -1),
                  16: (1, -1),
                  8: (1, 0),
                  4: (1, 1)}
    d8_inflow_wb = [32, 16, 8, 4, 2, 1, 128, 64]

    # delta row and col as the same sequence as d8delta_ag, d8delta_td, and d8delta_wb
    ccw_drow = [0, -1, -1, -1, 0, 1, 1, 1]  # row, not include itself
    ccw_dcol = [1, 1, 0, -1, -1, -1, 0, 1]  # col

    # available D8 flow direction algorithms
    d8_dirs = {"taudem": d8dir_td,
               "arcgis": d8dir_ag,
               "whitebox": d8dir_wb}
    d8_lens = {"taudem": d8len_td,
               "arcgis": d8len_ag,
               "whitebox": d8len_wb}
    d8_deltas = {"taudem": d8delta_td,
                 "arcgis": d8delta_ag,
                 "whitebox": d8delta_wb}
    d8_inflows = {"taudem": d8_inflow_td,
                  "arcgis": d8_inflow_ag,
                  "whitebox": d8_inflow_wb}

    @staticmethod
    def get_cell_length(flow_model):
        """Get flow direction induced cell length dict.
        Args:
            flow_model: Currently, "TauDEM", "ArcGIS", and "Whitebox" are supported.
        """
        assert flow_model.lower() in FlowModelConst.d8_lens
        return FlowModelConst.d8_lens.get(flow_model.lower())

    @staticmethod
    def get_cell_shift(flow_model):
        """Get flow direction induced cell shift dict.
        Args:
            flow_model: Currently, "TauDEM", "ArcGIS", and "Whitebox" are supported.
        """
        assert flow_model.lower() in FlowModelConst.d8_deltas
        return FlowModelConst.d8_deltas.get(flow_model.lower())


class D8Util(object):
    """Utility functions based on D8 flow direction
       Currently, D8 flow direction code of TauDEM, ArcGIS, Whitebox GAT are supported.
    """

    def __init__(self):
        pass

    @staticmethod
    def downstream_index(dir_value, i, j, alg="taudem"):
        """find downslope coordinate for D8 direction."""
        assert alg.lower() in FlowModelConst.d8_deltas
        delta = FlowModelConst.d8_deltas.get(alg.lower())
        drow, dcol = delta[int(dir_value)]
        return i + drow, j + dcol

    @staticmethod
    def convert_code(in_file, out_file, in_alg="taudem", out_alg="arcgis", datatype=None):
        """
        convert D8 flow direction code from one algorithm to another.
        Args:
            in_file: input raster file path
            out_file: output raster file path
            in_alg: available algorithms are in FlowModelConst.d8_dirs. "taudem" is the default
            out_alg: same as in_alg. "arcgis" is the default
            datatype: default is None and use the datatype of the in_file
        """
        FileClass.check_file_exists(in_file)
        in_alg = in_alg.lower()
        out_alg = out_alg.lower()
        if in_alg not in FlowModelConst.d8_dirs or out_alg not in FlowModelConst.d8_dirs:
            raise RuntimeError("The input algorithm name should one of %s" %
                               ', '.join(FlowModelConst.d8_dirs.keys()))
        convert_dict = dict()
        in_code = FlowModelConst.d8_dirs.get(in_alg)
        out_code = FlowModelConst.d8_dirs.get(out_alg)
        assert len(in_code) == len(out_code)
        for i, tmp_in_code in enumerate(in_code):
            convert_dict[tmp_in_code] = out_code[i]
        if datatype is not None and datatype in GDALDataType:
            RasterUtilClass.raster_reclassify(in_file, convert_dict, out_file, datatype)
        else:
            RasterUtilClass.raster_reclassify(in_file, convert_dict, out_file)
