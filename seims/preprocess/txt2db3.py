#! /usr/bin/env python
# coding=utf-8
# @ Import parameters and lookup tables to SQLite database
# @Author: Liang-Jun Zhu, Fang Shen
#
import sqlite3

from config import *
from util import *


def txt2Sqlite(dataFiles, dbFile):
    dataImport = {}  # format: {tabName:[fieldName, Units, dataRows]}
    for dataFileItem in dataFiles:
        # print dataFileItem
        dataPath = TXT_DB_DIR + os.sep + dataFileItem[1] + ".txt"
        dataItems = ReadDataItemsFromTxt(dataPath)
        # print dataItems
        if dataFileItem[0] == Tag_Params:
            fieldNames = dataItems[0][1:]
            units = dataItems[1][1:]
            CHANGE_IDX = fieldNames.index(PARAM_FLD_CHANGE) + 1
            IMPACT_IDX = fieldNames.index(PARAM_FLD_IMPACT) + 1
            MAX_IDX = fieldNames.index(PARAM_FLD_MAX) + 1
            MIN_IDX = fieldNames.index(PARAM_FLD_MIN) + 1
            for i in range(2, len(dataItems)):
                curDataItem = dataItems[i]
                # print curDataItem
                if curDataItem[CHANGE_IDX] == PARAM_CHANGE_NC or curDataItem[CHANGE_IDX] == '':
                    curDataItem[IMPACT_IDX] = 0
                if curDataItem[CHANGE_IDX] == PARAM_CHANGE_RC and curDataItem[IMPACT_IDX] == '':
                    curDataItem[IMPACT_IDX] = 1
                if curDataItem[CHANGE_IDX] == PARAM_CHANGE_AC and curDataItem[IMPACT_IDX] == '':
                    curDataItem[IMPACT_IDX] = 0
                if curDataItem[MAX_IDX] == '':
                    curDataItem[MAX_IDX] = DEFAULT_NODATA
                if curDataItem[MIN_IDX] == '':
                    curDataItem[MIN_IDX] = DEFAULT_NODATA
                if curDataItem[0] in dataImport.keys():
                    dataImport[curDataItem[0]][2].append(curDataItem[1:])
                else:
                    dataImport[curDataItem[0]] = [
                        fieldNames, units, [curDataItem[1:]]]

        else:
            fieldNames = dataItems[0]
            units = dataItems[1]
            if dataFileItem[1] not in dataImport:
                dataImport[dataFileItem[1]] = [fieldNames, units, []]
            for i in range(2, len(dataItems)):
                dataImport[dataFileItem[1]][2].append(dataItems[i])
    # print dataImport
    importData2Sqlite(dataImport, dbFile)


def importData2Sqlite(dataImport, dbFile):
    conn = sqlite3.connect(dbFile)
    cur = conn.cursor()
    for tabName in dataImport.keys():
        flds = dataImport[tabName][0]
        unitTypes = dataImport[tabName][1]
        dataRow = dataImport[tabName][2]
        fieldNameStr = ''
        for i in range(len(flds)):
            fieldNameStr += flds[i] + ' ' + unitTypes[i] + ' DEFAULT NULL,'
        create_table_sql = '''CREATE TABLE IF NOT EXISTS %s (%s)''' % (
            tabName, fieldNameStr[:-1])
        # print create_table_sql
        cur.execute(create_table_sql)
        load_sql = '''insert into %(table)s values (%(arg)s)''' % {'table': tabName, 'arg': ','.join(
            ['?' for i in range(0, len(flds))]), }
        # print load_sql
        for singledatarow in dataRow:
            cur.execute(load_sql, singledatarow)
    conn.commit()
    cur.close()
    conn.close()


def reConstructSQLiteDB():
    # If the database file existed, DELETE it.
    sqlPath = TXT_DB_DIR + os.sep + sqliteFile
    if os.path.exists(sqlPath):
        os.remove(sqlPath)
    dataFiles = [[Tag_Params, init_params]]
    for df in lookup_tabs:
        dataFiles.append([Tag_Lookup, df])
    txt2Sqlite(dataFiles, sqlPath)


if __name__ == "__main__":
    reConstructSQLiteDB()
