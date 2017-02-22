#! /usr/bin/env python
# coding=utf-8
# @Meteorological daily data import, and calculate related statistical values
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu, Fang Shen
#
import time
import datetime

import math
import pymongo
from pygeoc.utils.utils import DateClass

from hydro_climate_utility import HydroClimateUtilClass
from config import *
from utility import LoadConfiguration, ReadDataItemsFromTxt
from utility import DEFAULT_NODATA
# for test main
from db_import_sites import ImportHydroClimateSitesInfo
from db_mongodb import ConnectMongoDB


class climateStats(object):
    """Common used annual climate statistics based on mean temperature, e.g. PHU."""

    def __init__(self):
        self.Count = {}
        self.MeanTmp = {}
        self.PHUTOT = {}
        self.MeanTmp0 = 0.
        self.PHU0 = 0.

    def addItem(self, itemDict):
        """Add mean temperature of each day. Dict MUST have {YEAR: 2017, TMEAN: 10.} at least."""
        if Tag_DT_Year.upper() not in itemDict.keys():
            raise ValueError("The hydroClimate dict must have year!")
        if DataType_MeanTemperature.upper() not in itemDict.keys():
            raise ValueError("The hydroClimate dict must have mean temperature!")
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
        """Calculate annual statistics."""
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
        if not StringClass.stringinlist(fld, climFlds):
            raise ValueError(
                "Meteorological Daily data is invalid, please Check!")
    for i in range(1, len(climDataItems)):
        dic = {}
        curSSD = DEFAULT_NODATA
        curY = 0
        curM = 0
        curD = 0
        for j in range(len(climDataItems[i])):
            if StringClass.stringmatch(climFlds[j], Tag_DT_StationID):
                dic[Tag_DT_StationID.upper()] = int(climDataItems[i][j])
            elif StringClass.stringmatch(climFlds[j], Tag_DT_Year):
                curY = int(climDataItems[i][j])
                dic[Tag_DT_Year.upper()] = curY
            elif StringClass.stringmatch(climFlds[j], Tag_DT_Month):
                curM = int(climDataItems[i][j])
            elif StringClass.stringmatch(climFlds[j], Tag_DT_Day):
                curD = int(climDataItems[i][j])
            elif StringClass.stringmatch(climFlds[j], DataType_MeanTemperature):
                dic[DataType_MeanTemperature.upper()] = float(
                    climDataItems[i][j])
            elif StringClass.stringmatch(climFlds[j], DataType_MinimumTemperature):
                dic[DataType_MinimumTemperature.upper()] = float(
                    climDataItems[i][j])
            elif StringClass.stringmatch(climFlds[j], DataType_MaximumTemperature):
                dic[DataType_MaximumTemperature.upper()] = float(
                    climDataItems[i][j])
            elif StringClass.stringmatch(climFlds[j], DataType_PotentialEvapotranspiration):
                dic[DataType_PotentialEvapotranspiration.upper()] = float(
                    climDataItems[i][j])
            elif StringClass.stringmatch(climFlds[j], DataType_SolarRadiation):
                dic[DataType_SolarRadiation.upper()] = float(
                    climDataItems[i][j])
            elif StringClass.stringmatch(climFlds[j], DataType_WindSpeed):
                dic[DataType_WindSpeed.upper()] = float(climDataItems[i][j])
            elif StringClass.stringmatch(climFlds[j], DataType_RelativeAirMoisture):
                dic[DataType_RelativeAirMoisture.upper()] = float(
                    climDataItems[i][j]) * 0.01
            elif StringClass.stringmatch(climFlds[j], DataType_SunDurationHour):
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
                    dic[DataType_SolarRadiation.upper()] = round(
                        HydroClimateUtilClass.Rs(DateClass.dayofyear(dt), float(curSSD), curLat * math.pi / 180.), 1)
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
    # Create index
    db[DB_TAB_DATAVALUES.upper()].create_index([(Tag_DT_StationID.upper(), pymongo.ASCENDING),
                                                (Tag_DT_Type.upper(),
                                                 pymongo.ASCENDING),
                                                (Tag_DT_UTC.upper(), pymongo.ASCENDING)])
    # prepare dic for MongoDB
    for sID in HydroClimateStats.keys():
        for YYYY in HydroClimateStats[sID].Count.keys():
            curDic = {}
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


def ImportDailyMeteoData(db, siteMLoc):
    print ("Import Daily Meteorological Data... ")
    cList = db.collection_names()
    tables = [DB_TAB_DATAVALUES.upper(), Tag_ClimateDB_ANNUAL_STATS.upper()]
    firstImport = False
    for tb in tables:
        if not StringClass.stringinlist(tb, cList):
            db.create_collection(tb)
            firstImport = True
    ImportDayData(db, MeteoDailyFile, siteMLoc, firstImport)


if __name__ == "__main__":
    LoadConfiguration(getconfigfile())
    client = ConnectMongoDB(HOSTNAME, PORT)
    conn = client.get_conn()
    db = conn[ClimateDBName]
    SitesMList, SitesPList = ImportHydroClimateSitesInfo(db)
    ImportDailyMeteoData(db, SitesPList)
    client.close()
