#! /usr/bin/env python
# coding=utf-8

import subprocess

import numpy
from osgeo import gdal, ogr, osr

from pygeoc.utils.utils import *


class Raster:
    '''
    Basic Raster Class
    Build-in functions:
        1. GetAverage()
        2. GetMax()
        3. GetMin()
        4. GetSTD()
        5. GetSum()
        6. GetValueByRowCol(row, col)
        7. GetValueByXY(x, y)

    '''

    def __init__(self, nRows, nCols, data, noDataValue=None, geotransform=None, srs=None):
        self.nRows = nRows
        self.nCols = nCols
        self.data = numpy.copy(data)
        self.noDataValue = noDataValue
        self.geotrans = geotransform
        self.srs = srs

        self.dx = geotransform[1]
        self.xMin = geotransform[0]
        self.xMax = geotransform[0] + nCols * geotransform[1]
        self.yMax = geotransform[3]
        self.yMin = geotransform[3] + nRows * geotransform[5]
        self.validZone = self.data != self.noDataValue
        self.validValues = numpy.where(self.validZone, self.data, numpy.nan)

    def GetAverage(self):
        return numpy.nanmean(self.validValues)

    def GetMax(self):
        return numpy.nanmax(self.validValues)

    def GetMin(self):
        return numpy.nanmin(self.validValues)

    def GetSTD(self):
        return numpy.nanstd(self.validValues)

    def GetSum(self):
        return numpy.nansum(self.validValues)

    def GetValueByRowCol(self, row, col):
        if row < 0 or row >= self.nRows or col < 0 or col >= self.nCols:
            raise ValueError(
                "The row or col must be >=0 and less than nRows (%d) or nCols (%d)!"
                % (self.nRows, self.nCols))
        else:
            value = self.data[int(round(row))][int(round(col))]
            if value == self.noDataValue:
                return None
            else:
                return value

    def GetValueByXY(self, x, y):
        if x < self.xMin or x > self.xMax or y < self.yMin or y > self.yMax:
            return None
            # raise ValueError("The x or y value must be within the Min and Max!")
        else:
            row = self.nRows - int(numpy.ceil((y - self.yMin) / self.dx))
            col = int(numpy.floor((x - self.xMin) / self.dx))
            value = self.data[row][col]
            if value == self.noDataValue:
                return None
            else:
                return value

    def GetCentralCoors(self, row, col):
        if row < 0 or row >= self.nRows or col < 0 or col >= self.nCols:
            raise ValueError(
                "The row or col must be >=0 and less than nRows (%d) or nCols (%d)!"
                % (self.nRows, self.nCols))
        else:
            tmpx = self.xMin + (col - 0.5) * self.dx
            tmpy = self.yMax - (row - 0.5) * self.dx
            return tmpx, tmpy


def ReadRaster(rasterFile):
    ds = gdal.Open(rasterFile)
    band = ds.GetRasterBand(1)
    data = band.ReadAsArray()
    xsize = band.XSize
    ysize = band.YSize

    noDataValue = band.GetNoDataValue()
    geotrans = ds.GetGeoTransform()

    srs = osr.SpatialReference()
    srs.ImportFromWkt(ds.GetProjection())
    # print srs.ExportToProj4()
    if noDataValue is None:
        noDataValue = DEFAULT_NODATA
    band = None
    ds = None
    return Raster(ysize, xsize, data, noDataValue, geotrans, srs)


def WriteGTiffFile(filename, nRows, nCols, data, geotransform, srs, noDataValue, gdalType):
    format = "GTiff"
    driver = gdal.GetDriverByName(format)
    ds = driver.Create(filename, nCols, nRows, 1, gdalType)
    ds.SetGeoTransform(geotransform)
    ds.SetProjection(srs.ExportToWkt())
    ds.GetRasterBand(1).SetNoDataValue(noDataValue)
    ds.GetRasterBand(1).WriteArray(data)
    ds = None


def WriteAscFile(filename, data, xsize, ysize, geotransform, noDataValue):
    header = """NCOLS %d
NROWS %d
XLLCENTER %f
YLLCENTER %f
CELLSIZE %f
NODATA_VALUE %f
""" % (xsize, ysize, geotransform[0] + 0.5 * geotransform[1], geotransform[3] - (ysize - 0.5) * geotransform[1],
       geotransform[1], noDataValue)

    f = open(filename, 'w')
    f.write(header)
    for i in range(0, ysize):
        for j in range(0, xsize):
            f.write(str(data[i][j]) + "\t")
        f.write("\n")
    f.close()


def Raster2GeoTIFF(tif, geotif, gdalType=gdal.GDT_Float32):
    # print "Convering Raster format to GeoTIFF..."
    rstFile = ReadRaster(tif)
    WriteGTiffFile(geotif, rstFile.nRows, rstFile.nCols, rstFile.data,
                   rstFile.geotrans, rstFile.srs, rstFile.noDataValue, gdalType)


def Raster2Asc(rasterF, ascF):
    rasterR = ReadRaster(rasterF)
    WriteAscFile(ascF, rasterR.data, rasterR.nCols, rasterR.nRows,
                 rasterR.geotrans, rasterR.noDataValue)


def RasterStatistics(rasterFile):
    ds = gdal.Open(rasterFile)
    band = ds.GetRasterBand(1)
    min, max, mean, std = band.ComputeStatistics(False)
    return (min, max, mean, std)


def SplitRasters(rs, splitShp, fieldName, tempDir):
    rmmkdir(tempDir)
    ds = ogr.Open(splitShp)
    lyr = ds.GetLayer(0)
    lyr.ResetReading()
    ft = lyr.GetNextFeature()
    while ft:
        cur_field_name = ft.GetFieldAsString(fieldName)
        for r in rs:
            curFileName = r.split(os.sep)[-1]
            outraster = tempDir + os.sep + \
                        curFileName.replace('.tif', '_%s.tif' %
                                            cur_field_name.replace(' ', '_'))
            subprocess.call(['gdalwarp', r, outraster, '-cutline', splitShp,
                             '-crop_to_cutline', '-cwhere', "'%s'='%s'" % (fieldName, cur_field_name), '-dstnodata',
                             '-9999'])
        ft = lyr.GetNextFeature()
    ds = None
    # rmtree(tempDir,True)
