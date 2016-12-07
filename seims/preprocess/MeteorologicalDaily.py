#! /usr/bin/env python
# coding=utf-8
# @Meteorological daily data import, and calculate related statistical values
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu, Fang Shen
#
import pymongo
from pymongo import MongoClient
from pymongo.errors import ConnectionFailure

from config import *
from util import *


class climateStats:

    def __init__(self):
        self.Count = {}
        self.MeanTmp = {}
        self.PHUTOT = {}
        self.MeanTmp0 = 0.
        self.PHU0 = 0.

    def addItem(self, itemDict):
        if Tag_DT_Year.upper() not in itemDict.keys():
            raise ValueError("The hydroClimate dict must have year!")
        if DataType_MeanTemperature.upper() not in itemDict.keys():
            raise ValueError(
                "The hydroClimate dict must have mean temperature!")
        curY = itemDict[Tag_DT_Year.upper()]
        curTmp = itemDict[DataType_MeanTemperature.upper()]
        if curY not in self.Count.keys():
            self.Count[curY] = 1
            self.MeanTmp[curY] = curTmp
            if curTmp > T_base:
                self.PHUTOT[curY] = curTmp
            else:
                self.PHUTOT[curY] = 0.
        else:
            self.Count[curY] += 1
            self.MeanTmp[curY] += curTmp
            if curTmp > T_base:
                self.PHUTOT[curY] += curTmp

    def annualStats(self):
        for Y in self.Count.keys():
            self.MeanTmp[Y] = round(self.MeanTmp[Y] / self.Count[Y], 1)
            self.MeanTmp0 += self.MeanTmp[Y]
            self.PHUTOT[Y] = round(self.PHUTOT[Y], 1)
            self.PHU0 += self.PHUTOT[Y]
        self.PHU0 = round(self.PHU0 / len(self.Count.keys()), 1)
        self.MeanTmp0 = round(self.MeanTmp0 / len(self.Count.keys()), 1)


