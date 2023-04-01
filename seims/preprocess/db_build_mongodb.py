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

from pygeoc.utils import UtilClass, FileClass

from utility import status_output, DEFAULT_NODATA, mask_rasterio
from preprocess.config import SpatialNamesUtils, PreprocessConfig
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
    def climate_data(cfg):  # type: (PreprocessConfig) -> None
        """Climate data."""
        ImportHydroClimateSites.workflow(cfg)
        ImportMeteoData.workflow(cfg)
        ImportPrecipitation.workflow(cfg)

    @staticmethod
    def spatial_rasters(cfg):  # type: (PreprocessConfig) -> None
        """Mask and decompose spatial raster data to MongoDB
        """
        mask_raster_cfg = list()
        # format: <in>, <out>[, <defaultValue>, <updatedNodata>, <outDataType>]
        # from SpatialDelineation.original_delineation()
        mask_raster_cfg.append([cfg.spatials.subbsn, SpatialNamesUtils._SUBBASINOUT,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'INT32'])  # subbasin, aka mask

        mask_raster_cfg.append([cfg.taudems.stream_order, SpatialNamesUtils._STREAMORDERM,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'INT32'])  # stream order
        mask_raster_cfg.append([cfg.spatials.stream_link, SpatialNamesUtils._STREAMLINKOUT,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'INT32'])  # stream link

        mask_raster_cfg.append([cfg.spatials.filldem, SpatialNamesUtils._FILLEDDEMM,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # filled dem
        mask_raster_cfg.append([cfg.spatials.slope, SpatialNamesUtils._SLOPEM,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # slope
        mask_raster_cfg.append([cfg.spatials.d8flow, SpatialNamesUtils._FLOWDIROUT,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'INT32'])  # flow direction D8
        mask_raster_cfg.append([cfg.spatials.d8acc, SpatialNamesUtils._ACCM,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # d8-acc

        mask_raster_cfg.append([cfg.taudems.dinf_d8dir, SpatialNamesUtils._DIRCODEDINFM,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'INT32'])  # dinf compound dir
        mask_raster_cfg.append([cfg.taudems.dinf_weight, SpatialNamesUtils._WEIGHTDINFM,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # flow weight, dinf

        mask_raster_cfg.append([cfg.taudems.mfdmd_dir, SpatialNamesUtils._DIRCODEMFDMD,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'INT32'])  # mfdmd compound dir
        mask_raster_cfg.append([[FileClass.add_postfix(cfg.taudems.mfdmd_frac, '%d' % i)
                                for i in range(1, 9, 1)], SpatialNamesUtils._FLOWFRACTIONMFDMD,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # flow fraction, mfdmd

        mask_raster_cfg.append([cfg.taudems.dist2stream_d8, SpatialNamesUtils._DIST2STREAMD8M,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # dist down V, d8
        mask_raster_cfg.append([cfg.taudems.dist2stream_dinf, SpatialNamesUtils._DIST2STREAMDINFM,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # dist down V, dinf


        # from SpatialDelineation.calculate_terrain_related_params()
        mask_raster_cfg.append([cfg.spatials.cell_lat, SpatialNamesUtils._CELLLAT,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # latitude

        # from SpatialDelineation.delineate_spatial_units()
        mask_raster_cfg.append([cfg.spatials.hillslope, SpatialNamesUtils._HILLSLOPEOUT,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'INT32'])  # hillslope
        for mgt_fld in cfg.spatials.mgt_field:
            mask_raster_cfg.append([mgt_fld, FileClass.get_core_name_without_suffix(mgt_fld),
                                    DEFAULT_NODATA, DEFAULT_NODATA, 'INT32'])  # mgt fields

        # from TerrainUtilClass.parameters_extraction()
        mask_raster_cfg.append([cfg.spatials.chwidth, SpatialNamesUtils._CHWIDTH,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # channel width
        mask_raster_cfg.append([cfg.spatials.chdepth, SpatialNamesUtils._CHDEPTH,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # channel depth
        mask_raster_cfg.append([cfg.spatials.init_somo, SpatialNamesUtils._INITSOILMOIST,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # initial soil mstr
        mask_raster_cfg.append([cfg.spatials.depression, SpatialNamesUtils._DEPRESSIONFILE,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # depression
        mask_raster_cfg.append([cfg.spatials.radius, SpatialNamesUtils._RADIUSFILE,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # radius
        mask_raster_cfg.append([cfg.spatials.velocity, SpatialNamesUtils._VELOCITYFILE,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # velocity
        mask_raster_cfg.append([cfg.spatials.t0_s, SpatialNamesUtils._T0_SFILE,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # flow time to river
        mask_raster_cfg.append([cfg.spatials.delta_s, SpatialNamesUtils._DELTA_SFILE,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # std of flow time
        mask_raster_cfg.append([cfg.spatials.dayl_min, SpatialNamesUtils._DAYLMIN,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # minimum daylength
        mask_raster_cfg.append([cfg.spatials.dorm_hr, SpatialNamesUtils._DORMHR,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # dormancy threshold

        # from SoilUtilClass.
        mask_raster_cfg.append([cfg.spatials.soil_type, SpatialNamesUtils._SOILTYPEMFILE,
                                cfg.default_landuse, DEFAULT_NODATA, 'INT32'])  # soil type
        # from LanduseUtilClass.parameters_extraction()
        mask_raster_cfg.append([cfg.spatials.landuse, SpatialNamesUtils._LANDUSEMFILE,
                                cfg.default_landuse, DEFAULT_NODATA, 'INT32'])  # landuse type
        mask_raster_cfg.append([cfg.spatials.cn2, SpatialNamesUtils._CN2FILE,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # minimum daylength
        mask_raster_cfg.append([cfg.spatials.runoff_coef, SpatialNamesUtils._RUNOFF_COEFFILE,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # dormancy threshold

        # Additional raster file
        for k, v in cfg.additional_rs.items():
            org_v = v
            if not FileClass.is_file_exists(org_v):
                v = cfg.spatial_dir + os.path.sep + org_v
                if not FileClass.is_file_exists(v):
                    print('WARNING: The additional file %s MUST be located in '
                          'SPATIAL_DATA_DIR, or provided as full file path!' % k)
                    continue
            mask_raster_cfg.append([v, k.upper(), DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])

        mongoargs = [cfg.hostname, cfg.port, cfg.spatial_db, 'SPATIAL']
        mask_rasterio(cfg.seims_bin, mask_raster_cfg, mongoargs=mongoargs,
                      maskfile=cfg.spatials.subbsn,
                      include_nodata=False, mode='MASKDEC')

        # We also need to save fullsize raster of subbasin to be used as MASK!
        mask_rasterio(cfg.seims_bin,
                      [[cfg.spatials.subbsn, SpatialNamesUtils._SUBBASINOUT,
                       DEFAULT_NODATA, DEFAULT_NODATA, 'INT32']],
                      mongoargs=mongoargs, include_nodata=True, mode='MASKDEC')

    @staticmethod
    def iuh(cfg, n_subbasins):  # type: (PreprocessConfig, int) -> None
        """Invoke IUH program"""
        dt = 24
        str_cmd = '"%s/iuh" %s %d %s %s %s %d' % (cfg.seims_bin, cfg.hostname, cfg.port,
                                                  cfg.spatial_db,
                                                  DBTableNames.gridfs_spatial,
                                                  dt, n_subbasins)
        UtilClass.run_command(str_cmd)

    @staticmethod
    def grid_layering(cfg, n_subbasins):  # type: (PreprocessConfig, int) -> None
        """Invoke grid layering program."""
        layering_dir = cfg.dirs.layerinfo
        UtilClass.mkdir(layering_dir)
        for alg in ['d8', 'dinf', 'mfdmd']:
            str_cmd = '"%s/grid_layering" -alg %s -stream %s -outdir %s -mongo %s %d %s %s %d' %\
                      (cfg.seims_bin, alg, cfg.vecs.reach, layering_dir,
                       cfg.hostname, cfg.port,
                       cfg.spatial_db, DBTableNames.gridfs_spatial, n_subbasins)
            UtilClass.run_command(str_cmd)

    @staticmethod
    def workflow(cfg):  # type: (PreprocessConfig) -> None
        """Building MongoDB workflow"""
        f = cfg.logs.build_mongo

        status_output('Import model parameters to MongoDB', 10, f)
        ImportParam2Mongo.workflow(cfg)
        n_subbasins = MongoQuery.get_init_parameter_value(cfg.maindb, SubbsnStatsName.subbsn_num)
        print('Number of subbasins: %d' % n_subbasins)

        status_output('Extract spatial parameters for reaches, landuse, soil, etc...', 20, f)
        extract_spatial_parameters(cfg)

        status_output('Generating reach table with initialized parameters...', 40, f)
        ImportReaches2Mongo.generate_reach_table(cfg)

        status_output('Importing necessary raster to MongoDB....', 50, f)
        ImportMongodbClass.spatial_rasters(cfg)

        status_output('Generating and importing IUH (Instantaneous Unit Hydrograph)....', 60, f)
        ImportMongodbClass.iuh(cfg, 0)
        ImportMongodbClass.iuh(cfg, n_subbasins)

        # Import grid layering data
        status_output('Generating and importing grid layering....', 70, f)
        ImportMongodbClass.grid_layering(cfg, 0)
        ImportMongodbClass.grid_layering(cfg, n_subbasins)

        # Import hydro-climate data
        status_output('Import climate data....', 80, f)
        ImportMongodbClass.climate_data(cfg)

        # Import weight and related data, this should after ImportMongodbClass.climate_data()
        status_output('Generating weight data for interpolation of meteorology data '
                      'and weight dependent parameters....', 85, f)
        ImportWeightData.workflow(cfg, 0)
        ImportWeightData.workflow(cfg, n_subbasins)

        # Measurement Data, such as discharge, sediment yield.
        status_output('Import observed data, such as discharge, sediment yield....', 90, f)
        ImportObservedData.workflow(cfg)

        # Import BMP scenario database to MongoDB
        status_output('Importing bmp scenario....', 95, f)
        ImportScenario2Mongo.scenario_from_texts(cfg)

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
