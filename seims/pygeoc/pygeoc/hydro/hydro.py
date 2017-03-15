#! /usr/bin/env python
# coding=utf-8
import math

from ..utils.utils import StringClass

SQ2 = math.sqrt(2.)


class FlowDirectionCode(object):
    """Definition of flow direction code and shift coordinate."""

    def __init__(self, flow_model):
        """
        Initialization.
        :param flow_model(string): Currently, "TauDEM" and "ArcGIS" are supported.
        """
        self.cell_length = {}
        self.cell_shift = {}
        if StringClass.stringmatch(flow_model, "TauDEM"):
            # The value of direction is as following (TauDEM):
            # 4  3  2
            # 5     1
            # 6  7  8
            # TauDEM flow direction code
            self.cell_length = {1: 1, 3: 1, 5: 1, 7: 1, 2: SQ2, 4: SQ2, 6: SQ2, 8: SQ2}
            self.cell_shift = {1: [0, 1], 2: [-1, 1], 3: [-1, 0], 4: [-1, -1],
                               5: [0, -1], 6: [1, -1], 7: [1, 0], 8: [1, 1]}
        elif StringClass.stringmatch(flow_model, "ArcGIS"):
            # The value of direction is as following (ArcGIS):
            # 32 64 128
            # 64     1
            # 8   4  2
            # ArcGIS flow direction code
            self.cell_length = {1: 1, 4: 1, 16: 1, 64: 1, 2: SQ2, 8: SQ2, 32: SQ2, 128: SQ2}
            self.cell_shift = {1 : [0, 1], 2: [1, 1], 4: [1, 0], 8: [1, -1],
                               16: [0, -1], 32: [-1, -1], 64: [-1, 0], 128: [-1, 1]}

    def get_cell_length(self):
        return self.cell_length

    def get_cell_shift(self):
        return self.cell_shift
