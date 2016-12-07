#! /usr/bin/env python
# coding=utf-8
# @Import BMP Scenario related parameters to MongoDB
# @Author Liang-Jun Zhu
# @Date   2016-6-16
#

from pymongo import MongoClient
from pymongo.errors import ConnectionFailure

from config import *
from util import *


def ImportBMPTables():
    '''
    Import BMPs Scenario data to MongoDB
    '''
    if not useScernario:
        return False
    BMPFiles = GetFileNameWithSuffixes(BMP_DATA_DIR, ['.txt'])
    BMP_tabs = []
    BMP_tabs_path = []
    for f in BMPFiles:
        BMP_tabs.append(f.split('.')[0])
        BMP_tabs_path.append(BMP_DATA_DIR + os.sep + f)
    # connect to MongoDB
    try:
        conn = MongoClient(HOSTNAME, PORT)
        print ("Import BMP Scenario Data... ")
    except ConnectionFailure:
        raise IOError("Could not connect to MongoDB: %s" % ConnectionFailure.message)
    db = conn[BMPScenarioDBName]
    # create if collection not existed
    cList = db.collection_names()
    for item in BMP_tabs:
        if not StringInList(item.upper(), cList):
            db.create_collection(item.upper())
        else:
            db.drop_collection(item.upper())
    # Read subbasin.tif and dist2Stream.tif
    subbasinR = ReadRaster(WORKING_DIR + os.sep + subbasinOut)
    dist2StreamR = ReadRaster(WORKING_DIR + os.sep + dist2StreamD8M)
    # End reading
    for j in range(len(BMP_tabs_path)):
        bmpTxt = BMP_tabs_path[j]
        bmpTabName = BMP_tabs[j]
        dataArray = ReadDataItemsFromTxt(bmpTxt)
        fieldArray = dataArray[0]
        dataArray = dataArray[1:]
        for item in dataArray:
            dic = {}
            for i in range(len(fieldArray)):
                if isNumericValue(item[i]):
                    dic[fieldArray[i].upper()] = float(item[i])
                else:
                    dic[fieldArray[i].upper()] = str(item[i]).upper()
            if StringInList(Tag_ST_LocalX, dic.keys()) and StringInList(Tag_ST_LocalY, dic.keys()):
                subbsnID = subbasinR.GetValueByXY(
                    dic[Tag_ST_LocalX.upper()], dic[Tag_ST_LocalY.upper()])
                distance = dist2StreamR.GetValueByXY(
                    dic[Tag_ST_LocalX.upper()], dic[Tag_ST_LocalY.upper()])
                if subbsnID is not None and distance is not None:
                    dic[REACH_SUBBASIN.upper()] = float(subbsnID)
                    dic[BMP_FLD_PT_DISTDOWN.upper()] = float(distance)
                    db[bmpTabName.upper()].find_one_and_replace(
                        dic, dic, upsert=True)
            else:
                db[bmpTabName.upper()].find_one_and_replace(
                    dic, dic, upsert=True)
    # print 'BMP tables are imported.'
    # Write BMP database name into Model main database
    mainDB = conn[SpatialDBName]
    cList = mainDB.collection_names()
    if not StringInList(DB_TAB_BMP_DB.upper(), cList):
        mainDB.create_collection(DB_TAB_BMP_DB.upper())

    bmpInfoDic = dict()
    bmpInfoDic[FLD_DB.upper()] = BMPScenarioDBName
    mainDB[DB_TAB_BMP_DB.upper()].find_one_and_replace(
        bmpInfoDic, bmpInfoDic, upsert=True)
    conn.close()
    return True

if __name__ == "__main__":
    LoadConfiguration(GetINIfile())
    ImportBMPTables()
