#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Delineate hillslopes for each subbasin, include header, left, and right hillslope.
    @author   : Liangjun Zhu
    @changelog: 17-05-15  lj - initial version modified from Whitebox GAT.
                17-06-29  lj - reorganized according to pylint and google style
"""
import numpy
from gdal import GDT_Int16

from seims.pygeoc.pygeoc.hydro.hydro import FlowModelConst
from seims.pygeoc.pygeoc.hydro.postTauDEM import D8Util
from seims.pygeoc.pygeoc.raster.raster import RasterUtilClass
from seims.pygeoc.pygeoc.utils.utils import DEFAULT_NODATA


class DelineateHillslope(object):
    """Delineate hillslope for each subbasin, include header, left, and right hillslope"""

    @staticmethod
    def downstream_method_whitebox(stream_raster, flow_dir_raster, hillslope_out,
                                   stream_value_method=0):
        """Algorithm modified from Whitebox GAT.
        Args:
            stream_raster: Stream cell value greater than 0 is identified by stream
                              The input stream are recommended sequenced as 1, 2, 3...
            flow_dir_raster: D8 flow direction in TauDEM code
            hillslope_out: With the sequenced stream IDs, the output hillslope will be numbered:
                                  - Header hillslope: MaxStreamID * current_id + 1
                                  - Right hillslope: MaxStreamID * current_id + 2
                                  - Left hillslope: MaxStreamID * current_id + 3
            stream_value_method: method for stream cell' value.
                                    0 - keep stream link code, which is default
                                    1 - Set to the value of right hillslope and head hillslope
                                    2 - Set to the value of left hillslope and head hillslope
                                    3 - Set stream cell to NoData
        """
        print ("Delineating hillslopes (header, left, and right hillslope)...")
        streamr = RasterUtilClass.read_raster(stream_raster)
        stream_data = streamr.data
        stream_nodata = streamr.noDataValue
        geotrans = streamr.geotrans
        srs = streamr.srs
        nrows = streamr.nRows
        ncols = streamr.nCols

        flowd8r = RasterUtilClass.read_raster(flow_dir_raster)
        flowd8_data = flowd8r.data
        flowd8_nodata = flowd8r.noDataValue
        if flowd8r.nRows != nrows or flowd8r.nCols != ncols:
            raise ValueError("The input extent of D8 flow direction is not "
                             "consistent with stream data!")

        # definition of utility functions
        def inflow_stream_number(vrow, vcol):
            """Count the inflow stream cell number, and the coordinate"""
            neighb_stream_cell_num = 0
            cell_coors = []
            for c in range(8):
                newrow = vrow + FlowModelConst.ccw_drow[c]
                newcol = vcol + FlowModelConst.ccw_dcol[c]
                if newrow < 0 or newrow >= nrows or newcol < 0 or newcol >= ncols:
                    continue
                if flowd8_data[newrow][newcol] == FlowModelConst.d8_inflow_td[c]:
                    cell_coors.append((newrow, newcol))
                    if stream_data[newrow][newcol] > 0 \
                            and stream_data[newrow][newcol] != stream_nodata:
                        neighb_stream_cell_num += 1
            return neighb_stream_cell_num, cell_coors

        # 1. assign a unique id to each link in the stream network if needed
        assign_stream_id = False
        tmp = numpy.where((stream_data > 0) & (stream_data != stream_nodata),
                          stream_data, numpy.nan)
        current_max_id = numpy.nanmax(tmp)
        current_id = 0
        if current_max_id == numpy.nanmin(tmp):
            assign_stream_id = True
        if assign_stream_id:
            # calculate and output sequenced stream raster
            hillslope_mtx_tmp = numpy.ones((nrows, ncols)) * DEFAULT_NODATA
            for row in range(nrows):
                for col in range(ncols):
                    if stream_data[row][col] <= 0:
                        continue
                    inflownum, inflowcoors = inflow_stream_number(row, col)
                    if inflownum == 0:
                        # it's a headwater location so start a downstream flowpath
                        current_id += 1
                        tmprow = row
                        tmpcol = col
                        hillslope_mtx_tmp[tmprow][tmpcol] = current_id
                        flag = True
                        while flag:
                            # find the downslope neighbour
                            tmpflowd8 = flowd8_data[tmprow][tmpcol]
                            if tmpflowd8 < 0 or tmpflowd8 == flowd8_nodata:
                                if stream_data[tmprow][tmpcol] > 0 \
                                        and stream_data[tmprow][tmpcol] != stream_nodata:
                                    # it is a valid stream cell and probably just has no downslope
                                    # neighbour (e.g. at the edge of the grid)
                                    hillslope_mtx_tmp[tmprow][tmpcol] = current_id
                                break
                            tmprow, tmpcol = D8Util.downstream_index(tmpflowd8, tmprow, tmpcol)
                            if stream_data[tmprow][tmpcol] <= 0:
                                flag = False  # it is not a stream cell
                            else:
                                if hillslope_mtx_tmp[tmprow][tmpcol] > 0:
                                    # run into a larger stream, end the downstream search
                                    break
                                # is it a confluence (conjunction node)
                                inflownum, inflowcoors = inflow_stream_number(tmprow, tmpcol)
                                if inflownum >= 2:
                                    current_id += 1
                                hillslope_mtx_tmp[tmprow][tmpcol] = current_id
            stream_data = numpy.copy(hillslope_mtx_tmp)
            stream_nodata = DEFAULT_NODATA
            RasterUtilClass.write_gtiff_file(hillslope_out, nrows, ncols, hillslope_mtx_tmp,
                                             geotrans, srs, DEFAULT_NODATA, GDT_Int16)
            current_max_id = current_id
        # 2. a). find the channel heads and give them unique identifier,
        #    b). assign hillslope code according to the 3*3 neighbour of stream cells
        hillslope_mtx = numpy.copy(stream_data)
        hillslope_mtx[stream_data == stream_nodata] = DEFAULT_NODATA
        headstream_coors = []
        headstream_no_inflow_coors = []
        for row in range(nrows):
            for col in range(ncols):
                if stream_data[row][col] <= 0:
                    continue
                inflownum, inflowcoors = inflow_stream_number(row, col)
                current_id = stream_data[row][col]
                # set hillslope IDs
                head_hillslope_id = current_id * current_max_id + 1
                right_hillslope_id = current_id * current_max_id + 2
                left_hillslope_id = current_id * current_max_id + 3
                if inflownum == 0:
                    # it is a one-order stream head
                    hillslope_mtx[row][col] = head_hillslope_id
                    headstream_coors.append((row, col))
                    continue
                elif inflownum >= 2:
                    # it is a confluence, assign hillslope code to the non-stream inflow cells
                    non_stream_inflow_count = 0
                    for (tmprow, tmpcol) in inflowcoors:
                        if stream_data[tmprow][tmpcol] <= 0:
                            hillslope_mtx[tmprow][tmpcol] = head_hillslope_id
                            non_stream_inflow_count += 1
                    if non_stream_inflow_count > 0:
                        hillslope_mtx[row][col] = head_hillslope_id
                        headstream_coors.append((row, col))
                    else:
                        # the first stream cell without inflow will be assigned according to its
                        # downslope stream cell, it maybe right hillslope id or left, or NODATA.
                        headstream_no_inflow_coors.append((row, col))
                    continue
                # then, the inflownum is 1
                # search the 3*3 neighbours by clockwise and counterclockwise separately
                curflowdir = flowd8_data[row][col]
                if curflowdir <= 0 or curflowdir == flowd8_nodata:
                    continue
                d = int(curflowdir)
                # look to the right side, i.e. clockwise
                flag = False
                state = False
                while not flag:
                    d -= 1
                    if d > 8:
                        d = 1
                    if d < 1:
                        d = 8
                    tmprow = row + FlowModelConst.ccw_drow[d - 1]
                    tmpcol = col + FlowModelConst.ccw_dcol[d - 1]
                    tmpstream = stream_data[tmprow][tmpcol]
                    if tmpstream <= 0:
                        state = True
                        # see if it flows into the stream cell at (row, col)
                        if int(flowd8_data[tmprow][tmpcol]) == FlowModelConst.d8_inflow_td[d - 1]:
                            hillslope_mtx[tmprow][tmpcol] = right_hillslope_id
                    else:  # searched for the first stream cell on the clockwise
                        if state:
                            flag = True
                # look to the left side, i.e. counterclockwise
                flag = False
                d = int(curflowdir)
                k = 0
                while not flag:
                    d += 1
                    if d > 8:
                        d = 1
                    if d < 1:
                        d = 8
                    tmprow = row + FlowModelConst.ccw_drow[d - 1]
                    tmpcol = col + FlowModelConst.ccw_dcol[d - 1]
                    tmpstream = stream_data[tmprow][tmpcol]
                    if tmpstream <= 0:
                        # see if it flows into the stream cell at (row, col)
                        if int(flowd8_data[tmprow][tmpcol]) == FlowModelConst.d8_inflow_td[d - 1] \
                                and hillslope_mtx[tmprow][tmpcol] <= 0:
                            hillslope_mtx[tmprow][tmpcol] = left_hillslope_id
                    k += 1
                    if k == 7:
                        flag = True

        # 3. From each cell, search downstream for not assigned hillslope
        for row in range(nrows):
            for col in range(ncols):
                if hillslope_mtx[row][col] > 0 or flowd8_data[row][col] == flowd8_nodata:
                    continue
                flag = False
                tmprow = row
                tmpcol = col
                tmpcoors = []
                stream_id = DEFAULT_NODATA
                while not flag:
                    # find it's downslope neighbour
                    curflowdir = flowd8_data[tmprow][tmpcol]
                    if curflowdir > 0:
                        tmprow, tmpcol = D8Util.downstream_index(curflowdir, tmprow, tmpcol)
                        # if the new cell already has a hillslope value, use that
                        if hillslope_mtx[tmprow][tmpcol] > 0:
                            stream_id = hillslope_mtx[tmprow][tmpcol]
                            flag = True
                    else:
                        flag = True
                    if not flag:
                        tmpcoors.append((tmprow, tmpcol))
                hillslope_mtx[row][col] = stream_id
                for (crow, ccol) in tmpcoors:
                    hillslope_mtx[crow][ccol] = stream_id

        # 4. reassign stream cell's value according to stream_value_method
        if stream_value_method != 0:
            for row in range(nrows):
                for col in range(ncols):
                    if hillslope_mtx[row][col] == stream_data[row][col] > 0:
                        right_hillslope_id = stream_data[row][col] * current_max_id + 2
                        left_hillslope_id = stream_data[row][col] * current_max_id + 3
                        if stream_value_method == 1:  # right hillslope
                            hillslope_mtx[row][col] = right_hillslope_id
                        elif stream_value_method == 2:  # left hillslope
                            hillslope_mtx[row][col] = left_hillslope_id
                        else:
                            hillslope_mtx[row][col] = DEFAULT_NODATA
        else:
            for (tmprow, tmpcol) in headstream_coors:
                hillslope_mtx[tmprow][tmpcol] = (hillslope_mtx[tmprow][tmpcol] - 1) / 3
        if stream_value_method == 3:
            for (tmprow, tmpcol) in headstream_coors:
                hillslope_mtx[tmprow][tmpcol] = DEFAULT_NODATA
        else:
            for (tmprow, tmpcol) in headstream_no_inflow_coors:
                # find it's downslope neighbour
                curflowdir = flowd8_data[tmprow][tmpcol]
                if curflowdir > 0:
                    tmprow2, tmpcol2 = D8Util.downstream_index(curflowdir, tmprow, tmpcol)
                    hillslope_mtx[tmprow][tmpcol] = hillslope_mtx[tmprow2][tmpcol2]
                else:
                    hillslope_mtx[tmprow][tmpcol] = DEFAULT_NODATA

        # 6. Output to raster file
        if 'asc' in hillslope_out.lower():
            RasterUtilClass.write_asc_file(hillslope_out, hillslope_mtx, ncols, nrows,
                                           geotrans, DEFAULT_NODATA)
        else:
            RasterUtilClass.write_gtiff_file(hillslope_out, nrows, ncols, hillslope_mtx,
                                             geotrans, srs, DEFAULT_NODATA, GDT_Int16)


def main():
    """TEST CODE"""
    streamf = r'C:\z_data_m\SEIMS2017\dianbu2_30m_omp\spatial_raster\stream_link.tif'
    flowdirf = r'C:\z_data_m\SEIMS2017\dianbu2_30m_omp\taudem_delineated\flowDirTauM.tif'
    hillslpf = r'C:\z_data_m\SEIMS2017\dianbu2_30m_omp\spatial_raster\hillslope.tif'
    DelineateHillslope.downstream_method_whitebox(streamf, flowdirf, hillslpf, 1)


if __name__ == '__main__':
    main()
