#! /usr/bin/env python
# coding=utf-8
# @Import measurement data, such as discharge, sediment yield, and nutrient export etc.
# @Author: Fang Shen
# @Revised: Liang-Jun Zhu
# @Description: Check the location of observed stations and add subbasinID field. 2016-10-31, TODO, not finished yet!
#
from pymongo import MongoClient
from pymongo.errors import ConnectionFailure

from config import *

def matchSubbasin(siteDict):
    '''
    Match the ID of subbasin
        1. Read the coordinates of each subbasin's outlet, and
           the outlet ID of the whole basin (not finished yet)
        2. If the isOutlet field equals to
           2.1 - 0, then return the subbasinID of the site's location
           2.2 - 1, then return the outlet ID of the whole basiin
           2.3 - 2, then return the outlet ID of nearest subbasin
           2.4 - 3, then return the outlet IDs of the conjunct subbasins
    :param siteDict:
    :return:
    '''
    subbasinRasterFile = WORKING_DIR + os.sep + subbasinOut
    subbasinRaster = ReadRaster(subbasinRasterFile)
    localx = siteDict.get(Tag_ST_LocalX.upper())
    localy = siteDict.get(Tag_ST_LocalY.upper())
    siteType = siteDict.get(Tag_ST_IsOutlet.upper())
    subbasinID = subbasinRaster.GetValueByXY(localx, localy)
    if subbasinID is None and siteType != 1:  # the site is not inside the basin and not the outlet either.
        return False, None
    if siteType == 0:
        return True, [subbasinID]
    elif siteType == 1:
        return True, [subbasinID]  # TODO, get outlet ID of the whole basin, refers to gen_subbasin.py
    elif siteType == 2:
        return True, [subbasinID]  # TODO
