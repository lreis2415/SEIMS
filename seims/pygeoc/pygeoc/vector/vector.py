#! /usr/bin/env python
# coding=utf-8

import os
import sys

from osgeo import ogr


# Export ESRI Shapefile -- Line feature
def WriteLineShp(lineList, outShp):
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
