#! /usr/bin/env python
# coding=utf-8
# @Post process of TauDEM
# @Author: Junzhi Liu, 2012-4-12
# @Revised: Liang-Jun Zhu, 2016-7-7
# @Revised: Liang-Jun Zhu, 2017-2-17
#

import os
import sys
import numpy
from gdal import GDT_Int16, GDT_Float32
from osgeo import ogr

from ..utils.utils import MathClass, FileClass, UtilClass
from ..utils.const import *
from ..raster.raster import RasterUtilClass
from text import *


class D8Util(object):
    """Utility functions based on D8 flow direction of TauDEM"""

    def __init__(self):
        pass

    @staticmethod
    def converttoarcgiscode(d8tau, d8esri):
        """Convert D8 flow direction code to ArcGIS rule"""
        dirconvertmap = {1.: 1.,
                         2.: 128.,
                         3.: 64.,
                         4.: 32.,
                         5.: 16.,
                         6.: 8.,
                         7.: 4.,
                         8.: 2.}
        RasterUtilClass.RasterReclassify(d8tau, dirconvertmap, d8esri)

    @staticmethod
    def downstream_index(dir_value, i, j):
        """find downslope coordinate for D8 of TauDEM."""
        drow, dcol = D8DIR_TD_DELTA[dir_value]
        return i + drow, j + dcol


class DinfUtil(object):
    """Utility functions based on Dinf flow direction"""

    def __init__(self):
        pass

    @staticmethod
    def checkorthogonal(angle):
        """Check the given Dinf angle based on D8 flow direction encoding code by ArcGIS"""
        if MathClass.floatequal(angle, e):
            return 1  # 1
        elif MathClass.floatequal(angle, ne):
            return 128  # 2
        elif MathClass.floatequal(angle, n):
            return 64  # 3
        elif MathClass.floatequal(angle, nw):
            return 32  # 4
        elif MathClass.floatequal(angle, w):
            return 16  # 5
        elif MathClass.floatequal(angle, sw):
            return 8  # 6
        elif MathClass.floatequal(angle, s):
            return 4  # 7
        elif MathClass.floatequal(angle, se):
            return 2  # 8

    @staticmethod
    def compressdinf(angle, nodata):
        """
        :param angle: D-inf flow direction angle
        :param nodata: NoData value
        :return: Compressed flow direction and weight of the first direction
        """
        if MathClass.floatequal(angle, nodata):
            return DEFAULT_NODATA, DEFAULT_NODATA
        d = DinfUtil.checkorthogonal(angle)
        if d is not None:
            return d, 1
        if angle < ne:
            a1 = angle
            d = 129  # 1+128
        elif angle < n:
            a1 = angle - ne
            d = 192  # 128+64
        elif angle < nw:
            a1 = angle - n
            d = 96  # 64+32
        elif angle < w:
            a1 = angle - nw
            d = 48  # 32+16
        elif angle < sw:
            a1 = angle - w
            d = 24  # 16+8
        elif angle < s:
            a1 = angle - sw
            d = 12  # 8+4
        elif angle < se:
            a1 = angle - s
            d = 6  # 4+2
        else:
            a1 = angle - se
            d = 3  # 2+1
        return d, a1 / math.pi * 4.0

    @staticmethod
    def outputcompresseddinf(dinfflowang, compdinffile, weightfile):
        dinf_R = RasterUtilClass.ReadRaster(dinfflowang)
        data = dinf_R.data
        xsize = dinf_R.nCols
        ysize = dinf_R.nRows
        noDataValue = dinf_R.noDataValue

        calDirCode = numpy.frompyfunc(DinfUtil.compressdinf, 2, 2)
        dirCode, weight = calDirCode(data, noDataValue)

        RasterUtilClass.WriteGTiffFile(compdinffile, ysize, xsize, dirCode,
                                       dinf_R.geotrans, dinf_R.srs, DEFAULT_NODATA, GDT_Int16)
        RasterUtilClass.WriteGTiffFile(weightfile, ysize, xsize, weight, dinf_R.geotrans,
                                       dinf_R.srs, DEFAULT_NODATA, GDT_Float32)

    @staticmethod
    def dinfdownslopedirection(a):
        d = DinfUtil.checkorthogonal(a)
        if d != 0:
            down = [d]
            return down
        else:
            if a < ne:  # 129 = 1+128
                down = [1, 2]
            elif a < n:  # 192 = 128+64
                down = [2, 3]
            elif a < nw:  # 96 = 64+32
                down = [3, 4]
            elif a < w:  # 48 = 32+16
                down = [4, 5]
            elif a < sw:  # 24 = 16+8
                down = [5, 6]
            elif a < s:  # 12 = 8+4
                down = [6, 7]
            elif a < se:  # 6 = 4+2
                down = [7, 8]
            else:  # 3 = 2+1
                down = [8, 1]
            return down

    @staticmethod
    def downstream_index_dinf(dinfdir_value, i, j):
        """find downslope coordinate for Dinf of TauDEM."""
        downDirs = DinfUtil.dinfdownslopedirection(dinfdir_value)
        downCoors = []
        for dir in downDirs:
            row, col = D8Util.downstream_index(dir, i, j)
            downCoors.append([row, col])
        return downCoors


