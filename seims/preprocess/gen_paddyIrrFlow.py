#! /usr/bin/env python
# coding=utf-8
#
# Author: Shen fang

from __future__ import absolute_import
import os
import sys
import math
import numpy
import sqlite3
# import arcpy
import argparse
from osgeo import gdal
from config import *
from pymongo import MongoClient
from pymongo.errors import ConnectionFailure

DEFAULT_NODATA = -9999.
DB_TAB_POND = "ponds"
### FIELDS IN DB_TAB_PADDYPONFFLOW
POND_PADDYID = "PADDY_CELLID"
POND_PONDID1 = "PONDID1"
POND_PONDID2 = "PONDID2"
POND_PONDID3 = "PONDID3"
POND_REACHID = "REACHID"
PADDYPONDFLOW_DEPTH = "DEPTH"

def StringInList(str, strList):
    newStrList = strList[:]
    for i in range(len(newStrList)):
        newStrList[i] = newStrList[i].lower()
    if str.lower() in newStrList:
        return True
    else:
        return False

LFs = ['\r\n', '\n\r', '\r', '\n']
LF = '\r\n'

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

def GetValueByRowCol(row, col, nRows, nCols, data):
    if row < 0 or row >= nRows or col < 0 or col >= nCols:
        raise ValueError(
            "The row or col must be >=0 and less than nRows or nCols!")
    else:
        value = data[int(round(row))][int(round(col))]
        if value == DEFAULT_NODATA:
            return None
        else:
            return value

def ReadRaster(raster):
    raster = gdal.Open(raster)
    band_raster = raster.GetRasterBand(1)
    data_raster = band_raster.ReadAsArray()
    return data_raster

def find_neighbour_pond(dem, landuse, subbasin, pond):
    # read col, row, cell width from dem
    dem = gdal.Open(dem)
    band_dem = dem.GetRasterBand(1)
    data_dem = band_dem.ReadAsArray()
    nCols = band_dem.XSize
    nRows = band_dem.YSize
    geotrans = dem.GetGeoTransform()
    cell_width = geotrans[1]



    # from pygeoc.raster import RasterUtilClass
    # from osgeo.gdal import GDT_Byte
    # landuse2 = path +  os.sep + "landuse_8bit.tif"
    # lu_r = RasterUtilClass.read_raster(landuse)
    # data_landuse = lu_r.data
    # RasterUtilClass.write_gtiff_file(landuse2, nRows, nCols, data_landuse,
    #                                      geotrans, lu_r.srs, DEFAULT_NODATA, GDT_Byte)

    # read data value of landuse, subbasin, pond
    data_landuse = ReadRaster(landuse)
    data_subbasin = ReadRaster(subbasin)
    data_pond = ReadRaster(pond)

    # set some parameters to search nearest pond
    maxLength = 1000.
    maxNum = 3
    num = int(maxLength / cell_width / 2.)
    nCells = []

    for i in range(nRows):
        for j in range(nCols):
            if data_landuse[i][j] != DEFAULT_NODATA:
                cell_index = i * nCols + j
                nCells.append(cell_index)
                if data_landuse[i][j] == 1.:
                    # get paddy cell id, which is corresponding to the id in the main module
                    cell_id = len(nCells) - 1
                    reach_id = data_subbasin[i][j]
                    dic = {}
                    for k in range(max(0, i - num), min(i + num, nRows)):
                        for m in range(max(0, j - num), min(j + num, nCols)):
                            # for a paddy cell id, search the window (1km * 1km),
                            # if the cell is pond and in the subbasin, then compute the distance
                            if data_subbasin[k][m] == reach_id:
                                if data_landuse[k][m] == 200.:
                                    # the pond id
                                    index = data_pond[k][m]
                                    distance = cell_width * math.sqrt(pow(k - i, 2) + pow(m - j, 2))
                                    dic[index] = distance
                    # sort the dic by distance , and choose the 5 nearest pond, if not enough ,then make it -9999
                    dic_sort = sorted(dic.iteritems(), key=lambda t:t[1], reverse=False)
                    if len(dic_sort) >= maxNum:
                        for n in range(maxNum):
                            locals()['nearest_pond_id_%s' % n]  = dic_sort[n][0]
                    else:
                        for n in range(len(dic_sort)):
                            locals()['nearest_pond_id_%s' % n] = dic_sort[n][0]
                        for n in range(len(dic_sort), maxNum):
                            locals()['nearest_pond_id_%s' % n] = -9999

                    # write to txt
                    flow_table = [cell_id, int(reach_id)]
                    for n in range(maxNum):
                        flow_table.append(locals()['nearest_pond_id_%s' % n])
                    f = open(txtName, 'a')
                    for s in range(len(flow_table)):
                        f.write(str(flow_table[s]) + '\t')
                    f.write("\n")
                    f.close

