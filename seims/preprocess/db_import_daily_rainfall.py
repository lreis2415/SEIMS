#! /usr/bin/env python
# coding=utf-8
# @Import daily precipitation data
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
#
import time
import datetime

import pymongo

from config import *
from utility import LoadConfiguration, ReadDataItemsFromTxt
# for test main
from db_import_sites import ImportHydroClimateSitesInfo
from db_mongodb import ConnectMongoDB


def ImportPrecipitation(db, ClimateDateFile, sitesLoc, isFirst):
    climDataItems = ReadDataItemsFromTxt(ClimateDateFile)
    climFlds = climDataItems[0]
    StationID = []
    for i in range(3, len(climFlds)):
        StationID.append(climFlds[i])
    for i in range(1, len(climDataItems)):
        dic = {}
        precipitation = []
        curY = 0
        curM = 0
        curD = 0
        for j in range(len(climDataItems[i])):
            if StringClass.stringmatch(climFlds[j], Tag_DT_Year):
                curY = int(climDataItems[i][j])
            elif StringClass.stringmatch(climFlds[j], Tag_DT_Month):
                curM = int(climDataItems[i][j])
            elif StringClass.stringmatch(climFlds[j], Tag_DT_Day):
                curD = int(climDataItems[i][j])
            else:
                for k in range(len(StationID)):
                    if StringClass.stringmatch(climFlds[j], StationID[k]):
                        precipitation.append(float(climDataItems[i][j]))

        dt = datetime.datetime(curY, curM, curD, 0, 0)
        sec = time.mktime(dt.timetuple())
        utcTime = time.gmtime(sec)
        dic[Tag_DT_LocalT.upper()] = dt
        dic[Tag_DT_Zone.upper()] = time.timezone / 3600.
        dic[Tag_DT_UTC.upper()] = datetime.datetime(
            utcTime[0], utcTime[1], utcTime[2], utcTime[3])

        for j in range(len(StationID)):
            curDic = {}
            curDic[Tag_DT_Value.upper()] = precipitation[j]
            curDic[Tag_DT_StationID.upper()] = int(StationID[j])
            curDic[Tag_DT_Type.upper()] = DataType_Precipitation
            curDic[Tag_DT_Zone.upper()] = dic[Tag_DT_Zone.upper()]
            curDic[Tag_DT_LocalT.upper()] = dic[Tag_DT_LocalT.upper()]
            curDic[Tag_DT_UTC.upper()] = dic[Tag_DT_UTC.upper()]
            curfilter = {Tag_DT_StationID.upper(): curDic[Tag_DT_StationID.upper()],
                         Tag_DT_Type.upper()     : curDic[Tag_DT_Type.upper()],
                         Tag_DT_UTC.upper()      : curDic[Tag_DT_UTC.upper()]}
            if (isFirst):
                db[DB_TAB_DATAVALUES.upper()].insert_one(curDic)
            else:
                db[DB_TAB_DATAVALUES.upper()].find_one_and_replace(curfilter, curDic, upsert = True)
    # Create index
    db[DB_TAB_DATAVALUES.upper()].create_index(
        [(Tag_DT_StationID.upper(), pymongo.ASCENDING),
         (Tag_DT_Type.upper(), pymongo.ASCENDING),
         (Tag_DT_UTC.upper(), pymongo.ASCENDING)])


def ImportDailyPrecData(db, sitePLoc):
    print ("Import Daily Precipitation Data... ")
    cList = db.collection_names()
    firstImport = False
    if not StringClass.stringinlist(DB_TAB_DATAVALUES, cList):
        db.create_collection(DB_TAB_DATAVALUES.upper())
        firstImport = True

    ImportPrecipitation(db, PrecDailyFile, sitePLoc, firstImport)


if __name__ == "__main__":
    LoadConfiguration(getconfigfile())
    client = ConnectMongoDB(HOSTNAME, PORT)
    conn = client.get_conn()
    db = conn[ClimateDBName]
    SitesMList, SitesPList = ImportHydroClimateSitesInfo(db)
    ImportDailyPrecData(db, SitesPList)
    client.close()