def ImportData(db, ObsTxtsList, sitesInfoTxtsList):
    '''
    Import observed values for current model. The procedure including several steps:
        1. Read monitor station information, filter by LocalX and LocalY coordinates,
           and store variables information (siteDic) and station IDs (siteIDs)
        2. Read observed data and import to MongoDB
        3. Add observed data with unit converted

    :param db:
    :param ObsTxtsList:
    :param sitesInfoTxtsList:
    :return: True or False
    '''
    # 1. Read monitor station information, and store variables information and station IDs
    variableLists = []
    siteIDs = []
    for siteFile in sitesInfoTxtsList:
        siteDataItems = ReadDataItemsFromTxt(siteFile)
        siteFlds = siteDataItems[0]
        for i in range(1, len(siteDataItems)):
            dic = {}
            for j in range(len(siteDataItems[i])):
                if StringMatch(siteFlds[j], Tag_ST_StationID):
                    dic[Tag_ST_StationID.upper()] = int(siteDataItems[i][j])
                    siteIDs.append(dic[Tag_ST_StationID.upper()])
                elif StringMatch(siteFlds[j], Tag_ST_StationName):
                    dic[Tag_ST_StationName.upper()] = StripStr(siteDataItems[i][j])
                elif StringMatch(siteFlds[j], Tag_ST_Type):
                    type = SplitStr(StripStr(siteDataItems[i][j]), ',')
                elif StringMatch(siteFlds[j], Tag_ST_Latitude):
                    dic[Tag_ST_Latitude.upper()] = float(siteDataItems[i][j])
                elif StringMatch(siteFlds[j], Tag_ST_Longitude):
                    dic[Tag_ST_Longitude.upper()] = float(siteDataItems[i][j])
                elif StringMatch(siteFlds[j], Tag_ST_LocalX):
                    dic[Tag_ST_LocalX.upper()] = float(siteDataItems[i][j])
                elif StringMatch(siteFlds[j], Tag_ST_LocalY):
                    dic[Tag_ST_LocalY.upper()] = float(siteDataItems[i][j])
                elif StringMatch(siteFlds[j], Tag_ST_UNIT):
                    dic[Tag_ST_UNIT.upper()] = StripStr(siteDataItems[i][j])
                elif StringMatch(siteFlds[j], Tag_ST_Elevation):
                    dic[Tag_ST_Elevation.upper()] = float(siteDataItems[i][j])
                elif StringMatch(siteFlds[j], Tag_ST_IsOutlet):
                    dic[Tag_ST_IsOutlet.upper()] = float(siteDataItems[i][j])

            for j in range(len(type)):
                siteDic = {}
                siteDic[Tag_ST_StationID.upper()] = dic[Tag_ST_StationID.upper()]
                siteDic[Tag_ST_StationName.upper()] = dic[Tag_ST_StationName.upper()]
                siteDic[Tag_ST_Type.upper()] = type[j]
                siteDic[Tag_ST_Latitude.upper()] = dic[Tag_ST_Latitude.upper()]
                siteDic[Tag_ST_Longitude.upper()] = dic[Tag_ST_Longitude.upper()]
                siteDic[Tag_ST_LocalX.upper()] = dic[Tag_ST_LocalX.upper()]
                siteDic[Tag_ST_LocalY.upper()] = dic[Tag_ST_LocalY.upper()]
                siteDic[Tag_ST_Elevation.upper()] = dic[Tag_ST_Elevation.upper()]
                siteDic[Tag_ST_IsOutlet.upper()] = dic[Tag_ST_IsOutlet.upper()]
                # Add SubbasinID field
                matched, curSubbasinID = matchSubbasin(siteDic)
                if not matched:
                    break
                curSubbasinIDStr = ''
                for id in curSubbasinID:
                    if id is not None:
                        curSubbasinIDStr += str(id) + ','
                curSubbasinIDStr = curSubbasinIDStr[:-1]
                siteDic[Tag_ST_SubbasinID.upper()] = curSubbasinIDStr
                curfilter = {Tag_ST_StationID.upper(): siteDic[Tag_ST_StationID.upper()],
                             Tag_ST_Type.upper(): siteDic[Tag_ST_Type.upper()]}
                # print (curfilter)
                db[Tag_ClimateDB_Sites.upper()].find_one_and_replace(curfilter, siteDic, upsert=True)

                varDic = {}
                varDic[Tag_ST_Type.upper()] = type[j]
                varDic[Tag_ST_UNIT.upper()] = dic[Tag_ST_UNIT.upper()]
                if varDic not in variableLists:
                    variableLists.append(varDic)
                # curfilter = {Tag_ST_Type.upper(): varDic[Tag_ST_Type.upper()]}
                # db[Tag_ClimateDB_VARs.upper()].find_one_and_replace(curfilter, varDic, upsert = True)
    siteIDs = list(set(siteIDs))
    # 2. Read measurement data and import to MongoDB
    for measDataFile in ObsTxtsList:
        # print measDataFile
        measDataItems = ReadDataItemsFromTxt(measDataFile)
        # If the data items is EMPTY or only have one header row, then goto
        # next data file.
        if measDataItems == [] or len(measDataItems) == 1:
            continue
        measFlds = measDataItems[0]
        requiredFlds = [Tag_ST_StationID.upper(), Tag_DT_Year.upper(), Tag_DT_Month.upper(),
                        Tag_DT_Day.upper(), Tag_DT_Type.upper(), Tag_DT_Value.upper()]
        for fld in requiredFlds:
            if not StringInList(fld, measFlds):  # data can not meet the request!
                raise ValueError("The %s cann't meet the required format!" % measDataFile)
        for i in range(1, len(measDataItems)):
            dic = {}
            for j in range(len(measDataItems[i])):
                if StringMatch(measFlds[j], Tag_ST_StationID):
                    dic[Tag_ST_StationID.upper()] = int(measDataItems[i][j])
                    # if current site ID is not included, goto next data item
                    if dic[Tag_ST_StationID.upper()] not in siteIDs:
                        continue
                elif StringMatch(measFlds[j], Tag_DT_Year):
                    curY = int(measDataItems[i][j])
                elif StringMatch(measFlds[j], Tag_DT_Month):
                    curM = int(measDataItems[i][j])
                elif StringMatch(measFlds[j], Tag_DT_Day):
                    curD = int(measDataItems[i][j])
                elif StringMatch(measFlds[j], Tag_DT_Type):
                    dic[Tag_DT_Type.upper()] = measDataItems[i][j]
                elif StringMatch(measFlds[j], Tag_DT_Value):
                    dic[Tag_DT_Value.upper()] = float(measDataItems[i][j])
            dt = datetime.datetime(curY, curM, curD, 0, 0)
            sec = time.mktime(dt.timetuple())
            utcTime = time.gmtime(sec)
            dic[Tag_DT_LocalT.upper()] = dt
            dic[Tag_DT_Zone.upper()] = time.timezone / 3600
            dic[Tag_DT_UTC.upper()] = datetime.datetime(
                utcTime[0], utcTime[1], utcTime[2], utcTime[3])
            curfilter = {Tag_ST_StationID.upper(): dic[Tag_ST_StationID.upper()],
                         Tag_DT_Type.upper(): dic[Tag_DT_Type.upper()],
                         Tag_DT_UTC.upper(): dic[Tag_DT_UTC.upper()]}
            db[Tag_ClimateDB_Measurement.upper()].find_one_and_replace(curfilter, dic, upsert=True)
    # 3. Add measurement data with unit converted
    # loop variables list
    addedDics = []
    for curVar in variableLists:
        # print curVar
        # if the unit is mg/L, then change the Type name with the suffix "Conc",
        # and convert the corresponding data to kg if the discharge data is
        # available.
        curType = curVar[Tag_ST_Type.upper()]
        curUnit = curVar[Tag_ST_UNIT.upper()]
        # Find data by Type
        for item in db[Tag_ClimateDB_Measurement.upper()].find({Tag_ST_Type.upper(): curType}):
            # print item
            dic = {}
            dic[Tag_ST_StationID.upper()] = item[Tag_ST_StationID.upper()]
            dic[Tag_DT_Value.upper()] = item[Tag_DT_Value.upper()]
            dic[Tag_ST_Type.upper()] = item[Tag_ST_Type.upper()]
            dic[Tag_DT_LocalT.upper()] = item[Tag_DT_LocalT.upper()]
            dic[Tag_DT_Zone.upper()] = item[Tag_DT_Zone.upper()]
            dic[Tag_DT_UTC.upper()] = item[Tag_DT_UTC.upper()]

            if curUnit == "mg/L":
                # update the Type name
                dic[Tag_ST_Type.upper()] = curType + "Conc"
                curfilter = {Tag_ST_StationID.upper(): dic[Tag_ST_StationID.upper()],
                             Tag_DT_Type.upper(): curType,
                             Tag_DT_UTC.upper(): dic[Tag_DT_UTC.upper()]}
                db[Tag_ClimateDB_Measurement.upper()].find_one_and_replace(
                    curfilter, dic, upsert=True)
                dic[Tag_ST_Type.upper()] = curType

            # find discharge on current day
            filter = {Tag_ST_Type.upper(): "Q",
                      Tag_DT_UTC.upper(): dic[Tag_DT_UTC.upper()],
                      Tag_ST_StationID.upper(): dic[Tag_ST_StationID.upper()]}
            QDic = db[Tag_ClimateDB_Measurement.upper()].find_one(
                filter=filter)

            q = -9999.
            if QDic is not None:  # and QDic.has_key(Tag_DT_Value.upper()):
                q = QDic[Tag_DT_Value.upper()]
            else:
                continue
            if curUnit == "mg/L":
                # convert mg/L to kg
                dic[Tag_DT_Value.upper()] = round(dic[Tag_DT_Value.upper()] * q * 86400. / 1000., 2)
            elif curUnit == "kg":
                dic[Tag_ST_Type.upper()] = curType + "Conc"
                # convert kg to mg/L
                dic[Tag_DT_Value.upper()] = round(dic[Tag_DT_Value.upper()] / q * 1000. / 86400., 2)
            # add new data item
            addedDics.append(dic)
    # import to MongoDB
    for dic in addedDics:
        curfilter = {Tag_ST_StationID.upper(): dic[Tag_ST_StationID.upper()],
                     Tag_DT_Type.upper(): dic[Tag_DT_Type.upper()],
                     Tag_DT_UTC.upper(): dic[Tag_DT_UTC.upper()]}
        db[Tag_ClimateDB_Measurement.upper()].find_one_and_replace(curfilter, dic, upsert=True)


