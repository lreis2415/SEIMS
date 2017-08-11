#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""post process of TauDEM.

    author: Liangjun Zhu

    changlog: 12-04-12 jz - origin version.\n
              16-07-01 lj - reorganized for pygeoc.\n
              17-06-25 lj - check by pylint and reformat by Google style.\n
"""
from numpy import frompyfunc, ones, where
from osgeo.gdal import GDT_Int16, GDT_Float32
from osgeo.ogr import Open as ogr_Open

from ..hydro.hydro import FlowModelConst, D8Util
from ..raster.raster import RasterUtilClass
from ..utils.utils import MathClass, FileClass, DEFAULT_NODATA, PI, DELTA

# Field name of stream ESRI shapefile of TauDEM
FLD_LINKNO = "LINKNO"
FLD_DSLINKNO = "DSLINKNO"
REACH_WIDTH = "WIDTH"
REACH_LENGTH = "LENGTH"
REACH_DEPTH = "DEPTH"


class DinfUtil(object):
    """Utility functions based on Dinf flow direction"""

    def __init__(self):
        pass

    @staticmethod
    def check_orthogonal(angle):
        """Check the given Dinf angle based on D8 flow direction encoding code by ArcGIS"""
        flow_dir = -1
        if MathClass.floatequal(angle, FlowModelConst.e):
            flow_dir = 1  # 1
        elif MathClass.floatequal(angle, FlowModelConst.ne):
            flow_dir = 2  # 128
        elif MathClass.floatequal(angle, FlowModelConst.n):
            flow_dir = 3  # 64
        elif MathClass.floatequal(angle, FlowModelConst.nw):
            flow_dir = 4  # 32
        elif MathClass.floatequal(angle, FlowModelConst.w):
            flow_dir = 5  # 16
        elif MathClass.floatequal(angle, FlowModelConst.sw):
            flow_dir = 6  # 8
        elif MathClass.floatequal(angle, FlowModelConst.s):
            flow_dir = 7  # 4
        elif MathClass.floatequal(angle, FlowModelConst.se):
            flow_dir = 8  # 2
        return flow_dir

    @staticmethod
    def compress_dinf(angle, nodata):
        """Compress dinf flow direction to D8 direction with weight
        Args:
            angle: D-inf flow direction angle
            nodata: NoData value

        Returns:
            Compressed flow direction and weight of the first direction
        """
        if MathClass.floatequal(angle, nodata):
            return DEFAULT_NODATA, DEFAULT_NODATA
        d = DinfUtil.check_orthogonal(angle)
        if d is not None:
            return d, 1
        if angle < FlowModelConst.ne:
            a1 = angle
            d = 129  # 1+128
        elif angle < FlowModelConst.n:
            a1 = angle - FlowModelConst.ne
            d = 192  # 128+64
        elif angle < FlowModelConst.nw:
            a1 = angle - FlowModelConst.n
            d = 96  # 64+32
        elif angle < FlowModelConst.w:
            a1 = angle - FlowModelConst.nw
            d = 48  # 32+16
        elif angle < FlowModelConst.sw:
            a1 = angle - FlowModelConst.w
            d = 24  # 16+8
        elif angle < FlowModelConst.s:
            a1 = angle - FlowModelConst.sw
            d = 12  # 8+4
        elif angle < FlowModelConst.se:
            a1 = angle - FlowModelConst.s
            d = 6  # 4+2
        else:
            a1 = angle - FlowModelConst.se
            d = 3  # 2+1
        return d, a1 / PI * 4.0

    @staticmethod
    def output_compressed_dinf(dinfflowang, compdinffile, weightfile):
        """Output compressed Dinf flow direction and weight to raster file
        Args:
            dinfflowang: Dinf flow direction raster file
            compdinffile: Compressed D8 flow code
            weightfile: The correspond weight
        """
        dinf_r = RasterUtilClass.read_raster(dinfflowang)
        data = dinf_r.data
        xsize = dinf_r.nCols
        ysize = dinf_r.nRows
        nodata_value = dinf_r.noDataValue

        cal_dir_code = frompyfunc(DinfUtil.compress_dinf, 2, 2)
        dir_code, weight = cal_dir_code(data, nodata_value)

        RasterUtilClass.write_gtiff_file(compdinffile, ysize, xsize, dir_code,
                                         dinf_r.geotrans, dinf_r.srs, DEFAULT_NODATA, GDT_Int16)
        RasterUtilClass.write_gtiff_file(weightfile, ysize, xsize, weight, dinf_r.geotrans,
                                         dinf_r.srs, DEFAULT_NODATA, GDT_Float32)

    @staticmethod
    def dinf_downslope_direction(a):
        """Get the downslope directions of an dinf direction value
        Args:
            a: Dinf value

        Returns:
            downslope directions
        """
        d = DinfUtil.check_orthogonal(a)
        if d != -1:
            down = [d]
            return down
        else:
            if a < FlowModelConst.ne:  # 129 = 1+128
                down = [1, 2]
            elif a < FlowModelConst.n:  # 192 = 128+64
                down = [2, 3]
            elif a < FlowModelConst.nw:  # 96 = 64+32
                down = [3, 4]
            elif a < FlowModelConst.w:  # 48 = 32+16
                down = [4, 5]
            elif a < FlowModelConst.sw:  # 24 = 16+8
                down = [5, 6]
            elif a < FlowModelConst.s:  # 12 = 8+4
                down = [6, 7]
            elif a < FlowModelConst.se:  # 6 = 4+2
                down = [7, 8]
            else:  # 3 = 2+1
                down = [8, 1]
            return down

    @staticmethod
    def downstream_index_dinf(dinfdir_value, i, j):
        """find downslope coordinate for Dinf of TauDEM
        Args:
            dinfdir_value: dinf direction value
            i: current row
            j: current col

        Returns:
            downstream (row, col)s
        """
        down_dirs = DinfUtil.dinf_downslope_direction(dinfdir_value)
        down_coors = []
        for dir_code in down_dirs:
            row, col = D8Util.downstream_index(dir_code, i, j)
            down_coors.append([row, col])
        return down_coors


# class SubbasinUtil(object):
#     """Utility functions of subbasin (raster and vector)"""
#
#     def __init__(self):
#         pass


class StreamnetUtil(object):
    """Utility functions of stream network"""

    def __init__(self):
        pass

    @staticmethod
    def serialize_streamnet(streamnet_file, output_reach_file):
        """Eliminate reach with zero length and return the reach ID map.
        Args:
            streamnet_file: original stream net ESRI shapefile
            output_reach_file: serialized stream net, ESRI shapefile

        Returns:
            id pairs {origin: newly assigned}
        """
        FileClass.copy_files(streamnet_file, output_reach_file)
        ds_reach = ogr_Open(output_reach_file, update=True)
        layer_reach = ds_reach.GetLayer(0)
        layer_def = layer_reach.GetLayerDefn()
        i_link = layer_def.GetFieldIndex(FLD_LINKNO)
        i_link_downslope = layer_def.GetFieldIndex(FLD_DSLINKNO)
        i_len = layer_def.GetFieldIndex(REACH_LENGTH)

        old_id_list = []
        # there are some reaches with zero length.
        # this program will remove these zero-length reaches
        # output_dic is used to store the downstream reaches of these zero-length
        # reaches
        output_dic = {}
        ft = layer_reach.GetNextFeature()
        while ft is not None:
            link_id = ft.GetFieldAsInteger(i_link)
            reach_len = ft.GetFieldAsDouble(i_len)
            if link_id not in old_id_list:
                if reach_len < DELTA:
                    downstream_id = ft.GetFieldAsInteger(i_link_downslope)
                    output_dic[link_id] = downstream_id
                else:
                    old_id_list.append(link_id)

            ft = layer_reach.GetNextFeature()
        old_id_list.sort()

        id_map = {}
        for i, old_id in enumerate(old_id_list):
            id_map[old_id] = i + 1
        # print id_map
        # change old ID to new ID
        layer_reach.ResetReading()
        ft = layer_reach.GetNextFeature()
        while ft is not None:
            link_id = ft.GetFieldAsInteger(i_link)
            if link_id not in id_map:
                layer_reach.DeleteFeature(ft.GetFID())
                ft = layer_reach.GetNextFeature()
                continue

            ds_id = ft.GetFieldAsInteger(i_link_downslope)
            ds_id = output_dic.get(ds_id, ds_id)
            ds_id = output_dic.get(ds_id, ds_id)

            ft.SetField(FLD_LINKNO, id_map[link_id])
            if ds_id in id_map:
                ft.SetField(FLD_DSLINKNO, id_map[ds_id])
            else:
                # print ds_id
                ft.SetField(FLD_DSLINKNO, -1)
            layer_reach.SetFeature(ft)
            ft = layer_reach.GetNextFeature()
        ds_reach.ExecuteSQL("REPACK reach")
        layer_reach.SyncToDisk()
        ds_reach.Destroy()
        del ds_reach
        return id_map

    @staticmethod
    def assign_stream_id_raster(stream_file, subbasin_file, out_stream_file):
        """Assign stream link ID according to subbasin ID.
        Args:
            stream_file: input stream raster file
            subbasin_file: subbasin raster file
            out_stream_file: output stream raster file
        """
        stream_raster = RasterUtilClass.read_raster(stream_file)
        stream_data = stream_raster.data
        nrows = stream_raster.nRows
        ncols = stream_raster.nCols
        nodata = stream_raster.noDataValue
        subbain_data = RasterUtilClass.read_raster(subbasin_file).data
        nodata_array = ones((nrows, ncols)) * DEFAULT_NODATA
        newstream_data = where((stream_data > 0) & (stream_data != nodata),
                               subbain_data, nodata_array)
        RasterUtilClass.write_gtiff_file(out_stream_file, nrows, ncols, newstream_data,
                                         stream_raster.geotrans, stream_raster.srs,
                                         DEFAULT_NODATA, GDT_Int16)
