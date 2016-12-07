#! /usr/bin/env python
# coding=utf-8
# Calculating channel width using accumulated data
# Author: Junzhi Liu
# Revised: Liang-Jun Zhu, 2016-7-6
# Note: Using numpy build-in functions to replace python native for loops, which saving time about 40 times!
#

from config import *
from util import *


def chwidth(accFile, chwidthFile):
    accR = ReadRaster(accFile)
    xsize = accR.nCols
    ysize = accR.nRows
    noDataValue = accR.noDataValue
    dx = accR.dx
    cellArea = dx * dx

    # storm frequency   a      b
    # 2                 1      0.56
    # 10                1.2    0.56
    # 100               1.4    0.56
    a = 1.2
    b = 0.56
    # TODO: Figure out what's means, and move it to text.py or config.py. LJ

    tmpOnes = numpy.ones((ysize, xsize))
    width = tmpOnes * DEFAULT_NODATA
    validValues = numpy.where(accR.validZone, accR.data, tmpOnes)
    width = numpy.where(accR.validZone, numpy.power(
        (a * (validValues + 1) * cellArea / 1000000.), b), width)
    # for i in range(0, ysize):
    #     for j in range(0, xsize):
    #         if(abs(dataAcc[i][j] - noDataValue) < UTIL_ZERO):
    #             width[i][j] = noDataValue
    #         else:
    #             width[i][j] =  math.pow(a * (dataAcc[i][j] + 1) * cellArea / 1000000., b)
    #
    WriteGTiffFile(chwidthFile, ysize, xsize, width, accR.geotrans,
                   accR.srs, noDataValue, gdal.GDT_Float32)
    return width


if __name__ == '__main__':
    accFile = WORKING_DIR + os.sep + accM
    chwidthFile = WORKING_DIR + os.sep + chwidthName
    width = chwidth(accFile, chwidthFile)
