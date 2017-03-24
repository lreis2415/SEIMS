#! /usr/bin/env python
# coding=utf-8
# @Import BMP Scenario related parameters to MongoDB
# @Author Liang-Jun Zhu
# @Date   2016-6-16
#

from pygeoc.raster.raster import RasterUtilClass
from pygeoc.utils.utils import MathClass

from config import *
from utility import LoadConfiguration, ReadDataItemsFromTxt
# for test main
from db_mongodb import ConnectMongoDB


def ImportBMPTables(maindb, scenariodb):
    """
    Import BMPs Scenario data to MongoDB
    """
    if not useScernario:
        return False
    print ("Import BMP Scenario Data... ")
    BMPFiles = FileClass.getfilenamebysuffixes(BMP_DATA_DIR, ['.txt'])
    BMP_tabs = []
    BMP_tabs_path = []
    for f in BMPFiles:
        BMP_tabs.append(f.split('.')[0])
        BMP_tabs_path.append(BMP_DATA_DIR + os.sep + f)

    # create if collection not existed
    cList = scenariodb.collection_names()
    for item in BMP_tabs:
        if not StringClass.stringinlist(item.upper(), cList):
            scenariodb.create_collection(item.upper())
        else:
            scenariodb.drop_collection(item.upper())
    # Read subbasin.tif and dist2Stream.tif
    subbasinR = RasterUtilClass.ReadRaster(WORKING_DIR + os.sep + DIR_NAME_GEODATA2DB + os.sep + subbasinOut)
    dist2StreamR = RasterUtilClass.ReadRaster(WORKING_DIR + os.sep + DIR_NAME_GEODATA2DB + os.sep + dist2StreamD8M)
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
                if MathClass.isnumerical(item[i]):
                    dic[fieldArray[i].upper()] = float(item[i])
                else:
                    dic[fieldArray[i].upper()] = str(item[i]).upper()
            if StringClass.stringinlist(Tag_ST_LocalX, dic.keys()) and StringClass.stringinlist(Tag_ST_LocalY,
                                                                                                dic.keys()):
                subbsnID = subbasinR.GetValueByXY(
                    dic[Tag_ST_LocalX.upper()], dic[Tag_ST_LocalY.upper()])
                distance = dist2StreamR.GetValueByXY(
                    dic[Tag_ST_LocalX.upper()], dic[Tag_ST_LocalY.upper()])
                if subbsnID is not None and distance is not None:
                    dic[REACH_SUBBASIN.upper()] = float(subbsnID)
                    dic[BMP_FLD_PT_DISTDOWN.upper()] = float(distance)
                    scenariodb[bmpTabName.upper()].find_one_and_replace(dic, dic, upsert = True)
            else:
                scenariodb[bmpTabName.upper()].find_one_and_replace(dic, dic, upsert = True)
    # print 'BMP tables are imported.'
    # Write BMP database name into Model main database
    cList = maindb.collection_names()
    if not StringClass.stringinlist(DB_TAB_BMP_DB.upper(), cList):
        maindb.create_collection(DB_TAB_BMP_DB.upper())

    bmpInfoDic = dict()
    bmpInfoDic[FLD_DB.upper()] = BMPScenarioDBName
    maindb[DB_TAB_BMP_DB.upper()].find_one_and_replace(bmpInfoDic, bmpInfoDic, upsert = True)
    return True


if __name__ == "__main__":
    LoadConfiguration(getconfigfile())
    client = ConnectMongoDB(HOSTNAME, PORT)
    conn = client.get_conn()
    miandb = conn[ClimateDBName]
    scenariodb = conn[BMPScenarioDBName]
    ImportBMPTables(miandb, scenariodb)
    client.close()
