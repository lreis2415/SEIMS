#! /usr/bin/env python
# coding=utf-8
# @Utility functions
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
#
import argparse
import datetime
import math
import os
import subprocess
import shutil
import socket
import sys
import time
import platform

import numpy
from gdalconst import *
from osgeo import gdal, osr

UTIL_ZERO = 1.e-6
MINI_SLOPE = 0.0001
DEFAULT_NODATA = -9999.
SQ2 = math.sqrt(2.)
# all possible file extensions of ArcGIS shapefile
shp_ext_list = ['.shp', '.dbf', '.shx', '.prj', 'sbn', 'sbx', 'cpg']


class C(object):
    pass


def GetINIfile():
    # Get model configuration file name
    c = C()
    parser = argparse.ArgumentParser(
        description="Read SEIMS Preprocessing configuration file.")
    parser.add_argument('-ini', help="Full path of configuration file")
    args = parser.parse_args(namespace=c)
    iniFile = args.ini
    if not os.path.exists(iniFile):
        raise IOError("%s is not exist, please check and retry!" % iniFile)
    else:
        return iniFile


def isIPValid(address):
    try:
        socket.inet_aton(address)
        return True
    except:
        return False


def FloatEqual(a, b):
    return abs(a - b) < UTIL_ZERO


def isNumericValue(x):
    try:
        xx = float(x)
    except TypeError:
        return False
    except ValueError:
        return False
    except 'Exception':
        return False
    else:
        return True


####  Climate Utility Functions  ####
def IsLeapYear(year):
    if ((year % 4 == 0 and year % 100 != 0) or (year % 400 == 0)):
        return True
    else:
        return False


def GetDayNumber(year, month):
    if month in [1, 3, 5, 7, 8, 10, 12]:
        return 31
    elif month in [4, 6, 9, 11]:
        return 30
    elif IsLeapYear(year):
        return 29
    else:
        return 28


# Solar Radiation Calculation
#  @param doy day of year
#  @param n   sunshine duration
#  @param lat latitude of sites
#  invoke   Rs(doy, n, lat)
# day of year
def doy(dt):
    sec = time.mktime(dt.timetuple())
    t = time.localtime(sec)
    return t.tm_yday


# earth-sun distance
def dr(doy):
    return 1. + 0.033 * math.cos(2. * math.pi * doy / 365.)


# declination
def dec(doy):
    return 0.409 * math.sin(2. * math.pi * doy / 365. - 1.39)


# sunset hour angle
def ws(lat, dec):
    x = 1. - math.pow(math.tan(lat), 2.) * math.pow(math.tan(dec), 2.)
    if x < 0:
        x = 0.00001
    # print x
    return 0.5 * math.pi - math.atan(-math.tan(lat) * math.tan(dec) / math.sqrt(x))


# solar radiation
def Rs(doy, n, lat):
    """n is sunshine duration"""
    lat = lat * math.pi / 180.
    a = 0.25
    b = 0.5
    d = dec(doy)
    w = ws(lat, d)
    N = 24. * w / math.pi
    # Extraterrestrial radiation for daily periods
    ra = (24. * 60. * 0.082 * dr(doy) / math.pi) * (
        w * math.sin(lat) * math.sin(d) + math.cos(lat) * math.cos(d) * math.sin(w))
    return (a + b * n / N) * ra


####  Spatial Utility Functions  ####
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
                "The row or col must be >=0 and less than nRows or nCols!")
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
    return Raster(ysize, xsize, data, noDataValue, geotrans, srs)


def CopyShpFile(shpFile, dstFile):
    # copy the reach file to new file
    RemoveShpFile(dstFile)
    prefix = os.path.splitext(shpFile)[0]
    dstPrefix = os.path.splitext(dstFile)[0]
    for ext in shp_ext_list:
        src = prefix + ext
        if os.path.exists(src):
            dst = dstPrefix + ext
            shutil.copy(src, dst)


def RemoveShpFile(shpFile):
    # shp_ext_list = ['.shp', '.dbf', '.shx', '.prj', 'sbn', 'sbx']
    prefix = os.path.splitext(shpFile)[0]
    for ext in shp_ext_list:
        filename = prefix + ext
        if os.path.exists(filename):
            os.remove(filename)


# TODO: This function can be simply replaced by date +=
# datetime.timedelta(days=1). PLZ check and update. LJ
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
    # print year, mon, day, h
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


def GetNumberList(s):
    a = []
    iCursor = 0
    for i in range(len(s)):
        if not s[i].isdigit():
            if (s[iCursor:i].isdigit()):
                a.append(int(s[iCursor:i]))
            iCursor = i + 1
    if s[iCursor:].isdigit():
        a.append(int(s[iCursor:]))
    return a


