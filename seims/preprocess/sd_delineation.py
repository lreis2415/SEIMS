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

from numpy import where, fromfunction
from osgeo.gdal import GDT_Int32, GDT_Float32
from osgeo.ogr import CreateGeometryFromWkt as ogr_CreateGeometryFromWkt
from osgeo.osr import CoordinateTransformation as osr_CoordinateTransformation
from osgeo.osr import SpatialReference as osr_SpatialReference
from pygeoc.TauDEM import TauDEMFilesUtils, TauDEM, TauDEMWorkflow
from pygeoc.postTauDEM import D8Util, DinfUtil, StreamnetUtil
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import FileClass, UtilClass
from pygeoc.vector import VectorUtilClass

from utility import DEFAULT_NODATA
from preprocess.sd_connected_field import connected_field_partition_wu2018
from preprocess.sd_hillslope import DelineateHillslope
from preprocess.text import FieldNames


class SpatialDelineation(object):
    """Subbasin delineation based on TauDEM,
    as well as calculation of latitude dependent parameters"""

    # Field in SiteList table or other tables, such as subbasin.shp
    # _FLD_SUBBASINID = 'SUBBASINID'
    # _FLD_BASINID = 'BASIN'

    @staticmethod
    def output_wgs84_geojson(cfg):
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
    def original_delineation(cfg):
        """Original watershed delineation by TauDEM functions and
        other terrain attributes by TauDEM-ext functions"""
        # Check directories
        UtilClass.mkdir(cfg.workspace)
        UtilClass.mkdir(cfg.dirs.log)
        bin_dir = cfg.seims_bin
        mpi_bin = cfg.mpi_bin
        np = cfg.np
        # Watershed delineation
        TauDEMWorkflow.watershed_delineation(np, cfg.dem,  # required arguments
                                             outlet_file=cfg.outlet_file,
                                             thresh=cfg.acc_thresh,
                                             singlebasin=True,
                                             workingdir=cfg.dirs.taudem,
                                             mpi_bin=mpi_bin,
                                             bin_dir=bin_dir,
                                             logfile=cfg.logs.delineation,
                                             avoid_redo=True)
        # D-inf flow direction
        TauDEM.dinfflowdir(np, cfg.taudems.filldem, cfg.taudems.dinf, cfg.taudems.dinf_slp,
                           workingdir=cfg.dirs.taudem, mpiexedir=mpi_bin, exedir=bin_dir,
                           log_file=cfg.logs.delineation)
        # MFD-md flow directions
        # TODO, arguments for calculating MFD-md might be specified in configuration file. lj
        TauDEM.mfdmdflowdir(np, cfg.taudems.filldem, cfg.taudems.mfdmd_dir, cfg.taudems.mfdmd_frac,
                            min_portion=0.01, p0=1.1, rng=8.9, lb=0., ub=1.,
                            workingdir=cfg.dirs.taudem, mpiexedir=mpi_bin, exedir=bin_dir,
                            log_file=cfg.logs.delineation)
        # Distance to stream using Surface method based on D8 flow direction
        TauDEM.d8distdowntostream(np, cfg.taudems.d8flow, cfg.taudems.filldem,
                                  cfg.taudems.stream_raster, cfg.taudems.dist2stream_d8,
                                  cfg.distdown_method, 1,
                                  workingdir=cfg.dirs.taudem, mpiexedir=mpi_bin, exedir=bin_dir,
                                  log_file=cfg.logs.delineation)
        # Distance to stream using Surface method in Average length based on D-inf flow direction
        TauDEM.dinfdistdown(np, cfg.taudems.dinf, cfg.taudems.filldem, cfg.taudems.dinf_slp,
                            cfg.taudems.stream_raster, 'Average', cfg.distdown_method, False,
                            cfg.taudems.dinf, cfg.taudems.dist2stream_dinf,
                            workingdir=cfg.dirs.taudem, mpiexedir=mpi_bin, exedir=bin_dir,
                            log_file=cfg.logs.delineation)

    @staticmethod
    def mask_raster_cpp(bin_dir, maskfile, originalfiles, outputfiles, default_values, configfile):
        """Call mask_raster program (cpp version) to mask raster"""
        # write mask configuration file
        n = len(originalfiles)
        # write mask config file
        with open(configfile, 'w', encoding='utf-8') as f:
            f.write('%s\n' % maskfile)
            f.write('%d\n' % (n,))
            for i in range(n):
                s = '%s\t%d\t%s\n' % (originalfiles[i], default_values[i], outputfiles[i])
                f.write('%s' % s)
        # run command
        UtilClass.run_command('"%s/mask_raster" %s' % (bin_dir, configfile))

    @staticmethod
    def mask_origin_delineated_data(cfg):
        """Mask the original delineated data by Subbasin raster."""
        subbasin_tau_file = cfg.taudems.subbsn
        geodata2dbdir = cfg.dirs.geodata2db
        UtilClass.mkdir(geodata2dbdir)
        mask_file = cfg.spatials.mask
        RasterUtilClass.get_mask_from_raster(subbasin_tau_file, mask_file)
        # Total 21 raster files
        original_files = [cfg.taudems.subbsn, cfg.taudems.d8flow, cfg.taudems.stream_raster,
                          cfg.taudems.slp, cfg.taudems.filldem, cfg.taudems.d8acc,
                          cfg.taudems.stream_order, cfg.taudems.dinf, cfg.taudems.dinf_d8dir,
                          cfg.taudems.dinf_slp, cfg.taudems.dinf_weight,
                          cfg.taudems.mfdmd_dir,
                          cfg.taudems.dist2stream_d8, cfg.taudems.dist2stream_dinf]
        original_files += [FileClass.add_postfix(cfg.taudems.mfdmd_frac, '%d' % i)
                           for i in range(1, 9, 1)]
        # output masked files
        output_files = [cfg.taudems.subbsn_m, cfg.taudems.d8flow_m, cfg.taudems.stream_m,
                        cfg.spatials.slope, cfg.spatials.filldem, cfg.spatials.d8acc,
                        cfg.spatials.stream_order, cfg.spatials.dinf, cfg.spatials.dinf_d8dir,
                        cfg.spatials.dinf_slp, cfg.spatials.dinf_weight, cfg.spatials.mfdmd_d8dir,
                        cfg.spatials.dist2stream_d8, cfg.spatials.dist2stream_dinf]
        output_files += [FileClass.add_postfix(cfg.spatials.mfdmd_fraction, '%d' % i)
                         for i in range(1, 9, 1)]

        default_values = [DEFAULT_NODATA] * len(original_files)

        # other input rasters need to be masked
        # soil and landuse
        FileClass.check_file_exists(cfg.soil)
        FileClass.check_file_exists(cfg.landuse)
        original_files.append(cfg.soil)
        output_files.append(cfg.spatials.soil_type)
        default_values.append(cfg.default_soil)
        original_files.append(cfg.landuse)
        output_files.append(cfg.spatials.landuse)
        default_values.append(cfg.default_landuse)

        # Additional raster file
        for k, v in cfg.additional_rs.items():
            org_v = v
            if not FileClass.is_file_exists(org_v):
                v = cfg.spatial_dir + os.path.sep + org_v
                if not FileClass.is_file_exists(v):
                    print('WARNING: The additional file %s MUST be located in '
                          'SPATIAL_DATA_DIR, or provided as full file path!' % k)
                    continue
            original_files.append(v)
            output_files.append(cfg.dirs.geodata2db + os.path.sep + k + '.tif')
            default_values.append(DEFAULT_NODATA)

        config_file = cfg.logs.mask_cfg
        # run mask operation
        print('Mask original delineated data by Subbasin raster...')
        SpatialDelineation.mask_raster_cpp(cfg.seims_bin, mask_file, original_files,
                                           output_files, default_values, config_file)

    @staticmethod
    def post_process_of_delineated_data(cfg):
        """Necessary data processing of subbasin, stream, and flow direction rasters."""
        # Outlet shapefile
        UtilClass.mkdir(cfg.dirs.geoshp)
        FileClass.copy_files(cfg.taudems.outlet_m, cfg.vecs.outlet)
        # Recoding stream IDs and the corresponding rasters of subbasin and stream
        id_map = StreamnetUtil.serialize_streamnet(cfg.taudems.streamnet_shp, cfg.vecs.reach)
        RasterUtilClass.raster_reclassify(cfg.taudems.subbsn_m, id_map,
                                          cfg.spatials.subbsn, GDT_Int32)
        StreamnetUtil.assign_stream_id_raster(cfg.taudems.stream_m, cfg.spatials.subbsn,
                                              cfg.spatials.stream_link)
        # Convert D8 encoding rule to ArcGIS
        D8Util.convert_code(cfg.taudems.d8flow_m, cfg.spatials.d8flow)
        # Convert Dinf to compressed flow direction according to ArcGIS encoding rule
        DinfUtil.output_compressed_dinf(cfg.spatials.dinf, cfg.spatials.dinf_d8dir,
                                        cfg.spatials.dinf_weight,
                                        minfraction=cfg.min_flowfrac,
                                        upddinffile=cfg.spatials.dinf_upd)


        # convert raster to shapefile (for subbasin and basin)
        print('Generating subbasin vector...')
        VectorUtilClass.raster2shp(cfg.spatials.subbsn, cfg.vecs.subbsn, 'subbasin',
                                   FieldNames.subbasin_id)
        mask_file = cfg.spatials.mask
        basin_vector = cfg.vecs.bsn
        print('Generating basin vector...')
        VectorUtilClass.raster2shp(mask_file, basin_vector, 'basin', FieldNames.basin)
        # delineate hillslope
        DelineateHillslope.downstream_method_whitebox(cfg.spatials.stream_link,
                                                      cfg.taudems.d8flow_m,
                                                      cfg.spatials.hillslope)

    @staticmethod
    def generate_lat_raster(cfg):
        """Generate latitude raster"""
        dem_file = cfg.spatials.filldem
        ds = RasterUtilClass.read_raster(dem_file)
        src_srs = ds.srs
        if not src_srs.ExportToProj4():
            raise ValueError('The source raster %s has not coordinate, '
                             'which is required!' % dem_file)
        dst_srs = osr_SpatialReference()
        dst_srs.ImportFromEPSG(4326)  # WGS84
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
    def workflow(cfg):
        """Subbasin delineation workflow"""
        # 1. Originally delineated by TauDEM and DTA algorithms based on TauDEM framework
        SpatialDelineation.original_delineation(cfg)
        # 2. Mask delineated raster by subbasin
        SpatialDelineation.mask_origin_delineated_data(cfg)
        # 3. Post processing, such as serialize stream ID, flow direction recoding etc.
        SpatialDelineation.post_process_of_delineated_data(cfg)
        # 4. Convert current coordinate to WGS84 and convert shapefile to GeoJson.
        # todo: convert to geojson may failed in Windows for some reason caused by compiled GDAL.
        #       since the geojson is not used for further purpose, comment it!
        # SpatialDelineation.output_wgs84_geojson(cfg)
        # 5. Convert to WGS84 coordinate and output latitude raster.
        SpatialDelineation.generate_lat_raster(cfg)
        # 6. Field partition based on spatial topology
        connected_field_partition_wu2018(cfg)


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration
    seims_cfg = parse_ini_configuration()
    SpatialDelineation.workflow(seims_cfg)


if __name__ == "__main__":
    main()
