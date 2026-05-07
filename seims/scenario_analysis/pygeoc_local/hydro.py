
# -*- coding: utf-8 -*-
"""Hydrological analysis functions.

    @author: Liangjun Zhu

    @changlog:
    - 17-06-25 lj - check by pylint and reformat by Google style.
    - 18-02-05 lj - compatible with Python3
    - 20-03-28 lj - add delineation function of Hillslopes.
"""
from __future__ import absolute_import, unicode_literals

import os
import numpy
from pygeoc.raster import RasterUtilClass, GDALDataType
from pygeoc.utils import FileClass, PI, SQ2, DEFAULT_NODATA


class FlowModelConst(object):
    """flow direction constants according to different flow model"""
    # CounterClockwise radian from east direction
    #        nw   n   ne
    #           \ | /
    #         w - + - e
    #           / | \
    #        sw   s   se
    e = 0.
    ne = PI * 0.25
    n = PI * 0.5
    nw = PI * 0.75
    w = PI
    sw = PI * 1.25
    s = PI * 1.5
    se = PI * 1.75
    d8anglelist = [e, ne, n, nw, w, sw, s, se]

    # 1. D8 flow directions in TauDEM
    # 4  3  2
    # 5     1
    # 6  7  8
    d8dir_td = [1, 2, 3, 4, 5, 6, 7, 8]
    d8len_td = {1: 1, 3: 1, 5: 1, 7: 1, 2: SQ2, 4: SQ2, 6: SQ2, 8: SQ2}
    d8delta_td = {1: (0, 1),
                  2: (-1, 1),
                  3: (-1, 0),
                  4: (-1, -1),
                  5: (0, -1),
                  6: (1, -1),
                  7: (1, 0),
                  8: (1, 1)}
    d8_inflow_td = [5, 6, 7, 8, 1, 2, 3, 4]

    # 2. D8 flow directions in ArcGIS
    # 32 64 128
    # 64     1
    # 8   4  2
    d8dir_ag = [1, 128, 64, 32, 16, 8, 4, 2]
    d8len_ag = {1: 1, 4: 1, 16: 1, 64: 1, 2: SQ2, 8: SQ2, 32: SQ2, 128: SQ2}
    d8delta_ag = {1: (0, 1),
                  128: (-1, 1),
                  64: (-1, 0),
                  32: (-1, -1),
                  16: (0, -1),
                  8: (1, -1),
                  4: (1, 0),
                  2: (1, 1)}
    d8_inflow_ag = [16, 8, 4, 2, 1, 128, 64, 32]

    # 3. D8 flow directions in Whitebox GAT (Geospatial Analysis Tools)
    # http://www.uoguelph.ca/~hydrogeo/Whitebox/index.html
    # 64 128  1
    # 32      2
    # 16   8  4
    d8dir_wb = [2, 1, 128, 64, 32, 16, 8, 4]
    d8len_wb = {1: 1, 4: 1, 16: 1, 64: 1, 2: SQ2, 8: SQ2, 32: SQ2, 128: SQ2}
    d8delta_wb = {2: (0, 1),
                  1: (-1, 1),
                  128: (-1, 0),
                  64: (-1, -1),
                  32: (0, -1),
                  16: (1, -1),
                  8: (1, 0),
                  4: (1, 1)}
    d8_inflow_wb = [32, 16, 8, 4, 2, 1, 128, 64]

    # delta row and col as the same sequence as d8delta_ag, d8delta_td, and d8delta_wb
    ccw_drow = [0, -1, -1, -1, 0, 1, 1, 1]  # row, not include itself
    ccw_dcol = [1, 1, 0, -1, -1, -1, 0, 1]  # col

    # available D8 flow direction algorithms
    d8_dirs = {'taudem': d8dir_td,
               'arcgis': d8dir_ag,
               'whitebox': d8dir_wb}
    d8_lens = {'taudem': d8len_td,
               'arcgis': d8len_ag,
               'whitebox': d8len_wb}
    d8_deltas = {'taudem': d8delta_td,
                 'arcgis': d8delta_ag,
                 'whitebox': d8delta_wb}
    d8_inflows = {'taudem': d8_inflow_td,
                  'arcgis': d8_inflow_ag,
                  'whitebox': d8_inflow_wb}

    @staticmethod
    def get_cell_length(flow_model):
        """Get flow direction induced cell length dict.
        Args:
            flow_model: Currently, "TauDEM", "ArcGIS", and "Whitebox" are supported.
        """
        assert flow_model.lower() in FlowModelConst.d8_lens
        return FlowModelConst.d8_lens.get(flow_model.lower())

    @staticmethod
    def get_cell_shift(flow_model):
        """Get flow direction induced cell shift dict.
        Args:
            flow_model: Currently, "TauDEM", "ArcGIS", and "Whitebox" are supported.
        """
        assert flow_model.lower() in FlowModelConst.d8_deltas
        return FlowModelConst.d8_deltas.get(flow_model.lower())


