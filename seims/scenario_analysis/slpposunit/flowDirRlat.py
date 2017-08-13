# -*- coding: utf-8 -*-

# Author: Huiran GAO
# Date: Sep 11, 2016

import json
from util import *
from main import *

## 统计summit与subbasin的一对多关系
def SummitforSub(dataDir, subBasin):
    print "Statistic relationship between Summit and Subbasins......"
    Summit_Subs = []  ##json格式存储
    summit_polygonized = dataDir + os.sep + "summit_polygonized.shp"
    summit = ReadRaster(dataDir + os.sep + "summit_new.tif").data
    smtNum = GetFeatureCount(summit_polygonized)
    # print smtNum
    for i in range(smtNum):
        Summit_Sub = {}
        Summit_Sub["id"] = int(i)
        smtArr = numpy.where(summit == i)
        # print smtArr[0][1]
        subbs = []
        for k in range(len(smtArr[0])):
            # print subBasin[smtArr[0][k]][smtArr[1][k]]
            if (subBasin[smtArr[0][k]][smtArr[1][k]] in subbs):
                continue
            else:
                subbs.append(subBasin[smtArr[0][k]][smtArr[1][k]])
        Summit_Sub["subbasins"] = str(subbs)
        Summit_Subs.append(Summit_Sub)
    data = {}
    data["info"] = Summit_Subs
    SmtSub_json = json.dumps(data, sort_keys=True, indent=4)
    # print SmtSub_json

    ## output
    SmtSub_jsonfile = dataDir + os.sep + "FlowDir" + os.sep + "Smt_Sub.json"
    if (os.path.isdir(dataDir + os.sep + "FlowDir") == False):
        os.makedirs(dataDir + os.sep + "FlowDir")
    SmtSub_output = open(SmtSub_jsonfile, 'w')
    SmtSub_output.write(SmtSub_json)
    SmtSub_output.close()
    print "Save result as '%s'" % SmtSub_jsonfile


## 统计summit与slope的一对多关系
def SummitforSlp(dataDir, field):
    print "Statistic relationship between Summit and Slope......"
    Summit_Slopes = []  ##json格式存储
    summit_polygonized = dataDir + os.sep + "summit_polygonized.shp"
    summit = ReadRaster(dataDir + os.sep + "summit_new.tif").data
    smtNum = GetFeatureCount(summit_polygonized)
    # print smtNum
    for i in range(smtNum):
        Summit_Slp = {}
        smtArr = numpy.where(summit == i)
        # print smtArr
        if int(field[smtArr[0][0]][smtArr[1][0]]) >= 0:
            Summit_Slp["id"] = int(field[smtArr[0][0]][smtArr[1][0]])
            # print smtArr[0][1]
            slps = []
            for k in range(len(smtArr[0])):
                m = smtArr[0][k]
                n = smtArr[1][k]
                ## 循坏遍历8邻域，寻找相邻Slope
                if (m != 0 and n != 0 and m != nRows - 1 and n != nCols - 1):
                    direct_8 = [
                        field[m][n + 1], field[m + 1][n + 1], field[m + 1][n], field[m + 1][n - 1], \
                        field[m][n - 1], field[m - 1][n - 1], field[m - 1][n], field[m - 1][n + 1]
                    ]
                    direct_8_smtPos = [
                        slpPos[m][n + 1], slpPos[m + 1][n + 1], slpPos[m + 1][n], slpPos[m + 1][n - 1], \
                        slpPos[m][n - 1], slpPos[m - 1][n - 1], slpPos[m - 1][n], slpPos[m - 1][n + 1]
                    ]
                else:
                    direct_8 = [field[m][n]]
                for j in range(len(direct_8)):
                    if (direct_8[j] != noDataValue):
                        if (direct_8[j] in slps):
                            continue
                        else:
                            if (direct_8[j] != field[m][n] and direct_8_smtPos[j] == 1):
                                slps.append(int(direct_8[j]))
            Summit_Slp["slope"] = slps
            Summit_Slopes.append(Summit_Slp)
    data = {}
    data["info"] = Summit_Slopes
    SmtSlp_json = json.dumps(data, sort_keys=True, indent=4)
    # print SmtSlp_json

    ## output
    SmtSlp_jsonfile = dataDir + os.sep + "FlowDir" + os.sep + "Smt_Slp.json"
    if (os.path.isdir(dataDir + os.sep + "FlowDir") == False):
        os.makedirs(dataDir + os.sep + "FlowDir")
    SmtSub_output = open(SmtSlp_jsonfile, 'w')
    SmtSub_output.write(SmtSlp_json)
    SmtSub_output.close()
    print "Save result as '%s'" % SmtSlp_jsonfile


