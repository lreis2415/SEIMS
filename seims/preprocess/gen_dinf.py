#! /usr/bin/env python
# coding=utf-8
#
# Author: Junzhi Liu
# Revised: Liang-Jun Zhu
# Note: The compressed flow direction is based on ArcGIS rule.
#
import numpy

from config import *
from TauDEM import *
from text import *
from util import *

# CounterClockwise radian from east direction
e = 0
ne = math.pi * 0.25
n = math.pi * 0.5
nw = math.pi * 0.75
w = math.pi
sw = math.pi * 1.25
s = math.pi * 1.5
se = math.pi * 1.75

angleList = [e, ne, n, nw, w, sw, s, se]


def CheckOrtho(a):
    if FloatEqual(a, e):
        return 1  # 1
    elif FloatEqual(a, ne):
        return 128  # 2
    elif FloatEqual(a, n):
        return 64  # 3
    elif FloatEqual(a, nw):
        return 32  # 4
    elif FloatEqual(a, w):
        return 16  # 5
    elif FloatEqual(a, sw):
        return 8  # 6
    elif FloatEqual(a, s):
        return 4  # 7
    elif FloatEqual(a, se):
        return 2  # 8


def AssignDirCode(a, noDataValue):
    '''
    :param a: D-inf flow direction
    :param noDataValue: NoData value
    :return: Compressed flow direction and weight of the first direction
    '''
    if FloatEqual(a, noDataValue):
        return (DEFAULT_NODATA, DEFAULT_NODATA)

    d = CheckOrtho(a)
    if d is not None:
        return (d, 1)

    if a < ne:
        a1 = a
        d = 129  # 1+128
    elif a < n:
        a1 = a - ne
        d = 192  # 128+64
    elif a < nw:
        a1 = a - n
        d = 96  # 64+32
    elif a < w:
        a1 = a - nw
        d = 48  # 32+16
    elif a < sw:
        a1 = a - w
        d = 24  # 16+8
    elif a < s:
        a1 = a - sw
        d = 12  # 8+4
    elif a < se:
        a1 = a - s
        d = 6  # 4+2
    else:
        a1 = a - se
        d = 3  # 2+1

    return (d, a1 / math.pi * 4.0)


def GenerateDinf(np, wdir, demFilled, flowDir, slopeFile, dirCodeFile, weightFile, mpiexeDir=None, exeDir=None):
    # Invoke TauDEM to get D-inf direction
    FlowDirDinf(np, wdir, demFilled, flowDir, slopeFile,
                mpiexeDir=mpiexeDir, exeDir=exeDir)
    dinf_R = ReadRaster(flowDir)
    data = dinf_R.data
    xsize = dinf_R.nCols
    ysize = dinf_R.nRows
    noDataValue = dinf_R.noDataValue

    calDirCode = numpy.frompyfunc(AssignDirCode, 2, 2)
    dirCode, weight = calDirCode(data, noDataValue)

    WriteGTiffFile(dirCodeFile, ysize, xsize, dirCode,
                   dinf_R.geotrans, dinf_R.srs, DEFAULT_NODATA, gdal.GDT_Int16)
    WriteGTiffFile(weightFile, ysize, xsize, weight, dinf_R.geotrans,
                   dinf_R.srs, DEFAULT_NODATA, gdal.GDT_Float32)

if __name__ == '__main__':
    import os

    tauDir = WORKING_DIR + os.sep + DIR_NAME_TAUDEM
    FlowDirD8(np, tauDir, filledDem, flowDir, slope,
              mpiexeDir=MPIEXEC_DIR, exeDir=CPP_PROGRAM_DIR)
    GenerateDinf(np, tauDir, filledDem, flowDirDinf, slopeDinf, dirCodeDinf, weightDinf, mpiexeDir=MPIEXEC_DIR,
                 exeDir=CPP_PROGRAM_DIR)