def ImportMeasurementData():
    # , , ,
    '''
    This function mainly to import measurement data to MongoDB
    data type may include Q (discharge, m3/s), tn, tp, etc.
    the required parameters that defined in configuration file (*.ini)
    :param HOSTNAME: host IP address of MongoDB
    :param PORT: PORT number of MongoDB
    :param ClimateDBName: HydroClimate database name
    :param MEASUREMENT_DATA_DIR: MEASUREMENT_DATA_DIR of measurement data files
    '''
    if not useObserved:
        return False
    try:
        connMongo = MongoClient(HOSTNAME, PORT)
        print ("Import Site Measurement Data... ")
    except ConnectionFailure:
        sys.stderr.write("Could not connect to MongoDB: %s" % ConnectionFailure.message)
        sys.exit(1)
    db = connMongo[ClimateDBName]
    cList = db.collection_names()
    if not StringInList(Tag_ClimateDB_Measurement.upper(), cList):
        db.create_collection(Tag_ClimateDB_Measurement.upper())
    else:
        db.drop_collection(Tag_ClimateDB_Measurement.upper())
    if not StringInList(Tag_ClimateDB_Sites.upper(), cList):
        db.create_collection(Tag_ClimateDB_Sites.upper())
    if not StringInList(Tag_ClimateDB_VARs.upper(), cList):
        db.create_collection(Tag_ClimateDB_VARs.upper())

    fileList = GetFullPathWithSuffixes(MEASUREMENT_DATA_DIR, ['.txt'])
    measFileList = []
    siteLoc = []
    for fl in fileList:
        if (IsSubString('observed_', fl)):
            measFileList.append(fl)
        else:
            siteLoc.append(fl)
    ImportData(db, measFileList, siteLoc)
    return True


if __name__ == "__main__":
    LoadConfiguration(GetINIfile())
    ImportMeasurementData()