## 统计slope与valley的一对多关系
def SlpforVly(dataDir, field):
    print "Statistic relationship between Slope and Valley......"
    Slp_Vlys = []  ##json格式存储
    slope_polygonized = dataDir + os.sep + "slope_polygonized.shp"
    slope = ReadRaster(dataDir + os.sep + "slope_new.tif").data
    valley = ReadRaster(dataDir + os.sep + "valley_new.tif").data
    slpNum = GetFeatureCount(slope_polygonized)
    for i in range(slpNum):
        Slp_Vly = {}
        slpArr = numpy.where(slope == i)
        # print slpArr
        Slp_Vly["id"] = int(field[slpArr[0][0]][slpArr[1][0]])
        vlys = []
        for k in range(len(slpArr[0])):
            m = slpArr[0][k]
            n = slpArr[1][k]
            ## 循坏遍历8邻域，寻找相邻Slope
            if (m != 0 and n != 0 and m != nRows - 1 and n != nCols - 1):
                direct_8 = [
                    field[m][n + 1], field[m + 1][n + 1], field[m + 1][n], field[m + 1][n - 1], \
                    field[m][n - 1], field[m - 1][n - 1], field[m - 1][n], field[m - 1][n + 1]
                ]
                direct_8_slpPos = [
                    slpPos[m][n + 1], slpPos[m + 1][n + 1], slpPos[m + 1][n], slpPos[m + 1][n - 1], \
                    slpPos[m][n - 1], slpPos[m - 1][n - 1], slpPos[m - 1][n], slpPos[m - 1][n + 1]
                ]
            else:
                direct_8 = [field[m][n]]
            for j in range(len(direct_8)):
                if (direct_8[j] != noDataValue):
                    if (direct_8[j] in vlys):
                        continue
                    else:
                        if (direct_8[j] != field[m][n] and direct_8_slpPos[j] == 2):
                            vlys.append(int(direct_8[j]))
        Slp_Vly["valley"] = vlys
        Slp_Vlys.append(Slp_Vly)
    data = {}
    data["info"] = Slp_Vlys
    SlpVly_json = json.dumps(data, sort_keys=True, indent=4)
    # print SmtSlp_json

    ## output
    SlpVly_jsonfile = dataDir + os.sep + "FlowDir" + os.sep + "Slp_Vly.json"
    if (os.path.isdir(dataDir + os.sep + "FlowDir") == False):
        os.makedirs(dataDir + os.sep + "FlowDir")
    SmtSub_output = open(SlpVly_jsonfile, 'w')
    SmtSub_output.write(SlpVly_json)
    SmtSub_output.close()
    print "Save result as '%s'" % SlpVly_jsonfile


# if __name__ == "__main__":
#     dataDir = r'D:\MasterWorks\SA\data\youfang_data\Data_Youfang\field_partion\FieldPartion'
#     subBasinfile = r'subBasin.tif'
#     fieldPartion = r'prePartion.tif'
#     slpPosfile_ywzh = r'SlpPos_ywzh.tif'
#     slpPos = ReadRaster(dataDir + os.sep + slpPosfile_ywzh).data
#     subbasin = ReadRaster(dataDir + os.sep + subBasinfile).data
#     field = ReadRaster(dataDir + os.sep + fieldPartion).data
#
#     SummitforSub(dataDir, subbasin)
#     SummitforSlp(dataDir, field)
#     SlpforVly(dataDir, field)
