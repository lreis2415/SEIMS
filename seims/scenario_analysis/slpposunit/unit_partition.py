# -*- coding: utf-8 -*-

# Author: GAO Huiran
# Date: Sep 10, 2016

from main import *
import os, sys
import math
from osgeo import gdal, ogr, osr
import subprocess
import numpy
from util import *

def PrefieldParti(slpPos, subBasin):
    preParti = numpy.zeros((nRows, nCols))
    for m in range(nRows):
        for n in range(nCols):
            if slpPos[m][n] != noDtVal_slpP and subBasin[m][n] != noDtVal_subB:
                if (slpPos[m][n] != 0):
                    preParti[m][n] = subBasin[m][n] * slpPos[m][n]
                else:
                    preParti[m][n] = 0
            elif slpPos[m][n] != noDtVal_slpP and subBasin[m][n] == noDtVal_subB:
                if preParti[m][n - 1] != noDataValue:
                    if n != 0:
                        preParti[m][n] = preParti[m][n - 1]
                else:
                    if m != 0:
                        preParti[m][n] = preParti[m - 1][n]
            else:
                preParti[m][n] = noDataValue
    outputRaster = dataDir + os.sep + prePartion
    WriteGTiffFile(outputRaster, nRows, nCols, preParti, geotrans, srs, noDataValue, gdal.GDT_Float32)
    print "prePartion raster save as '%s'" % outputRaster
    # return preParti

## Integrated prePartion
def integRaster(prepartionfile, outputfile):
    prepartion = ReadRaster(prepartionfile).data
    prepartion_integ = numpy.zeros((nRows, nCols))
    for m in range(nRows):
        for n in range(nCols):
            if (m == 0 or n == 0 or m == nRows - 1 or n == nCols - 1):
                # print inputRaster[m][n]
                prepartion_integ[m][n] = prepartion[m][n]
            else:
                direct_8 = [
                    prepartion[m][n + 1], prepartion[m + 1][n + 1], prepartion[m + 1][n], prepartion[m + 1][n - 1],
                    prepartion[m][n - 1], prepartion[m - 1][n - 1], prepartion[m - 1][n], prepartion[m - 1][n + 1],
                ]
                if prepartion[m][n] != noDataValue and noDataValue in direct_8:
                    if prepartion[m][n] not in direct_8:
                        prepartion_integ[m][n] = Mode(direct_8, len(direct_8), noDataValue)
                    else:
                        prepartion_integ[m][n] = prepartion[m][n]
                else:
                    prepartion_integ[m][n] = prepartion[m][n]
    WriteGTiffFile(outputfile, nRows, nCols, prepartion_integ, geotrans, srs, noDataValue, gdal.GDT_Float32)
    # return prepartion_integ


## 将Summit, Slope和Valley单独分离出来
def extractSlpPos():
    preParti = ReadRaster(dataDir + os.sep + prePartion).data
    summitRaster = numpy.zeros((nRows, nCols))
    slopeRaster = numpy.zeros((nRows, nCols))
    valleyRaster = numpy.zeros((nRows, nCols))
    for m in range(nRows):
        for n in range(nCols):
            if (slpPos[m][n] != noDtVal_slpP):
                if (slpPos[m][n] == 0):
                    summitRaster[m][n] = 1
                else:
                    summitRaster[m][n] = noDataValue
                if (slpPos[m][n] == 1):
                    slopeRaster[m][n] = preParti[m][n]
                else:
                    slopeRaster[m][n] = noDataValue
                if (slpPos[m][n] == 2):
                    valleyRaster[m][n] = preParti[m][n]
                else:
                    valleyRaster[m][n] = noDataValue
            else:
                summitRaster[m][n] = noDataValue
                slopeRaster[m][n] = noDataValue
                valleyRaster[m][n] = noDataValue
    outputRaster_smt = dataDir + os.sep + "summit.tif"
    outputRaster_slp = dataDir + os.sep + "slope.tif"
    outputRaster_vly = dataDir + os.sep + "valley.tif"
    WriteGTiffFile(outputRaster_smt, nRows, nCols, summitRaster, geotrans, srs, noDataValue, gdal.GDT_Float32)
    WriteGTiffFile(outputRaster_slp, nRows, nCols, slopeRaster, geotrans, srs, noDataValue, gdal.GDT_Float32)
    WriteGTiffFile(outputRaster_vly, nRows, nCols, valleyRaster, geotrans, srs, noDataValue, gdal.GDT_Float32)
    print "Extract summit as '%s'" % "summit.tif"
    print "Extract slope as '%s'" % "slope.tif"
    print "Extract valley as '%s'" % "valley.tif"

    ## Raster convert to vector
    smtVector = "summit_polygonized"
    slpVector = "slope_polygonized"
    vlyVector = "valley_polygonized"
    RastertoVector(outputRaster_smt, outputRaster_smt, dataDir, smtVector)
    RastertoVector(outputRaster_slp, outputRaster_slp, dataDir, slpVector)
    RastertoVector(outputRaster_vly, outputRaster_vly, dataDir, vlyVector)
    print "Convert summit raster to vector, save as '%s'" % smtVector + ".shp"
    print "Convert slope raster to vector, save as '%s'" % slpVector + ".shp"
    print "Convert valley raster to vector, save as '%s'" % vlyVector + ".shp"

    ## Vector to raster
    aoi_raster = dataDir + os.sep + subBasinfile
    smt_shapefile = dataDir + os.sep + smtVector + ".shp"
    slp_shapefile = dataDir + os.sep + slpVector + ".shp"
    vly_shapefile = dataDir + os.sep + vlyVector + ".shp"
    smt_raster_out = dataDir + os.sep + "summit_new.tif"
    slp_raster_out = dataDir + os.sep + "slope_new.tif"
    vly_raster_out = dataDir + os.sep + "valley_new.tif"
    VectortoRaster(aoi_raster, smt_shapefile, smt_raster_out)
    VectortoRaster(aoi_raster, slp_shapefile, slp_raster_out)
    VectortoRaster(aoi_raster, vly_shapefile, vly_raster_out)
    print "Convert summit vector to raster, save as '%s'" % smt_raster_out
    print "Convert slope vector to raster, save as '%s'" % slp_raster_out
    print "Convert valley vector to raster, save as '%s'" % vly_raster_out

    ## prePation
    prePatiVector = "prePartion_polygonized"
    mask = dataDir + os.sep + "mask.tif"
    RastertoVector(dataDir + os.sep + prePartion, mask, dataDir, prePatiVector)
    # print "Convert prePartion raster to vector, save as '%s'" % prePatiVector + ".shp"
    preParti_raster_out = dataDir + os.sep + "prePartion.tif"
    VectortoRaster(aoi_raster, dataDir + os.sep + prePatiVector + ".shp", preParti_raster_out)
    # print "Convert prePartion vector to raster, save as '%s'" % preParti_raster_out

    prePatiVector = "Partion_integ_polygonized"
    integRaster(preParti_raster_out, fieldPartion)
    RastertoVector(fieldPartion, mask, dataDir, prePatiVector)
    print "Convert prePartion raster to vector, save as '%s'" % prePatiVector + ".shp"
    VectortoRaster(aoi_raster, dataDir + os.sep + prePatiVector + ".shp", dataDir + os.sep + fieldPartion)
    print "Convert prePartion vector to raster, save as '%s'" % (dataDir + os.sep + fieldPartion)

