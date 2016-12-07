#! /usr/bin/env python
# coding=utf-8
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
#

import sys

from config import *
from delta_s import cal_flowlen
from util import *

sys.setrecursionlimit(10000)

# # the flowout of outletId is nodata
# sqrt2 = math.sqrt(2)
# celllen = {1: 1, 4: 1, 16: 1, 64: 1, 2: sqrt2, 8: sqrt2, 32: sqrt2, 128: sqrt2}
# differ = {1: [0, 1], 2: [1, 1], 4: [1, 0], 8: [1, -1], 16: [0, -1], 32: [-1, -1], 64: [-1, 0], 128: [-1, 1]}

celllen = {1: 1, 3: 1, 5: 1, 7: 1,
           2: SQ2, 4: SQ2, 6: SQ2, 8: SQ2}
differ = {1: [0, 1],
          2: [-1, 1],
          3: [-1, 0],
          4: [-1, -1],
          5: [0, -1],
          6: [1, -1],
          7: [1, 0],
          8: [1, 1]}


def GenerateT0_s(filepath):
    streamlink = filepath + os.sep + streamLinkOut
    strlk_data = ReadRaster(streamlink).data

    velocity = filepath + os.sep + velocityFile
    vel_R = ReadRaster(velocity)
    vel_data = vel_R.data
    xsize = vel_R.nCols
    ysize = vel_R.nRows
    noDataValue = vel_R.noDataValue

    weight = numpy.where(strlk_data <= 0, numpy.ones(
        (ysize, xsize)), numpy.zeros((ysize, xsize)))
    traveltime = numpy.where(
        vel_R.validZone, numpy.zeros((ysize, xsize)), vel_data)
    # weight = numpy.zeros((ysize, xsize))
    # traveltime = numpy.zeros((ysize, xsize))
    # for i in range(0, ysize):
    #     for j in range(0, xsize):
    #         if (abs(vel_data[i][j] - noDataValue) < UTIL_ZERO):
    #             traveltime[i][j] = noDataValue
    #             continue
    #         if (strlk_data[i][j] <= 0):
    #             weight[i][j] = 1
    #         else:
    #             weight[i][j] = 0
    #             # 0 is river
    flowlen = cal_flowlen(filepath, weight)
    traveltime = numpy.where(vel_R.validZone, flowlen /
                             (vel_data * 5. / 3.) / 3600., traveltime)
    # for i in range(0, ysize):
    #     for j in range(0, xsize):
    #         if (abs(vel_data[i][j] - noDataValue) < UTIL_ZERO):
    #             traveltime[i][j] = DEFAULT_NODATA
    #             continue
    #         celerity = vel_data[i][j] * 5.0 / 3.
    #         traveltime[i][j] = flowlen[i][j] / celerity / 3600.

    filename = filepath + os.sep + t0_sFile
    WriteGTiffFile(filename, ysize, xsize, traveltime,
                   vel_R.geotrans, vel_R.srs, DEFAULT_NODATA, gdal.GDT_Float32)

    print 'The t0_s file is generated!'

    return filename


if __name__ == "__main__":
    GenerateT0_s(WORKING_DIR)
