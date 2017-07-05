#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Vector related Classes and Functions

    author: Liangjun Zhu
    changlog: 12-04-12 jz - origin version
              16-07-01 lj - reorganized for pygeoc
              17-06-25 lj - check by pylint and reformat by Google style
"""

import os
import sys

from osgeo import ogr

from ..utils.utils import FileClass, UtilClass, sysstr


class VectorUtilClass(object):
    """Utility function to handle vector data."""

    def __init__(self):
        pass

    @staticmethod
    def raster2shp(rasterfile, vectorshp, layername, fieldname):
        """Convert raster to ESRI shapefile"""
        FileClass.remove_files(vectorshp)
        FileClass.check_file_exists(rasterfile)
        # raster to polygon vector
        if sysstr == 'Windows':
            exepath = '"%s/Scripts/gdal_polygonize.py"' % sys.exec_prefix
        else:
            exepath = FileClass.get_executable_fullpath("gdal_polygonize.py")
        str_cmd = '%s -f "ESRI Shapefile" %s %s %s %s' % (
            exepath, rasterfile, vectorshp, layername, fieldname)
        print (str_cmd)
        print (UtilClass.run_command(str_cmd))

    @staticmethod
    def convert2geojson(jsonfile, src_srs, dst_srs, src_file):
        """convert shapefile to geojson file"""
        if os.path.exists(jsonfile):
            os.remove(jsonfile)
        if sysstr == 'Windows':
            exepath = '"%s/Lib/site-packages/osgeo/ogr2ogr"' % sys.exec_prefix
        else:
            exepath = FileClass.get_executable_fullpath("ogr2ogr")
        # os.system(s)
        s = '%s -f GeoJSON -s_srs "%s" -t_srs %s %s %s' % (
            exepath, src_srs, dst_srs, jsonfile, src_file)
        UtilClass.run_command(s)

    @staticmethod
    def write_line_shp(line_list, out_shp):
        """Export ESRI Shapefile -- Line feature"""
        print ("Write line shapefile: %s" % out_shp)
        driver = ogr.GetDriverByName("ESRI Shapefile")
        if driver is None:
            print ("ESRI Shapefile driver not available.")
            sys.exit(1)
        if os.path.exists(out_shp):
            driver.DeleteDataSource(out_shp)
        ds = driver.CreateDataSource(out_shp.rpartition(os.sep)[0])
        if ds is None:
            print ("ERROR Output: Creation of output file failed.")
            sys.exit(1)
        lyr = ds.CreateLayer(out_shp.rpartition(os.sep)[2].split('.')[0], None, ogr.wkbLineString)
        #    for field in fields:
        #        fieldDefn = ogr.FieldDefn(field,ogr.OFTString)
        #        fieldDefn.SetWidth(255)
        #        lyr.CreateField(fieldDefn)
        for l in line_list:
            #        defn = lyr.GetLayerDefn()
            #        featureFields = ogr.Feature(defn)
            #        for field in fields:
            #            featureFields.SetField(field,"test")
            line = ogr.Geometry(ogr.wkbLineString)
            for i in l:
                line.AddPoint(i[0], i[1])
            templine = ogr.CreateGeometryFromJson(line.ExportToJson())
            feature = ogr.Feature(lyr.GetLayerDefn())
            feature.SetGeometry(templine)
            lyr.CreateFeature(feature)
            feature.Destroy()
        ds.Destroy()
