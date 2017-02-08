#! /usr/bin/env python
# coding=utf-8

from pygeoc.utils import FloatEqual
from pygeoc.utils.const import *


# find downslope coordinate for D8 and D-inf flow models
def downstream_index(DIR_VALUE, i, j):
    drow, dcol = D8DIR_TD_DELTA[DIR_VALUE]
    return i + drow, j + dcol


def CheckOrtho(a):
    if FloatEqual(a, e):
        return 1
    elif FloatEqual(a, ne):
        return 2
    elif FloatEqual(a, n):
        return 3
    elif FloatEqual(a, nw):
        return 4
    elif FloatEqual(a, w):
        return 5
    elif FloatEqual(a, sw):
        return 6
    elif FloatEqual(a, s):
        return 7
    elif FloatEqual(a, se):
        return 8
    else:
        return 0


def AssignDirCode(a):
    d = CheckOrtho(a)
    if d != 0:
        down = [d]
        return down
    else:
        if a < ne:  # 129 = 1+128
            down = [1, 2]
        elif a < n:  # 192 = 128+64
            down = [2, 3]
        elif a < nw:  # 96 = 64+32
            down = [3, 4]
        elif a < w:  # 48 = 32+16
            down = [4, 5]
        elif a < sw:  # 24 = 16+8
            down = [5, 6]
        elif a < s:  # 12 = 8+4
            down = [6, 7]
        elif a < se:  # 6 = 4+2
            down = [7, 8]
        else:  # 3 = 2+1
            down = [8, 1]
        return down


def downstream_index_dinf(dinfDir, i, j):
    downDirs = AssignDirCode(dinfDir)
    downCoors = []
    for dir in downDirs:
        row, col = downstream_index(dir, i, j)
        downCoors.append([row, col])
    return downCoors
