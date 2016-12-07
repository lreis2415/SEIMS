#! /usr/bin/env python
# coding=utf-8
# Initializa soil moisture fraction of field capacity, based on TWI
#     improve calculation efficiency by numpy
# Author: Junzhi Liu
# Revised: Liang-Jun Zhu
#

from config import *
from util import *


def InitMoisture(dstdir):
    acc_name = dstdir + os.sep + accM
    slope_name = dstdir + os.sep + slopeM

    acc_R = ReadRaster(acc_name)
    dataAcc = acc_R.data
    xsize = acc_R.nCols
    ysize = acc_R.nRows
    noDataValue = acc_R.noDataValue
    srs = acc_R.srs
    geotrans = acc_R.geotrans
    dx = acc_R.dx
    dataSlope = ReadRaster(slope_name).data
    cellArea = dx * dx

    # TWI, ln(a/tan(b))
    def wiGridCal(acc, slp):
        if (abs(acc - noDataValue) < UTIL_ZERO):
            return DEFAULT_NODATA
        else:
            if (abs(slp) < UTIL_ZERO):
                slp = UTIL_ZERO
                # slp = 0.1 / dx * 100.
            return math.log((acc + 1.) * cellArea / slp)
            # return math.log((acc + 1.) * cellArea / (slp / 100.))

    wiGridCal_numpy = numpy.frompyfunc(wiGridCal, 2, 1)
    wiGrid = wiGridCal_numpy(dataAcc, dataSlope)
    # wiGrid_valid = numpy.where(acc_R.validZone, wiGrid, numpy.nan)
    # wiMax = numpy.nanmax(wiGrid_valid)
    # wiMin = numpy.nanmin(wiGrid_valid)
    # WARNING: numpy.nanmax and numpy.nanmin are un-stabilized in Linux, so
    # replaced by the for loops. By LJ
    wiMax = -numpy.inf
    wiMin = numpy.inf
    for i in range(0, ysize):
        for j in range(0, xsize):
            if wiMax < wiGrid[i][j]:
                wiMax = wiGrid[i][j]
            if wiMin > wiGrid[i][j] and wiGrid[i][j] != DEFAULT_NODATA:
                wiMin = wiGrid[i][j]
    # print "TWIMax:%f, TWIMin:%f" % (wiMax, wiMin)
    # wiGrid = zeros((ysize, xsize))
    # wiMax = -1
    # wiMin = 1000
    # for i in range(0, ysize):
    #     for j in range(0, xsize):
    #         if (abs(dataAcc[i][j] - noDataValue) < UTIL_ZERO):
    #             wiGrid[i][j] = DEFAULT_NODATA
    #         else:
    #             if (abs(dataSlope[i][j]) < UTIL_ZERO):
    #                 dataSlope[i][j] = 0.1 / dx * 100.
    #
    #             wiGrid[i][j] = math.log((dataAcc[i][j] + 1) * cellArea / (dataSlope[i][j] / 100.))
    #             if (wiGrid[i][j] > wiMax):
    #                 wiMax = wiGrid[i][j]
    #             elif (wiGrid[i][j] < wiMin):
    #                 wiMin = wiGrid[i][j]

    soilMoisFrMin = 0.6  # minimum relative saturation
    soilMoisFrMax = 1.0

    wiUplimit = wiMax
    a = (soilMoisFrMax - soilMoisFrMin) / (wiUplimit - wiMin)
    b = soilMoisFrMin - a * wiMin

    def moistureCal(acc, wigrid):
        if (abs(acc - noDataValue) < UTIL_ZERO):
            return DEFAULT_NODATA
        else:
            tmp = a * wigrid + b
            if tmp > soilMoisFrMax:
                return soilMoisFrMax
            elif tmp < soilMoisFrMin:
                return soilMoisFrMin
            else:
                return tmp

    moistureCal_numpy = numpy.frompyfunc(moistureCal, 2, 1)
    moisture = moistureCal_numpy(dataAcc, wiGrid)
    # moisture = zeros((ysize, xsize))
    # for i in range(0, ysize):
    #     for j in range(0, xsize):
    #         if (abs(dataAcc[i][j] - noDataValue) < UTIL_ZERO):
    #             moisture[i][j] = DEFAULT_NODATA
    #         else:
    #             moisture[i][j] = a * wiGrid[i][j] + b

    filename = dstdir + os.sep + initSoilMoist
    WriteGTiffFile(filename, ysize, xsize, moisture, geotrans,
                   srs, DEFAULT_NODATA, gdal.GDT_Float32)

    print 'The initial moisture is generated!'
    return filename


if __name__ == "__main__":
    LoadConfiguration(GetINIfile())
    InitMoisture(WORKING_DIR)