def NashCoef(qObs, qSimu):
    '''
    Calculate Nash coefficient
    :param qObs:
    :param qSimu:
    :return: NSE, numeric value
    '''
    if len(qObs) != len(qSimu):
        raise ValueError("The size of observed and simulated values must be the same for NSE calculation!")
    n = len(qObs)
    ave = sum(qObs) / n
    a1 = 0.
    a2 = 0.
    for i in range(n):
        a1 += pow(float(qObs[i]) - float(qSimu[i]), 2.)
        a2 += pow(float(qObs[i]) - ave, 2.)
    if a2 == 0.:
        return 1.
    return 1. - a1 / a2

def RSquare(qObs, qSimu):
    '''
    Calculate R-square
    :param qObs:
    :param qSimu:
    :return: R-square numeric value
    '''
    if len(qObs) != len(qSimu):
        raise ValueError("The size of observed and simulated values must be the same for R-square calculation!")
    n = len(qObs)
    obsAvg = sum(qObs) / n
    predAvg = sum(qSimu) / n
    obsMinusAvgSq = 0.
    predMinusAvgSq = 0.
    obsPredMinusAvgs = 0.
    for i in range(n):
        obsMinusAvgSq += pow((qObs[i] - obsAvg), 2.)
        predMinusAvgSq += pow((qSimu[i] - predAvg), 2.)
        obsPredMinusAvgs += (qObs[i] - obsAvg) * (qSimu[i] - predAvg)
    # Calculate R-square
    yy = (pow(obsMinusAvgSq, 0.5) * pow(predMinusAvgSq, 0.5))
    if yy == 0.:
        return 1.
    return pow((obsPredMinusAvgs / yy), 2.)


def RMSE(list1, list2):
    n = len(list1)
    s = 0.
    for i in range(n):
        s += pow(list1[i] - list2[i], 2.)
    return math.sqrt(s / n)


def GetRasterStat(rasterFile):
    rs = ReadRaster(rasterFile)
    max = rs.GetMax()
    min = rs.GetMin()
    mean = rs.GetAverage()
    std = rs.GetSTD()
    return max, min, mean, std


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


def MaskRaster(cppDir, maskFile, inputFile, outputFile, outputAsc=False, noDataValue=DEFAULT_NODATA):
    id = os.path.basename(maskFile) + "_" + os.path.basename(inputFile)
    configFile = "%s%smaskConfig_%s_%s.txt" % (
        cppDir, os.sep, id, str(time.time()))
    fMask = open(configFile, 'w')
    fMask.write(maskFile + "\n1\n")
    fMask.write("%s\t%d\t%s\n" % (inputFile, noDataValue, outputFile))
    fMask.close()

    asc = ""
    if outputAsc:
        asc = "-asc"
    s = "%s/mask_rasters/build/mask_raster %s %s" % (cppDir, configFile, asc)
    os.system(s)
    os.remove(configFile)


def StringInList(str, strList):
    newStrList = strList[:]
    for i in range(len(newStrList)):
        newStrList[i] = newStrList[i].lower()
    if str.lower() in newStrList:
        return True
    else:
        return False


def StringMatch(str1, str2):
    if str1.lower() == str2.lower():
        return True
    else:
        return False


LFs = ['\r\n', '\n\r', '\r', '\n']

sysstr = platform.system()
if sysstr == "Windows":
    LF = '\r\n'
elif sysstr == "Linux":
    LF = '\n'

def ReadDataItemsFromTxt(txtFile):
    '''
    Read data items include title from text file
    :param txtFile: data file
    :return: 2D data array
    '''
    f = open(txtFile)
    dataItems = []
    for line in f:
        strLine = line
        for LF in LFs:
            if LF in line:
                strLine = line.split(LF)[0]
                break

        # strLine = line.split(LF)[0]
        if strLine != '' and strLine.find('#') < 0:
            lineList = strLine.split('\t')
            dataItems.append(lineList)
    f.close()
    return dataItems


def StripStr(str):
    # @Function: Remove space(' ') and indent('\t') at the begin and end of the string
    oldStr = ''
    newStr = str
    while oldStr != newStr:
        oldStr = newStr
        newStr = oldStr.strip('\t')
        newStr = newStr.strip(' ')
    return newStr


