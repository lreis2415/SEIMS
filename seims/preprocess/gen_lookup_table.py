#! /usr/bin/env python
# coding=utf-8
# @Generate landuse lookup table
# Author: Junzhi Liu
# Revised: Liang-Jun Zhu
#

import sqlite3

from config import *


def CreateLanduseLookupTable(dbname, property_namelist, str_sql, dstdir):
    property_map = {}
    conn = sqlite3.connect(dbname)
    cursor = conn.cursor()

    cursor.execute(str_sql)
    property_namelist.append("USLE_P")
    for row in cursor:
        # print row
        id = int(row[0])
        value_map = {}
        for i in range(len(property_namelist)):
            pName = property_namelist[i]
            if pName == "USLE_P":  # Currently, USLE_P is set as 1 for all landuse.
                value_map[pName] = 1
            else:
                if pName == "Manning":
                    value_map[pName] = row[i + 1] * 10
                else:
                    value_map[pName] = row[i + 1]
        property_map[id] = value_map

    n = len(property_map)
    os.chdir(dstdir)
    for propertyName in property_namelist:
        if not os.path.exists(DIR_NAME_LOOKUP):
            os.mkdir(DIR_NAME_LOOKUP)
        f = open("lookup/%s.txt" % (propertyName,), 'w')
        f.write("%d\n" % (n))
        for id in property_map:
            s = "%d %f\n" % (id, property_map[id][propertyName])
            f.write(s)
        f.close()
