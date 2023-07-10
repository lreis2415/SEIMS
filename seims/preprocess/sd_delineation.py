"""Subbasin delineation based on TauDEM, as well as calculation of latitude dependent parameters
    @author   : Liangjun Zhu, Junzhi Liu
    @changelog: 13-01-10  jz - initial implementation
                16-12-07  lj - rewrite for version 2.0, improve calculation efficiency by numpy
                17-06-23  lj - reorganize as basic class
                18-02-08  lj - compatible with Python3.\n
"""
from __future__ import absolute_import, unicode_literals

import os
import sys
from io import open

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import osgeo
from numpy import where, fromfunction
from osgeo.gdal import GDT_Int32, GDT_Float32
from osgeo.ogr import CreateGeometryFromWkt as ogr_CreateGeometryFromWkt
from osgeo.osr import CoordinateTransformation as osr_CoordinateTransformation
from osgeo.osr import SpatialReference as osr_SpatialReference
from osgeo.osr import OAMS_TRADITIONAL_GIS_ORDER

from pygeoc.TauDEM import TauDEM, TauDEM_Ext, TauDEMWorkflow
from pygeoc.postTauDEM import D8Util, DinfUtil, StreamnetUtil
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import FileClass, UtilClass
from pygeoc.vector import VectorUtilClass

from utility import DEFAULT_NODATA, mask_rasterio
from preprocess.sd_connected_field import connected_field_partition_wu2018
from preprocess.sd_hillslope import DelineateHillslope
from preprocess.text import FieldNames
from preprocess.config import PreprocessConfig


