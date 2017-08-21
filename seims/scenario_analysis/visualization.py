#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Visualization of Scenarios, which is relative independent with SA.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-09-12  hr - initial implementation.\n
                17-08-18  lj - reorganization.\n
"""
import json
import os
import random
import time

import matplotlib.pyplot as plt
import numpy
from osgeo import gdal


def plot_pareto_front(pop, ws, pop_size, gen_id):
    front = numpy.array([ind.fitness.values for ind in pop])
    # Plot
    plt.figure(gen_id)
    plt.title('Pareto frontier of Scenarios Optimization\n', color='#aa0903')
    # plt.xlabel('Economic calculate_economy(Million Yuan)')
    plt.xlabel('Economic effectiveness')
    plt.ylabel('Environmental effectiveness')
    # front[:, 0] /= 1000000.
    front[:, 1] /= 1000.
    plt.scatter(front[:, 0], front[:, 1], c='r', alpha=0.8, s=12)
    plt.title('\nPopulation: %d, Generation: %d' % (pop_size, gen_id), color='green', fontsize=9,
              loc='right')
    img_path = ws + os.sep + 'Pareto_Gen_%d_Pop_%d.png' % (gen_id, pop_size)
    plt.savefig(img_path)
    # plt.show()


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


def BMPConf(dataDir, confRate=0.5):
    '''
    :param dataDir: Path of BMPs.json file
    :param confRate: Rate of BMP configure, default is 0.5
    :return: BMPs in each field(Array)
    '''
    # print "Begin configuration......"
    ## Get all BMPs in each slope position
    SmtBMPs = PosBMPs(dataDir)[0]
    # SmtBMPs.append(0)
    SlpBMPs = PosBMPs(dataDir)[1]
    # SlpBMPs.append(0)
    VlyBMPs = PosBMPs(dataDir)[2]
    # VlyBMPs.append(0)
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

    wshd_polygonized = dataDir + os.sep + "prePartion_polygonized.shp"
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
        smt_confRate = random.random()
        if smt_confRate < confRate:
            bmpId = SmtBMPs[random.randint(0, len(SmtBMPs) - 1)]
        else:
            bmpId = 0
        smt_bmp.append(bmpId)
        smtBMPConf.append(smt_bmp)
    # print len(smtBMPConf)

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
                slp_confRate = random.random()
                if slp_confRate < confRate:
                    bmpId = SlpBMPs[random.randint(0, len(SlpBMPs) - 1)]
                else:
                    bmpId = 0
                if smtbmp != 0 and bmpId != 0:
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
                vly_confRate = random.random()
                if vly_confRate < confRate:
                    bmpId = VlyBMPs[random.randint(0, len(VlyBMPs) - 1)]
                else:
                    bmpId = 0
                if slpbmp != 0 and bmpId != 0:
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
        if t not in [x[0] for x in smtBMPConf] and t not in [x[0] for x in
                                                             slpBMPConf] and t not in [x[0] for x in
                                                                                       vlyBMPConf]:
            vly_bmp.append(t)
            bmpId = VlyBMPs[random.randint(0, len(VlyBMPs) - 1)]
            vly_bmp.append(bmpId)
            vlyBMPConf.append(vly_bmp)
    # print vlyBMPConf

    ###############################
    ## Last Step: merge all unit ##
    ###############################
    # print "--Last Step: merge all unit"
    # print len(smtBMPConf) + len(slpBMPConf) + len(vlyBMPConf)
    allBMPConf = numpy.concatenate((numpy.concatenate((smtBMPConf, slpBMPConf)), vlyBMPConf))
    # print "BMPs configure finished!"

    ## Sort and remove repeated
    bmpsConf = allBMPConf.tolist()
    # print bmpsConf
    bmpsConf.sort()
    bmpsConf_tmp = [bmpsConf[0]]
    bmpsConf_last = []
    for m in range(1, len(bmpsConf)):
        # print m
        if bmpsConf[m][0] == bmpsConf[m - 1][0]:
            bmpsConf_tmp.append([])
        else:
            bmpsConf_tmp.append(bmpsConf[m])

    for n in range(len(bmpsConf_tmp)):
        if len(bmpsConf_tmp[n]) != 0:
            bmpsConf_last.append(bmpsConf_tmp[n])
    scenarios = [x[1] for x in bmpsConf_last]
    return scenarios


def BMPConf_random(field_num, BMPs, confRate=0.5):
    scenarios = []
    for i in range(field_num):
        conRate = random.random()
        if conRate < confRate:
            ind = random.randint(0, len(BMPs) - 1)
            scenarios.append(BMPs[ind])
        else:
            scenarios.append(0)
    return scenarios


def writeSceRaste(outdataDir, fieldPartion, BMPConf, wRaster=False):
    field = ReadRaster(fieldPartion).data
    nCols = ReadRaster(fieldPartion).nCols
    nRows = ReadRaster(fieldPartion).nRows
    noDataValue = ReadRaster(fieldPartion).noDataValue
    geotrans = ReadRaster(fieldPartion).geotrans
    srs = ReadRaster(fieldPartion).srs

    currentTime = time.ctime().split(" ")
    hms = currentTime[3].split(":")
    timeStr = "%s%s%s%s%s%s" % (
    currentTime[4], currentTime[1], currentTime[2], hms[0], hms[1], hms[2])
    scenarioTxt = outdataDir + os.sep + "Scenario_" + timeStr + ".txt"
    ## Write to txt
    scenarioTxt_output = open(scenarioTxt, 'w')
    scenarioTxt_arr = BMPConf[:]
    scenarioTxt_con = "#Unit: 0 - %s\n" % str(len(scenarioTxt_arr))
    for m in range(len(scenarioTxt_arr)):
        scenarioTxt_con += str(scenarioTxt_arr[m]) + " "
    scenarioTxt_output.write(scenarioTxt_con)
    scenarioTxt_output.close()
    ## Write to raster
    if wRaster == True:
        # print "Start to write raster......"
        scenario = numpy.zeros((nRows, nCols))
        for m in range(nRows):
            for n in range(nCols):
                if field[m][n] != noDataValue:
                    scenario[m][n] = BMPConf[int(field[m][n]) - 1]
                else:
                    scenario[m][n] = noDataValue
        scenarioRaster = outdataDir + os.sep + "Scenario_" + timeStr + ".tif"
        WriteGTiffFile(scenarioRaster, nRows, nCols, scenario, geotrans, srs, noDataValue,
                       gdal.GDT_Float32)
        # print "Save current scenario raster as '%s'" % scenarioRaster
        # print "Save current scenario info as '%s'" % scenarioTxt


#
# def sort2D(arr):
#     col0 = [x[0] for x in arr]
#     col1 = []
#     col0_sort = numpy.sort(col0)
#     for i in range(len(col0_sort)):
#         pos = numpy.where(col0 == col0_sort[i])
#         col1.append(arr[pos[0][0]][1])
#     return col1


def BMPConf_field_Wh(dataDir, confRate=0.5):
    BMPs_jsonfile = dataDir + os.sep + "BMPConfig" + os.sep + "BMPs.json"
    Up_Down_jonfile = dataDir + os.sep + "FlowDir" + os.sep + "Up_Down.json"
    BMPs = json.load(file(BMPs_jsonfile))
    Up_Down = json.load(file(Up_Down_jonfile))
    file(BMPs_jsonfile).close
    file(Up_Down_jonfile).close

    fBMPs = []
    for b in range(len(BMPs['BMPs'])):
        fBMPs.append(BMPs['BMPs'][b]['id'])

    fieldNum = len(Up_Down['info'])
    BMPConf = numpy.zeros(fieldNum)
    for i in range(fieldNum):
        # fieldId = Up_Down['info'][i]['id']
        downslpIdList = Up_Down['info'][i]['downslope']
        f_confRate = random.random()
        if f_confRate < confRate:
            if len(downslpIdList) > 0:
                for ds in downslpIdList:
                    if BMPConf[int(ds)] != 0:
                        BMPConf[i] = 0
                        break
                    else:
                        BMPConf[i] = fBMPs[random.randint(0, len(fBMPs) - 1)]
        else:
            BMPConf[i] = 0

    return BMPConf

# if __name__ == "__main__":
#     dataDir = r'D:\SEIMS_model\Model_data\youwuzhen\model_youwuzhen_10m_longterm\NSGAII_OUTPUT'
#     fieldPartion = r'D:\MasterWorks\SA\data\youfang_data\Data_Youfang\field_partion_result\FieldPartion\prePartion.tif'
#     sce = [1.0, 2.0, 0.0, 0.0, 2.0, 2.0, 2.0, 0.0, 1.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 2.0, 0.0, 2.0, 0.0, 1.0, 1.0, 2.0, 2.0, 0.0, 0.0, 2.0, 3.0, 2.0, 0.0, 0.0, 0.0, 2.0, 1.0, 2.0, 0.0, 1.0, 4.0, 0.0, 2.0, 2.0, 0.0, 2.0, 1.0, 0.0, 5.0, 2.0, 0.0, 0.0, 2.0, 1.0, 0.0, 3.0, 2.0, 3.0, 1.0, 1.0, 1.0, 2.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 2.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 2.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 2.0, 2.0, 0.0, 0.0, 0.0, 2.0, 2.0, 0.0, 4.0, 0.0, 1.0, 4.0, 2.0, 0.0, 2.0, 1.0, 4.0, 2.0, 2.0, 0.0, 2.0, 0.0, 3.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 2.0, 0.0, 1.0, 2.0, 0.0, 5.0, 4.0, 2.0, 2.0, 0.0, 2.0, 0.0, 2.0, 2.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 2.0, 0.0, 2.0, 0.0, 1.0, 0.0, 3.0, 2.0, 1.0, 0.0, 1.0, 2.0, 1.0, 1.0, 2.0, 1.0, 1.0, 2.0, 1.0, 1.0, 2.0, 1.0, 2.0, 0.0, 0.0, 4.0, 2.0, 3.0, 1.0, 0.0, 1.0, 0.0, 0.0, 2.0, 1.0, 2.0, 1.0, 2.0, 1.0, 0.0, 1.0, 3.0, 2.0, 3.0, 0.0, 5.0, 2.0, 1.0, 2.0, 0.0, 2.0, 1.0, 0.0, 4.0, 3.0, 0.0, 4.0, 3.0, 0.0, 0.0, 1.0, 2.0, 2.0, 4.0, 4.0, 0.0, 1.0, 1.0, 4.0, 0.0, 2.0, 3.0, 0.0, 1.0, 1.0, 5.0, 1.0, 2.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 2.0, 0.0, 0.0, 1.0, 2.0, 1.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 3.0, 1.0, 1.0, 0.0, 3.0, 2.0, 2.0, 0.0, 2.0, 0.0, 1.0, 1.0, 3.0, 0.0, 4.0, 0.0, 0.0, 3.0, 1.0, 1.0, 0.0, 1.0, 1.0, 3.0, 2.0, 1.0, 1.0, 2.0, 1.0, 1.0, 0.0, 2.0, 1.0, 1.0, 3.0, 0.0, 3.0, 0.0, 2.0, 0.0, 1.0, 2.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 4.0, 2.0, 1.0, 4.0, 1.0, 2.0, 5.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 2.0, 0.0, 0.0, 2.0, 1.0, 4.0, 1.0, 3.0, 4.0, 0.0, 0.0, 0.0, 1.0, 2.0, 1.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 4.0, 0.0, 2.0, 2.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 2.0, 4.0, 2.0, 2.0, 1.0, 1.0, 2.0, 4.0, 1.0, 1.0, 1.0, 0.0, 0.0, 2.0, 3.0, 0.0, 0.0, 0.0, 4.0, 0.0, 2.0, 2.0, 0.0, 0.0, 4.0, 3.0, 0.0, 1.0, 1.0, 2.0, 0.0, 1.0, 0.0, 1.0, 0.0, 2.0, 0.0, 0.0, 1.0, 2.0, 2.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 4.0, 0.0, 4.0, 2.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 1.0, 2.0, 1.0, 0.0, 0.0, 1.0, 0.0, 4.0, 4.0, 1.0, 0.0, 5.0, 2.0, 0.0, 2.0, 0.0, 0.0, 2.0, 0.0, 0.0, 1.0, 2.0, 3.0, 2.0, 4.0, 2.0, 4.0, 0.0, 1.0, 0.0, 2.0, 1.0, 2.0, 0.0, 1.0, 0.0, 2.0, 1.0, 3.0, 2.0, 3.0]
#     # sce = [0.0, 1.0, 0.0, 5.0, 0.0, 5.0, 0.0, 1.0, 0.0, 3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 2.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 4.0, 0.0, 0.0, 2.0, 5.0, 4.0, 0.0, 0.0, 0.0, 0.0, 3.0, 2.0, 5.0, 2.0, 1.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 4.0, 1.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 3.0, 0.0, 2.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 2.0, 5.0, 1.0, 0.0, 4.0, 2.0, 1.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 1.0, 0.0, 3.0, 0.0, 3.0, 4.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 4.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 5.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 4.0, 1.0, 0.0, 0.0, 2.0, 4.0, 4.0, 2.0, 0.0, 0.0, 0.0, 0.0, 4.0, 2.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 2.0, 0.0, 2.0, 4.0, 0.0, 4.0, 2.0, 3.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 4.0, 2.0, 4.0, 5.0, 2.0, 2.0, 2.0, 0.0, 3.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 2.0, 0.0, 4.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 5.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 4.0, 0.0, 0.0, 4.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 2.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 3.0, 1.0, 0.0, 1.0, 0.0, 3.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0]
#     writeSceRaste(dataDir, fieldPartion, sce, wRaster=True)

# dataDir = r'D:\MasterWorks\SA\data\youfang_data\Data_Youfang\field_partion_result\FieldPartion'
# print BMPConf(dataDir, confRate=0.5)
