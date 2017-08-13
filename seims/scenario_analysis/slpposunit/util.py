#! /usr/bin/env python
#coding=utf-8
## @Utility functions
#
#
import sys,os,math,datetime,time
from osgeo import gdal,ogr,osr,gdalconst
from gdalconst import *
import shutil
import random
import geojson
import numpy

####  Climate Utility Functions  ####
def IsLeapYear(year):
    if( (year%4 == 0 and year%100 != 0) or (year%400 == 0)):
        return True
    else:
        return False

def GetDayNumber(year, month):
    if month in [1,3,5,7,8,10,12]:
        return 31
    elif month in [4,6,9,11]:
        return 30
    elif IsLeapYear(year):
        return 29
    else:
        return 28

## Solar Radiation Calculation
#  @param doy day of year
#  @param n   sunshine duration
#  @param lat latitude of sites
#  invoke   Rs(doy, n, lat)
# day of year
def doy(dt):
    sec = time.mktime(dt.timetuple())
    t = time.localtime(sec)
    return t.tm_yday

#earth-sun distance 
def dr(doy):
    return 1 + 0.033*math.cos(2*math.pi*doy/365)

#declination
def dec(doy):
    return 0.409*math.sin(2*math.pi*doy/365 - 1.39)

#sunset hour angle
def ws(lat, dec):
    x = 1 - math.pow(math.tan(lat), 2)*math.pow(math.tan(dec), 2)
    if x < 0:
        x = 0.00001
    #print x
    return 0.5*math.pi - math.atan(-math.tan(lat)*math.tan(dec)/math.sqrt(x))

#solar radiation
def Rs(doy, n, lat):
    """n is sunshine duration"""
    lat = lat * math.pi / 180.
    a = 0.25
    b = 0.5
    d = dec(doy)
    w = ws(lat, d)
    N = 24*w/math.pi
    #Extraterrestrial radiation for daily periods
    ra = (24*60*0.082*dr(doy)/math.pi)*(w*math.sin(lat)*math.sin(d) + math.cos(lat)*math.cos(d)*math.sin(w))
    return (a + b*n/N)*ra


####  Spatial Utility Functions  ####

DELTA = 0.000001

def FloatEqual(a, b):
    return abs(a - b) < DELTA

class Raster:
    def __init__(self, nRows, nCols, data, noDataValue=None, geotransform=None, srs=None):
        self.nRows = nRows
        self.nCols = nCols
        self.data = data
        self.noDataValue = noDataValue
        self.geotrans = geotransform
        self.srs = srs
        
        self.dx = geotransform[1]
        self.xMin = geotransform[0]
        self.xMax = geotransform[0] + nCols*geotransform[1]
        self.yMax = geotransform[3]
        self.yMin = geotransform[3] + nRows*geotransform[5]

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
    #print srs.ExportToProj4()
    if noDataValue is None:
        noDataValue = -9999
    return Raster(ysize, xsize, data, noDataValue, geotrans, srs) 

def CopyShpFile(shpFile, dstFile):
    #copy the reach file to new file
    RemoveShpFile(dstFile)
    extlist = ['.shp', '.dbf', '.shx', '.prj']
    prefix = os.path.splitext(shpFile)[0]
    dstPrefix = os.path.splitext(dstFile)[0]
    for ext in extlist:
        src = prefix + ext
        if os.path.exists(src):
            dst = dstPrefix + ext
            shutil.copy(src, dst)
    
def RemoveShpFile(shpFile):
    extlist = ['.shp', '.dbf', '.shx', '.prj']
    prefix = os.path.splitext(shpFile)[0]
    for ext in extlist:
        filename = prefix + ext
        if os.path.exists(filename):
            os.remove(filename)

def NextDay(date):
    year = date.year
    mon = date.month
    day = date.day
    day = day + 1
    if day > GetDayNumber(year, mon):
        day = 1
        mon = mon + 1
    if mon > 12:
        mon = 1
        year = year + 1
    return datetime.datetime(year, mon, day)

def NextHalfDay(date):
    year = date.year
    mon = date.month
    day = date.day
    h = date.hour
    h = h + 12
    if h >= 24:
        h = h - 24
        day = day + 1
    if day > GetDayNumber(year, mon):
        day = 1
        mon = mon + 1
    if mon > 12:
        mon = 1
        year = year + 1
    #print year, mon, day, h
    return datetime.datetime(year, mon, day, h)


