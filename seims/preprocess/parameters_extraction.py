#! /usr/bin/env python
# coding=utf-8
# @Extract parameters from landuse, soil properties etc.
#
import types

import numpy

from cn2 import GenerateCN2
from config import *
from delta_s import GenerateDelta_s
from depression import DepressionCap
from gen_lookup_table import CreateLanduseLookupTable
from init_moisture import InitMoisture
from radius import GenerateRadius
from reclass_crop import ReclassCrop
from runoff_coef import RunoffCoefficent
from soil_param import *
from t0_s import GenerateT0_s
from velocity import GenerateVelocity


def soil_parameters2(dstdir, maskFile, soilSEQNTif, soilSEQNTxt):
    soilSEQNData = ReadDataItemsFromTxt(soilSEQNText)
    defaultSoilType = float(soilSEQNData[1][0])
    # Mask soil_SEQN tif
    configFile = "%s%smaskSoilConfig.txt" % (dstdir, os.sep)
    fMask = open(configFile, 'w')
    fMask.write(maskFile + "\n")
    fMask.write("%d\n" % (1,))
    soiltypeFile = dstdir + os.sep + soiltypeMFile
    s = "%s\t%d\t%s\n" % (soilSEQNTif, defaultSoilType, soiltypeFile)
    fMask.write(s)
    fMask.close()
    s = '"%s/mask_raster" %s' % (CPP_PROGRAM_DIR, configFile)
    # os.system(s)
    RunExternalCmd(s)

    # Read soil properties from txt file
    soilInstances = []
    soilPropFlds = soilSEQNData[0][:]
    for i in range(1, len(soilSEQNData)):
        curSoilDataItem = soilSEQNData[i][:]
        curSEQN = curSoilDataItem[0]
        curSNAM = curSoilDataItem[1]
        curSoilIns = SoilProperty(curSEQN, curSNAM)
        for j in range(2, len(soilPropFlds)):
            curFlds = SplitStr(curSoilDataItem[j], ',')  # Get field values
            for k in range(len(curFlds)):
                curFlds[k] = float(curFlds[k])  # Convert to float
            if StringMatch(soilPropFlds[j], SOL_NLYRS):
                curSoilIns.SOILLAYERS = int(curFlds[0])
            elif StringMatch(soilPropFlds[j], SOL_Z):
                curSoilIns.SOILDEPTH = curFlds
            elif StringMatch(soilPropFlds[j], SOL_OM):
                curSoilIns.OM = curFlds
            elif StringMatch(soilPropFlds[j], SOL_CLAY):
                curSoilIns.CLAY = curFlds
            elif StringMatch(soilPropFlds[j], SOL_SILT):
                curSoilIns.SILT = curFlds
            elif StringMatch(soilPropFlds[j], SOL_SAND):
                curSoilIns.SAND = curFlds
            elif StringMatch(soilPropFlds[j], SOL_ROCK):
                curSoilIns.ROCK = curFlds
            elif StringMatch(soilPropFlds[j], SOL_ZMX):
                curSoilIns.SOL_ZMX = curFlds[0]
            elif StringMatch(soilPropFlds[j], SOL_ANIONEXCL):
                curSoilIns.ANION_EXCL = curFlds[0]
            elif StringMatch(soilPropFlds[j], SOL_CRK):
                curSoilIns.SOL_CRK = curFlds[0]
            elif StringMatch(soilPropFlds[j], SOL_BD):
                curSoilIns.DENSITY = curFlds
            elif StringMatch(soilPropFlds[j], SOL_K):
                curSoilIns.CONDUCTIVITY = curFlds
            elif StringMatch(soilPropFlds[j], SOL_WP):
                curSoilIns.WILTINGPOINT = curFlds
            elif StringMatch(soilPropFlds[j], SOL_FC):
                curSoilIns.FIELDCAP = curFlds
            elif StringMatch(soilPropFlds[j], SOL_AWC):
                curSoilIns.AWC = curFlds
            elif StringMatch(soilPropFlds[j], SOL_POROSITY):
                curSoilIns.POROSITY = curFlds
            elif StringMatch(soilPropFlds[j], SOL_USLE_K):
                curSoilIns.USLE_K = curFlds
            elif StringMatch(soilPropFlds[j], SOL_ALB):
                curSoilIns.SOL_ALB = curFlds
            elif StringMatch(soilPropFlds[j], ESCO):
                curSoilIns.ESCO = curFlds[0]
            elif StringMatch(soilPropFlds[j], SOL_NO3):
                curSoilIns.SOL_NO3 = curFlds
            elif StringMatch(soilPropFlds[j], SOL_NH4):
                curSoilIns.SOL_NH4 = curFlds
            elif StringMatch(soilPropFlds[j], SOL_ORGN):
                curSoilIns.SOL_ORGN = curFlds
            elif StringMatch(soilPropFlds[j], SOL_SOLP):
                curSoilIns.SOL_SOLP = curFlds
            elif StringMatch(soilPropFlds[j], SOL_ORGP):
                curSoilIns.SOL_ORGP = curFlds
        curSoilIns.CheckData()
        soilInstances.append(curSoilIns)
    soilPropDict = {}
    for sol in soilInstances:
        curSolDict = sol.SoilDict()
        for fld in curSolDict.keys():
            if fld in soilPropDict.keys():
                soilPropDict[fld].append(curSolDict[fld])
            else:
                soilPropDict[fld] = [curSolDict[fld]]
    # print soilPropDict.keys()
    # print soilPropDict.values()

    replaceDicts = []
    dstSoilTifs = []
    SEQNs = soilPropDict[SOL_SEQN]
    maxLyrNum = int(numpy.max(soilPropDict[SOL_NLYRS]))
    for key in soilPropDict.keys():
        if key != SOL_SEQN and key != SOL_NAME:
            keyL = 1
            for keyV in soilPropDict[key]:
                if type(keyV) is types.ListType:
                    if len(keyV) > keyL:
                        keyL = len(keyV)
            if keyL == 1:
                curDict = {}
                for i in range(len(SEQNs)):
                    curDict[float(SEQNs[i])] = soilPropDict[key][i]
                replaceDicts.append(curDict)
                dstSoilTifs.append(dstdir + os.sep + key + '.tif')
            else:
                for i in range(maxLyrNum):
                    curDict = {}
                    for j in range(len(SEQNs)):
                        if i < soilPropDict[SOL_NLYRS][j]:
                            curDict[float(SEQNs[j])] = soilPropDict[key][j][i]
                        else:
                            curDict[float(SEQNs[j])] = DEFAULT_NODATA
                    replaceDicts.append(curDict)
                    dstSoilTifs.append(dstdir + os.sep +
                                       key + '_' + str(i + 1) + '.tif')
    # print replaceDicts
    # print(len(replaceDicts))
    # print dstSoilTifs
    # print(len(dstSoilTifs))

    # Generate GTIFF
    for i in range(len(dstSoilTifs)):
        print (dstSoilTifs[i])
        replaceByDict(soiltypeFile, replaceDicts[i], dstSoilTifs[i])