class SubbasinUtil(object):
    """Utility functions of subbasin (raster and vector)"""

    def __init__(self):
        pass


class StreamnetUtil(object):
    """Utility functions of stream network"""

    def __init__(self):
        pass

    @staticmethod
    def serializestreamnet(streamNetFile, outputReachFile):
        """Eliminate reach with zero length and return the reach ID map."""
        FileClass.copyfiles(streamNetFile, outputReachFile)
        dsReach = ogr.Open(outputReachFile, update = True)
        layerReach = dsReach.GetLayer(0)
        layerDef = layerReach.GetLayerDefn()
        iLink = layerDef.GetFieldIndex(FLD_LINKNO)
        iLinkDownSlope = layerDef.GetFieldIndex(FLD_DSLINKNO)
        iLen = layerDef.GetFieldIndex(REACH_LENGTH)

        oldIdList = []
        # there are some reaches with zero length.
        # this program will remove these zero-length reaches
        # outputDic is used to store the downstream reaches of these zero-length
        # reaches
        outputDic = {}
        ft = layerReach.GetNextFeature()
        while ft is not None:
            id = ft.GetFieldAsInteger(iLink)
            reachLen = ft.GetFieldAsDouble(iLen)
            if not id in oldIdList:
                if reachLen < DELTA:
                    downstreamId = ft.GetFieldAsInteger(iLinkDownSlope)
                    outputDic[id] = downstreamId
                else:
                    oldIdList.append(id)

            ft = layerReach.GetNextFeature()
        oldIdList.sort()

        idMap = {}
        n = len(oldIdList)
        for i in range(n):
            idMap[oldIdList[i]] = i + 1
        # print idMap
        # change old ID to new ID
        layerReach.ResetReading()
        ft = layerReach.GetNextFeature()
        while ft is not None:
            id = ft.GetFieldAsInteger(iLink)
            if not id in idMap:
                layerReach.DeleteFeature(ft.GetFID())
                ft = layerReach.GetNextFeature()
                continue

            dsId = ft.GetFieldAsInteger(iLinkDownSlope)
            dsId = outputDic.get(dsId, dsId)
            dsId = outputDic.get(dsId, dsId)

            ft.SetField(FLD_LINKNO, idMap[id])
            if dsId in idMap:
                ft.SetField(FLD_DSLINKNO, idMap[dsId])
            else:
                # print dsId
                ft.SetField(FLD_DSLINKNO, -1)
            layerReach.SetFeature(ft)
            ft = layerReach.GetNextFeature()
        dsReach.ExecuteSQL("REPACK reach")
        layerReach.SyncToDisk()
        dsReach.Destroy()
        del dsReach
        return idMap

    @staticmethod
    def assignstreamidraster(stream_file, subbasin_file, out_stream_file):
        """Assign stream link ID according to subbasin ID."""
        stream_raster = RasterUtilClass.ReadRaster(stream_file)
        stream_data = stream_raster.data
        nrows = stream_raster.nRows
        ncols = stream_raster.nCols
        nodata = stream_raster.noDataValue
        subbain_data = RasterUtilClass.ReadRaster(subbasin_file).data
        nodata_array = numpy.ones((nrows, ncols)) * nodata
        newstream_data = numpy.where(stream_data > 0, subbain_data, nodata_array)
        RasterUtilClass.WriteGTiffFile(out_stream_file, nrows, ncols, newstream_data, stream_raster.geotrans,
                                       stream_raster.srs, nodata, GDT_Int16)
