#! /usr/bin/env python
# coding=utf-8
# @Calculate hydrological radius, efficiency improved by numpy
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
#
from config import *
from util import *


def GenerateRadius(filepath, stormProbability):
    accfile = filepath + os.sep + accM
    acc_R = ReadRaster(accfile)
    xsize = acc_R.nCols
    ysize = acc_R.nRows
    noDataValue = acc_R.noDataValue
    cellsize = acc_R.dx
    data = acc_R.data
    coeTable = {"T2": [0.05, 0.48],
                "T10": [0.12, 0.52],
                "T100": [0.18, 0.55]}
    ap = coeTable[stormProbability][0]
    bp = coeTable[stormProbability][1]

    def radiusCal(acc):
        if (abs(acc - noDataValue) < UTIL_ZERO):
            return DEFAULT_NODATA
        return numpy.power(ap * ((acc + 1) * cellsize * cellsize / 1000000.), bp)

    radiusCal_numpy = numpy.frompyfunc(radiusCal, 1, 1)
    radius = radiusCal_numpy(data)

    filename = filepath + os.sep + radiusFile
    WriteGTiffFile(filename, ysize, xsize, radius,
                   acc_R.geotrans, acc_R.srs, DEFAULT_NODATA, gdal.GDT_Float32)
    print 'The radius file is generated!'
    return filename


if __name__ == "__main__":
    GenerateRadius(WORKING_DIR, "T2")