def LastHalfDay(date):
    year = date.year
    mon = date.month
    day = date.day
    h = date.hour
    h = h - 12
    if h < 0:
        h = h + 24
        day = day - 1
    
    if day < 1:
        if mon == 1:
            year = year - 1
            mon = 12
            day = 31
        else:
            mon = mon - 1
            day = GetDayNumber(year, mon)
            
    return datetime.datetime(year, mon, day, h)


def IsLeapYear(year):
    if( (year%4 == 0 and year%100 != 0) or (year%400 == 0)):
        return True
    else:
        return False

def GetDayNumber(year, month):
    if month in [1,3,5,7,8,10,12]:
        return 31
    elif month in [4,6,9,11]:
        return 30
    elif IsLeapYear(year):
        return 29
    else:
        return 28

def GetNumberList(s):
    a = []
    iCursor = 0
    for i in range(len(s)):
        if not s[i].isdigit():
            if(s[iCursor:i].isdigit()):
                a.append(int(s[iCursor:i]))
            iCursor = i + 1
    if s[iCursor:].isdigit():
        a.append(int(s[iCursor:]))
    return a

def NashCoef(qObs, qSimu):
    n = len(qObs)
    ave = sum(qObs)/n
    a1 = 0
    a2 = 0
    for i in range(n):
        a1 = a1 + pow(qObs[i]-qSimu[i], 2)
        a2 = a2 + pow(qObs[i] - ave, 2)
    return 1 - a1/a2

def RMSE(list1, list2):
    n = len(list1)
    s = 0
    for i in range(n):
        s = s + pow(list1[i] - list2[i], 2)
    return math.sqrt(s/n)

def GetRasterStat(rasterFile):
    dataset = gdal.Open(rasterFile,GA_ReadOnly)
    if not dataset is None:
        band = dataset.GetRasterBand(1)
        max = band.GetMaximum()
        min = band.GetMinimum()
        if max is None or min is None:
            (min,max) = band.ComputeRasterMinMax(1)
        mean, std = band.ComputeBandStats()
        band = None
        dataset = None
        return (max,min,mean,std)
    dataset = None

def WriteAscFile(filename, data, xsize, ysize, geotransform, noDataValue):
    header = """NCOLS %d
NROWS %d
XLLCENTER %f
YLLCENTER %f
CELLSIZE %f
NODATA_VALUE %f
""" % (xsize, ysize, geotransform[0] + 0.5*geotransform[1], geotransform[3]-(ysize-0.5)*geotransform[1], geotransform[1], noDataValue)
        
    f = open(filename, 'w')
    f.write(header)
    for i in range(0, ysize):
        for j in range(0, xsize):
            f.write(str(data[i][j]) + "\t")
        f.write("\n")
    f.close() 
    
def WriteGTiffFile(filename, nRows, nCols, data, geotransform, srs, noDataValue, gdalType):
    format = "GTiff"
    driver = gdal.GetDriverByName(format)
    ds = driver.Create(filename, nCols, nRows, 1, gdalType)
    ds.SetGeoTransform(geotransform)
    ds.SetProjection(srs.ExportToWkt())
    ds.GetRasterBand(1).SetNoDataValue(noDataValue)
    ds.GetRasterBand(1).WriteArray(data)
    
    ds = None

def WriteGTiffFileByMask(filename, data, mask, gdalType):
    format = "GTiff"
    driver = gdal.GetDriverByName(format)
    ds = driver.Create(filename, mask.nCols, mask.nRows, 1, gdalType)
    ds.SetGeoTransform(mask.geotrans)
    ds.SetProjection(mask.srs.ExportToWkt())
    ds.GetRasterBand(1).SetNoDataValue(mask.noDataValue)
    ds.GetRasterBand(1).WriteArray(data)
    
    ds = None
    