def ImportPaddyPondFlow(db):
    # delete if existed, create if not existed
    cList = db.collection_names()
    if not StringInList(DB_TAB_POND.upper(), cList):
        db.create_collection(DB_TAB_POND.upper())
    else:
        db.drop_collection(DB_TAB_POND.upper())

    dataItems = ReadDataItemsFromTxt(txtName)
    for id in range(len(dataItems)):
        dic = {}
        dic[POND_PADDYID.upper()] = int(dataItems[id][0])
        dic[POND_REACHID.upper()] = int(dataItems[id][1])
        dic[POND_PONDID1.upper()] = int(dataItems[id][2])
        dic[POND_PONDID2.upper()] = int(dataItems[id][3])
        dic[POND_PONDID3.upper()] = int(dataItems[id][4])
        db[DB_TAB_POND.upper()].insert(dic)

    print 'Paddy pond flow tables are imported.'

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

def LoadConfiguration(inifile):
    strCmd = '%s %s/config.py -ini %s' % (sys.executable,
                                          currentPath(), inifile)
    # print strCmd
    os.system(strCmd)

def currentPath():
    path = sys.path[0]
    if os.path.isdir(path):
        return path
    elif os.path.isfile(path):
        return os.path.dirname(path)

if __name__ == '__main__':
    path = r"D:\code\oryza_20180614\paddyIrr"
    dem  = path +  os.sep + "dem.tif"
    landuse = path +  os.sep + "landuse.tif"
    subbasin = path +  os.sep + "SUBBASIN.tif"
    pond = path +  os.sep + "pond_100m.tif"
    txtName = path + os.sep + "paddy_pond_flow.txt"


    # pond_shp = path + os.sep + "pond.shp"
    # qjrws = arcpy.SearchCursor(pond_shp)
    # pond_area = {}
    # for qjrw in qjrws:
    #     id = qjrw.getValue("PONDID")
    #     pond_area[id] = qjrw.getValue("Shape_Area")
    # find_neighbour_pond(dem, landuse, subbasin, pond)

    ## Load Configuration file
    # LoadConfiguration(GetINIfile())
    import sys
    try:
        conn = MongoClient("127.0.0.1", 27017)
    except ConnectionFailure, e:
        sys.stderr.write("Could not connect to MongoDB: %s" % e)
        sys.exit(1)
    SpatialDBName = "zhongtianshe_100m_model_oryza"
    db = conn[SpatialDBName]
    # ImportPaddyPondFlow(db)

    # import pond raster to mongodb
    str_cmd = '"D:\\code\\oryza_20180614\\SEIMS\\bin/import_raster" ' \
             'D:\\data\\workspace_100m_oryza\\spatial_raster\\mask.tif ' \
             'D:\\code\\oryza_20180614\\pond ' \
             'zhongtianshe_100m_model_oryza SPATIAL 127.0.0.1 27017'

    str_cmd2 = '"D:\\code\\oryza_20180614\\SEIMS\\bin/import_raster" ' \
             'D:\\data\\workspace_100m_oryza\\spatial_raster\\subbasin.tif ' \
             'D:\\code\\oryza_20180614\\pond ' \
             'zhongtianshe_100m_model_oryza SPATIAL 127.0.0.1 27017'

    from pygeoc.utils import UtilClass
    UtilClass.run_command(str_cmd)
    UtilClass.run_command(str_cmd2)

