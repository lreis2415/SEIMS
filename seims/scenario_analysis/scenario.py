#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base class of Scenario for coupling NSAG-II and SEIMS.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-10-29  hr - initial implementation.\n
                17-08-18  lj - redesign and rewrite.\n
"""
import os, sys
import random
import time
import scoop
from pymongo import MongoClient
from subprocess import Popen
from subprocess import PIPE
from config import *
from readTextInfo import *
from bmpsConfig import *


class Scenario:
    def __init__(self):
        self.id = None
        self.attributes = []
        self.field_Num = field_Num
        # self.point_cattle_Num = point_cattle_Num
        # self.point_pig_Num = point_pig_Num
        # self.point_sewage_Num = point_sewage_Num
        self.sce_list = []
        self.cost_eco = 0.
        self.benefit_env = 0.

    def getIdfromMongo(self):
        '''
        set new scenario id according to the existing
         scenario ids, i.e., the max id + 1
        '''
        client = MongoClient(HOSTNAME, PORT)
        db = client[BMPScenarioDBName]
        collection = db.BMP_SCENARIOS
        idsList = []
        for s in collection.find():
            idsList.append(int(s['ID']))
        idList = list(set(idsList))
        self.id = idList[-1] + 1

    def setId(self, id):
        '''
        set new scenario id by given number
        :param id:
        '''
        self.id = id

    def create(self):
        # Create a scenario numeric string
        # for _ in range(self.field_Num):
        #     self.attributes.append(selectBMPatRandom(bmps_farm))
        # for _ in range(self.point_cattle_Num):
        #     self.attributes.append(selectBMPatRandom(bmps_cattle))
        # for _ in range(self.point_pig_Num):
        #     self.attributes.append(selectBMPatRandom(bmps_pig))
        # for pt_swg in range(self.point_sewage_Num):
        #     p_s_index = int(point_sewage[pt_swg]) % 10000 - 1
        #     self.attributes.append(selectBMPatRandom(bmps_sewage[p_s_index]))

        # Create confRate value randomly, 0.25~0.75
        cr = random.randint(25, 75) / 100.
        if BMPs_rule:
            self.attributes.extend(BMPConf(MODEL_DIR, confRate=cr))
            # self.attributes.extend(BMPConf_field_Wh(MODEL_DIR, confRate=cr))
        else:
            self.attributes.extend(BMPConf_random(self.field_Num, bmps_areal_struct, confRate=cr))

    def decoding(self):
        # scenario section
        if len(self.attributes) == 0:
            raise Exception("'attributes' cannot be Null!")
        if self.id is None:
            raise Exception("'id' cannot be None!")

        # field_index = self.field_Num
        locations = getBMPsLocations(self.attributes, bmps_areal_struct)
        sinfo = str(self.id) + "\tS_" + str(self.id) + "\t17\t"
        for bmp in bmps_areal_struct:
            scenario_Row = ""
            scenario_Row += sinfo
            scenario_Row += str(bmp) + "\tRASTER|MGT_FIELDS\tAREAL_STRUCT_MANAGEMENT\t"
            locationStr = ""
            for loc in locations[bmp - 1]:
                locationStr += str(loc) + ","
            locationStr = locationStr[:-1]
            scenario_Row += locationStr
            self.sce_list.append(scenario_Row)
        #
        # point_cattle_index = self.point_cattle_Num + field_index
        # point_pig_index = self.point_pig_Num + point_cattle_index
        # point_sewage_index = self.point_sewage_Num + point_pig_index
        # # farm field
        # sinfo = str(self.id) + "\tS_" + str(self.id) + "\t12\t"
        # farm_BMP_dedupl = list(set(self.attributes[0:field_index]))
        # if len(farm_BMP_dedupl) > 1:
        #     for f1 in range(4):
        #         scenario_Row = ""
        #         scenario_Row += sinfo
        #         farm_conf = getFarmConfig(self.attributes[0:field_index], field_farm)
        #         if f1 == 0 or f1 == 1:
        #             scenario_Row += str(f1) + "\tRASTER|MGT_FIELDS\tPLANT_MANAGEMENT\t" + farm_conf[0]
        #         else:
        #             scenario_Row += str(f1) + "\tRASTER|MGT_FIELDS\tPLANT_MANAGEMENT\t" + farm_conf[1]
        #         self.sce_list.append(scenario_Row)
        # else:
        #     for f2 in range(2):
        #         scenario_Row = ""
        #         scenario_Row += sinfo
        #         farm_BMP_do = False
        #         for i in range(0, field_index):
        #             if self.attributes[i] == 1:
        #                 farm_BMP_do = True
        #             else:
        #                 farm_BMP_do = False
        #         if farm_BMP_do:
        #             scenario_Row += str(bmps_farm[f2] + 2) + "\t"
        #         else:
        #             scenario_Row += str(bmps_farm[f2]) + "\t"
        #         scenario_Row += "RASTER|MGT_FIELDS\tPLANT_MANAGEMENT\tALL"
        #         self.sce_list.append(scenario_Row)
        # # point source
        # # Cattle
        # cattleConfig = getPointConfig(self.attributes, bmps_cattle, point_cattle, field_index, point_cattle_index)
        # if len(cattleConfig) > 0:
        #     self.sce_list.extend(decodPointScenario(self.id, cattleConfig, 10000))
        # pigConfig = getPointConfig(self.attributes, bmps_pig, point_pig, point_cattle_index, point_pig_index)
        # # Pig
        # if len(pigConfig) > 0:
        #     self.sce_list.extend(decodPointScenario(self.id, pigConfig, 20000))
        # # Sewage
        # for p_s in range(self.point_sewage_Num):
        #     s_index = int(int(point_sewage[p_s]) % 10000)
        #     sewageConfig = getPointConfig(self.attributes, bmps_sewage[s_index], point_sewage, point_pig_index, point_sewage_index)
        #     if len(sewageConfig) > 0:
        #         self.sce_list.extend(decodPointScenario(self.id, sewageConfig, 40000))

    def importoMongo(self, hostname, port, dbname):
        '''
        Import scenario list to MongoDB
        :return:
        '''
        client = MongoClient(hostname, port)
        db = client[dbname]
        collection = db.BMP_SCENARIOS
        keyarray = ["ID", "NAME", "BMPID", "SUBSCENARIO", "DISTRIBUTION", "COLLECTION", "LOCATION"]
        for line in self.sce_list:
            conf = {}
            li_list = line.split('\t')
            for i in range(len(li_list)):
                if isNumericValue(li_list[i]):
                    conf[keyarray[i]] = float(li_list[i])
                else:
                    conf[keyarray[i]] = str(li_list[i]).upper()
            collection.insert(conf)

    def cost(self):
        if len(self.attributes) == 0:
            raise Exception("'attributes' cannot be Null!")
        # field_index = self.field_Num
        # point_cattle_index = self.point_cattle_Num + field_index
        # point_pig_index = self.point_pig_Num + point_cattle_index
        # point_sewage_index = self.point_sewage_Num + point_pig_index
        # for i1 in range(0, field_index):
        #     self.cost_eco += bmps_farm_cost[int(self.attributes[i1])] * farm_area[i1]
        # for i2 in range(field_index, point_cattle_index):
        #     self.cost_eco += bmps_cattle_cost[int(self.attributes[i2])] * point_cattle_size[i2 - field_index]
        # for i3 in range(point_cattle_index, point_pig_index):
        #     self.cost_eco += bmps_pig_cost[int(self.attributes[i3])] * point_pig_size[i3 - point_cattle_index]
        # for i4 in range(point_pig_index, point_sewage_index):
        #     self.cost_eco += bmps_sewage_cost[int(self.attributes[i4])]

        for f in range(field_Num):
            bmpPosArr = numpy.where(numpy.array(bmps_areal_struct) == self.attributes[f])
            if len(bmpPosArr[0]) > 0:
                self.cost_eco += bmps_areal_struct_cost[bmpPosArr[0][0]] * field_Area[f]


    def benefit(self):
        printInfo("Scenario ID: " + str(self.id))
        # startT = time.time()
        cmdStr = "%s %s %d %d %s %d %d" % (
        model_Exe, model_Workdir, threadsNum, layeringMethod, HOSTNAME, PORT, self.id)
        # print cmdStr
        process = Popen(cmdStr, shell=True, stdout=PIPE)
        while process.stdout.readline() != "":
            line = process.stdout.readline().split("\n")
            if line[0] != "" and len(line[0]) == 20:
                lineArr = line[0].split(' ')[0].split('-')
                if int(lineArr[2]) == 1:
                    sys.stdout.write(str(lineArr[0]) + "-" + str(lineArr[1]) + " ")
            continue
        process.wait()

        dataDir = model_Workdir + os.sep + "OUTPUT" + str(self.id)
        if process.returncode == 0:
            ## Outlet
            # polluteList = ['SED']
            # polluteWt = [1.]
            # for pp in range(len(polluteList)):
            #     simData = ReadSimfromTxt(timeStart, timeEnd, dataDir, polluteList[pp], subbasinID=0)
            #     self.benefit_env += sum(simData) / polluteWt[pp]

            ## soil loss in each cell
            while not os.path.isfile(dataDir + os.sep + "0_" + soilErosion):
                time.sleep(2)
            if os.path.isfile(dataDir + os.sep + "0_" + soilErosion):
                self.benefit_env = getSoilErosion(dataDir, soilErosion, subbasinID=0)
            else:
                generationsInfoFile = model_Workdir + os.sep + "NSGAII_OUTPUT" + os.sep + "scenarios_info.txt"
                Sces_env = getScesInfo(generationsInfoFile)
                self.benefit_env = numpy.max(Sces_env)
        else:
            # process.kill()
            ## If process is killed, benefit_env is replaced by average of exist benefit_env value
            generationsInfoFile = model_Workdir + os.sep + "NSGAII_OUTPUT" + os.sep + "scenarios_info.txt"
            Sces_env = getScesInfo(generationsInfoFile)
            self.benefit_env = numpy.max(Sces_env)

        # Save scenario raster
        # createForld(dataDir)
        # writeSceRaste(dataDir, MODEL_DIR + os.sep + fieldRaster, self.attributes, wRaster=False)

        printInfo(os.linesep + "cost_eco: " + str(self.cost_eco))
        printInfo("benefit_env: " + str(self.benefit_env))
        # endT = time.time()
        # printInfo("SEIMS running time: %.2fs" % (endT - startT))

    def saveInfo(self, txtfile):
        outfile = file(txtfile, 'a')
        infoStr = str(self.id) + "\t" + str(self.cost_eco) + "\t" + str(self.benefit_env) \
                  + "\t" + str(self.attributes) + LF
        outfile.write(infoStr)
        outfile.close()


# if __name__ == "__main__":
#     s = Scenario()
#     s.id = 14323685
#     s.attributes = [0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 1.0, 0.0, 0.0, 2.0, 0.0, 1.0, 0.0, 4.0, 0.0, 1.0, 0.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 4.0, 0.0, 3.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 5.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 2.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 4.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 5.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 4.0, 1.0, 0.0, 0.0, 2.0, 4.0, 4.0, 2.0, 0.0, 0.0, 0.0, 0.0, 4.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 1.0, 0.0, 0.0, 3.0, 5.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 2.0, 0.0, 4.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 5.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 2.0, 4.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 5.0, 5.0, 0.0, 1.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 3.0, 1.0, 0.0, 1.0, 0.0, 3.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 4.0, 5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
#     # s.create()
#     # s.cost()
#     s.decoding()
#     s.importoMongo(HOSTNAME, PORT, BMPScenarioDBName)
#     # s.benefit()
#     # print s.sce_list
#     # print s.benefit_env