## Export ESRI Shapefile -- Line feature
def WriteLineShp(lineList, outShp):
    print "Write line shapefile: %s" % outShp
    driver = ogr.GetDriverByName("ESRI Shapefile")
    if driver is None:
        print "ESRI Shapefile driver not available."
        sys.exit(1)
    if os.path.exists(outShp):
        driver.DeleteDataSource(outShp)
    ds = driver.CreateDataSource(outShp.rpartition(os.sep)[0])
    if ds is None:
        print "ERROR Output: Creation of output file failed."
        sys.exit(1)
    lyr = ds.CreateLayer(outShp.rpartition(os.sep)[2].split('.')[0], None, ogr.wkbLineString)
    #    for field in fields:
    #        fieldDefn = ogr.FieldDefn(field,ogr.OFTString)
    #        fieldDefn.SetWidth(255)
    #        lyr.CreateField(fieldDefn)
    for l in lineList:
        #        defn = lyr.GetLayerDefn()
        #        featureFields = ogr.Feature(defn)
        #        for field in fields:
        #            featureFields.SetField(field,"test")
        line = ogr.Geometry(ogr.wkbLineString)
        for i in l:
            line.AddPoint(i[0], i[1])
        templine = ogr.CreateGeometryFromJson(line.ExportToJson())
        feature = ogr.Feature(lyr.GetLayerDefn())
        feature.SetGeometry(templine)
        lyr.CreateFeature(feature)
        feature.Destroy()
    ds.Destroy()

## Convert Raster to Vector
def RastertoVector(rasterfile, mask, filepath, vectorfile):
    sourceRaster = gdal.Open(rasterfile)
    mask = gdal.Open(mask)
    band = sourceRaster.GetRasterBand(1)
    band_mask = mask.GetRasterBand(1)
    driver = ogr.GetDriverByName("ESRI Shapefile")
    if(os.path.exists(filepath + os.sep + vectorfile + ".shp")):
        driver.DeleteDataSource(filepath + os.sep + vectorfile + ".shp")
    outDatasource = driver.CreateDataSource(filepath + os.sep + vectorfile + ".shp")
    outLayer = outDatasource.CreateLayer(vectorfile, srs = ReadRaster(rasterfile).srs)
    gdal.Polygonize(band, band_mask, outLayer, -1, [], callback = None) ## band1 is raster, band2 is mask
    outDatasource.Destroy()

def new_raster_from_base(base, outputURI, format, nodata, datatype):
    cols = base.RasterXSize
    rows = base.RasterYSize
    projection = base.GetProjection()
    geotransform = base.GetGeoTransform()
    bands = base.RasterCount
    driver = gdal.GetDriverByName(format)
    new_raster = driver.Create(str(outputURI), cols, rows, bands, datatype)
    new_raster.SetProjection(projection)
    new_raster.SetGeoTransform(geotransform)
    for i in range(bands):
        new_raster.GetRasterBand(i + 1).SetNoDataValue(nodata)
        new_raster.GetRasterBand(i + 1).Fill(nodata)
    return new_raster
## Convert Vector to Raster
def VectortoRaster(raster, shapefile, raster_out):
    aoi_raster = gdal.Open(raster)
    shape_datasource = ogr.Open(shapefile)
    shape_layer = shape_datasource.GetLayer()
    raster_dataset = new_raster_from_base(aoi_raster, raster_out, 'GTiff', -9999, gdal.GDT_Float32)
    band = raster_dataset.GetRasterBand(1)
    nodata = band.GetNoDataValue()
    band.Fill(nodata)
    gdal.RasterizeLayer(raster_dataset, [1], shape_layer, None, None, burn_values=[1], options=["ATTRIBUTE=FID"])

## Feature count in shapefile
def GetFeatureCount(shapefile):
    dataSource = ogr.Open(shapefile, 0)
    if dataSource is None:
        print 'Could not open %s' % shapefile
        os.exit(1)
    layer = dataSource.GetLayer(0)
    fn = layer.GetFeatureCount()
    return fn

def Count(arr, size, x, noDataValue):
    frequency = 0
    if (x == noDataValue):
        return 0
    for i in range(size):
        if (arr[i] == x):
            frequency = frequency + 1
    return frequency

def Mode(arr, size, noDataValue):
    maxv = arr[0]
    rangev = []
    for i in range(size):
        if (maxv < Count(arr, size, arr[i], noDataValue)):
            maxv = arr[i]
            rangev.append(arr[i])
    # print maxv
    if maxv != noDataValue:
        return maxv
    else:
        return numpy.sort(rangev)[-1]

#if __name__ == "__main__":
#    print GetFeatureCount("D:\MasterWorks\AutoFuzSlpPos\Data\Youwuzhen\shp\Ywzh_subbasin.shp")