def landuse_parameters(dstdir, maskFile, inputLanduse, landuseFile, sqliteFile, defaultLanduse):
    #  TODO, this function should be replaced by replaceByDict() function! By LJ
    # mask landuse map using the mask_raster program
    configFile = "%s%s%s" % (dstdir, os.sep, FN_STATUS_MASKLANDUSE)
    fMask = open(configFile, 'w')
    fMask.write(maskFile + "\n")
    fMask.write("%d\n" % (1,))
    s = "%s\t%d\t%s\n" % (inputLanduse, defaultLanduse, landuseFile)
    fMask.write(s)
    fMask.close()

    s = '"%s/mask_raster" %s' % (CPP_PROGRAM_DIR, configFile)
    # os.system(s)
    RunExternalCmd(s)

    # reclassify
    reclassLuFile = "%s/reclassLanduseConfig.txt" % (dstdir)
    fReclassLu = open(reclassLuFile, 'w')
    fReclassLu.write("%s\t%d\n" % (landuseFile, defaultLanduse))
    fReclassLu.write("%s/lookup\n" % (dstdir))
    fReclassLu.write(dstdir + "\n")
    landuseAttrList = LANDUSE_ATTR_LIST
    n = len(landuseAttrList)
    fReclassLu.write("%d\n" % (n))
    fReclassLu.write("\n".join(landuseAttrList))
    fReclassLu.close()
    s = '"%s/reclassify" %s %s/neigh.nbr' % (
        CPP_PROGRAM_DIR, reclassLuFile, PREPROC_SCRIPT_DIR)
    # MPI version is problematic, do not know why, By LJ
    # s = "mpiexec -n %d %s/reclassify %s %s/cpp_src/reclassify/neigh.nbr" % (
    # np, CPP_PROGRAM_DIR, reclassLuFile, PREPROC_SCRIPT_DIR)
    # if MPIEXEC_DIR is not None:
    #     s = MPIEXEC_DIR + os.sep + s
    # os.system(s)
    RunExternalCmd(s)
    # ReclassLanduse(landuseFile, sqliteFile, dstdir)


