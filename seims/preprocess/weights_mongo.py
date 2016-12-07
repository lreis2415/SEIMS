#! /usr/bin/env python
# coding=utf-8
# @Generate weight data for interpolate of hydroclimate data
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
#

from struct import *

from gridfs import *
from pymongo import MongoClient
from pymongo.errors import ConnectionFailure

from config import *
from util import *


def cal_dis(x1, y1, x2, y2):
    dx = x2 - x1
    dy = y2 - y1
    return math.sqrt(dx * dx + dy * dy)


# there exist some problem in this function
def idw(x, y, locList):
    ex = 2
    coef_list = []
    sum = 0
    for pt in locList:
        dis = cal_dis(x, y, pt[0], pt[1])
        coef = math.pow(dis, -ex)
        coef_list.append(coef)
        sum += coef
    weightList = []
    for coef in coef_list:
        weightList.append(coef / sum)
    # print weightList
    fmt = '%df' % (len(weightList))
    s = pack(fmt, *weightList)
    return s


def thiessen(x, y, locList):
    iMin = 0
    coef_list = []
    # print locList
    if len(locList) <= 1:
        coef_list.append(1)
        fmt = '%df' % (1)
        return pack(fmt, *coef_list), iMin

    disMin = cal_dis(x, y, locList[0][0], locList[0][1])

    coef_list.append(0)

    for i in range(1, len(locList)):
        coef_list.append(0)
        dis = cal_dis(x, y, locList[i][0], locList[i][1])
        # print x, y, locList[i][0], locList[i][1], dis
        if dis < disMin:
            iMin = i
            disMin = dis
    coef_list[iMin] = 1
    fmt = '%df' % (len(coef_list))

    s = pack(fmt, *coef_list)
    return s, iMin