# Import climate data table
def ImportDayData(db, ClimateDateFile, sitesLoc, isFirst):
    climDataItems = ReadDataItemsFromTxt(ClimateDateFile)
    climFlds = climDataItems[0]
    # PHUCalDic is used for Calculating potential heat units (PHU)
    # for each climate station and each year.
    # format is {StationID:{Year1:[values],Year2:[Values]...}, ...}
    # PHUCalDic = {}
    # format: {StationID1: climateStats1, ...}
    HydroClimateStats = {}
    requiredFlds = [Tag_DT_Year.upper(), Tag_DT_Month.upper(), Tag_DT_Day.upper(),
                    DataType_MaximumTemperature.upper(), DataType_MinimumTemperature.upper(),
                    DataType_RelativeAirMoisture.upper(), DataType_WindSpeed.upper()]
    for fld in requiredFlds:
        if not StringInList(fld, climFlds):
            raise ValueError(
                "Meteorological Daily data is invalid, please Check!")
    for i in range(1, len(climDataItems)):
        dic = {}
        curSSD = DEFAULT_NODATA
        for j in range(len(climDataItems[i])):
            if StringMatch(climFlds[j], Tag_DT_StationID):
                dic[Tag_DT_StationID.upper()] = int(climDataItems[i][j])
            elif StringMatch(climFlds[j], Tag_DT_Year):
                curY = int(climDataItems[i][j])
                dic[Tag_DT_Year.upper()] = curY
            elif StringMatch(climFlds[j], Tag_DT_Month):
                curM = int(climDataItems[i][j])
            elif StringMatch(climFlds[j], Tag_DT_Day):
                curD = int(climDataItems[i][j])
            elif StringMatch(climFlds[j], DataType_MeanTemperature):
                dic[DataType_MeanTemperature.upper()] = float(
                    climDataItems[i][j])
            elif StringMatch(climFlds[j], DataType_MinimumTemperature):
                dic[DataType_MinimumTemperature.upper()] = float(
                    climDataItems[i][j])
            elif StringMatch(climFlds[j], DataType_MaximumTemperature):
                dic[DataType_MaximumTemperature.upper()] = float(
                    climDataItems[i][j])
            elif StringMatch(climFlds[j], DataType_PotentialEvapotranspiration):
                dic[DataType_PotentialEvapotranspiration.upper()] = float(
                    climDataItems[i][j])
            elif StringMatch(climFlds[j], DataType_SolarRadiation):
                dic[DataType_SolarRadiation.upper()] = float(
                    climDataItems[i][j])
            elif StringMatch(climFlds[j], DataType_WindSpeed):
                dic[DataType_WindSpeed.upper()] = float(climDataItems[i][j])
            elif StringMatch(climFlds[j], DataType_RelativeAirMoisture):
                dic[DataType_RelativeAirMoisture.upper()] = float(
                    climDataItems[i][j]) * 0.01
            elif StringMatch(climFlds[j], DataType_SunDurationHour):
                curSSD = float(climDataItems[i][j])
        # Date transformation
        dt = datetime.datetime(curY, curM, curD, 0, 0)
        sec = time.mktime(dt.timetuple())
        utcTime = time.gmtime(sec)
        dic[Tag_DT_LocalT.upper()] = dt
        dic[Tag_DT_Zone.upper()] = time.timezone / 3600
        dic[Tag_DT_UTC.upper()] = datetime.datetime(
            utcTime[0], utcTime[1], utcTime[2], utcTime[3])

        # Do if some of these data are not provided
        if DataType_MeanTemperature.upper() not in dic.keys():
            dic[DataType_MeanTemperature.upper()] = (dic[DataType_MaximumTemperature.upper()] +
                                                     dic[DataType_MinimumTemperature.upper()]) / 2.
        if DataType_SolarRadiation.upper() not in dic.keys():
            if curSSD == DEFAULT_NODATA:
                print DataType_SolarRadiation + " or " + DataType_SunDurationHour + " must be provided!"
                exit(0)
            else:
                if dic[Tag_DT_StationID.upper()] in sitesLoc.keys():
                    curLon, curLat = sitesLoc[
                        dic[Tag_DT_StationID.upper()]].LonLat()
                    dic[DataType_SolarRadiation.upper()] = round(Rs(doy(dt),
                                                                    float(curSSD), curLat * math.pi / 180.), 1)
        outputFlds = [DataType_MeanTemperature.upper(), DataType_MaximumTemperature.upper(),
                      DataType_MinimumTemperature.upper(), DataType_RelativeAirMoisture.upper(),
                      DataType_PotentialEvapotranspiration.upper(), DataType_WindSpeed.upper(),
                      DataType_SolarRadiation.upper()]
        for fld in outputFlds:
            curDic = {}
            if fld in dic.keys():
                curDic[Tag_DT_Value.upper()] = dic[fld]
                curDic[Tag_DT_StationID.upper()] = dic[
                    Tag_DT_StationID.upper()]
                curDic[Tag_DT_UTC.upper()] = dic[Tag_DT_UTC.upper()]
                curDic[Tag_DT_Zone.upper()] = dic[Tag_DT_Zone.upper()]
                curDic[Tag_DT_LocalT.upper()] = dic[Tag_DT_LocalT.upper()]
                curDic[Tag_DT_Type.upper()] = fld
                curfilter = {Tag_DT_StationID.upper(): dic[Tag_DT_StationID.upper()],
                             Tag_DT_UTC.upper(): dic[Tag_DT_UTC.upper()],
                             Tag_DT_Type.upper(): fld}
                if (isFirst):
                    db[DB_TAB_DATAVALUES.upper()].insert_one(curDic)
                else:
                    db[DB_TAB_DATAVALUES.upper()].find_one_and_replace(
                        curfilter, curDic, upsert=True)

        if dic[Tag_DT_StationID.upper()] not in HydroClimateStats.keys():
            HydroClimateStats[dic[Tag_DT_StationID.upper()]] = climateStats()
        HydroClimateStats[dic[Tag_DT_StationID.upper()]].addItem(dic)
    for item in HydroClimateStats.keys():
        HydroClimateStats[item].annualStats()
        # if (dic[Tag_DT_StationID.upper()] in PHUCalDic.keys()):
        #     if (curY in PHUCalDic[dic[Tag_DT_StationID.upper()]].keys()):
        #         PHUCalDic[dic[Tag_DT_StationID.upper()]][curY][0].append(dic[DataType_MeanTemperature.upper()])
        #     else:
        #         PHUCalDic[dic[Tag_DT_StationID.upper()]][curY] = [[dic[DataType_MeanTemperature.upper()]]]
        # else:
        #     PHUCalDic[dic[Tag_DT_StationID.upper()]] = {curY: [[dic[DataType_MeanTemperature.upper()]]]}
    # Create index
    db[DB_TAB_DATAVALUES.upper()].create_index([(Tag_DT_StationID.upper(), pymongo.ASCENDING),
                                                (Tag_DT_Type.upper(),
                                                 pymongo.ASCENDING),
                                                (Tag_DT_UTC.upper(), pymongo.ASCENDING)])
    # prepare dic for MongoDB
    for sID in HydroClimateStats.keys():
        # for sID in PHUCalDic.keys():
        #     curPHU0 = 0.
        #     curTMEAN0 = 0.  ## multi-annual mean
        #     for YYYY in PHUCalDic[sID].keys():
        for YYYY in HydroClimateStats[sID].Count.keys():
            curDic = {}
            # xx = numpy.array(PHUCalDic[sID][YYYY][0])
            # curSumPHU = numpy.sum(xx[numpy.where(xx > T_base)])
            # curTMEAN = numpy.mean(xx)
            # curTMEAN0 += curTMEAN
            # PHUCalDic[sID][YYYY].append(curSumPHU)
            # curPHU0 += curSumPHU
            # curDic[Tag_DT_Value.upper()] = curSumPHU
            curDic[Tag_DT_Value.upper()] = HydroClimateStats[sID].PHUTOT[YYYY]
            curDic[Tag_DT_StationID.upper()] = sID
            curDic[Tag_DT_Year.upper()] = YYYY
            curDic[Tag_VAR_UNIT.upper()] = "heat units"
            curDic[Tag_VAR_Type.upper()] = DataType_YearlyHeatUnit
            curfilter = {Tag_DT_StationID.upper(): sID,
                         Tag_VAR_Type.upper(): DataType_YearlyHeatUnit,
                         Tag_DT_Year.upper(): YYYY}
            db[Tag_ClimateDB_ANNUAL_STATS.upper()].find_one_and_replace(
                curfilter, curDic, upsert=True)
            # import annual mean temperature
            curDic[Tag_VAR_Type.upper()] = DataType_MeanTemperature
            curDic[Tag_VAR_UNIT.upper()] = "deg C"
            curDic[Tag_DT_Value.upper()] = HydroClimateStats[sID].MeanTmp[YYYY]
            curfilter = {Tag_DT_StationID.upper(): sID,
                         Tag_VAR_Type.upper(): DataType_MeanTemperature,
                         Tag_DT_Year.upper(): YYYY}
            db[Tag_ClimateDB_ANNUAL_STATS.upper()].find_one_and_replace(
                curfilter, curDic, upsert=True)
        # yrNum = float(len(PHUCalDic[sID].keys()))
        # curPHU0 /= yrNum
        # PHUCalDic[sID][Datatype_PHU0] = round(curPHU0, 1)
        # curTMEAN0 /= yrNum
        # curDic[Tag_DT_Value.upper()] = PHUCalDic[sID][Datatype_PHU0]
        curDic[Tag_DT_Value.upper()] = HydroClimateStats[sID].PHU0
        curDic[Tag_DT_StationID.upper()] = sID
        curDic[Tag_DT_Year.upper()] = DEFAULT_NODATA
        curDic[Tag_VAR_UNIT.upper()] = "heat units"
        curDic[Tag_VAR_Type.upper()] = Datatype_PHU0
        curfilter = {Tag_DT_StationID.upper(): sID,
                     Tag_VAR_Type.upper(): Datatype_PHU0,
                     Tag_DT_Year.upper(): DEFAULT_NODATA}
        db[Tag_ClimateDB_ANNUAL_STATS.upper()].find_one_and_replace(
            curfilter, curDic, upsert=True)
        # import annual mean temperature
        curDic[Tag_VAR_Type.upper()] = DataType_MeanTemperature0
        curDic[Tag_VAR_UNIT.upper()] = "deg C"
        curDic[Tag_DT_Value.upper()] = HydroClimateStats[sID].MeanTmp0
        curfilter = {Tag_DT_StationID.upper(): sID,
                     Tag_VAR_Type.upper(): DataType_MeanTemperature0,
                     Tag_DT_Year.upper(): DEFAULT_NODATA}
        db[Tag_ClimateDB_ANNUAL_STATS.upper()].find_one_and_replace(
            curfilter, curDic, upsert=True)


def ImportDailyMeteoData(siteMLoc):
    try:
        connMongo = MongoClient(HOSTNAME, PORT)
        print "Import Daily Meteorological Data... "
        # print "Connected successfully"
    except ConnectionFailure, e:
        sys.stderr.write("Could not connect to MongoDB: %s" % e)
        sys.exit(1)
    db = connMongo[ClimateDBName]
    cList = db.collection_names()
    tables = [DB_TAB_DATAVALUES.upper(), Tag_ClimateDB_ANNUAL_STATS.upper()]
    firstImport = False
    for tb in tables:
        if not StringInList(tb, cList):
            db.create_collection(tb)
            firstImport = True
    ImportDayData(db, MeteoDailyFile, siteMLoc, firstImport)
    connMongo.close()

if __name__ == "__main__":
    LoadConfiguration(GetINIfile())
    from hydroclimate_sites import ImportHydroClimateSitesInfo
    SitesMList, SitesPList = ImportHydroClimateSitesInfo()
    ImportDailyMeteoData(SitesMList)
