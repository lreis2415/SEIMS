#! /usr/bin/env python
# coding=utf-8
# @Calculate velocity
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
#

from numpy import *

from config import *
from util import *


def GenerateVelocity(filepath):
    slope = filepath + os.sep + slopeM
    slp_R = ReadRaster(slope)
    slo_data = slp_R.data
    xsize = slp_R.nCols
    ysize = slp_R.nRows
    noDataValue = slp_R.noDataValue

    radius = filepath + os.sep + radiusFile
    rad_data = ReadRaster(radius).data

    Manning = filepath + os.sep + ManningFile
    Man_data = ReadRaster(Manning).data

    vel_max = 3.0
    vel_min = 0.0001

    def velocityCal(rad, man, slp):
        if (abs(rad - noDataValue) < UTIL_ZERO):
            return DEFAULT_NODATA
        tmp = numpy.power(man, -1) * numpy.power(rad, 2 / 3) * \
            numpy.power(slp, 0.5)
        # print tmp
        if tmp < vel_min:
            return vel_min
        if tmp > vel_max:
            return vel_max
        return tmp

    velocityCal_numpy = numpy.frompyfunc(velocityCal, 3, 1)
    velocity = velocityCal_numpy(rad_data, Man_data, slo_data)

    filename = filepath + os.sep + velocityFile
    WriteGTiffFile(filename, ysize, xsize, velocity,
                   slp_R.geotrans, slp_R.srs, DEFAULT_NODATA, gdal.GDT_Float32)
    print 'The velocity file is generated!'
    return filename

if __name__ == "__main__":
    LoadConfiguration(GetINIfile())
    GenerateVelocity(WORKING_DIR)
