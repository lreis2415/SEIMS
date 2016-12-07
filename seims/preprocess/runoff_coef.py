#! /usr/bin/env python
# coding=utf-8
# @Calculate runoff coefficient
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
#

import sqlite3

from config import *
from util import *


def RunoffCoefficent(filepath, sqliteFile):
    # read landuselookup table from sqlite
    prcFields = ["PRC_ST%d" % (i,) for i in range(1, 13)]
    scFields = ["SC_ST%d" % (i,) for i in range(1, 13)]
    sqlLanduse = 'select LANDUSE_ID, %s, %s from LanduseLookup' % \
                 (','.join(prcFields), ','.join(scFields))

    conn = sqlite3.connect(sqliteFile)
    cursor = conn.cursor()
    cursor.execute(sqlLanduse)

    runoff_c0 = {}
    runoff_s0 = {}
    for row in cursor:
        id = int(row[0])
        runoff_c0[id] = [float(item) for item in row[1:13]]
        runoff_s0[id] = [float(item) for item in row[13:25]]

    cursor.close()
    conn.close()
    # end read data

    slope = filepath + os.sep + slopeM
    slo_data = ReadRaster(slope).data
    # ds = gdal.Open(slope)
    # band = ds.GetRasterBand(1)
    # slo_data = band.ReadAsArray()

    mask = filepath + os.sep + mask_to_ext
    maskR = ReadRaster(mask)
    mask_data = maskR.data
    noDataValue1 = maskR.noDataValue
    # ds = gdal.Open(mask)
    # band = ds.GetRasterBand(1)
    # mask_data = band.ReadAsArray()
    # noDataValue1 = band.GetNoDataValue()

    landuse = filepath + os.sep + landuseMFile
    landu_data = ReadRaster(landuse).data
    # ds = gdal.Open(landuse)
    # band = ds.GetRasterBand(1)
    # landu_data = band.ReadAsArray()

    soilTextureFile = filepath + os.sep + soilTexture
    soilTextureR = ReadRaster(soilTextureFile)
    soilTextureArray = soilTextureR.data
    # ds = gdal.Open(soilTextureFile)
    # band = ds.GetRasterBand(1)
    # soilTextureArray = band.ReadAsArray()
    xsize = soilTextureR.nCols
    ysize = soilTextureR.nRows
    noDataValue2 = soilTextureR.noDataValue
    cellsize = soilTextureR.dx
    # xsize = band.XSize
    # ysize = band.YSize
    # # print xsize,ysize  226,77
    # noDataValue2 = band.GetNoDataValue()
    # geotransform = ds.GetGeoTransform()
    #
    # srs = osr.SpatialReference()
    # srs.ImportFromWkt(ds.GetProjection())
    #
    # cellsize = geotransform[1]

    # coef = numpy.zeros((ysize, xsize))
    # Set impervious percentage for urban cells
    # imperviousPercInUrbanCell = 0.3

    defaultLanduseId = 8
    idOmited = []

    def coefCal(mask, landuID, soilTexture, slope):
        if abs(mask - noDataValue1) < UTIL_ZERO or int(landuID) < 0:
            return noDataValue2
        if not int(landuID) in runoff_c0.keys():
            if not int(landuID) in idOmited:
                print 'The landuse ID: %d does not exist.' % (int(landuID),)
                idOmited.append(int(landuID))
            landuID = defaultLanduseId
        stid = int(soilTexture) - 1
        c0 = runoff_c0[int(landuID)][stid]
        s0 = runoff_s0[int(landuID)][stid] / 100.
        # slp = slope / 100. # For TauDEM, there is no need to divide 100. By
        # LJ
        slp = slope

        if slp + s0 < 0.0001:
            return c0
        coef1 = (1 - c0) * slp / (slp + s0)
        coef2 = c0 + coef1
        # TODO, Check if it is (landuID >= 98), by lj
        if (int(landuID) == 106 or int(landuID) == 107 or int(landuID) == 105):
            return coef2 * (1 - imperviousPercInUrbanCell) + imperviousPercInUrbanCell
        else:
            return coef2

    coefCal_numpy = numpy.frompyfunc(coefCal, 4, 1)
    coef = coefCal_numpy(mask_data, landu_data, soilTextureArray, slo_data)
    # for i in range(0, ysize):
    #     for j in range(0, xsize):
    #         landuID = int(landu_data[i][j])
    #         if (abs(mask_data[i][j] - noDataValue1) < UTIL_ZERO or landuID < 0):
    #             coef[i][j] = noDataValue2
    #             continue
    #
    #         if not landuID in runoff_c0.keys():
    #             if not landuID in idOmited:
    #                 print 'The landuse ID: %d does not exist.' % (landuID,)
    #                 idOmited.append(landuID)
    #             landuID = defaultLanduseId
    #
    #         stid = int(soilTextureArray[i][j]) - 1
    #
    #         c0 = runoff_c0[landuID][stid]
    #         s0 = runoff_s0[landuID][stid] / 100.
    #         slp = slo_data[i][j] / 100.
    #
    #         if slp + s0 < 0.0001:
    #             coef[i][j] = c0
    #             continue
    #
    #         coef1 = (1 - c0) * slp / (slp + s0)
    #         coef2 = c0 + coef1
    #         if (landuID == 106 or landuID == 107 or landuID == 105):  ## TODO, Check if it is (landuID >= 98)
    #             coef[i][j] = coef2 * (1 - imperviousPercInUrbanCell) + imperviousPercInUrbanCell
    #         else:
    #             coef[i][j] = coef2
    #             # TODO: What's means?
    #             # if coef[i][j] < 0:
    #             #    print c0, slp, s0, coe1, coef2

    filename = filepath + os.sep + runoff_coefFile
    WriteGTiffFile(filename, ysize, xsize, coef,
                   maskR.geotrans, maskR.srs, noDataValue2, gdal.GDT_Float32)

    print 'The Runoffcoefficient file is generated!'


if __name__ == "__main__":
    RunoffCoefficent(WORKING_DIR, TXT_DB_DIR + os.sep + sqliteFile)
