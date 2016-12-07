#! /usr/bin/env python
# coding=utf-8
# Find meteorology and precipitation sites in study area
# Author: Junzhi Liu
# Revised: Liang-Jun Zhu
#
import pymongo
from osgeo import ogr
from pymongo import MongoClient
from pymongo.errors import ConnectionFailure
from shapely.wkt import loads

from config import *


def OGRWkt2Shapely(input_shape, idField):
    shapely_objects = []
    idList = []
    # print input_shape
    shp = ogr.Open(input_shape)
    lyr = shp.GetLayer()
    for n in range(0, lyr.GetFeatureCount()):
        feat = lyr.GetFeature(n)
        wkt_feat = loads(feat.geometry().ExportToWkt())
        shapely_objects.append(wkt_feat)
        idIndex = feat.GetFieldIndex(idField)
        idList.append(feat.GetField(idIndex))
    return shapely_objects, idList


def FindSites(db, hydroDBName, subbasinFile, subbasinIdField, thiessenFileList, siteTypeList, mode):
    subbasinList, subbasinIDList = OGRWkt2Shapely(subbasinFile, subbasinIdField)
    nSubbasins = len(subbasinList)

    n = len(thiessenFileList)
    siteDic = {}
    for i in range(len(subbasinList)):
        subbasin = subbasinList[i]
        id = subbasinIDList[i]
        # if not for cluster and basin.shp is used, force the SUBBASINID to 0. by LJ.
        if not forCluster and len(subbasinList) == 1:
            id = 0
        dic = {}
        dic[FLD_SUBBASINID.upper()] = id
        dic[FLD_DB.upper()] = hydroDBName
        dic[FLD_MODE.upper()] = mode
        curFileter = {FLD_SUBBASINID.upper(): id,
                      FLD_DB.upper(): hydroDBName,
                      FLD_MODE.upper(): mode}
        for metroID in range(0, n):
            thiessenFile = thiessenFileList[metroID]
            # print thiessenFile
            siteType = siteTypeList[metroID]
            try:
                thiessenList, thiessenIDList = OGRWkt2Shapely(thiessenFile, thiessenIdField)
            except:
                thiessenList, thiessenIDList = OGRWkt2Shapely(thiessenFile, 'Subbasin')
            siteList = []
            for polyID in range(len(thiessenList)):
                thiessen = thiessenList[polyID]
                if subbasin.intersects(thiessen):
                    siteList.append(thiessenIDList[polyID])
            siteList.sort()
            slist = [str(item) for item in siteList]
            siteListStr = ','.join(slist)

            siteField = '%s%s' % (DB_TAB_SITELIST.upper(), siteType)
            dic[siteField] = siteListStr
        db[DB_TAB_SITELIST.upper()].find_one_and_replace(curFileter, dic, upsert=True)

    db[DB_TAB_SITELIST.upper()].create_index([(FLD_SUBBASINID.upper(), pymongo.ASCENDING),
                                              (FLD_MODE.upper(), pymongo.ASCENDING)])
    # print 'Meteorology sites table was generated.'
    return nSubbasins


# TEST CODE
if __name__ == "__main__":
    hostname = '127.0.0.1'
    port = 27017
    try:
        conn = MongoClient(host=hostname, port=27017)
        print "Connected successfully"
    except ConnectionFailure, e:
        sys.stderr.write("Could not connect to MongoDB: %s" % e)
        sys.exit(1)

    thiessenFileList = [MeteorSitesThiessen, PrecSitesThiessen]
    typeList = [DataType_Meteorology, DataType_Precipitation]
    db = conn[SpatialDBName]
    if not forCluster:
        basinFile = WORKING_DIR + os.sep + basinVec
        FindSites(db, ClimateDBName, basinFile, FLD_BASINID, thiessenFileList, typeList, 'DAILY')
    subbasinFile = WORKING_DIR + os.sep + DIR_NAME_SUBBSN + os.sep + subbasinVec  # subbasin.shp, for MPI version
    FindSites(db, ClimateDBName, subbasinFile, FLD_SUBBASINID, thiessenFileList, typeList, 'DAILY')
