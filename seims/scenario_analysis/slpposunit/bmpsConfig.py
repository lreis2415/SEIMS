# -*- coding: utf-8 -*-

# Author: Huiran GAO
# Date: Sep 12, 2016

import os
import math
import json
from osgeo import gdal, ogr, osr
import numpy
import random
import time
from util import *
from main import *


## Extract all BMPs in each slope position
def PosBMPs(dataDir):
    if (os.path.isdir(dataDir + os.sep + "BMPConfig") == False):
        os.makedirs(dataDir + os.sep + "BMPConfig")
    BMPs_jsonfile = dataDir + os.sep + "BMPConfig" + os.sep + "BMPs.json"
    jsonfile = file(BMPs_jsonfile)
    jsonobj = json.load(jsonfile)
    jsonfile.close
    SmtBMPs = []
    SlpBMPs = []
    VlyBMPs = []
    for i in range(len(jsonobj['BMPs'])):
        bmpId = jsonobj['BMPs'][i]['id']
        for k in range(len(jsonobj['BMPs'][i]['position'])):
            bmpPos = jsonobj['BMPs'][i]['position'][k]
            if bmpPos == 0:
                if bmpId in SmtBMPs:
                    continue
                else:
                    SmtBMPs.append(bmpId)
            if bmpPos == 1:
                if bmpId in SlpBMPs:
                    continue
                else:
                    SlpBMPs.append(bmpId)
            if bmpPos == 2:
                if bmpId in VlyBMPs:
                    continue
                else:
                    VlyBMPs.append(bmpId)
    # print (SmtBMPs, SlpBMPs, VlyBMPs)
    return (SmtBMPs, SlpBMPs, VlyBMPs)