class D8Util(object):
    """Utility functions based on D8 flow direction
       Currently, D8 flow direction code of TauDEM, ArcGIS, Whitebox GAT are supported.
    """

    def __init__(self):
        pass

    @staticmethod
    def downstream_index(dir_value, i, j, alg='taudem'):
        """find downslope coordinate for D8 direction."""
        assert alg.lower() in FlowModelConst.d8_deltas
        delta = FlowModelConst.d8_deltas.get(alg.lower())
        drow, dcol = delta[int(dir_value)]
        return i + drow, j + dcol

    @staticmethod
    def convert_code(in_file, out_file, in_alg='taudem', out_alg='arcgis', datatype=None):
        """
        convert D8 flow direction code from one algorithm to another.
        Args:
            in_file: input raster file path
            out_file: output raster file path
            in_alg: available algorithms are in FlowModelConst.d8_dirs. "taudem" is the default
            out_alg: same as in_alg. "arcgis" is the default
            datatype: default is None and use the datatype of the in_file
        """
        FileClass.check_file_exists(in_file)
        in_alg = in_alg.lower()
        out_alg = out_alg.lower()
        if in_alg not in FlowModelConst.d8_dirs or out_alg not in FlowModelConst.d8_dirs:
            raise RuntimeError('The input algorithm name should one of %s' %
                               ', '.join(list(FlowModelConst.d8_dirs.keys())))
        convert_dict = dict()
        in_code = FlowModelConst.d8_dirs.get(in_alg)
        out_code = FlowModelConst.d8_dirs.get(out_alg)
        assert len(in_code) == len(out_code)
        for i, tmp_in_code in enumerate(in_code):
            convert_dict[tmp_in_code] = out_code[i]
        if datatype is not None and datatype in GDALDataType:
            RasterUtilClass.raster_reclassify(in_file, convert_dict, out_file, datatype)
        else:
            RasterUtilClass.raster_reclassify(in_file, convert_dict, out_file)


