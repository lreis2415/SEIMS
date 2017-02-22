#! /usr/bin/env python
# coding=utf-8

import os
import sys

from osgeo import ogr

from ..utils.utils import FileClass, UtilClass
from ..utils.const import *

class VectorUtilClass(object):
    """Utility function to handle vector data."""
    def __init__(self):
        pass

    @staticmethod
    def raster2shp(rasterfile, vectorshp, layername, fieldname):
        """Convert raster to ESRI shapefile"""
        FileClass.removefiles(vectorshp)
        FileClass.checkfileexists(rasterfile)
        # raster to polygon vector
        if sysstr == 'Windows':
            exepath = '"%s/Scripts/gdal_polygonize.py"' % sys.exec_prefix
        else:
            exepath = FileClass.getexecutablefullpath("gdal_polygonize.py")
        strCmd = '%s -f "ESRI Shapefile" %s %s %s %s' % (exepath, rasterfile, vectorshp, layername, fieldname)
        print strCmd
        print UtilClass.runcommand(strCmd)

    @staticmethod
    def convert2geojson(jsonfile, src_srs, dst_srs, src_file):
        if os.path.exists(jsonfile):
            os.remove(jsonfile)
        if platform.system() == 'Windows':
            exepath = '"%s/Lib/site-packages/osgeo/ogr2ogr"' % sys.exec_prefix
        else:
            exepath = FileClass.getexecutablefullpath("ogr2ogr")
        # os.system(s)
        s = '%s -f GeoJSON -s_srs "%s" -t_srs %s %s %s' % (exepath, src_srs, dst_srs, jsonfile, src_file)
        UtilClass.runcommand(s)

    @staticmethod
    def WriteLineShp(lineList, outShp):
        """Export ESRI Shapefile -- Line feature"""
        print "Write line shapefile: %s" % outShp
        driver = ogr.GetDriverByName("ESRI Shapefile")
        if driver is None:
            print "ESRI Shapefile driver not available."
            sys.exit(1)
        if os.path.exists(outShp):
            driver.DeleteDataSource(outShp)
        ds = driver.CreateDataSource(outShp.rpartition(os.sep)[0])
        if ds is None:
            print "ERROR Output: Creation of output file failed."
            sys.exit(1)
        lyr = ds.CreateLayer(outShp.rpartition(os.sep)[2].split('.')[
                             0], None, ogr.wkbLineString)
    #    for field in fields:
    #        fieldDefn = ogr.FieldDefn(field,ogr.OFTString)
    #        fieldDefn.SetWidth(255)
    #        lyr.CreateField(fieldDefn)
        for l in lineList:
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