def BMPConf(dataDir):
    # print "Begin configuration......"
    ## Get all BMPs in each slope position
    SmtBMPs = PosBMPs(dataDir)[0]
    SmtBMPs.append(0)
    SlpBMPs = PosBMPs(dataDir)[1]
    SlpBMPs.append(0)
    VlyBMPs = PosBMPs(dataDir)[2]
    VlyBMPs.append(0)
    # print SmtBMPs
    BMPs_jsonfile = dataDir + os.sep + "BMPConfig" + os.sep + "BMPs.json"
    Smt_Slp_jonfile = dataDir + os.sep + "FlowDir" + os.sep + "Smt_Slp.json"
    Slp_Vly_jonfile = dataDir + os.sep + "FlowDir" + os.sep + "Slp_Vly.json"
    BMPs = json.load(file(BMPs_jsonfile))
    Smt_Slp = json.load(file(Smt_Slp_jonfile))
    Slp_Vly = json.load(file(Slp_Vly_jonfile))
    file(BMPs_jsonfile).close
    file(Smt_Slp_jonfile).close
    file(Slp_Vly_jonfile).close

    wshd_polygonized = dataDir + os.sep + "Partion_integ_polygonized.shp"
    allNum = GetFeatureCount(wshd_polygonized)
    smtNum = len(Smt_Slp['info'])
    slpNum = len(Slp_Vly['info'])
    smtBMPConf = []
    slpBMPConf = []
    vlyBMPConf = []

    ##########################################
    ## First step: configure BMPs on Summit ##
    ##########################################
    # print "--First step: configure BMPs on Summit"
    for i in range(smtNum):
        smt_bmp = []
        smtId = Smt_Slp['info'][i]['id']
        smt_bmp.append(smtId)
        ## Select a BMP of Summit randomly
        bmpId = SmtBMPs[random.randint(0, len(SmtBMPs) - 1)]
        smt_bmp.append(bmpId)
        smtBMPConf.append(smt_bmp)
    # print smtBMPConf

    ##########################################
    ## Second step: configure BMPs on Slope ##
    ##########################################
    # print "--Second step: configure BMPs on Slope"
    for j in range(smtNum):
        smtId = Smt_Slp['info'][j]['id']
        slpIdList = Smt_Slp['info'][j]['slope']
        # print slpBMPConf
        if len(slpBMPConf) > 0:
            slp_Confed = [a[0] for a in slpBMPConf]
        else:
            slp_Confed = []
        # print len(slpIdList)
        for p in range(len(slpIdList)):
            slp_bmp = []
            ## search BMP in upstream unit
            smtIndex = [x[0] for x in smtBMPConf].index(smtId)
            smtbmp = smtBMPConf[smtIndex][1]
            if slpIdList[p] in slp_Confed:
                if smtbmp != 0:
                    incmtbmp = []
                    ## traversal the incompat BMPs' id
                    for b in range(len(BMPs['BMPs'])):
                        if BMPs['BMPs'][b]['id'] == smtbmp:
                            incmtbmp = BMPs['BMPs'][b]['incompat']
                            break
                    slpIndex = slp_Confed.index(slpIdList[p])
                    if slpBMPConf[slpIndex][1] in incmtbmp:
                        slpBMPConf[slpIndex][1] = 0
                else:
                    continue
            else:
                slp_bmp.append(slpIdList[p])
                ## Select a BMP of Slope randomly, and rule out the incompat BMP
                bmpId = SlpBMPs[random.randint(0, len(SlpBMPs) - 1)]
                if smtbmp != 0:
                    incmtbmp = []
                    ## traversal the incompat BMPs' id
                    for b in range(len(BMPs['BMPs'])):
                        if BMPs['BMPs'][b]['id'] == smtbmp:
                            incmtbmp = BMPs['BMPs'][b]['incompat']
                            break
                    if bmpId in incmtbmp:
                        slp_bmp.append(0)
                    else:
                        slp_bmp.append(bmpId)
                else:
                    slp_bmp.append(bmpId)
                slpBMPConf.append(slp_bmp)
    ## search the slope unit that not configured BMP
    for s in range(slpNum):
        slp_bmp = []
        slpId = int(Slp_Vly['info'][s]['id'])
        if slpId not in [x[0] for x in slpBMPConf]:
            slp_bmp.append(slpId)
            bmpId = SlpBMPs[random.randint(0, len(SlpBMPs) - 1)]
            slp_bmp.append(bmpId)
            slpBMPConf.append(slp_bmp)

    ####################################################################
    ## Third step: configure BMPs on Valley, the same way as step two ##
    ####################################################################
    # print "--Third step: configure BMPs on Valley"
    for k in range(slpNum):
        slpId = Slp_Vly['info'][k]['id']
        vlyIdList = Slp_Vly['info'][k]['valley']
        # print slpIdList
        # print slpBMPConf
        if len(vlyBMPConf) > 0:
            vly_Confed = [a[0] for a in vlyBMPConf]
        else:
            vly_Confed = []
        # print len(slpIdList)
        for q in range(len(vlyIdList)):
            vly_bmp = []
            ## search BMP in upstream unit
            slpIndex = [x[0] for x in slpBMPConf].index(slpId)
            slpbmp = slpBMPConf[slpIndex][1]
            if vlyIdList[q] in vly_Confed:
                if slpbmp != 0:
                    incmtbmp = []
                    ## traversal the incompat BMPs' id
                    for b in range(len(BMPs['BMPs'])):
                        if BMPs['BMPs'][b]['id'] == slpbmp:
                            incmtbmp = BMPs['BMPs'][b]['incompat']
                            break
                    vlyIndex = vly_Confed.index(vlyIdList[q])
                    if vlyBMPConf[vlyIndex][1] in incmtbmp:
                        vlyBMPConf[vlyIndex][1] = 0
                else:
                    continue
            else:
                vly_bmp.append(vlyIdList[q])
                ## Select a BMP of Slope randomly, and rule out the incompat BMP
                bmpId = VlyBMPs[random.randint(0, len(VlyBMPs) - 1)]
                if slpbmp != 0:
                    incmtbmp = []
                    ## traversal the incompat BMPs' id
                    for b in range(len(BMPs['BMPs'])):
                        if BMPs['BMPs'][b]['id'] == slpbmp:
                            incmtbmp = BMPs['BMPs'][b]['incompat']
                            break
                    if bmpId in incmtbmp:
                        vly_bmp.append(0)
                    else:
                        vly_bmp.append(bmpId)
                else:
                    vly_bmp.append(bmpId)
                vlyBMPConf.append(vly_bmp)
    ## search the valley unit that not configured BMP
    for t in range(allNum):
        vly_bmp = []
        if t not in [x[0] for x in smtBMPConf] and t not in [x[0] for x in slpBMPConf] and t not in [x[0] for x in vlyBMPConf]:
            vly_bmp.append(t)
            bmpId = VlyBMPs[random.randint(0, len(VlyBMPs) - 1)]
            vly_bmp.append(bmpId)
            slpBMPConf.append(vly_bmp)
    #print vlyBMPConf

    ###############################
    ## Last Step: merge all unit ##
    ###############################
    # print "--Last Step: merge all unit"
    # print len(smtBMPConf) + len(slpBMPConf) + len(vlyBMPConf)
    allBMPConf = numpy.concatenate((numpy.concatenate((smtBMPConf, slpBMPConf)), vlyBMPConf))
    # print "BMPs configure finished!"
    # return allBMPConf

