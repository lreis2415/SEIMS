# -*- coding: utf-8 -*-
"""post process of TauDEM.

    @author: Liangjun Zhu

    @changlog:
    - 12-04-12 jz - origin version.
    - 16-07-01 lj - reorganized for pygeoc.
    - 17-06-25 lj - check by pylint and reformat by Google style.
    - 19-11-07 lj - fixed "TypeError: ... type 'char const *'" bugs caused by the import of unicode_literals
    - 21-04-01 lj - ignore very tiny flow fraction and bug fixed in updating dinf flow direction
"""
from __future__ import absolute_import, unicode_literals

from typing import List, Tuple
from numpy import frompyfunc, ones, where
from osgeo.gdal import GDT_Int16, GDT_Float32
from osgeo.ogr import Open as ogr_Open

from pygeoc.hydro import FlowModelConst, D8Util
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import MathClass, FileClass, DEFAULT_NODATA, PI, DELTA

# Field name of stream ESRI shapefile of TauDEM
FLD_LINKNO = str('LINKNO')
FLD_DSLINKNO = str('DSLINKNO')
REACH_WIDTH = str('WIDTH')
REACH_LENGTH = str('LENGTH')
REACH_DEPTH = str('DEPTH')


class DinfUtil(object):
    """Utility functions based on Dinf flow direction"""

    def __init__(self):
        pass

    @staticmethod
    def check_orthogonal(angle, minfrac=0.01):
        # type: (float, float) -> Tuple[float, int]
        """Check the given Dinf angle based on D8 flow direction encoding code by ArcGIS

        Examples:
            >>> DinfUtil.check_orthogonal(FlowModelConst.e)  # doctest: +ELLIPSIS
            (0.0, 1)
            >>> DinfUtil.check_orthogonal(2 * PI - 0.009 * PI / 4., 0.01)
            (0.0, 1)

        Args:
            angle: D-inf flow angle, [0, 2*pi]
            minfrac: Min. flow fraction that accounts

        Returns:
            Tuple[revised_dinf_flowdir, single_flowdir_code_ArcGIS]
        """
        flow_dir = -1
        frac_to_rad = minfrac * PI / 4. + DELTA
        if angle <= FlowModelConst.e + frac_to_rad or angle >= 2 * PI - frac_to_rad:
            flow_dir_taudem = FlowModelConst.e
            flow_dir = 1
        elif FlowModelConst.ne - frac_to_rad <= angle <= FlowModelConst.ne + frac_to_rad:
            flow_dir_taudem = FlowModelConst.ne
            flow_dir = 128
        elif FlowModelConst.n - frac_to_rad <= angle <= FlowModelConst.n + frac_to_rad:
            flow_dir_taudem = FlowModelConst.n
            flow_dir = 64
        elif FlowModelConst.nw - frac_to_rad <= angle <= FlowModelConst.nw + frac_to_rad:
            flow_dir_taudem = FlowModelConst.nw
            flow_dir = 32
        elif FlowModelConst.w - frac_to_rad <= angle <= FlowModelConst.w + frac_to_rad:
            flow_dir_taudem = FlowModelConst.w
            flow_dir = 16
        elif FlowModelConst.sw - frac_to_rad <= angle <= FlowModelConst.sw + frac_to_rad:
            flow_dir_taudem = FlowModelConst.sw
            flow_dir = 8
        elif FlowModelConst.s - frac_to_rad <= angle <= FlowModelConst.s + frac_to_rad:
            flow_dir_taudem = FlowModelConst.s
            flow_dir = 4
        elif FlowModelConst.se - frac_to_rad <= angle <= FlowModelConst.se + frac_to_rad:
            flow_dir_taudem = FlowModelConst.se
            flow_dir = 2
        else:
            flow_dir_taudem = angle
        return flow_dir_taudem, flow_dir

    @staticmethod
    def compress_dinf(angle, nodata, minfrac=0.01):
        """Compress dinf flow direction to D8 direction with weight follows ArcGIS D8 codes.

        Args:
            angle: D-inf flow direction angle
            nodata: NoData value
            minfrac: Minimum flow fraction that accounted, percent, e.g., 0.01

        Returns:
            1. Updated Dinf values
            2. Compressed flow direction follows ArcGIS D8 codes rule
            3. Weight of the first direction by counter-clockwise
        """
        if MathClass.floatequal(angle, nodata):
            return DEFAULT_NODATA, DEFAULT_NODATA, DEFAULT_NODATA
        angle, d = DinfUtil.check_orthogonal(angle, minfrac=minfrac)
        if d != -1:
            return angle, d, 1.
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
        return angle, d, 1. - a1 / PI * 4.0

    @staticmethod
    def output_compressed_dinf(dinfflowang,  # input
                               compdinffile, weightfile,  # outputs
                               minfraction=0.01, subbasin=None, stream=None,  # optional inputs
                               upddinffile=None  # optional output
                               ):
        """Output updated Dinf, compressed flow directions, and flow fractions to raster files
        Args:
            dinfflowang: Dinf flow direction raster file
            compdinffile: Compressed D8 flow code
            weightfile: The correspond weight to the first flow direction
            minfraction: Minimum flow fraction that accounted, percent, e.g., 0.01
            subbasin: Subbasin raster to satisfy that one cell only flow downstream within subbasin
            stream: Stream raster to satisfy that river cell only flow into one downstream cell
            upddinffile: Updated Dinf flow direction raster file
        """
        dinf_r = RasterUtilClass.read_raster(dinfflowang)
        data = dinf_r.data
        xsize = dinf_r.nCols
        ysize = dinf_r.nRows
        nodata_value = dinf_r.noDataValue

        use_subbsn = False
        use_stream = False
        if subbasin is not None:
            subbsn_r = RasterUtilClass.read_raster(subbasin)
            if xsize == subbsn_r.nCols and ysize == subbsn_r.nRows:
                use_subbsn = True
        if stream is not None:
            stream_r = RasterUtilClass.read_raster(stream)
            if xsize == stream_r.nCols and ysize == stream_r.nRows:
                use_stream = True

        # if use_stream:
        #     for i in range(ysize):
        #         for j in range(xsize):
        #             strm_v = stream_r.get_value_by_row_col(i, j)
        #             if strm_v is None or strm_v <= 0 or dinf_r.is_nodata(i, j):
        #                 continue
        #             dinf_v = dinf_r.get_value_by_row_col(i, j)
        #             dinf_upd, flowdir, weight = DinfUtil.compress_dinf(dinf_v,
        #                                                                dinf_r.noDataValue,
        #                                                                minfrac=minfraction)
        #             if flowdir in FlowModelConst.d8dir_ag:
        #                 continue
        #
        #
        #             down_cs = DinfUtil.downstream_index_dinf(dinf_v, i, j)
        #             weight = list()
        #             strm_l = list()
        #             for [ci, cj] in down_cs:
        #                 strm_tmp = stream_r.get_value_by_row_col(ci, cj)
        #                 if strm_v == strm_tmp:


        if use_subbsn or use_stream:
            for i in range(ysize):
                for j in range(xsize):
                    if dinf_r.is_nodata(i, j):
                        continue
                    # dinf_v = dinf_r.get_value_by_row_col(i, j)
                    # down_cs = DinfUtil.downstream_index_dinf(dinf_v, i, j)
                    # if use_subbsn:
                    #     sid = subbsn_r.get_value_by_row_col(i, j)
                    #
                    #     for [ci, cj] in down_cs:
                    #         sid_tmp = subbsn_r.get_value_by_row_col(ci, cj)
                    #         if
                    # if use_stream:
                    #     strmid = stream_r.get_value_by_row_col(i, j)

        cal_dir_code = frompyfunc(DinfUtil.compress_dinf, 3, 3)
        updated_angle, dir_code, weight = cal_dir_code(data, nodata_value, minfraction)

        if upddinffile is None:
            upddinffile = dinfflowang
        RasterUtilClass.write_gtiff_file(upddinffile, ysize, xsize, updated_angle,
                                         dinf_r.geotrans, dinf_r.srs, DEFAULT_NODATA, GDT_Float32)
        RasterUtilClass.write_gtiff_file(compdinffile, ysize, xsize, dir_code,
                                         dinf_r.geotrans, dinf_r.srs, DEFAULT_NODATA, GDT_Int16)
        RasterUtilClass.write_gtiff_file(weightfile, ysize, xsize, weight,
                                         dinf_r.geotrans, dinf_r.srs, DEFAULT_NODATA, GDT_Float32)

    @staticmethod
    def dinf_downslope_direction(a):
        """Get the downslope directions of an dinf direction value
        Args:
            a: Dinf value

        Returns:
            downslope directions
        """
        taud, d = DinfUtil.check_orthogonal(a)
        if d != -1:
            return [FlowModelConst.d8dir_td[FlowModelConst.d8dir_ag.index(d)]]
        else:
            if a < FlowModelConst.ne:  # 129 = 1+128
                return [1, 2]
            elif a < FlowModelConst.n:  # 192 = 128+64
                return [2, 3]
            elif a < FlowModelConst.nw:  # 96 = 64+32
                return [3, 4]
            elif a < FlowModelConst.w:  # 48 = 32+16
                return [4, 5]
            elif a < FlowModelConst.sw:  # 24 = 16+8
                return [5, 6]
            elif a < FlowModelConst.s:  # 12 = 8+4
                return [6, 7]
            elif a < FlowModelConst.se:  # 6 = 4+2
                return [7, 8]
            else:  # 3 = 2+1
                return [8, 1]

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
        down_coors = list()
        for dir_code in down_dirs:
            row, col = D8Util.downstream_index(dir_code, i, j, alg='taudem')
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

        old_id_list = list()
        # there are some reaches with zero length.
        # this program will remove these zero-length reaches
        # output_dic is used to store the downstream reaches of these zero-length
        # reaches
        output_dic = dict()
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

        id_map = dict()
        for i, old_id in enumerate(old_id_list):
            id_map[old_id] = i + 1
        # print(id_map)
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
                # print(ds_id)
                ft.SetField(FLD_DSLINKNO, -1)
            layer_reach.SetFeature(ft)
            ft = layer_reach.GetNextFeature()
        ds_reach.ExecuteSQL(str('REPACK %s' % layer_reach.GetName()))
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


def main():
    """Test code"""
    import os
    wp = r'D:\code\WatershedModels\SEIMS\data\youwuzhen\workspace\watershed delineation'
    dinfflowang = wp + os.sep + 'flowDirDinfTau.tif'
    compdinffile = wp + os.sep + 'dirCodeDinfTau.tif'
    weightfile = wp + os.sep + 'weightDinfTau.tif'
    updflowdinffile = wp + os.sep + 'updatedFlowDirDinf.tif'
    DinfUtil.output_compressed_dinf(dinfflowang, compdinffile, weightfile,
                                    minfraction=0.01, upddinffile=updflowdinffile)


if __name__ == '__main__':
    # Run doctest in docstrings of Google code style
    # python -m doctest utils.py (only when doctest.ELLIPSIS is not specified)
    # or python utils.py -v
    # or py.test --doctest-modules utils.py
    import doctest

    doctest.testmod()