def GenerateWeightDependentParameters(conn, subbasinID):
    '''
    Generate some parameters dependent on weight data and only should be calculated once.
    Such as PHU0 (annual average total potential heat units)
            TMEAN0 (annual average temperature)
    :return:
    '''
    dbModel = conn[SpatialDBName]
    spatial = GridFS(dbModel, DB_TAB_SPATIAL.upper())
    # read mask file from mongodb
    maskName = str(subbasinID) + '_MASK'
    # read WEIGHT_M file from mongodb
    weightMName = str(subbasinID) + '_WEIGHT_M'
    mask = dbModel[DB_TAB_SPATIAL.upper()].files.find(
        {"filename": maskName})[0]
    weightM = dbModel[DB_TAB_SPATIAL.upper()].files.find(
        {"filename": weightMName})[0]
    numCells = int(weightM["metadata"]["NUM_CELLS"])
    numSites = int(weightM["metadata"]["NUM_SITES"])
    # read meteorlogy sites
    siteLists = dbModel[DB_TAB_SITELIST.upper()].find(
        {FLD_SUBBASINID.upper(): subbasinID})
    siteList = siteLists.next()
    dbName = siteList[FLD_DB.upper()]
    mList = siteList.get('SITELISTM')
    dbHydro = conn[dbName]

    siteList = mList.split(',')
    siteList = [int(item) for item in siteList]

    qDic = {Tag_ST_StationID.upper(): {'$in': siteList},
            Tag_DT_Type.upper(): Datatype_PHU0}
    cursor = dbHydro[Tag_ClimateDB_ANNUAL_STATS.upper()].find(
        qDic).sort(Tag_ST_StationID.upper(), 1)

    qDic2 = {Tag_ST_StationID.upper(): {'$in': siteList}, Tag_DT_Type.upper()             : DataType_MeanTemperature0.upper()}
    cursor2 = dbHydro[Tag_ClimateDB_ANNUAL_STATS.upper()].find(
        qDic2).sort(Tag_ST_StationID.upper(), 1)

    idList = []
    phuList = []
    for site in cursor:
        idList.append(site[Tag_ST_StationID.upper()])
        phuList.append(site[Tag_DT_Value.upper()])

    idList2 = []
    tmeanList = []
    for site in cursor2:
        idList2.append(site[Tag_ST_StationID.upper()])
        tmeanList.append(site[Tag_DT_Value.upper()])

    weightMData = spatial.get(weightM["_id"])
    totalLen = numCells * numSites
    fmt = '%df' % (totalLen,)
    weightMData = unpack(fmt, weightMData.read())

    # calculate PHU0
    phu0Data = numpy.zeros((numCells))
    # calculate TMEAN0
    tmean0Data = numpy.zeros((numCells))
    for i in range(numCells):
        for j in range(numSites):
            phu0Data[i] += phuList[j] * weightMData[i * numSites + j]
            tmean0Data[i] += tmeanList[j] * weightMData[i * numSites + j]
    ysize = int(mask["metadata"]["NROWS"])
    xsize = int(mask["metadata"]["NCOLS"])
    noDataValue = mask["metadata"]["NODATA_VALUE"]
    maskData = spatial.get(mask["_id"])
    totalLen = xsize * ysize
    fmt = '%df' % (totalLen,)
    maskData = unpack(fmt, maskData.read())
    fname = "%s_%s" % (str(subbasinID), Datatype_PHU0.upper())
    fname2 = "%s_%s" % (str(subbasinID), DataType_MeanTemperature0.upper())
    if (spatial.exists(filename=fname)):
        x = spatial.get_version(filename=fname)
        spatial.delete(x._id)
    if (spatial.exists(filename=fname2)):
        x = spatial.get_version(filename=fname2)
        spatial.delete(x._id)
    metaDic = mask["metadata"]
    metaDic["TYPE"] = Datatype_PHU0
    metaDic["ID"] = fname
    metaDic["DESCRIPTION"] = Datatype_PHU0

    metaDic2 = mask["metadata"]
    metaDic2["TYPE"] = DataType_MeanTemperature0
    metaDic2["ID"] = fname2
    metaDic2["DESCRIPTION"] = DataType_MeanTemperature0

    myfile = spatial.new_file(filename=fname, metadata=metaDic)
    myfile2 = spatial.new_file(filename=fname2, metadata=metaDic2)
    vaildCount = 0
    for i in range(0, ysize):
        curRow = []
        curRow2 = []
        for j in range(0, xsize):
            index = i * xsize + j
            # print index
            if (abs(maskData[index] - noDataValue) > UTIL_ZERO):
                curRow.append(phu0Data[vaildCount])
                curRow2.append(tmean0Data[vaildCount])
                vaildCount += 1
            else:
                curRow.append(noDataValue)
                curRow2.append(noDataValue)
        fmt = '%df' % (xsize)
        myfile.write(pack(fmt, *curRow))
        myfile2.write(pack(fmt, *curRow2))
    myfile.close()
    myfile2.close()
    print "Valid Cell Number is: %d" % vaildCount


