#! /usr/bin/env python
# coding=utf-8
# Import all model parameters and spatial datasets to MongoDB
# Author: Junzhi Liu
# Revised: Liang-Jun Zhu
#
from gridfs import GridFS
from pygeoc.utils.utils import UtilClass

from config import *
from utility import LoadConfiguration, status_output
from db_mongodb import ConnectMongoDB
from db_sqlite import reConstructSQLiteDB
from db_import_model_parameters import ImportModelParameters, ImportLookupTables, ImportModelConfiguration
from db_import_stream_parameters import GenerateReachTable
from hydro_find_sites import FindSites
from db_import_sites import ImportHydroClimateSitesInfo
from db_import_daily_meteo import ImportDailyMeteoData
from db_import_daily_rainfall import ImportDailyPrecData
from db_import_observed import ImportMeasurementData
from db_import_interpolation_weights import GenerateWeightInfo, GenerateWeightDependentParameters
from bmp_import_scenario import ImportBMPTables
from db_import_subbasin_statistics import ImportSubbasinStatistics


class ImportMongodbClass(object):
    """Separated function to import data into MongoDB."""

    def __init__(self):
        pass

    @staticmethod
    def import_model_parameters(db, sqlite3db):
        if not FileClass.isfileexists(sqlite3db):
            reConstructSQLiteDB()
        ImportModelParameters(sqlite3db, db)
        # import model configuration
        ImportModelConfiguration(db)

    @staticmethod
    def import_climate_data(climatedb):
        # Climate Data
        SitesMList, SitesPList = ImportHydroClimateSitesInfo(climatedb)
        ImportDailyMeteoData(climatedb, SitesMList)
        ImportDailyPrecData(climatedb, SitesPList)

    @staticmethod
    def import_spatial_raster(dst_dir, subbasin_file, start_id, subbasin_num):
        for i in range(start_id, subbasin_num + 1):
            subdir = dst_dir + os.sep + str(i)
            UtilClass.mkdir(subdir)
        strCmd = '"%s/import_raster" %s %s %s %s %s %d %s' % (CPP_PROGRAM_DIR, subbasin_file,
                                                              WORKING_DIR + os.sep + DIR_NAME_GEODATA2DB, SpatialDBName,
                                                              DB_TAB_SPATIAL.upper(), HOSTNAME, PORT, dst_dir)
        print strCmd
        UtilClass.runcommand(strCmd)

    @staticmethod
    def import_weight_related(conn, maindb, start_id, subbasin_num):
        for i in range(start_id, subbasin_num + 1):
            GenerateWeightInfo(conn, maindb, i, stormMode)
            # added by Liangjun, 2016-6-17
            if not GenerateWeightDependentParameters(conn, maindb, i):
                return False
        return True

    @staticmethod
    def import_grid_layering(maindb, nSubbasins):
        layeringDir = WORKING_DIR + os.sep + DIR_NAME_LAYERINFO
        UtilClass.mkdir(layeringDir)
        strCmd = '"%s/grid_layering" %s %d %s %s %s %d' % (CPP_PROGRAM_DIR, HOSTNAME, PORT, layeringDir,
                                                           SpatialDBName, DB_TAB_SPATIAL.upper(), nSubbasins)
        print strCmd
        UtilClass.runcommand(strCmd)
        # Test if the grid layering data is imported successfully. Added by LJ, 2016-11-3
        gridLayeringFiles = ['%d_FLOWOUT_INDEX_D8' % nSubbasins, '%d_FLOWIN_INDEX_D8' % nSubbasins]
        spatial = GridFS(maindb, DB_TAB_SPATIAL.upper())
        needReRun = False
        while not needReRun:
            needReRun = True
            for gridlyr in gridLayeringFiles:
                if not spatial.exists(filename = gridlyr):
                    needReRun = False
                    print "%s is not imported successfully, grid_layering will be rerun!" % gridlyr
                    UtilClass.runcommand(strCmd)
                    break


