#! /usr/bin/env python
# coding=utf-8
# @Redistribute crop parameters
# @Author: Junzhi Liu
# @Revised: Liang-Jun Zhu
#
import re

from config import *
from util import *


# Duplicate the function in reclass_landuse.py
# And deprecated this. By LJ, 2016-6-3
def ReadCropAttrs(cropFile):
    if not os.path.exists(cropFile):
        cropFile = TXT_DB_DIR + os.sep + CROP_FILE

    f = open(cropFile)
    lines = f.readlines()
    f.close()

    attrDic = {}

    fields = [item.replace('"', '')
              for item in re.split('\t|\n', lines[0]) if item is not '']
    n = len(fields)

    for i in range(n):
        attrDic[fields[i]] = {}

    for line in lines[2:]:
        items = [item.replace('"', '')
                 for item in re.split('\t', line) if item is not '']
        id = int(items[0])

        for i in range(n):
            dic = attrDic[fields[i]]
            try:
                dic[id] = float(items[i])
            except ValueError:
                dic[id] = items[i]

    return attrDic


def GenerateLandcoverInitialParameters(landuseFile, dstdir):
    LC_dataItems = ReadDataItemsFromTxt(landcoverInitFile)
    # print LC_dataItems
    fieldNames = LC_dataItems[0]
    LUID = -1
    for i in range(len(fieldNames)):
        if StringMatch(fieldNames[i], 'LANDUSE_ID'):
            LUID = i
            break
    dataItems = LC_dataItems[1:]
    replaceDicts = {}
    for item in dataItems:
        for i in range(len(item)):
            if i != LUID:
                if fieldNames[i].upper() not in replaceDicts.keys():
                    replaceDicts[fieldNames[i].upper()] = {float(item[LUID]): float(item[i])}
                else:
                    replaceDicts[fieldNames[i].upper()][float(item[LUID])] = float(item[i])
    # print replaceDicts

    # Generate GTIFF
    for item in replaceDicts.keys():
        filename = WORKING_DIR + os.sep + item + '.tif'
        print (filename)
        replaceByDict(landuseFile, replaceDicts[item], filename)
    return replaceDicts['LANDCOVER'].values()


def ReclassCrop(landuseFile, dstdir):
    LandCoverCodes = GenerateLandcoverInitialParameters(landuseFile, dstdir)
    cropFile = CROP_FILE
    attrMap = ReadCropAttrs(cropFile)
    attrNames = CROP_ATTR_LIST
    n = len(attrNames)
    replaceDicts = []
    dstCropTifs = []
    for i in range(n):
        curAttr = attrNames[i]
        curDict = {}
        dic = attrMap[curAttr]
        for code in LandCoverCodes:
            if code == DEFAULT_NODATA:
                continue
            if code not in curDict.keys():
                curDict[code] = dic[code]
        replaceDicts.append(curDict)
        dstCropTifs.append(dstdir + os.sep + curAttr + '.tif')
    # print replaceDicts
    # print(len(replaceDicts))
    # print dstCropTifs
    # print(len(dstCropTifs))
    # Generate GTIFF
    for i in range(len(dstCropTifs)):
        # print dstCropTifs[i]
        replaceByDict(dstdir + os.sep + cropMFile, replaceDicts[i], dstCropTifs[i])


if __name__ == "__main__":
    LoadConfiguration(GetINIfile())
    # ReclassCrop(WORKING_DIR + os.sep + landuseMFile, WORKING_DIR)
    GenerateLandcoverInitialParameters(WORKING_DIR + os.sep + landuseMFile, WORKING_DIR)