def writeRaste(dataDir, fieldPartion, allBMPConf, wRaster=False):
    currentTime = time.ctime().split(" ")
    hms = currentTime[3].split(":")
    timeStr = "%s%s%s%s%s%s" % (currentTime[4], currentTime[1], currentTime[2], hms[0], hms[1], hms[2])
    scenarioTxt = dataDir + os.sep + "BMPConfig" + os.sep + "Scenario_" + timeStr + ".txt"
    ## Write to txt
    scenarioTxt_output = open(scenarioTxt, 'w')
    scenarioTxt_arr = sort2D(allBMPConf)
    scenarioTxt_con = "#Unit: 0 - %s\n" % str(len(scenarioTxt_arr) - 1)
    for m in range(len(scenarioTxt_arr)):
        scenarioTxt_con += str(scenarioTxt_arr[m]) + " "
    scenarioTxt_output.write(scenarioTxt_con)
    scenarioTxt_output.close()
    ## Write to raster
    if wRaster == True:
        print "Start to write raster......"
        scenario = numpy.zeros((nRows, nCols))
        field = ReadRaster(dataDir + os.sep + fieldPartion).data
        noDataValue = ReadRaster(dataDir + os.sep + fieldPartion).noDataValue
        for m in range(nRows):
            for n in range(nCols):
                if field[m][n] != noDataValue:
                    cellValue = numpy.where([x[0] for x in allBMPConf] == field[m][n])
                    scenario[m][n] = allBMPConf[cellValue[0][0]][1]
                else:
                    scenario[m][n] = noDataValue
        scenarioRaster = dataDir + os.sep + "BMPConfig" + os.sep + "Scenario_" + timeStr + ".tif"
        WriteGTiffFile(scenarioRaster, nRows, nCols, scenario, geotrans, srs, noDataValue, gdal.GDT_Float32)
        print "Save current scenario raster as '%s'" % scenarioRaster
    print "Save current scenario info as '%s'" % scenarioTxt

def sort2D(arr):
    col0 = [x[0] for x in arr]
    col1 = []
    col0_sort = numpy.sort(col0)
    for i in range(len(col0_sort)):
        pos = numpy.where(col0 == col0_sort[i])
        col1.append(arr[pos[0][0]][1])
    return col1

if __name__ == "__main__":
    dataDir = r'D:\MasterWorks\SA\data\youfang_data\Data_Youfang\field_partion\FieldPartion'
    fieldPartion = r'Partion_Integ.tif'
    allBMPConf = BMPConf(dataDir)
    print allBMPConf
    # writeRaste(dataDir, fieldPartion, allBMPConf, wRaster=True)

