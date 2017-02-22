#! /usr/bin/env python
# coding=utf-8
# @Import model calibration parameters
# Author: Junzhi Liu
# Revised: Liang-Jun Zhu
#

import sqlite3
from struct import pack

from gridfs import GridFS
from pygeoc.utils.utils import MathClass

from config import *
from utility import LoadConfiguration, ReadDataItemsFromTxt
# for test main
from db_mongodb import ConnectMongoDB


def ImportModelParameters(sqlite_file, db):
    # delete if existed, create if not existed
    cList = db.collection_names()
    if not StringClass.stringinlist(DB_TAB_PARAMETERS.upper(), cList):
        db.create_collection(DB_TAB_PARAMETERS.upper())
    else:
        db.drop_collection(DB_TAB_PARAMETERS.upper())
    # read sqlite database
    conn = sqlite3.connect(sqlite_file)
    c = conn.cursor()
    # get all the tablename
    c.execute("select name from sqlite_master where type='table' order by name;")
    tablelist = c.fetchall()
    # Find parameter table list excluding "XXLookup"
    tablelist = [item[0].encode("ascii") for item in tablelist if (
        item[0].lower().find("lookup") < 0)]
    # print tablelist

    field_list = [PARAM_FLD_NAME.upper(), PARAM_FLD_DESC.upper(), PARAM_FLD_UNIT.upper(),
                  PARAM_FLD_MODS.upper(), PARAM_FLD_VALUE.upper(), PARAM_FLD_IMPACT.upper(),
                  PARAM_FLD_CHANGE.upper(), PARAM_FLD_MAX.upper(), PARAM_FLD_MIN.upper(),
                  PARAM_FLD_USE.upper()]
    for tablename in tablelist:
        # print tablename
        str_sql = "select * from %s;" % (tablename,)
        cur = c.execute(str_sql)
        records = cur.fetchall()
        for items in records:
            dic = dict()
            dic[Tag_DT_Type.upper()] = tablename
            for i in range(len(items)):
                if type(items[i]) == type('a') or type(items[i]) == type(u'a'):
                    dic[field_list[i].upper()] = items[i].encode('ascii')
                else:
                    dic[field_list[i].upper()] = items[i]
            curfilter = {PARAM_FLD_NAME.upper(): dic[PARAM_FLD_NAME.upper()], Tag_DT_Type.upper(): tablename}
            db[DB_TAB_PARAMETERS.upper()].find_one_and_replace(curfilter, dic, upsert=True)
    db[DB_TAB_PARAMETERS.upper()].create_index(PARAM_FLD_NAME.upper())
    c.close()
    conn.close()


def ImportLookupTables(db, sqlite_file):
    """
    :param sqlite_file: SQLite database file contains lookup tables
    :param db: MongoDB Client
    :return: None
    """
    # read sqlite database
    conn = sqlite3.connect(sqlite_file)
    c = conn.cursor()
    # get all the tablename
    c.execute("select name from sqlite_master where type='table' order by name;")
    tablelist = c.fetchall()
    # Find parameter table list excluding "XXLookup"
    tablelist = [item[0].encode("ascii") for item in tablelist if (
        item[0].lower().find("lookup") >= 0)]
    # print tablelist
    for tablename in tablelist:
        # print tablename
        str_sql = "select * from %s;" % (tablename,)
        cur = c.execute(str_sql)
        records = cur.fetchall()
        itemValues = []
        for items in records:
            itemValue = []
            for i in range(len(items)):
                if MathClass.isnumerical(items[i]):
                    itemValue.append(float(items[i]))
            itemValues.append(itemValue)
        nRow = len(itemValues)
        # print itemValues
        if nRow >= 1:
            nCol = len(itemValues[0])
            for i in range(nRow):
                if nCol != len(itemValues[i]):
                    raise ValueError(
                        "Please check %s to make sure each item has the same numeric dimension." % tablename)
                else:
                    itemValues[i].insert(0, nCol)
            # import to mongoDB as GridFS
            spatial = GridFS(db, DB_TAB_SPATIAL.upper())
            # delete if the tablename file existed already.
            if spatial.exists(filename=tablename.upper()):
                x = spatial.get_version(filename=tablename.upper())
                spatial.delete(x._id)
            metadic = {META_LOOKUP_ITEM_COUNT.upper(): nRow, META_LOOKUP_FIELD_COUNT.upper(): nCol}
            curLookupGridFS = spatial.new_file(filename=tablename.upper(), metadata=metadic)
            header = [nRow]
            fmt = '%df' % 1
            s = pack(fmt, *header)
            curLookupGridFS.write(s)
            fmt = '%df' % (nCol + 1)
            for i in range(nRow):
                s = pack(fmt, *itemValues[i])
                curLookupGridFS.write(s)
            curLookupGridFS.close()
    c.close()
    conn.close()


