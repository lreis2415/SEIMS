#! /usr/bin/env python
# coding=utf-8
# @Import hydroClimate sites information and variables
# @Author: Liang-Jun Zhu
#
import pymongo
from pymongo import MongoClient
from pymongo.errors import ConnectionFailure

from config import *
from util import *


class SiteInfo:
    '''
    base class of HydroClimate site information
    :method: init(ID, Name, lat, lon, LocalX, LocalY, alti)
    '''

    def __init__(self, ID=0, Name='',
                 lat=DEFAULT_NODATA, lon=DEFAULT_NODATA,
                 LocalX=DEFAULT_NODATA, LocalY=DEFAULT_NODATA,
                 alti=DEFAULT_NODATA):
        self.StationID = ID  # integer
        self.Name = Name  # station name, string
        self.lat = lat  # latitude, float degree
        self.lon = lon  # longitude, float degree
        self.LocalX = LocalX  # X coordinate in projection, float
        self.LocalY = LocalY  # Y coordinate in projection, float
        self.alti = alti  # altitude, as ORIGIN: unit 0.1 meter

    def LonLat(self):
        return (self.lon, self.lat)

    def LocalXY(self):
        return (self.LocalX, self.LocalY)


# Import HydroClimate sites table
def ImportSitesTable(db, siteFile, siteType, isFirst):
    sitesLoc = {}
    siteDataItems = ReadDataItemsFromTxt(siteFile)
    siteFlds = siteDataItems[0]
    for i in range(1, len(siteDataItems)):
        dic = {}
        for j in range(len(siteDataItems[i])):
            if StringMatch(siteFlds[j], Tag_ST_StationID):
                dic[Tag_ST_StationID.upper()] = int(siteDataItems[i][j])
            elif StringMatch(siteFlds[j], Tag_ST_StationName):
                # unicode(siteDataItems[i][j], 'gb2312')
                dic[Tag_ST_StationName.upper()] = siteDataItems[i][j]
            elif StringMatch(siteFlds[j], Tag_ST_LocalX):
                dic[Tag_ST_LocalX.upper()] = float(siteDataItems[i][j])
            elif StringMatch(siteFlds[j], Tag_ST_LocalY):
                dic[Tag_ST_LocalY.upper()] = float(siteDataItems[i][j])
            elif StringMatch(siteFlds[j], Tag_ST_Longitude):
                dic[Tag_ST_Longitude.upper()] = float(siteDataItems[i][j])
            elif StringMatch(siteFlds[j], Tag_ST_Latitude):
                dic[Tag_ST_Latitude.upper()] = float(siteDataItems[i][j])
            elif StringMatch(siteFlds[j], Tag_ST_Elevation):
                dic[Tag_ST_Elevation.upper()] = float(siteDataItems[i][j])
            elif StringMatch(siteFlds[j], Tag_ST_IsOutlet):
                dic[Tag_ST_IsOutlet.upper()] = float(siteDataItems[i][j])
        dic[Tag_ST_Type.upper()] = siteType
        curfilter = {Tag_ST_StationID.upper(): dic[Tag_ST_StationID.upper()],
                     Tag_ST_Type.upper(): dic[Tag_ST_Type.upper()]}
        db[Tag_ClimateDB_Sites.upper()].find_one_and_replace(
            curfilter, dic, upsert=True)

        if dic[Tag_ST_StationID.upper()] not in sitesLoc.keys():
            sitesLoc[dic[Tag_ST_StationID.upper()]] = SiteInfo(dic[Tag_ST_StationID.upper()],
                     dic[Tag_ST_StationName.upper()],
                     dic[Tag_ST_Latitude.upper()],
                     dic[Tag_ST_Longitude.upper()],
                     dic[Tag_ST_LocalX.upper()],
                     dic[Tag_ST_LocalY.upper()],
                     dic[Tag_ST_Elevation.upper()])
    db[Tag_ClimateDB_Sites.upper()].create_index(
        [(Tag_ST_StationID.upper(), pymongo.ASCENDING),
         (Tag_ST_Type.upper(), pymongo.ASCENDING)])
    return sitesLoc


# Import variables table
def ImportVariableTable(db, varFile, isFirst):
    varDataItems = ReadDataItemsFromTxt(varFile)
    varFlds = varDataItems[0]
    for i in range(1, len(varDataItems)):
        dic = {}
        for j in range(len(varDataItems[i])):
            if StringMatch(varFlds[j], Tag_VAR_Type):
                dic[Tag_VAR_Type.upper()] = varDataItems[i][j]
            elif StringMatch(varFlds[j], Tag_VAR_UNIT):
                dic[Tag_VAR_UNIT.upper()] = varDataItems[i][j]
                # elif StringMatch(varFlds[j], Tag_VAR_IsReg):
                #     dic[Tag_VAR_IsReg.upper()] = varDataItems[i][j]
                # elif StringMatch(varFlds[j], Tag_VAR_Time):
                #     dic[Tag_VAR_Time.upper()] = float(varDataItems[i][j])
        # If this item existed already, then update it, otherwise insert one.
        curfilter = {Tag_VAR_Type.upper(): dic[Tag_VAR_Type.upper()]}
        db[Tag_ClimateDB_VARs.upper()].find_one_and_replace(
            curfilter, dic, upsert=True)


def ImportHydroClimateSitesInfo():
    try:
        connMongo = MongoClient(HOSTNAME, PORT)
        print "Import Climate Sites Information..."
        # print "Connected successfully"
    except ConnectionFailure, e:
        sys.stderr.write("Could not connect to MongoDB: %s" % e)
        sys.exit(1)
    db = connMongo[ClimateDBName]
    cList = db.collection_names()
    tables = [Tag_ClimateDB_Sites.upper(), Tag_ClimateDB_VARs.upper()]
    firstImport = False
    for tb in tables:
        if not StringInList(tb, cList):
            db.create_collection(tb)
            firstImport = True
    ImportVariableTable(db, HydroClimateVarFile, firstImport)
    SiteMLoc = ImportSitesTable(
        db, MetroSiteFile, DataType_Meteorology, firstImport)
    SitePLoc = ImportSitesTable(
        db, PrecSiteFile, DataType_Precipitation, firstImport)
    connMongo.close()
    return (SiteMLoc, SitePLoc)


if __name__ == "__main__":
    LoadConfiguration(GetINIfile())
    SitesMList, SitesPList = ImportHydroClimateSitesInfo()
