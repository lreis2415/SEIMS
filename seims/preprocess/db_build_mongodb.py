"""Import all model parameters and spatial datasets to MongoDB

    @author   : Liangjun Zhu, Junzhi Liu

    @changelog:
    - 16-12-07  - lj - rewrite for version 2.0
    - 17-06-26  - lj - reformat according to pylint and google style
    - 17-07-07  - lj - remove sqlite3 database file as intermediate data
    - 18-02-08  - lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from pygeoc.utils import UtilClass

from utility import status_output
from preprocess.db_import_bmpscenario import ImportScenario2Mongo
from preprocess.db_import_interpolation_weights import ImportWeightData
from preprocess.db_import_meteorology import ImportMeteoData
from preprocess.db_import_model_parameters import ImportParam2Mongo
from preprocess.db_import_observed import ImportObservedData
from preprocess.db_import_precipitation import ImportPrecipitation
from preprocess.db_import_sites import ImportHydroClimateSites
from preprocess.db_import_stream_parameters import ImportReaches2Mongo
from preprocess.db_mongodb import ConnectMongoDB, MongoQuery
from preprocess.sp_extraction import extract_spatial_parameters
from preprocess.text import DBTableNames, SubbsnStatsName


class ImportMongodbClass(object):
    """Separated function to import data into MongoDB."""

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
        if subbasin_num == 0:  # the whole basin!
            subbasin_file = cfg.spatials.mask
        else:
            subbasin_file = cfg.spatials.subbsn
        str_cmd = '"%s/import_raster" %s %s %s %s %s %d' % (cfg.seims_bin, subbasin_file,
                                                            cfg.dirs.geodata2db,
                                                            cfg.spatial_db,
                                                            DBTableNames.gridfs_spatial,
                                                            cfg.hostname, cfg.port)
        UtilClass.run_command(str_cmd)

    @staticmethod
    def iuh(cfg, n_subbasins):
        """Invoke IUH program"""
        dt = 24
        str_cmd = '"%s/iuh" %s %d %s %s %s %d' % (cfg.seims_bin, cfg.hostname, cfg.port,
                                                  cfg.spatial_db,
                                                  DBTableNames.gridfs_spatial,
                                                  dt, n_subbasins)
        UtilClass.run_command(str_cmd)

    @staticmethod
    def grid_layering(cfg, n_subbasins):
        """Invoke grid layering program."""
        layering_dir = cfg.dirs.layerinfo
        UtilClass.rmmkdir(layering_dir)
        str_cmd = '"%s/grid_layering" %s %d %s %s %s %d' % (
            cfg.seims_bin, cfg.hostname, cfg.port, layering_dir,
            cfg.spatial_db, DBTableNames.gridfs_spatial, n_subbasins)
        UtilClass.run_command(str_cmd)

    @staticmethod
    def workflow(cfg):
        """Building MongoDB workflow"""
        f = cfg.logs.build_mongo
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
        n_subbasins = MongoQuery.get_init_parameter_value(maindb, SubbsnStatsName.subbsn_num)
        print('Number of subbasins: %d' % n_subbasins)

        # Extract spatial parameters for reaches, landuse, soil, etc.
        status_output('Extract spatial parameters for reaches, landuse, soil, etc...', 20, f)
        extract_spatial_parameters(cfg, maindb)

        # import stream parameters
        status_output('Generating reach table with initialized parameters...', 40, f)
        ImportReaches2Mongo.generate_reach_table(cfg, maindb)

        # import raster data to MongoDB
        status_output('Importing raster to MongoDB....', 50, f)
        ImportMongodbClass.spatial_rasters(cfg, 0)
        ImportMongodbClass.spatial_rasters(cfg, n_subbasins)

        # Import IUH
        status_output('Generating and importing IUH (Instantaneous Unit Hydrograph)....', 60, f)
        ImportMongodbClass.iuh(cfg, 0)
        ImportMongodbClass.iuh(cfg, n_subbasins)

        # Import grid layering data
        status_output('Generating and importing grid layering....', 70, f)
        ImportMongodbClass.grid_layering(cfg, 0)
        ImportMongodbClass.grid_layering(cfg, n_subbasins)

        # Import hydro-climate data
        status_output('Import climate data....', 80, f)
        ImportMongodbClass.climate_data(cfg, maindb, climatedb)

        # Import weight and related data, this should after ImportMongodbClass.climate_data()
        status_output('Generating weight data for interpolation of meteorology data '
                      'and weight dependent parameters....', 85, f)
        ImportWeightData.workflow(cfg, conn, 0)
        ImportWeightData.workflow(cfg, conn, n_subbasins)

        # Measurement Data, such as discharge, sediment yield.
        status_output('Import observed data, such as discharge, sediment yield....', 90, f)
        ImportObservedData.workflow(cfg, maindb, climatedb)

        # Import BMP scenario database to MongoDB
        status_output('Importing bmp scenario....', 95, f)
        ImportScenario2Mongo.scenario_from_texts(cfg, maindb, scenariodb)

        status_output('Build DB: %s finished!' % cfg.spatial_db, 100, f)

        # close connection to MongoDB
        # client.close()  # No need to explicitly close MongoClient! By lj.


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration
    seims_cfg = parse_ini_configuration()

    ImportMongodbClass.workflow(seims_cfg)


if __name__ == "__main__":
    main()