class SpatialDelineation(object):
    """Subbasin delineation based on TauDEM,
    as well as calculation of latitude dependent parameters"""

    # Field in SiteList table or other tables, such as subbasin.shp
    # _FLD_SUBBASINID = 'SUBBASINID'
    # _FLD_BASINID = 'BASIN'

    @staticmethod
    def output_wgs84_geojson(cfg):  # type: (PreprocessConfig) -> None
        """Convert ESRI shapefile to GeoJson based on WGS84 coordinate."""
        src_srs = RasterUtilClass.read_raster(cfg.dem).srs
        proj_srs = src_srs.ExportToProj4()
        if not proj_srs:
            raise ValueError('The source raster %s has not '
                             'coordinate, which is required!' % cfg.dem)
        # print(proj_srs)
        wgs84_srs = 'EPSG:4326'
        geo_json_dict = {'reach': [cfg.vecs.reach, cfg.vecs.json_reach],
                         'subbasin': [cfg.vecs.subbsn, cfg.vecs.json_subbsn],
                         'basin': [cfg.vecs.bsn, cfg.vecs.json_bsn],
                         'outlet': [cfg.vecs.outlet, cfg.vecs.json_outlet]}
        for jsonName, shp_json_list in list(geo_json_dict.items()):
            # delete if geojson file already existed
            if FileClass.is_file_exists(shp_json_list[1]):
                os.remove(shp_json_list[1])
            VectorUtilClass.convert2geojson(shp_json_list[1], proj_srs, wgs84_srs,
                                            shp_json_list[0])

    @staticmethod
    def original_delineation(cfg):  # type: (PreprocessConfig) -> None
        """Original watershed delineation by TauDEM functions and
        other terrain attributes by TauDEM-ext functions"""
        # Check directories
        UtilClass.mkdir(cfg.workspace)
        UtilClass.mkdir(cfg.dirs.log)
        # Watershed delineation
        TauDEMWorkflow.watershed_delineation(cfg.np, cfg.dem,  # required arguments
                                             outlet_file=cfg.outlet_file,
                                             thresh=cfg.acc_thresh,
                                             singlebasin=True,
                                             workingdir=cfg.dirs.taudem,
                                             mpi_bin=cfg.mpi_bin,
                                             bin_dir=cfg.seims_bin,
                                             logfile=cfg.logs.delineation,
                                             avoid_redo=True)
        # Convert D8 encoding rule to ArcGIS
        D8Util.convert_code(cfg.taudems.d8flow, cfg.taudems.d8flow_m)
        # D-inf flow direction
        TauDEM.dinfflowdir(cfg.np, cfg.taudems.filldem, cfg.taudems.dinf, cfg.taudems.dinf_slp,
                           workingdir=cfg.dirs.taudem, mpiexedir=cfg.mpi_bin, exedir=cfg.seims_bin,
                           log_file=cfg.logs.delineation)

        # Convert Dinf to compressed flow direction according to ArcGIS encoding rule
        DinfUtil.output_compressed_dinf(cfg.taudems.dinf, cfg.taudems.dinf_d8dir,
                                        cfg.taudems.dinf_weight,
                                        minfraction=cfg.min_flowfrac,
                                        subbasin=cfg.taudems.subbsn_m,
                                        stream=cfg.taudems.stream_m,
                                        upddinffile=cfg.taudems.dinf_upd)
        # MFD-md flow directions
        TauDEM_Ext.mfdmdflowdir(cfg.np, cfg.taudems.filldem, cfg.taudems.mfdmd_dir,
                                cfg.taudems.mfdmd_frac,
                                min_portion=cfg.min_flowfrac,
                                p0=1.1, rng=8.9, lb=0., ub=1.,  # TODO, specified in ini file. lj
                                workingdir=cfg.dirs.taudem, log_file=cfg.logs.delineation,
                                mpiexedir=cfg.mpi_bin, exedir=cfg.seims_bin)
        # Distance to stream using Surface method based on D8 flow direction
        TauDEM_Ext.d8distdowntostream(cfg.np, cfg.taudems.d8flow, cfg.taudems.filldem,
                                      cfg.taudems.stream_raster, cfg.taudems.dist2stream_d8,
                                      cfg.distdown_method, 1,
                                      workingdir=cfg.dirs.taudem, log_file=cfg.logs.delineation,
                                      mpiexedir=cfg.mpi_bin, exedir=cfg.seims_bin)
        # Distance to stream using Surface method in Average length based on D-inf flow direction
        TauDEM.dinfdistdown(cfg.np, cfg.taudems.dinf, cfg.taudems.filldem, cfg.taudems.dinf_slp,
                            cfg.taudems.stream_raster, 'Average', cfg.distdown_method, False,
                            cfg.taudems.dinf, cfg.taudems.dist2stream_dinf,
                            workingdir=cfg.dirs.taudem, log_file=cfg.logs.delineation,
                            mpiexedir=cfg.mpi_bin, exedir=cfg.seims_bin)
        # Copy shapefiles
        UtilClass.mkdir(cfg.dirs.geoshp)
        FileClass.copy_files(cfg.taudems.outlet_m, cfg.vecs.outlet)
        FileClass.copy_files(cfg.taudems.subbsn_shp, cfg.vecs.subbsn)
        FileClass.copy_files(cfg.taudems.streamnet_m, cfg.vecs.reach)
        # Get mask raster and shapefile (i.e., basin.shp) from subbasin raster
        UtilClass.mkdir(cfg.dirs.geodata2db)
        RasterUtilClass.get_mask_from_raster(cfg.taudems.subbsn_m, cfg.spatials.mask)
        VectorUtilClass.raster2shp(cfg.spatials.mask, cfg.vecs.bsn, 'basin', FieldNames.basin)
        # Convert current coordinate to WGS84 and convert shapefile to GeoJson.
        # todo: convert to geojson may failed in Windows for some reason caused by compiled GDAL.
        #       since the geojson is not used for further purpose, comment it!
        #       This function needs to be updated according to current configuration!
        # SpatialDelineation.output_wgs84_geojson(cfg)

    @staticmethod
    def mask_spatial_data(cfg):  # type: (PreprocessConfig) -> None
        """Mask necessary delineated and input spatial raster by the entire basin.

            1. subbasin, used to decompose and combine spatial data
            2. stream_link and d8flow, used to delineate hillslope
            3. filleddem, used to derive other terrain parameters
            4. landuse and soil type, used to extract corresponding parameters
        """
        mask_raster_cfg = list()
        # format: <in>, <out>[, <defaultValue>, <updatedNodata>, <outDataType>]
        mask_raster_cfg.append([cfg.taudems.subbsn_m, cfg.spatials.subbsn,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'INT32'])  # subbasin
        mask_raster_cfg.append([cfg.taudems.stream_m, cfg.spatials.stream_link,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'INT32'])  # stream link
        mask_raster_cfg.append([cfg.taudems.slp, cfg.spatials.slope,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # slope
        mask_raster_cfg.append([cfg.taudems.d8flow_m, cfg.spatials.d8flow,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'INT32'])  # flow direction D8
        mask_raster_cfg.append([cfg.taudems.filldem, cfg.spatials.filldem,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # filled dem
        mask_raster_cfg.append([cfg.taudems.d8acc, cfg.spatials.d8acc, DEFAULT_NODATA,
                                DEFAULT_NODATA, 'DOUBLE'])  # d8-acc, for init soil mstr
        mask_raster_cfg.append([cfg.taudems.dist2stream_dinf, cfg.spatials.dist2stream_dinf,
                                DEFAULT_NODATA, DEFAULT_NODATA, 'DOUBLE'])  # dinf-dist down V

        FileClass.check_file_exists(cfg.soil)
        mask_raster_cfg.append([cfg.soil, cfg.spatials.soil_type,
                                cfg.default_soil, DEFAULT_NODATA, 'INT32'])  # soil type
        FileClass.check_file_exists(cfg.landuse)
        mask_raster_cfg.append([cfg.landuse, cfg.spatials.landuse,
                                cfg.default_landuse, DEFAULT_NODATA, 'INT32'])  # landuse type

        mask_rasterio(cfg.seims_bin, mask_raster_cfg, maskfile=cfg.spatials.mask,
                      cfgfile=cfg.logs.mask_cfg)

    @staticmethod
    def generate_lat_raster(cfg):  # type: (PreprocessConfig) -> None
        """Generate latitude raster"""
        dem_file = cfg.spatials.filldem
        ds = RasterUtilClass.read_raster(dem_file)
        src_srs = ds.srs
        if not src_srs.ExportToProj4():
            raise ValueError('The source raster %s has not coordinate, '
                             'which is required!' % dem_file)
        dst_srs = osr_SpatialReference()
        dst_srs.ImportFromEPSG(4326)  # WGS84
        if osgeo.__version__ >= '3.0.0':
            dst_srs.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER)

        # dst_wkt = dst_srs.ExportToWkt()
        transform = osr_CoordinateTransformation(src_srs, dst_srs)

        point_ll = ogr_CreateGeometryFromWkt('POINT (%f %f)' % (ds.xMin, ds.yMin))
        point_ur = ogr_CreateGeometryFromWkt('POINT (%f %f)' % (ds.xMax, ds.yMax))

        point_ll.Transform(transform)
        point_ur.Transform(transform)

        lower_lat = point_ll.GetY()
        up_lat = point_ur.GetY()

        rows = ds.nRows
        cols = ds.nCols
        delta_lat = (up_lat - lower_lat) / float(rows)

        def cal_cell_lat(row, col):
            """calculate latitude of cell by row number"""
            return up_lat - (row + 0.5) * delta_lat

        data_lat = fromfunction(cal_cell_lat, (rows, cols))
        data_lat = where(ds.validZone, data_lat, ds.data)
        RasterUtilClass.write_gtiff_file(cfg.spatials.cell_lat, rows, cols, data_lat,
                                         ds.geotrans, ds.srs,
                                         ds.noDataValue, GDT_Float32)

    @staticmethod
    def delineate_spatial_units(cfg):  # type: (PreprocessConfig) -> None
        # delineate hillslope
        DelineateHillslope.downstream_method_whitebox(cfg.spatials.stream_link,
                                                      cfg.spatials.d8flow,
                                                      cfg.spatials.hillslope,
                                                      d8alg='arcgis',
                                                      stream_value_method=0)
        # Field partition based on spatial topology
        connected_field_partition_wu2018(cfg)

    @staticmethod
    def calculate_terrain_related_params(cfg):  # type: (PreprocessConfig) -> None
        # Convert to WGS84 coordinate and output latitude raster.
        SpatialDelineation.generate_lat_raster(cfg)

    @staticmethod
    def workflow(cfg):  # type: (PreprocessConfig) -> None
        """Subbasin delineation workflow"""
        # Originally delineated by TauDEM and DTA algorithms based on TauDEM framework
        SpatialDelineation.original_delineation(cfg)
        # Mask necessary delineated and input spatial raster by the entire basin
        SpatialDelineation.mask_spatial_data(cfg)
        # Calculate terrain related parameters
        SpatialDelineation.calculate_terrain_related_params(cfg)
        # Delineate spatial units
        SpatialDelineation.delineate_spatial_units(cfg)


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration
    seims_cfg = parse_ini_configuration()
    SpatialDelineation.workflow(seims_cfg)


if __name__ == "__main__":
    main()