def BuildMongoDB():
    statusFile = WORKING_DIR + os.sep + DIR_NAME_LOG + os.sep + FN_STATUS_MONGO
    f = open(statusFile, 'w')
    # build a connection to mongodb database
    client = ConnectMongoDB(HOSTNAME, PORT)
    conn = client.get_conn()
    maindb = conn[SpatialDBName]
    climatedb = conn[ClimateDBName]
    scenariodb = None
    if useScernario:
        scenariodb = conn[BMPScenarioDBName]

    # import model parameters information to MongoDB
    status_output('Import model parameter and configuration tables...', 10, f)
    sqlite3db = WORKING_DIR + os.sep + DIR_NAME_IMPORT2DB + os.sep + sqlite_file
    ImportMongodbClass.import_model_parameters(maindb, sqlite3db)

    # import lookup tables to MongoDB as GridFS. By LJ, 2016-6-13
    status_output('Import lookup tables as GridFS...', 15, f)
    ImportLookupTables(maindb, sqlite3db)

    # import stream parameters
    status_output("Generating reach table with initialized parameters...", 20, f)
    GenerateReachTable(maindb, WORKING_DIR, forCluster)

    # prepare meteorology data
    if not forCluster:
        subbasinRaster = WORKING_DIR + os.sep + DIR_NAME_GEODATA2DB + os.sep + mask_to_ext  # mask.tif
    else:
        subbasinRaster = WORKING_DIR + os.sep + DIR_NAME_GEODATA2DB + os.sep + subbasinOut  # subbasin.tif

    if stormMode:
        meteoThiessenList = [PrecSitesThiessen]
        meteoTypeList = [DataType_Precipitation]
    else:
        meteoThiessenList = [MeteorSitesThiessen, PrecSitesThiessen]
        meteoTypeList = [DataType_Meteorology, DataType_Precipitation]
    if not forCluster:  # OMP version
        basinFile = WORKING_DIR + os.sep + DIR_NAME_GEOSHP + os.sep + basinVec
        nSubbasins = FindSites(maindb, ClimateDBName, basinFile, FLD_BASINID, meteoThiessenList, meteoTypeList,
                               simuMode)
    subbasinFile = WORKING_DIR + os.sep + DIR_NAME_GEOSHP + os.sep + subbasinVec  # MPI version
    nSubbasins = FindSites(maindb, ClimateDBName, subbasinFile, FLD_SUBBASINID, meteoThiessenList, meteoTypeList,
                           simuMode)
    print "Number of sub-basins:%d" % nSubbasins

    status_output("Import climate data....", 30, f)
    ImportMongodbClass.import_climate_data(climatedb)

    status_output("Import observed data, such as discharge, sediment yield....", 35, f)
    # Measurement Data, such as discharge, sediment yield.
    ImportMeasurementData(climatedb)

    if not forCluster:  # changed by LJ, SubbasinID is 0 means the whole basin!
        nSubbasins = 0

    # import raster data to MongoDB
    status_output("Importing raster to MongoDB....", 40, f)
    tifFolder = WORKING_DIR + os.sep + DIR_NAME_IMPORT2DB
    UtilClass.mkdir(tifFolder)
    subbasinStartID = 1
    if not forCluster:
        subbasinStartID = 0
    ImportMongodbClass.import_spatial_raster(tifFolder, subbasinRaster, subbasinStartID, nSubbasins)

    # Import weight and related data
    status_output("Generating weight data for interpolation of meteorology data and weight dependent parameters....",
                  50, f)
    if not ImportMongodbClass.import_weight_related(conn, maindb, subbasinStartID, nSubbasins):
        #  reImport spatial raster
        ImportMongodbClass.import_spatial_raster(tifFolder, subbasinRaster, subbasinStartID, nSubbasins)
        ImportMongodbClass.import_weight_related(conn, maindb, subbasinStartID, nSubbasins)

    # Import IUH
    if genIUH:
        status_output("Generating and importing IUH (Instantaneous Unit Hydrograph)....", 60, f)
        dt = 24
        strCmd = '"%s/iuh" %s %d %s %s %s %d' % (CPP_PROGRAM_DIR, HOSTNAME, PORT, SpatialDBName,
                                                 DB_TAB_SPATIAL.upper(), dt, nSubbasins)
        print strCmd
        UtilClass.runcommand(strCmd)

    # Import grid layering data
    status_output("Generating and importing grid layering....", 70, f)
    ImportMongodbClass.import_grid_layering(maindb, nSubbasins)

    # Import BMP scenario database to MongoDB
    status_output("Importing bmp scenario....", 70, f)
    ImportBMPTables(maindb, scenariodb)

    # Import subbasin statistics information
    status_output("Importing subbasin statistics information....", 90, f)
    ImportSubbasinStatistics(maindb)

    status_output("Build DB: %s finished!" % SpatialDBName, 100, f)
    f.close()
    # close connection to MongoDB
    client.close()


# test code
if __name__ == "__main__":
    LoadConfiguration(getconfigfile())
    BuildMongoDB()