def GenerateWeightInfo(conn, modelName, subbasinID, stormMode=False, useRsData=False):
    '''
    Generate and import weight information using Thiessen polygon method.
    :param conn:
    :param modelName:
    :param subbasinID:
    :param stormMode:
    :param useRsData:
    :return:
    '''
    # print "\t Subbasin:", subbasinID
    dbModel = conn[modelName]
    spatial = GridFS(dbModel, DB_TAB_SPATIAL.upper())

    # read mask file from mongodb
    maskName = str(subbasinID) + '_MASK'
    mask = dbModel[DB_TAB_SPATIAL.upper()].files.find(
        {"filename": maskName})[0]

    ysize = int(mask["metadata"]["NROWS"])
    xsize = int(mask["metadata"]["NCOLS"])
    noDataValue = mask["metadata"]["NODATA_VALUE"]
    dx = mask["metadata"]["CELLSIZE"]
    xll = mask["metadata"]["XLLCENTER"]
    yll = mask["metadata"]["YLLCENTER"]

    data = spatial.get(mask["_id"])

    totalLen = xsize * ysize
    fmt = '%df' % (totalLen,)
    data = unpack(fmt, data.read())
    # print data[0], len(data), type(data)

    # count number of valid cells
    num = 0
    for i in range(0, totalLen):
        if (abs(data[i] - noDataValue) > UTIL_ZERO):
            num = num + 1

    # read stations information from database
    metadic = {
        'SUBBASIN': subbasinID,
        'NUM_CELLS': num}

    siteLists = dbModel[DB_TAB_SITELIST.upper()].find(
        {FLD_SUBBASINID.upper(): subbasinID})
    siteList = siteLists.next()
    dbName = siteList[FLD_DB.upper()]
    pList = siteList.get('SITELISTP')
    mList = siteList.get('SITELISTM')
    petList = siteList.get('SITELISTPET')
    # print pList
    # print mList
    dbHydro = conn[dbName]

    typeList = [DataType_Meteorology, DataType_Precipitation,
                DataType_PotentialEvapotranspiration]
    siteLists = [mList, pList, petList]
    if petList is None:
        del typeList[2]
        del siteLists[2]

    if (stormMode):
        typeList = [DataType_Precipitation]
        siteLists = [pList]
        # print typeList
    # print siteLists

    for i in range(len(typeList)):
        fname = '%d_WEIGHT_%s' % (subbasinID, typeList[i])
        print fname
        if (spatial.exists(filename=fname)):
            x = spatial.get_version(filename=fname)
            spatial.delete(x._id)
        siteList = siteLists[i]
        if siteList != None:
            siteList = siteList.split(',')
            # print siteList
            siteList = [int(item) for item in siteList]
            metadic['NUM_SITES'] = len(siteList)
            # print siteList
            qDic = {Tag_ST_StationID.upper(): {'$in': siteList},
                    Tag_ST_Type.upper(): typeList[i]}
            cursor = dbHydro[Tag_ClimateDB_Sites.upper()].find(
                qDic).sort(Tag_ST_StationID.upper(), 1)

            # meteorology station can also be used as precipitation station
            if cursor.count() == 0 and typeList[i] == DataType_Precipitation:
                qDic = {Tag_ST_StationID.upper(): {'$in': siteList},
                        Tag_ST_Type.upper(): DataType_Meteorology}
                cursor = dbHydro[Tag_ClimateDB_Sites.upper()].find(
                    qDic).sort(Tag_ST_StationID.upper(), 1)

            # get site locations
            idList = []
            LocList = []
            for site in cursor:
                if site[Tag_ST_StationID.upper()] in siteList:
                    idList.append(site[Tag_ST_StationID.upper()])
                    LocList.append([site[Tag_ST_LocalX.upper()],
                                    site[Tag_ST_LocalY.upper()]])
            # print 'loclist', locList
            # interpolate using the locations
            # weightList = []
            myfile = spatial.new_file(filename=fname, metadata=metadic)
            fTest = open(r"%s/weight_%d_%s.txt" %
                         (WORKING_DIR, subbasinID, typeList[i]), 'w')
            for i in range(0, ysize):
                for j in range(0, xsize):
                    index = i * xsize + j
                    # print index
                    if (abs(data[index] - noDataValue) > UTIL_ZERO):
                        # x = geo[0] + (j+0.5)*geo[1] + i*geo[2]
                        # y = geo[3] + j*geo[4] + (i+0.5)*geo[5]
                        x = xll + j * dx
                        y = yll + (ysize - i - 1) * dx
                        nearIndex = 0
                        # print locList
                        if useRsData:
                            line, nearIndex = thiessen(x, y, LocList)
                        else:
                            # line = idw(x, y, locList)
                            line, nearIndex = thiessen(x, y, LocList)
                        # weightList.append(line)
                        myfile.write(line)
                        fmt = '%df' % (len(LocList))
                        fTest.write("%f %f " % (x, y) +
                                    unpack(fmt, line).__str__() + "\n")

                        # print line
            # weightStr = '\n'.join(weightList)
            # spatial.put(weightStr, filename=fname, metadata=metadic)
            myfile.close()
            # fTest.close()

if __name__ == "__main__":
    LoadConfiguration(GetINIfile())
    try:
        conn = MongoClient(HOSTNAME, PORT)
    except ConnectionFailure, e:
        sys.stderr.write("Could not connect to MongoDB: %s" % e)
        sys.exit(1)
    subbasinStartID = 1
    if not forCluster:
        subbasinStartID = 0
    nSubbasins = 0
    # print subbasinStartID, nSubbasins + 1
    for i in range(subbasinStartID, nSubbasins + 1):
        GenerateWeightInfo(conn, SpatialDBName, i, stormMode)
        # 　added by Liangjun, 2016-6-17
        GenerateWeightDependentParameters(conn, i)