def SplitStr(str, spliters=None):
    # @Function: Split string by spliter space(' ') and indent('\t') as default
    # spliters = [' ', '\t']
    # spliters = []
    # if spliter is not None:
    #     spliters.append(spliter)
    if spliters is None:
        spliters = [' ', '\t']
    destStrs = []
    srcStrs = [str]
    while True:
        oldDestStrs = srcStrs[:]
        for s in spliters:
            for srcS in srcStrs:
                tempStrs = srcS.split(s)
                for tempS in tempStrs:
                    tempS = StripStr(tempS)
                    if tempS != '':
                        destStrs.append(tempS)
            srcStrs = destStrs[:]
            destStrs = []
        if oldDestStrs == srcStrs:
            destStrs = srcStrs[:]
            break
    return destStrs


def IsSubString(SubStr, Str):
    if SubStr.lower() in Str.lower():
        return True
    else:
        return False


def replaceByDict(srcfile, vDict, dstfile):
    srcR = ReadRaster(srcfile)
    srcData = srcR.data
    dstData = numpy.copy(srcData)
    for k, v in vDict.iteritems():
        dstData[srcData == k] = v
    WriteGTiffFile(dstfile, srcR.nRows, srcR.nCols, dstData,
                   srcR.geotrans, srcR.srs, srcR.noDataValue, GDT_Float32)


def GetFileNameWithSuffixes(filePath, suffixes):
    listFiles = os.listdir(filePath)
    reFiles = []
    for f in listFiles:
        name, ext = os.path.splitext(f)
        if StringInList(ext, suffixes):
            reFiles.append(f)
    return reFiles


def isPathExists(path):
    if os.path.isdir(path):
        if os.path.exists(path):
            return True
        else:
            return False
    else:
        return False


def GetFullPathWithSuffixes(filePath, suffixes):
    fullPaths = []
    for name in GetFileNameWithSuffixes(filePath, suffixes):
        fullPaths.append(filePath + os.sep + name)
    return fullPaths


def currentPath():
    path = sys.path[0]
    if os.path.isdir(path):
        return path
    elif os.path.isfile(path):
        return os.path.dirname(path)


def LoadConfiguration(inifile):
    strCmd = '%s %s/config.py -ini %s' % (sys.executable,
                                          currentPath(), inifile)
    # print strCmd
    os.system(strCmd)

def GetExecutableFullPath(name):
    '''
    Not for Windows
    get the full path of a given executable name
    :return:
    '''
    findout = RunExternalCmd('which %s' % name)
    if findout == [] or len(findout) == 0:
        print ("%s is not included in the env path" % name)
        exit(-1)
    return findout[0].split('\n')[0]

def RunExternalCmd(cmdStr):
    '''
    Execute external command, and return the output lines list
    :param cmdStr:
    :return: output lines
    '''
    process = subprocess.Popen(cmdStr, shell = True, stdout = subprocess.PIPE)
    return process.stdout.readlines()

# TEST CODE
if __name__ == "__main__":
    LoadConfiguration(GetINIfile())
    # p = r'E:\data\model_data\model_dianbu_10m_longterm\data_prepare\spatial'
    # print GetFileNameWithSuffixes(p,['.tif','.txt'])
    # print GetFullPathWithSuffixes(p,['.tif','.txt'])
    # dist2Stream = r'E:\data_m\SEIMS\dianbu_30m_output\dist2Stream.tif'
    # R = ReadRaster(dist2Stream)
    # print "XMin: %f" % R.xMin
    # print "XMax: %f" % R.xMax
    # print "YMin: %f" % R.yMin
    # print "YMax: %f" % R.yMax
    # print "Mean: %f" % R.GetAverage()
    # print "Max : %f" % R.GetMax()
    # print "Min : %f" % R.GetMin()
    # print "Sum : %f" % R.GetSum()
    # print "STD : %f" % R.GetSTD()
    # print "Value at (x, y): %f" % R.GetValueByXY(39542419.65, 3543174.289)
    datafile = r'G:\code_zhulj\SEIMS\model_data\dianbu\data_prepare\climate\Sites_P.txt'
    data = ReadDataItemsFromTxt(datafile)
    print (data)


def WriteLog(logfile, contentlist, MODE='replace'):
    if os.path.exists(logfile):
        if MODE == 'replace':
            os.remove(logfile)
            logStatus = open(logfile, 'w')
        else:
            logStatus = open(logfile, 'a')
    else:
        logStatus = open(logfile, 'w')
    if isinstance(contentlist, list) or isinstance(contentlist,tuple):
        for content in contentlist:
            logStatus.write("%s\n" % content)
    else:
        logStatus.write(contentlist)
    logStatus.flush()
    logStatus.close()