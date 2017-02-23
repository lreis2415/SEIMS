#! /usr/bin/env python
# coding=utf-8
import math
import platform

sysstr = platform.system()

ZERO = 1e-12
DELTA = 1e-6
DEFAULT_NODATA = -9999.
# Hydrological Constants
# D8 flow directions in TauDEM
D8DIR_TD_VALUES = [1, 2, 3, 4, 5, 6, 7, 8]
D8DIR_TD_DELTA = {1: (0, 1),
                  2: (-1, 1),
                  3: (-1, 0),
                  4: (-1, -1),
                  5: (0, -1),
                  6: (1, -1),
                  7: (1, 0),
                  8: (1, 1)}

# D8DIR_TD_VALUES corresponding to ArcGIS
D8DIR_AG_VALUES = [1, 128, 64, 32, 16, 8, 4, 2]
D8DIR_AG_DELTA = {1  : (0, 1),
                  128: (-1, 1),
                  64 : (-1, 0),
                  32 : (-1, -1),
                  16 : (0, -1),
                  8  : (1, -1),
                  4  : (1, 0),
                  2  : (1, 1)}
# drow and dcol as the same sequence as D8DIR_AG_DELTA and D8DIR_TD_DELTA
drow = [0, -1, -1, -1, 0, 1, 1, 1]  # row, not include itself
dcol = [1, 1, 0, -1, -1, -1, 0, 1]  # col

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