def ImportModelConfiguration(db):
    """
    Import Configuration information of SEIMS, i.e., file.in and file.out
    :return:
    """
    fileIn = MODEL_DIR + os.sep + FILE_IN
    fileOut = MODEL_DIR + os.sep + FILE_OUT
    # create if collection not existed
    cList = db.collection_names()
    conf_tabs = [DB_TAB_FILE_IN.upper(), DB_TAB_FILE_OUT.upper()]
    for item in conf_tabs:
        if not StringClass.stringinlist(item, cList):
            db.create_collection(item)
        else:
            db.drop_collection(item)
    fileInItems = ReadDataItemsFromTxt(fileIn)
    fileOutItems = ReadDataItemsFromTxt(fileOut)

    for item in fileInItems:
        fileInDict = {}
        values = StringClass.splitstring(StringClass.stripstring(item[0]), ['|'])
        if len(values) != 2:
            raise ValueError("One item should only have one Tag and one value string, split by '|'")
        fileInDict[FLD_CONF_TAG] = values[0]
        fileInDict[FLD_CONF_VALUE] = values[1]
        db[DB_TAB_FILE_IN.upper()].find_one_and_replace(
            fileInDict, fileInDict, upsert=True)

    outFieldArray = fileOutItems[0]
    outDataArray = fileOutItems[1:]
    # print outDataArray
    for item in outDataArray:
        fileOutDict = dict()
        for i in range(len(outFieldArray)):
            if StringClass.stringmatch(FLD_CONF_MODCLS, outFieldArray[i]):
                fileOutDict[FLD_CONF_MODCLS] = item[i]
            elif StringClass.stringmatch(FLD_CONF_OUTPUTID, outFieldArray[i]):
                fileOutDict[FLD_CONF_OUTPUTID] = item[i]
            elif StringClass.stringmatch(FLD_CONF_DESC, outFieldArray[i]):
                fileOutDict[FLD_CONF_DESC] = item[i]
            elif StringClass.stringmatch(FLD_CONF_UNIT, outFieldArray[i]):
                fileOutDict[FLD_CONF_UNIT] = item[i]
            elif StringClass.stringmatch(FLD_CONF_TYPE, outFieldArray[i]):
                fileOutDict[FLD_CONF_TYPE] = item[i]
            elif StringClass.stringmatch(FLD_CONF_STIME, outFieldArray[i]):
                fileOutDict[FLD_CONF_STIME] = item[i]
            elif StringClass.stringmatch(FLD_CONF_ETIME, outFieldArray[i]):
                fileOutDict[FLD_CONF_ETIME] = item[i]
            elif StringClass.stringmatch(FLD_CONF_INTERVAL, outFieldArray[i]):
                fileOutDict[FLD_CONF_INTERVAL] = item[i]
            elif StringClass.stringmatch(FLD_CONF_INTERVALUNIT, outFieldArray[i]):
                fileOutDict[FLD_CONF_INTERVALUNIT] = item[i]
            elif StringClass.stringmatch(FLD_CONF_FILENAME, outFieldArray[i]):
                fileOutDict[FLD_CONF_FILENAME] = item[i]
            elif StringClass.stringmatch(FLD_CONF_USE, outFieldArray[i]):
                fileOutDict[FLD_CONF_USE] = item[i]
            elif StringClass.stringmatch(FLD_CONF_SUBBSN, outFieldArray[i]):
                fileOutDict[FLD_CONF_SUBBSN] = item[i]
        if fileOutDict.keys() is []:
            raise ValueError("There are not any valid output item stored in file.out!")
        curFileter = {FLD_CONF_MODCLS: fileOutDict[FLD_CONF_MODCLS],
                      FLD_CONF_OUTPUTID: fileOutDict[FLD_CONF_OUTPUTID],
                      FLD_CONF_STIME: fileOutDict[FLD_CONF_STIME],
                      FLD_CONF_ETIME: fileOutDict[FLD_CONF_ETIME]}
        db[DB_TAB_FILE_OUT].find_one_and_replace(curFileter, fileOutDict, upsert=True)


if __name__ == "__main__":
    # Load Configuration file
    LoadConfiguration(getconfigfile())
    client = ConnectMongoDB(HOSTNAME, PORT)
    conn = client.get_conn()
    db = conn[SpatialDBName]
    sqlite3db = WORKING_DIR + os.sep + DIR_NAME_IMPORT2DB + os.sep + sqlite_file
    from db_sqlite import reConstructSQLiteDB

    if not FileClass.isfileexists(sqlite3db):
        reConstructSQLiteDB()
    ImportModelParameters(sqlite3db, db)
    # IMPORT LOOKUP TABLES AS GRIDFS, DT_Array2D
    ImportLookupTables(db, sqlite3db)
    ImportModelConfiguration(db)
    client.close()
