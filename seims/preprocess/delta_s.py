#! /usr/bin/env python
# coding=utf-8
# Generating delta_s for IUH
# Author: Junzhi Liu
# Revised: Liang-Jun Zhu
# Note: Code optimization by using numpy.
#

import sys

from config import *
from util import *

sys.setrecursionlimit(10000)

# Automatically find out the D8 coding system
celllen = {}
differ = {}
# The value of direction is as following (TauDEM):
# 4  3  2
# 5     1
# 6  7  8
# TauDEM flow direction code
if(isTauDEM):
    celllen = {1: 1, 3: 1, 5: 1, 7: 1, 2: SQ2, 4: SQ2, 6: SQ2, 8: SQ2}
    differ = {1: [0, 1], 2: [-1, 1], 3: [-1, 0], 4: [-1, -1],
              5: [0, -1], 6: [1, -1], 7: [1, 0], 8: [1, 1]}
else:
    # The value of direction is as following (ArcGIS):
    # 32 64 128
    # 64     1
    # 8   4  2
    # ArcGIS flow direction code
    celllen = {1: 1, 4: 1, 16: 1, 64: 1, 2: SQ2, 8: SQ2, 32: SQ2, 128: SQ2}
    differ = {1: [0, 1], 2: [1, 1], 4: [1, 0], 8: [1, -1],
              16: [0, -1], 32: [-1, -1], 64: [-1, 0], 128: [-1, 1]}


def flowlen_cell(i, j, ysize, xsize, fdir, cellsize, weight, length):
    # print i,j, weight[i][j]
    if i < ysize and j < xsize:
        if length[i][j] == 0:
            if weight[i][j] > 0:
                prei = i
                prej = j
                wt = weight[i][j]
                fdirV = fdir[i][j]
                di = differ[fdirV][0]
                dj = differ[fdirV][1]
                i = i + di
                j = j + dj
                relen = flowlen_cell(i, j, ysize, xsize,
                                     fdir, cellsize, weight, length)
                # print i,j,fdirV
                length[prei][prej] = cellsize * celllen[fdirV] * wt + relen
                return length[prei][prej]
            else:
                return 0
        if length[i][j] > 0:
            return length[i][j]

        if length[i][j] < 0:
            print "Error in calculating flowlen_cell function! i,j:"
            print i, j
            return -1
    return 0


def cal_flowlen(filepath, weight):
    flow_dir = filepath + os.sep + flowDirOut
    ds = gdal.Open(flow_dir)
    band = ds.GetRasterBand(1)
    fdir_data = band.ReadAsArray()
    xsize = band.XSize
    ysize = band.YSize
    noDataValue = band.GetNoDataValue()
    geotransform = ds.GetGeoTransform()
    cellsize = geotransform[1]
    length = numpy.zeros((ysize, xsize))

    for i in range(0, ysize):
        for j in range(0, xsize):
            if abs(fdir_data[i][j] - noDataValue) < UTIL_ZERO:
                length[i][j] = noDataValue
                continue

            flowlen_cell(i, j, ysize, xsize, fdir_data,
                         cellsize, weight, length)

    return length


def GenerateDelta_s(filepath):
    streamlink = filepath + os.sep + streamLinkOut  # stream_link.tif
    radius = filepath + os.sep + radiusFile  # radius.tif
    slope = filepath + os.sep + slopeM  # slope.tif
    velocity = filepath + os.sep + velocityFile  # velocity.tif

    strlkR = ReadRaster(streamlink)
    strlk_data = strlkR.data
    rad_data = ReadRaster(radius).data
    slo_data = ReadRaster(slope).data
    velR = ReadRaster(velocity)
    vel_data = velR.data

    xsize = strlkR.nCols
    ysize = strlkR.nRows
    noDataValue = strlkR.noDataValue

    def initialVariables(vel, strlk, slp, rad):
        if abs(vel - noDataValue) < UTIL_ZERO:
            return DEFAULT_NODATA
        if strlk <= 0:
            weight = 1
        else:
            weight = 0
        # 0 is river
        if slp < 0.0005:
            slp = 0.0005
        # dampGrid = vel * rad / (slp / 100. * 2.) # No need to divide 100 in
        # my view. By LJ
        dampGrid = vel * rad / (slp * 2.)
        celerity = vel * 5. / 3.
        weight = dampGrid * 2. / numpy.power(celerity, 3.) * weight
        return weight

    initialVariables_numpy = numpy.frompyfunc(initialVariables, 4, 1)
    weight = initialVariables_numpy(vel_data, strlk_data, slo_data, rad_data)

    delta_s_sqr = cal_flowlen(filepath, weight)

    def cal_delta_s(vel, sqr):
        if abs(vel - noDataValue) < UTIL_ZERO:
            return noDataValue
        else:
            return math.sqrt(sqr) / 3600.

    cal_delta_s_numpy = numpy.frompyfunc(cal_delta_s, 2, 1)
    delta_s = cal_delta_s_numpy(vel_data, delta_s_sqr)

    filename = filepath + os.sep + delta_sFile
    WriteGTiffFile(filename, ysize, xsize, delta_s, strlkR.geotrans, strlkR.srs,
                   noDataValue, gdal.GDT_Float32)
    print 'The delta_s file is generated!'
    return filename


if __name__ == "__main__":
    GenerateDelta_s(WORKING_DIR)