class Hillslopes(object):
    """Delineate hillslope for each subbasin, include header, left, and right hillslopes.

    Originally implemented in SEIMS (https://github.com/lreis2415/SEIMS).
    """

    @staticmethod
    def get_subbasin_from_hillslope_id(hillslp_id, subbsin_num):
        """Get subbasin ID from the hillslope ID and the subbasin number."""
        remainder = (hillslp_id - subbsin_num) % 3
        divid = (hillslp_id - subbsin_num) // 3
        if remainder == 0:
            return divid
        else:
            return divid + 1

    @staticmethod
    def cal_hs_codes(maxid, curid):
        """Set hillslope encode IDs."""
        return [maxid + (curid - 1) * 3 + 1,  # head
                maxid + (curid - 1) * 3 + 2,  # right
                maxid + (curid - 1) * 3 + 3]  # left

    @staticmethod
    def downstream_method_whitebox(stream_raster, flow_dir_raster, hillslope_out, d8alg="taudem",
                                   stream_value_method=4):
        """Algorithm modified from Whitebox GAT v3.4.0.
           source code: https://github.com/jblindsay/whitebox-geospatial-analysis-tools/blob/
                                master/HydroTools/plugins/Hillslopes.java

        Args:
            stream_raster: Stream cell value greater than 0 is identified by stream
                              The input stream are recommended sequenced as 1, 2, 3...
            flow_dir_raster: D8 flow direction in TauDEM code
            hillslope_out: With the sequenced stream IDs, the output hillslope will be numbered:
                                  - Header hillslope: MaxStreamID + (current_id - 1) * 3 + 1
                                  - Right hillslope: MaxStreamID + (current_id - 1) * 3 + 2
                                  - Left hillslope: MaxStreamID + (current_id - 1) * 3 + 3
            d8alg: Currently, "TauDEM", "ArcGIS", and "Whitebox" are supported.
            stream_value_method:  stream value assigned method, depend on this parameter,
                              the output hillslope will be appended as follows:
               -1 - all the four files will be output.
                0 - keep stream link code, which has the default file name
                1 - Set to the value of right hillslope and head hillslope, <name>_right.tif
                2 - Set to the value of left hillslope and head hillslope, <name>_left.tif
                3 - Set stream cell to NoData, <name>_nodata.tif
                4 (Default) - Set stream cell to 0, <name>_zero.tif
        """
        print('Delineating hillslopes (header, left, and right hillslopes)...')
        streamr = RasterUtilClass.read_raster(stream_raster)
        stream_data = streamr.data
        stream_nodata = streamr.noDataValue
        geotrans = streamr.geotrans
        srs = streamr.srs
        nrows = streamr.nRows
        ncols = streamr.nCols
        datatype = streamr.dataType

        flowd8r = RasterUtilClass.read_raster(flow_dir_raster)
        flowd8_data = flowd8r.data
        flowd8_nodata = flowd8r.noDataValue
        if flowd8r.nRows != nrows or flowd8r.nCols != ncols:
            raise ValueError("The input extent of D8 flow direction is not "
                             "consistent with stream data!")

        # definition of utility functions

        def inflow_stream_number(vrow, vcol, flowmodel="taudem"):
            """
            Count the inflow stream cell number and coordinates of all inflow cells

            Args:
                vrow: row number
                vcol: col number
                flowmodel: D8 flow direction algorithm.

            Returns:
                neighb_stream_cell_num: inflow cells number that is stream
                cell_coors: inflow cell coordinates, the size() is equal or greater
                            than neighb_stream_cell_num
            """
            neighb_stream_cell_num = 0
            cell_coors = list()
            for c in list(range(8)):
                newrow = vrow + FlowModelConst.ccw_drow[c]
                newcol = vcol + FlowModelConst.ccw_dcol[c]
                if newrow < 0 or newrow >= nrows or newcol < 0 or newcol >= ncols:
                    continue
                if flowd8_data[newrow][newcol] == \
                        FlowModelConst.d8_inflows.get(flowmodel.lower())[c]:
                    cell_coors.append((newrow, newcol))
                    if stream_data[newrow][newcol] > 0 \
                            and stream_data[newrow][newcol] != stream_nodata:
                        neighb_stream_cell_num += 1
            return neighb_stream_cell_num, cell_coors

        def assign_sequenced_stream_ids(c_id, vrow, vcol, flowmodel="taudem"):
            """set sequenced stream IDs"""
            in_strm_num, in_coors = inflow_stream_number(vrow, vcol, flowmodel)
            if in_strm_num == 0:
                # it's a headwater location so start a downstream flowpath
                c_id += 1
                tmp_row = vrow
                tmp_col = vcol
                sequenced_stream_d[tmp_row][tmp_col] = c_id
                searched_flag = True
                while searched_flag:
                    # find the downslope neighbour
                    tmpflowd8 = flowd8_data[tmp_row][tmp_col]
                    if tmpflowd8 < 0 or tmpflowd8 == flowd8_nodata:
                        if stream_data[tmp_row][tmp_col] > 0 \
                                and stream_data[tmp_row][tmp_col] != stream_nodata:
                            # it is a valid stream cell and probably just has no downslope
                            # neighbour (e.g. at the edge of the grid)
                            sequenced_stream_d[tmp_row][tmp_col] = c_id
                        break
                    tmp_row, tmp_col = D8Util.downstream_index(tmpflowd8, tmp_row,
                                                               tmp_col, flowmodel)
                    if tmp_row < 0 or tmp_row >= nrows or tmp_col < 0 or tmp_col >= ncols:
                        break
                    if stream_data[tmp_row][tmp_col] <= 0:
                        searched_flag = False  # it is not a stream cell
                    else:
                        if sequenced_stream_d[tmp_row][tmp_col] > 0:
                            # run into a larger stream, end the downstream search
                            break
                        # is it a confluence (conjunction node)
                        in_strm_num, in_coors = inflow_stream_number(tmp_row, tmp_col, flowmodel)
                        if in_strm_num >= 2:
                            c_id += 1
                        sequenced_stream_d[tmp_row][tmp_col] = c_id
            return c_id

        def assign_hillslope_code_of_neighbors(vrow, vcol, flowmodel="taudem"):
            """set hillslope code for neighbors of current stream cell."""
            stream_coors.append((vrow, vcol))
            in_strm_num, in_coors = inflow_stream_number(vrow, vcol, flowmodel)
            strm_id = stream_data[vrow][vcol]
            # print('Assign hillslope code for stream cell, r: %d, c: %d, ID: %d' % (vrow, vcol,
            #                                                                        int(strm_id)))
            # set hillslope IDs
            hillslp_ids = Hillslopes.cal_hs_codes(max_id, strm_id)
            cur_d8_value = flowd8_data[vrow][vcol]
            if in_strm_num == 0:  # it is a one-order stream head
                headstream_coors.append((vrow, vcol))
                for (in_nostrm_row, in_nostrm_col) in in_coors:
                    hillslope_mtx[in_nostrm_row][in_nostrm_col] = hillslp_ids[0]
            else:  # search the 3*3 neighbors by clockwise and counterclockwise separately
                if cur_d8_value <= 0 or cur_d8_value == flowd8_nodata:
                    return
                dirv = int(cur_d8_value)  # direction code
                d_idx = FlowModelConst.d8_dirs.get(flowmodel).index(dirv)  # direction index
                # look to the right side, i.e. clockwise
                d_idx_r = d_idx
                while True and len(in_coors) > 0:
                    d_idx_r -= 1
                    if d_idx_r > 7:
                        d_idx_r = 0
                    if d_idx_r < 0:
                        d_idx_r = 7
                    tmp_row = vrow + FlowModelConst.ccw_drow[d_idx_r]
                    tmp_col = vcol + FlowModelConst.ccw_dcol[d_idx_r]
                    if (tmp_row, tmp_col) not in in_coors:  # not inflow to this cell
                        continue
                    tmpstream = stream_data[tmp_row][tmp_col]
                    in_coors.remove((tmp_row, tmp_col))
                    if tmpstream <= 0 or tmpstream == stream_nodata:
                        hillslope_mtx[tmp_row][tmp_col] = hillslp_ids[1]  # right hillslope
                    else:  # encounter another in flow stream
                        break

                # look to the left side, i.e. counterclockwise
                d_idx_l = d_idx
                while True and len(in_coors) > 0:
                    d_idx_l += 1
                    if d_idx_l > 7:
                        d_idx_l = 0
                    if d_idx_l < 0:
                        d_idx_l = 7
                    tmp_row = vrow + FlowModelConst.ccw_drow[d_idx_l]
                    tmp_col = vcol + FlowModelConst.ccw_dcol[d_idx_l]
                    if (tmp_row, tmp_col) not in in_coors:  # not inflow to this cell
                        continue
                    tmpstream = stream_data[tmp_row][tmp_col]
                    in_coors.remove((tmp_row, tmp_col))
                    if tmpstream <= 0 or tmpstream == stream_nodata:
                        hillslope_mtx[tmp_row][tmp_col] = hillslp_ids[2]  # left hillslope
                    else:  # encounter another in flow stream
                        break
                # if any inflow cells existed?
                if len(in_coors) > 0:
                    for (tmp_row, tmp_col) in in_coors:
                        tmpstream = stream_data[tmp_row][tmp_col]
                        if tmpstream <= 0 or tmpstream == stream_nodata:
                            hillslope_mtx[tmp_row][tmp_col] = hillslp_ids[0]
                            # add the current cell as head stream
                            headstream_coors.append((vrow, vcol))

        def output_hillslope(method_id):
            """Output hillslope according different stream cell value method."""
            for (tmp_row, tmp_col) in stream_coors:
                tmp_hillslp_ids = Hillslopes.cal_hs_codes(max_id, stream_data[tmp_row][tmp_col])
                if 0 < method_id < 3:
                    hillslope_mtx[tmp_row][tmp_col] = tmp_hillslp_ids[method_id]
                    # is head stream cell?
                    if (tmp_row, tmp_col) in headstream_coors:
                        hillslope_mtx[tmp_row][tmp_col] = tmp_hillslp_ids[0]
                elif method_id == 3:
                    hillslope_mtx[tmp_row][tmp_col] = DEFAULT_NODATA
                elif method_id == 4:
                    hillslope_mtx[tmp_row][tmp_col] = 0
            # Output to raster file
            hillslope_out_new = hillslope_out
            dirpath = os.path.dirname(hillslope_out_new) + os.path.sep
            corename = FileClass.get_core_name_without_suffix(hillslope_out_new)
            if method_id == 1:
                hillslope_out_new = dirpath + corename + '_right.tif'
            elif method_id == 2:
                hillslope_out_new = dirpath + corename + '_left.tif'
            elif method_id == 3:
                hillslope_out_new = dirpath + corename + '_nodata.tif'
            elif method_id == 4:
                hillslope_out_new = dirpath + corename + '_zero.tif'
            RasterUtilClass.write_gtiff_file(hillslope_out_new, nrows, ncols,
                                             hillslope_mtx,
                                             geotrans, srs, DEFAULT_NODATA, datatype)

        # 1. assign a unique id to each link in the stream network if needed
        assign_stream_id = False
        tmp = numpy.where((stream_data > 0) & (stream_data != stream_nodata),
                          stream_data, numpy.nan)
        max_id = int(numpy.nanmax(tmp))  # i.e., stream link number
        min_id = int(numpy.nanmin(tmp))
        for i in range(min_id, max_id + 1):
            if i not in tmp:
                assign_stream_id = True
                break
        if max_id == min_id:
            assign_stream_id = True
        current_id = 0
        if assign_stream_id:
            # calculate and output sequenced stream raster
            sequenced_stream_d = numpy.ones((nrows, ncols)) * DEFAULT_NODATA
            for row in range(nrows):
                for col in range(ncols):
                    # if the cell is not a stream, or has been assigned an ID
                    if stream_data[row][col] <= 0 or stream_data[row][col] == stream_nodata \
                            or sequenced_stream_d[row][col] > 0:
                        continue
                    current_id = assign_sequenced_stream_ids(current_id, row, col, d8alg)
            stream_data = numpy.copy(sequenced_stream_d)
            stream_nodata = DEFAULT_NODATA
            stream_core = FileClass.get_core_name_without_suffix(stream_raster)
            stream_seq_file = os.path.dirname(stream_raster) + os.path.sep + \
                              stream_core + '_seq.tif'
            RasterUtilClass.write_gtiff_file(stream_seq_file, nrows, ncols, sequenced_stream_d,
                                             geotrans, srs, DEFAULT_NODATA, datatype)
            max_id = current_id
        # 2. assign hillslope code according to the 3*3 neighbors of stream cells
        hillslope_mtx = numpy.copy(stream_data)
        hillslope_mtx[stream_data == stream_nodata] = DEFAULT_NODATA
        headstream_coors = list()  # head stream cells
        stream_coors = list()  # all stream cells, include head stream cells.
        for row in list(range(nrows)):
            for col in list(range(ncols)):
                # if not a stream cell, or hillslope code has been assigned
                if stream_data[row][col] <= 0 or stream_data[row][col] == stream_nodata \
                        or hillslope_mtx[row][col] < 0:
                    continue
                assign_hillslope_code_of_neighbors(row, col, d8alg)

        # 3. From each cell, search downstream for not assigned hillslope
        for row in list(range(nrows)):
            for col in list(range(ncols)):
                if hillslope_mtx[row][col] > 0 or flowd8_data[row][col] == flowd8_nodata:
                    continue
                flag = False
                tmprow = row
                tmpcol = col
                tmpcoors = [(row, col)]
                hillslp_id = DEFAULT_NODATA
                while not flag:
                    # find it's downslope neighbour
                    curflowdir = flowd8_data[tmprow][tmpcol]
                    if curflowdir <= 0 or curflowdir == flowd8_nodata:
                        break
                    curflowdir = int(curflowdir)
                    tmprow, tmpcol = D8Util.downstream_index(curflowdir, tmprow, tmpcol, d8alg)
                    if tmprow < 0 or tmprow >= nrows or tmpcol < 0 or tmpcol >= ncols:
                        break
                    # if the new cell already has a hillslope value, use that
                    if hillslope_mtx[tmprow][tmpcol] > 0:
                        hillslp_id = hillslope_mtx[tmprow][tmpcol]
                        flag = True
                    if not flag:
                        tmpcoors.append((tmprow, tmpcol))
                # set the source cells
                for (crow, ccol) in tmpcoors:
                    hillslope_mtx[crow][ccol] = hillslp_id

        # 4. reassign stream cell's value according to stream_value_method, and output
        if stream_value_method < 0:  # output
            output_hillslope(0)
            output_hillslope(1)
            output_hillslope(2)
            output_hillslope(3)
            output_hillslope(4)
        else:
            output_hillslope(stream_value_method)