def ExtractParameters():
    # , , True, True, True, True
    maskFile = WORKING_DIR + os.sep + mask_to_ext
    landuseFile = WORKING_DIR + os.sep + landuseMFile
    statusFile = WORKING_DIR + os.sep + FN_STATUS_EXTRACTPARAM
    f = open(statusFile, 'w')
    # generate landuse and soil properties lookup tables
    status = "Generating landuse and soil properties lookup tables..."
    print ("[Output] %s, %s" % (WORKING_DIR, status))
    f.write("%d,%s\n" % (10, status))
    f.flush()
    str_sql = 'select landuse_id, ' + \
        ','.join(LANDUSE_ATTR_LIST) + ' from LanduseLookup'
    CreateLanduseLookupTable(
        TXT_DB_DIR + os.sep + sqliteFile, LANDUSE_ATTR_LIST, str_sql, WORKING_DIR)

    # Change soil properties to Raster2D data, 2016-5-20, LJ
    # str_sql = 'select soilcode,' + ','.join(SOIL_ATTR_DB) + ' from SoilLookup'
    # CreateLookupTable(sqliteFile, SOIL_ATTR_LIST, str_sql,WORKING_DIR)

    # landuse parameters
    status = "Generating landuse attributes..."
    print ("[Output] %s, %s" % (WORKING_DIR, status))
    f.write("%d,%s\n" % (20, status))
    f.flush()
    landuse_parameters(WORKING_DIR, maskFile, landuseOriginFile, landuseFile, TXT_DB_DIR + os.sep + sqliteFile,
                       defaultLanduse)

    # soil physical and chemical parameters
    status = "Calculating inital soil physical and chemical parameters..."
    print ("[Output] %s, %s" % (WORKING_DIR, status))
    f.write("%d,%s\n" % (30, status))
    f.flush()
    soil_parameters2(WORKING_DIR, maskFile, soilSEQNFile, soilSEQNText)
    # soil_parameters(WORKING_DIR, maskFile, sandList, clayList, orgList) ##
    # replaced by LJ, 2016-5-21

    # parameters derived from DEM
    status = "Calculating inital soil moisture..."
    print ("[Output] %s, %s" % (WORKING_DIR, status))
    f.write("%d,%s\n" % (40, status))
    f.flush()
    InitMoisture(WORKING_DIR)

    if genCrop:
        status = "Generating crop/landcover attributes..."
        print ("[Output] %s, %s" % (WORKING_DIR, status))
        f.write("%d,%s\n" % (50, status))
        f.flush()

        # THIS seems useless? BY LJ.
        # reclassFile = "%s/reclassCropConfig.txt" % (WORKING_DIR)
        # fReclass = open(reclassFile, 'w')
        # fReclass.write("%s\t%d\n" % (landuseFile, 8))
        # fReclass.write("%s/lookup\n" % (WORKING_DIR))
        # fReclass.write(WORKING_DIR + "\n")
        # attrList = CROP_ATTR_LIST
        # n = len(attrList)
        # fReclass.write("%d\n" % (n))
        # fReclass.write("\n".join(attrList))
        # fReclass.close()
        # s = "%s/reclassify %s %s/cpp_src/reclassify/neigh.nbr" % (CPP_PROGRAM_DIR, reclassFile, PREPROC_SCRIPT_DIR)
        # os.system(s)

        ReclassCrop(landuseFile, WORKING_DIR)

    status = "Calculating depression storage..."
    print ("[Output] %s, %s" % (WORKING_DIR, status))
    f.write("%d,%s\n" % (70, status))
    f.flush()
    DepressionCap(WORKING_DIR, TXT_DB_DIR + os.sep + sqliteFile)

    if genCN:
        status = "Calculating CN numbers..."
        print ("[Output] %s, %s" % (WORKING_DIR, status))
        f.write("%d,%s\n" % (80, status))
        f.flush()
        GenerateCN2(WORKING_DIR, TXT_DB_DIR + os.sep + sqliteFile)

    if genIUH:
        status = "Calculating IUH parameters..."
        print ("[Output] %s, %s" % (WORKING_DIR, status))
        f.write("%d,%s\n" % (85, status))
        f.flush()
        GenerateRadius(WORKING_DIR, "T2")
        GenerateVelocity(WORKING_DIR)
        GenerateT0_s(WORKING_DIR)
        GenerateDelta_s(WORKING_DIR)

    if genRunoffCoef:
        status = "Calculating runoff coefficient..."
        print ("[Output] %s, %s" % (WORKING_DIR, status))
        f.write("%d,%s\n" % (90, status))
        f.flush()
        RunoffCoefficent(WORKING_DIR, TXT_DB_DIR + os.sep + sqliteFile)
    f.write("%d,%s\n" % (100, "Finished!"))
    f.close()


if __name__ == "__main__":
    LoadConfiguration(GetINIfile())
    maskFile = WORKING_DIR + os.sep + mask_to_ext
    ExtractParameters()
