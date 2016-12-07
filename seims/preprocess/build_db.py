#! /usr/bin/env python
# coding=utf-8
# Import all model parameters and spatial datasets to MongoDB
# Author: Junzhi Liu
# Revised: Liang-Jun Zhu
#

from pymongo import MongoClient
from pymongo.errors import ConnectionFailure
from gridfs import *

from config import *
from find_sites import FindSites
from gen_subbasins import ImportSubbasinStatistics
from generate_stream_input import GenerateReachTable
from import_bmp_scenario import ImportBMPTables
from import_parameters import (ImportLookupTables, ImportModelConfiguration,
                               ImportParameters)
from weights_mongo import GenerateWeightDependentParameters, GenerateWeightInfo


def BuildMongoDB():
    statusFile = WORKING_DIR + os.sep + FN_STATUS_MONGO
    f = open(statusFile, 'w')
    # build mongodb database
    try:
        conn = MongoClient(host=HOSTNAME, port=PORT)
    except ConnectionFailure:
        sys.stderr.write("Could not connect to MongoDB: %s" % ConnectionFailure.message)
        sys.exit(1)

    db = conn[SpatialDBName]
    # import parameters information to MongoDB
    ImportParameters(TXT_DB_DIR + os.sep + sqliteFile, db)
    # import lookup tables from to MongoDB as GridFS. By LJ, 2016-6-13
    ImportLookupTables(TXT_DB_DIR + os.sep + sqliteFile, db)
    # import model configuration
    ImportModelConfiguration(db)
    f.write("10, Generating reach table...\n")
    f.flush()
    GenerateReachTable(WORKING_DIR, db, forCluster)

    # prepare meteorology data
    if not forCluster:
        subbasinRaster = WORKING_DIR + os.sep + mask_to_ext  # mask.tif
    else:
        subbasinRaster = WORKING_DIR + os.sep + subbasinOut  # subbasin.tif

    if stormMode:
        meteoThiessenList = [PrecSitesThiessen]
        meteoTypeList = [DataType_Precipitation]
    else:
        meteoThiessenList = [MeteorSitesThiessen, PrecSitesThiessen]
        meteoTypeList = [DataType_Meteorology, DataType_Precipitation]

    f.write("20, Finding nearby stations for each sub-basin...\n")
    f.flush()
    if not forCluster: #  OMP version
        basinFile = WORKING_DIR + os.sep + basinVec
        nSubbasins = FindSites(db, ClimateDBName, basinFile, FLD_BASINID, meteoThiessenList, meteoTypeList, simuMode)
    subbasinFile = WORKING_DIR + os.sep + DIR_NAME_SUBBSN + os.sep + subbasinVec  #  MPI version
    nSubbasins = FindSites(db, ClimateDBName, subbasinFile, FLD_SUBBASINID, meteoThiessenList, meteoTypeList, simuMode)

    print "Meteorology sites table generated done. Number of sub-basins:%d" % nSubbasins

    if not forCluster:  # changed by LJ, SubbasinID is 0 means the whole basin!
        nSubbasins = 0

    # import raster data to MongoDB
    f.write("40, Importing raster to MongoDB...\n")
    f.flush()

    tifFolder = WORKING_DIR + os.sep + DIR_NAME_TIFFIMPORT
    if not os.path.exists(tifFolder):
        os.mkdir(tifFolder)
    subbasinStartID = 1
    if not forCluster:
        subbasinStartID = 0
    for i in range(subbasinStartID, nSubbasins + 1):
        subdir = tifFolder + os.sep + str(i)
        if not os.path.exists(subdir):
            os.mkdir(subdir)
    strCmd = '"%s/import_raster" %s %s %s %s %s %d %s' % (
        CPP_PROGRAM_DIR, subbasinRaster, WORKING_DIR, SpatialDBName,
        DB_TAB_SPATIAL.upper(), HOSTNAME, PORT, tifFolder)
    print strCmd
    RunExternalCmd(strCmd)
    # os.system(strCmd)

    print 'Generating weight data...'
    f.write("70, Generating weight data for interpolation of meteorology data...\n")
    f.flush()
    for i in range(subbasinStartID, nSubbasins + 1):
        GenerateWeightInfo(conn, SpatialDBName, i, stormMode)
        # 　added by Liangjun, 2016-6-17
        GenerateWeightDependentParameters(conn, i)
    if genIUH:
        f.write("80, Generating IUH (Instantaneous Unit Hydrograph)...\n")
        f.flush()
        dt = 24
        print 'Generating IUH (Instantaneous Unit Hydrograph)...'
        strCmd = '"%s/iuh" %s %d %s %s %s %d' % (CPP_PROGRAM_DIR, HOSTNAME, PORT,
                                               SpatialDBName, DB_TAB_SPATIAL.upper(), dt, nSubbasins)
        print strCmd
        # os.system(strCmd)
        RunExternalCmd(strCmd)

    f.write("90, Generating Grid layering...\n")
    f.flush()
    layeringDir = WORKING_DIR + os.sep + DIR_NAME_LAYERINFO
    if not os.path.exists(layeringDir):
        os.mkdir(layeringDir)
    print 'Generating Grid layering...'
    strCmd = '"%s/grid_layering" %s %d %s %s %s %d' % (
        CPP_PROGRAM_DIR, HOSTNAME, PORT, layeringDir, SpatialDBName, DB_TAB_SPATIAL.upper(), nSubbasins)
    print strCmd
    # os.system(strCmd)
    RunExternalCmd(strCmd)
    # Test if the grid layering data is imported successfully. Added by LJ, 2016-11-3
    gridLayeringFiles = ['%d_FLOWOUT_INDEX_D8' % nSubbasins, '%d_FLOWIN_INDEX_D8' % nSubbasins]
    spatial = GridFS(db, DB_TAB_SPATIAL.upper())
    needReRun = False
    while not needReRun:
        needReRun = True
        for gridlyr in gridLayeringFiles:
            if not spatial.exists(filename=gridlyr):
                needReRun = False
                print "%s is not imported successfully, grid_layering will be rerun!" % gridlyr
                RunExternalCmd(strCmd)
                break

    # Import BMP scenario database to MongoDB
    ImportBMPTables()
    ImportLookupTables(TXT_DB_DIR + os.sep + sqliteFile, db)
    ImportModelConfiguration(db)
    ImportSubbasinStatistics()
    f.write("100,Finished!")
    f.close()
    print 'Build DB: %s finished!' % SpatialDBName


# test code
if __name__ == "__main__":
    LoadConfiguration(GetINIfile())
    BuildMongoDB()
