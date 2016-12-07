#! /usr/bin/env python
# coding=utf-8
# @Redistribute landuse parameters
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
#

import sqlite3

from config import *
from util import *


def ReclassLanduse(landuseFile, dbname, dstdir):
    # code:{property_name:value}
    # for example:
    # 1:{"clay":0.12, "sand":0.1}
    property_map = {}
    str_sql = 'select landuse_id, ' + \
        ','.join(LANDUSE_ATTR_LIST) + ' from LanduseLookup'
    property_namelist = LANDUSE_ATTR_LIST
    num_propeties = len(property_namelist)

    for i in xrange(num_propeties):
        property_map[property_namelist[i]] = {}

    conn = sqlite3.connect(dbname)
    cursor = conn.cursor()

    cursor.execute(str_sql)
    for row in cursor:
        # print row
        id = int(row[0])
        for i in xrange(num_propeties):
            pName = property_namelist[i]
            dic = property_map[pName]
            if pName != "USLE_P":
                dic[id] = row[i + 1]
            else:
                dic[id] = 1

    ds = gdal.Open(landuseFile)
    band = ds.GetRasterBand(1)
    data = band.ReadAsArray()

    xsize = band.XSize
    ysize = band.YSize
    noDataValue = band.GetNoDataValue()
    # print noDataValue
    geotransform = ds.GetGeoTransform()

    srs = osr.SpatialReference()
    srs.ImportFromWkt(ds.GetProjection())

    n = xsize * ysize
    data.shape = n
    attrList = []
    for iprop in xrange(num_propeties):
        pname = property_namelist[iprop]
        filename = dstdir + os.sep + pname + ".tif"
        attrList.append(filename)
        data_prop = numpy.zeros(n)
        dic = property_map[pname]
        for i in xrange(n):
            id = int(data[i])
            data_prop[i] = dic[id] if id > 0 else noDataValue
        data_prop.shape = (ysize, xsize)
        WriteGTiffFile(filename, ysize, xsize, data_prop,
                       geotransform, srs, noDataValue, gdal.GDT_Float32)
    print 'The landuse parameters are generated!'
    return attrList


if __name__ == '__main__':
    LoadConfiguration(GetINIfile())
    ReclassLanduse(WORKING_DIR + os.sep + landuseMFile,
                   TXT_DB_DIR + os.sep + sqliteFile, WORKING_DIR)
