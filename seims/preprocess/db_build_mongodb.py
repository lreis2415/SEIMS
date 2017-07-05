#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Import all model parameters and spatial datasets to MongoDB
    @author   : Liangjun Zhu, Junzhi Liu
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-26  lj - reformat according to pylint and google style
"""
import os
import sqlite3
from struct import pack

from gridfs import GridFS

from seims.preprocess.bmp_import_scenario import ImportScenario2Mongo
from seims.preprocess.config import parse_ini_configuration
from seims.preprocess.db_import_interpolation_weights import ImportWeightData
from seims.preprocess.db_import_meteorology import ImportMeteoData
from seims.preprocess.db_import_model_parameters import ImportParam2Mongo
from seims.preprocess.db_import_observed import ImportObservedData
from seims.preprocess.db_import_precipitation import ImportPrecipitation
from seims.preprocess.db_import_sites import ImportHydroClimateSites
from seims.preprocess.db_import_stream_parameters import ImportReaches2Mongo
from seims.preprocess.db_mongodb import ConnectMongoDB, MongoQuery
from seims.preprocess.text import DBTableNames
from seims.preprocess.utility import status_output
from seims.pygeoc.pygeoc.utils.utils import UtilClass, MathClass


class ImportMongodbClass(object):
    """Separated function to import data into MongoDB."""
    _TAB_LOOKUP_LANDUSE = "LanduseLookup"
    _TAB_LOOKUP_SOIL = "SoilLookup"
    # Metadata field names for lookup gridfs
    _LOOKUP_ITEM_COUNT = 'ITEM_COUNT'
    _LOOKUP_FIELD_COUNT = 'FIELD_COUNT'

    def __init__(self):
        """Empty"""
        pass

    @staticmethod
    def climate_data(cfg, main_db, clim_db):
        """Climate data."""
        ImportHydroClimateSites.workflow(cfg, main_db, clim_db)
        ImportMeteoData.workflow(cfg, clim_db)
        ImportPrecipitation.workflow(cfg, clim_db)

    @staticmethod
    def spatial_rasters(cfg, subbasin_num):
        """Import spatial raster data."""
        UtilClass.mkdir(cfg.dirs.import2db)
        if not cfg.cluster:  # changed by LJ, SubbasinID is 0 means the whole basin!
            subbasin_num = 0
            start_id = 0
            subbasin_file = cfg.spatials.mask
        else:
            start_id = 1
            subbasin_file = cfg.spatials.subbsn
        for i in range(start_id, subbasin_num + 1):
            subdir = cfg.dirs.import2db + os.sep + str(i)
            UtilClass.rmmkdir(subdir)
        str_cmd = '"%s/import_raster" %s %s %s %s %s %d %s' % (cfg.seims_bin, subbasin_file,
                                                               cfg.dirs.geodata2db,
                                                               cfg.spatial_db,
                                                               DBTableNames.gridfs_spatial,
                                                               cfg.hostname, cfg.port,
                                                               cfg.dirs.import2db)
        print (str_cmd)
        UtilClass.run_command(str_cmd)

    @staticmethod
    def lookup_tables(cfg, maindb):
        """Import lookup tables as GridFS
        Args:
            cfg: SEIMS config object
            maindb: main model database
        """
        sqlite_file = cfg.sqlitecfgs.sqlite_file
        # read sqlite database
        conn = sqlite3.connect(sqlite_file)
        c = conn.cursor()
        # get all the tablename
        c.execute("select name from sqlite_master where type='table' order by name;")
        tablelist = c.fetchall()
        # Find parameter table list excluding "XXLookup"
        tablelist = [item[0].encode("ascii") for item in tablelist if (
            item[0].lower().find("lookup") >= 0)]
        # print (tablelist)
        for tablename in tablelist:
            # print (tablename)
            str_sql = "select * from %s;" % (tablename,)
            cur = c.execute(str_sql)
            records = cur.fetchall()
            item_values = []
            for items in records:
                item_value = []
                for item in items:
                    if MathClass.isnumerical(item):
                        item_value.append(float(item))
                item_values.append(item_value)
            n_row = len(item_values)
            # print (item_values)
            if n_row >= 1:
                n_col = len(item_values[0])
                for i in range(n_row):
                    if n_col != len(item_values[i]):
                        raise ValueError("Please check %s to make sure each item has "
                                         "the same numeric dimension." % tablename)
                    else:
                        item_values[i].insert(0, n_col)
                # import to mongoDB as GridFS
                spatial = GridFS(maindb, DBTableNames.gridfs_spatial)
                # delete if the tablename file existed already.
                if spatial.exists(filename=tablename.upper()):
                    x = spatial.get_version(filename=tablename.upper())
                    spatial.delete(x._id)
                metadic = {ImportMongodbClass._LOOKUP_ITEM_COUNT: n_row,
                           ImportMongodbClass._LOOKUP_FIELD_COUNT: n_col}
                cur_lookup_gridfs = spatial.new_file(filename=tablename.upper(), metadata=metadic)
                header = [n_row]
                fmt = '%df' % 1
                s = pack(fmt, *header)
                cur_lookup_gridfs.write(s)
                fmt = '%df' % (n_col + 1)
                for i in range(n_row):
                    s = pack(fmt, *item_values[i])
                    cur_lookup_gridfs.write(s)
                cur_lookup_gridfs.close()
        c.close()
        conn.close()

    @staticmethod
    def iuh(cfg, n_subbasins):
        """Invoke IUH program"""
        if cfg.gen_iuh:
            dt = 24
            str_cmd = '"%s/iuh" %s %d %s %s %s %d' % (cfg.seims_bin, cfg.hostname, cfg.port,
                                                      cfg.spatial_db,
                                                      DBTableNames.gridfs_spatial,
                                                      dt, n_subbasins)
            print (str_cmd)
            UtilClass.run_command(str_cmd)

    @staticmethod
    def grid_layering(cfg, n_subbsn):
        """Invoke grid layering program."""
        layering_dir = cfg.dirs.layerinfo
        UtilClass.rmmkdir(layering_dir)
        str_cmd = '"%s/grid_layering" %s %d %s %s %s %d' % (
            cfg.seims_bin, cfg.hostname, cfg.port, layering_dir,
            cfg.spatial_db, DBTableNames.gridfs_spatial, n_subbsn)
        print (str_cmd)
        UtilClass.run_command(str_cmd)

    @staticmethod
    def workflow(cfg):
        """Building MongoDB workflow"""
        f = open(cfg.logs.build_mongo, 'w')
        # build a connection to mongodb database
        client = ConnectMongoDB(cfg.hostname, cfg.port)
        conn = client.get_conn()
        maindb = conn[cfg.spatial_db]
        climatedb = conn[cfg.climate_db]
        scenariodb = None
        if cfg.use_scernario:
            scenariodb = conn[cfg.bmp_scenario_db]

        # import model parameters information to MongoDB
        status_output('Import model parameters', 10, f)
        ImportParam2Mongo.workflow(cfg, maindb)
        n_subbasins = MongoQuery.get_subbasin_num(maindb)
        print ("Number of subbasins:%d" % n_subbasins)

        # import lookup tables to MongoDB as GridFS. By LJ, 2016-6-13
        status_output('Import lookup tables as GridFS...', 15, f)
        ImportMongodbClass.lookup_tables(cfg, maindb)

        # import stream parameters
        status_output("Generating reach table with initialized parameters...", 20, f)
        ImportReaches2Mongo.generate_reach_table(cfg, maindb)

        status_output("Import climate data....", 30, f)
        ImportMongodbClass.climate_data(cfg, maindb, climatedb)

        status_output("Import observed data, such as discharge, sediment yield....", 35, f)
        # Measurement Data, such as discharge, sediment yield.
        ImportObservedData.workflow(cfg, climatedb)

        # import raster data to MongoDB
        status_output("Importing raster to MongoDB....", 40, f)
        ImportMongodbClass.spatial_rasters(cfg, n_subbasins)

        # Import weight and related data
        status_output("Generating weight data for interpolation of meteorology data "
                      "and weight dependent parameters....", 50, f)
        ImportWeightData.workflow(cfg, conn)

        # Import IUH
        status_output("Generating and importing IUH (Instantaneous Unit Hydrograph)....", 60, f)
        ImportMongodbClass.iuh(cfg, n_subbasins)

        # Import grid layering data
        status_output("Generating and importing grid layering....", 70, f)
        ImportMongodbClass.grid_layering(cfg, n_subbasins)

        # Import BMP scenario database to MongoDB
        status_output("Importing bmp scenario....", 90, f)
        ImportScenario2Mongo.scenario_from_texts(cfg, maindb, scenariodb)

        status_output("Build DB: %s finished!" % cfg.spatial_db, 100, f)
        f.close()

        # close connection to MongoDB
        client.close()


def main():
    """TEST CODE"""
    seims_cfg = parse_ini_configuration()
    ImportMongodbClass.workflow(seims_cfg)


if __name__ == "__main__":
    main()
