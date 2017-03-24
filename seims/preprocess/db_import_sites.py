#! /usr/bin/env python
# coding=utf-8
# @Import hydroClimate sites information and variables
# @Author: Liang-Jun Zhu
#
import pymongo

from config import *
from utility import LoadConfiguration, ReadDataItemsFromTxt
from utility import DEFAULT_NODATA
# for test main
from db_mongodb import ConnectMongoDB


class SiteInfo(object):
    """base class of HydroClimate site information."""

    def __init__(self, ID = 0, Name = '', lat = DEFAULT_NODATA, lon = DEFAULT_NODATA,
                 LocalX = DEFAULT_NODATA, LocalY = DEFAULT_NODATA, alti = DEFAULT_NODATA):
        """Initialize a SiteInfo object."""
        self.StationID = ID  # integer
        self.Name = Name  # station name, string
        self.lat = lat  # latitude, float degree
        self.lon = lon  # longitude, float degree
        self.LocalX = LocalX  # X coordinate in projection, float
        self.LocalY = LocalY  # Y coordinate in projection, float
        self.alti = alti  # altitude, as ORIGIN: unit 0.1 meter

    def LonLat(self):
        return self.lon, self.lat

    def LocalXY(self):
        return self.LocalX, self.LocalY


# Import HydroClimate sites table
def ImportSitesTable(db, siteFile, siteType):
    sitesLoc = {}
    siteDataItems = ReadDataItemsFromTxt(siteFile)
    siteFlds = siteDataItems[0]
    for i in range(1, len(siteDataItems)):
        dic = {}
        for j in range(len(siteDataItems[i])):
            if StringClass.stringmatch(siteFlds[j], Tag_ST_StationID):
                dic[Tag_ST_StationID.upper()] = int(siteDataItems[i][j])
            elif StringClass.stringmatch(siteFlds[j], Tag_ST_StationName):
                # unicode(siteDataItems[i][j], 'gb2312')
                dic[Tag_ST_StationName.upper()] = siteDataItems[i][j]
            elif StringClass.stringmatch(siteFlds[j], Tag_ST_LocalX):
                dic[Tag_ST_LocalX.upper()] = float(siteDataItems[i][j])
            elif StringClass.stringmatch(siteFlds[j], Tag_ST_LocalY):
                dic[Tag_ST_LocalY.upper()] = float(siteDataItems[i][j])
            elif StringClass.stringmatch(siteFlds[j], Tag_ST_Longitude):
                dic[Tag_ST_Longitude.upper()] = float(siteDataItems[i][j])
            elif StringClass.stringmatch(siteFlds[j], Tag_ST_Latitude):
                dic[Tag_ST_Latitude.upper()] = float(siteDataItems[i][j])
            elif StringClass.stringmatch(siteFlds[j], Tag_ST_Elevation):
                dic[Tag_ST_Elevation.upper()] = float(siteDataItems[i][j])
            elif StringClass.stringmatch(siteFlds[j], Tag_ST_IsOutlet):
                dic[Tag_ST_IsOutlet.upper()] = float(siteDataItems[i][j])
        dic[Tag_ST_Type.upper()] = siteType
        curfilter = {Tag_ST_StationID.upper(): dic[Tag_ST_StationID.upper()],
                     Tag_ST_Type.upper()     : dic[Tag_ST_Type.upper()]}
        db[Tag_ClimateDB_Sites.upper()].find_one_and_replace(
            curfilter, dic, upsert = True)

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
def ImportVariableTable(db, varFile):
    varDataItems = ReadDataItemsFromTxt(varFile)
    varFlds = varDataItems[0]
    for i in range(1, len(varDataItems)):
        dic = {}
        for j in range(len(varDataItems[i])):
            if StringClass.stringmatch(varFlds[j], Tag_VAR_Type):
                dic[Tag_VAR_Type.upper()] = varDataItems[i][j]
            elif StringClass.stringmatch(varFlds[j], Tag_VAR_UNIT):
                dic[Tag_VAR_UNIT.upper()] = varDataItems[i][j]
        # If this item existed already, then update it, otherwise insert one.
        curfilter = {Tag_VAR_Type.upper(): dic[Tag_VAR_Type.upper()]}
        db[Tag_ClimateDB_VARs.upper()].find_one_and_replace(curfilter, dic, upsert = True)


def ImportHydroClimateSitesInfo(db):
    cList = db.collection_names()
    tables = [Tag_ClimateDB_Sites.upper(), Tag_ClimateDB_VARs.upper()]
    for tb in tables:
        if not StringClass.stringinlist(tb, cList):
            db.create_collection(tb)
    ImportVariableTable(db, HydroClimateVarFile)
    SiteMLoc = ImportSitesTable(db, MetroSiteFile, DataType_Meteorology)
    SitePLoc = ImportSitesTable(db, PrecSiteFile, DataType_Precipitation)
    return SiteMLoc, SitePLoc


if __name__ == "__main__":
    LoadConfiguration(getconfigfile())
    client = ConnectMongoDB(HOSTNAME, PORT)
    conn = client.get_conn()
    climatedb = conn[ClimateDBName]
    SitesMList, SitesPList = ImportHydroClimateSitesInfo(climatedb)
    client.close()
