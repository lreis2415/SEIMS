
# -*- coding: utf-8 -*-
"""Vector related Classes and Functions

    author: Liangjun Zhu

    changlog:

     - 12-04-12 jz - origin version
     - 16-07-01 lj - reorganized for pygeoc
     - 17-06-25 lj - check by pylint and reformat by Google style
"""
from __future__ import absolute_import, unicode_literals
import os
import sys

from osgeo.ogr import CreateGeometryFromJson as ogr_CreateGeometryFromJson
from osgeo.ogr import Feature as ogr_Feature
from osgeo.ogr import Geometry as ogr_Geometry
from osgeo.ogr import GetDriverByName as ogr_GetDriverByName
from osgeo.ogr import wkbLineString, OFTInteger
from osgeo.osr import SpatialReference as osr_SpatialReference
from osgeo.ogr import FieldDefn as ogr_FieldDefn
from osgeo import gdal

from pygeoc.utils import FileClass, UtilClass, sysstr


class VectorUtilClass(object):
    """Utility function to handle vector data."""

    def __init__(self):
        pass

    @staticmethod
    def raster2shp(rasterfile, vectorshp, layername=None, fieldname=None,
                   band_num=1, mask='default'):
        """Convert raster to ESRI shapefile"""
        FileClass.remove_files(vectorshp)
        FileClass.check_file_exists(rasterfile)
        # this allows GDAL to throw Python Exceptions
        gdal.UseExceptions()
        src_ds = gdal.Open(rasterfile)
        if src_ds is None:
            print('Unable to open %s' % rasterfile)
            sys.exit(1)
        try:
            srcband = src_ds.GetRasterBand(band_num)
        except RuntimeError as e:
            # for example, try GetRasterBand(10)
            print('Band ( %i ) not found, %s' % (band_num, e))
            sys.exit(1)
        if mask == 'default':
            maskband = srcband.GetMaskBand()
        elif mask is None or mask.upper() == 'NONE':
            maskband = None
        else:
            mask_ds = gdal.Open(mask)
            maskband = mask_ds.GetRasterBand(1)
        #  create output datasource
        if layername is None:
            layername = FileClass.get_core_name_without_suffix(rasterfile)
        drv = ogr_GetDriverByName(str('ESRI Shapefile'))
        dst_ds = drv.CreateDataSource(vectorshp)
        srs = None
        if src_ds.GetProjection() != '':
            srs = osr_SpatialReference()
            srs.ImportFromWkt(src_ds.GetProjection())
        dst_layer = dst_ds.CreateLayer(str(layername), srs=srs)
        if fieldname is None:
            fieldname = layername.upper()
        fd = ogr_FieldDefn(str(fieldname), OFTInteger)
        dst_layer.CreateField(fd)
        dst_field = 0
        result = gdal.Polygonize(srcband, maskband, dst_layer, dst_field,
                                 ['8CONNECTED=8'], callback=None)
        return result

    @staticmethod
    def convert2geojson(jsonfile, src_srs, dst_srs, src_file):
        """convert shapefile to geojson file"""
        if os.path.exists(jsonfile):
            os.remove(jsonfile)
        if sysstr == 'Windows':
            exepath = '"%s/Lib/site-packages/osgeo/ogr2ogr"' % sys.exec_prefix
        else:
            exepath = FileClass.get_executable_fullpath('ogr2ogr')
        # os.system(s)
        s = '%s -f GeoJSON -s_srs "%s" -t_srs %s %s %s' % (
            exepath, src_srs, dst_srs, jsonfile, src_file)
        UtilClass.run_command(s)

    @staticmethod
    def write_line_shp(line_list, out_shp):
        """Export ESRI Shapefile -- Line feature"""
        print('Write line shapefile: %s' % out_shp)
        driver = ogr_GetDriverByName(str('ESRI Shapefile'))
        if driver is None:
            print('ESRI Shapefile driver not available.')
            sys.exit(1)
        if os.path.exists(out_shp):
            driver.DeleteDataSource(out_shp)
        ds = driver.CreateDataSource(out_shp.rpartition(os.sep)[0])
        if ds is None:
            print('ERROR Output: Creation of output file failed.')
            sys.exit(1)
        lyr = ds.CreateLayer(str(out_shp.rpartition(os.sep)[2].split('.')[0]), None, wkbLineString)
        for l in line_list:
            line = ogr_Geometry(wkbLineString)
            for i in l:
                line.AddPoint(i[0], i[1])
            templine = ogr_CreateGeometryFromJson(line.ExportToJson())
            feature = ogr_Feature(lyr.GetLayerDefn())
            feature.SetGeometry(templine)
            lyr.CreateFeature(feature)
            feature.Destroy()
        ds.Destroy()


if __name__ == '__main__':
    rst = r'D:\data_m\SEIMS2018\demo_wap_90m\spatial_raster\mask.tif'
    shp = r'D:\data_m\SEIMS2018\demo_wap_90m\spatial_shp\basin.shp'
    VectorUtilClass.raster2shp(rst, shp, 'basin', 'BASIN')
