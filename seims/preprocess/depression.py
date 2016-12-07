#! /usr/bin/env python
# coding=utf-8
# Identify depression storage capacity from slope, soil, and landuse. Algorithm from WetSpa.
# Author: Junzhi Liu
# Revised: Liang-Jun Zhu
# Date: 2016-7-6
# Note: Code optimization by using numpy.
# TODO: 1. Add stream order modification, according to depression.ave of WetSpa.
# TODO: 2. Add another depressional storage method according to SWAT, depstor.f
import sqlite3

from config import *


def DepressionCap(filepath, sqliteFile):
    # read landuselookup table from sqlite
    stFields = ["DSC_ST%d" % (i,) for i in range(1, 13)]
    sqlLanduse = 'select LANDUSE_ID,%s from LanduseLookup' % (
        ','.join(stFields),)

    conn = sqlite3.connect(sqliteFile)
    cursor = conn.cursor()
    cursor.execute(sqlLanduse)

    dep_sd0 = {}
    for row in cursor:
        id = int(row[0])
        dep_sd0[id] = [float(item) for item in row[1:]]

    cursor.close()
    conn.close()
    # end read data

    slope = filepath + os.sep + slopeM
    slo_data = ReadRaster(slope).data

    soilTextureFile = filepath + os.sep + soilTexture
    soilTextureArray = ReadRaster(soilTextureFile).data

    mask = filepath + os.sep + mask_to_ext
    mask_R = ReadRaster(mask)
    mask_data = mask_R.data
    noDataValue1 = mask_R.noDataValue
    xsize = mask_R.nCols
    ysize = mask_R.nRows

    landuse = filepath + os.sep + landuseMFile
    landu_R = ReadRaster(landuse)
    landu_data = landu_R.data
    noDataValue2 = landu_R.noDataValue

    defaultLanduseId = 8
    idOmited = []

    def calDep(mask, landu, soilTexture, slp):
        lastStid = 0
        if (abs(mask - noDataValue1) < UTIL_ZERO):
            return noDataValue2
        landuID = int(landu)
        if not landuID in dep_sd0.keys():
            if not landuID in idOmited:
                print 'The landuse ID: %d does not exist.' % (landuID,)
                idOmited.append(landuID)
            landuID = defaultLanduseId
        stid = int(soilTexture) - 1
        try:
            depressionGrid0 = dep_sd0[landuID][stid]
            lastStid = stid
        except:
            depressionGrid0 = dep_sd0[landuID][lastStid]

        depressionGrid = math.exp(
            numpy.log(depressionGrid0 + 0.0001) + slp * (-9.5))
        # TODO, check if it is  (landuID >= 98)? By LJ
        if (landuID == 106 or landuID == 107 or landuID == 105):
            return 0.5 * imperviousPercInUrbanCell + (1. - imperviousPercInUrbanCell) * depressionGrid
        else:
            return depressionGrid

    calDep_numpy = numpy.frompyfunc(calDep, 4, 1)
    depStorageCap = calDep_numpy(mask_data, landu_data, soilTextureArray, slo_data)

    filename = filepath + os.sep + depressionFile
    WriteGTiffFile(filename, ysize, xsize, depStorageCap,
                   mask_R.geotrans, mask_R.srs, noDataValue2, gdal.GDT_Float32)

    print 'The depression storage capacity is generated!'
    return filename


if __name__ == '__main__':
    # Load Configuration file
    LoadConfiguration(GetINIfile())
    DepressionCap(WORKING_DIR, TXT_DB_DIR + os.sep + sqliteFile)
