import numpy
from gdal import GDT_Int16
from pygeoc.raster import RasterUtilClass
from pygeoc.hydro.postTauDEM import D8Util
from pygeoc.utils.const import DEFAULT_NODATA, CCW_DCOL, CCW_DROW, D8DIR_TD_INFLOW_VALUES


def delineate_hillslopes_downstream_method(stream_raster, flow_dir_raster, hillslope_out, stream_value_method=0):
    """
    Delineate hillslopes for each subbasin, include header, left, and right hillslope
    Algorithm refers to Whitebox GAT.
    :param stream_raster: Stream cell value greater than 0 is identified by stream
                          The input stream are recommended sequenced as 1, 2, 3...
    :param flow_dir_raster: D8 flow direction in TauDEM code
    :param stream_value_method: method for stream cell' value.
                                0 - keep stream link code, which is default
                                1 - Set to the value of right hillslope and head hillslope
                                2 - Set to the value of left hillslope and head hillslope
                                3 - Set stream cell to NoData
    :return hillslope_out: With the sequenced stream IDs, the output hillslope will be numbered:
                              - Header hillslope: MaxStreamID * currentID + 1
                              - Right hillslope: MaxStreamID * currentID + 2
                              - Left hillslope: MaxStreamID * currentID + 3
    """
    print "Delineating hillslopes (header, left, and right hillslope)..."
    streamr = RasterUtilClass.ReadRaster(stream_raster)
    stream_data = streamr.data
    stream_nodata = streamr.noDataValue
    geotrans = streamr.geotrans
    srs = streamr.srs
    nrows = streamr.nRows
    ncols = streamr.nCols

    flowd8r = RasterUtilClass.ReadRaster(flow_dir_raster)
    flowd8_data = flowd8r.data
    flowd8_nodata = flowd8r.noDataValue
    if flowd8r.nRows != nrows or flowd8r.nCols != ncols:
        raise ValueError("The input extent of D8 flow direction is not consistent with stream data!")

    # definition of utility functions
    def inflow_stream_number(vrow, vcol):
        """Count the inflow stream cell number, and the coordinate"""
        neighb_stream_cell_num = 0
        cell_coors = []
        for c in range(8):
            newrow = vrow + CCW_DROW[c]
            newcol = vcol + CCW_DCOL[c]
            if newrow < 0 or newrow >= nrows or newcol < 0 or newcol >= ncols:
                continue
            if flowd8_data[newrow][newcol] == D8DIR_TD_INFLOW_VALUES[c]:
                cell_coors.append((newrow, newcol))
                if stream_data[newrow][newcol] > 0 and stream_data[newrow][newcol] != stream_nodata:
                    neighb_stream_cell_num += 1
        return neighb_stream_cell_num, cell_coors

    # 1. assign a unique id to each link in the stream network if needed
    assignStreamID = False
    tmp = numpy.where((stream_data > 0) & (stream_data != stream_nodata), stream_data, numpy.nan)
    currentMaxID = numpy.nanmax(tmp)
    currentID = 0
    if currentMaxID == numpy.nanmin(tmp):
        assignStreamID = True
    if assignStreamID:
        # calculate and output sequenced stream raster
        HillslopeMtxTmp = numpy.ones((nrows, ncols)) * DEFAULT_NODATA
        for row in range(nrows):
            for col in range(ncols):
                if stream_data[row][col] <= 0:
                    continue
                inflownum, inflowcoors = inflow_stream_number(row, col)
                if inflownum == 0:
                    # it's a headwater location so start a downstream flowpath
                    currentID += 1
                    tmprow = row
                    tmpcol = col
                    HillslopeMtxTmp[tmprow][tmpcol] = currentID
                    flag = True
                    while flag:
                        # find the downslope neighbour
                        tmpflowd8 = flowd8_data[tmprow][tmpcol]
                        if tmpflowd8 < 0 or tmpflowd8 == flowd8_nodata:
                            if stream_data[tmprow][tmpcol] > 0 and stream_data[tmprow][tmpcol] != stream_nodata:
                                # it is a valid stream cell and probably just has no downslope
                                # neighbour (e.g. at the edge of the grid)
                                HillslopeMtxTmp[tmprow][tmpcol] = currentID
                            break
                        tmprow, tmpcol = D8Util.downstream_index(tmpflowd8, tmprow, tmpcol)
                        if stream_data[tmprow][tmpcol] <= 0:
                            flag = False  # it is not a stream cell
                        else:
                            if HillslopeMtxTmp[tmprow][tmpcol] > 0:
                                # run into a larger stream, end the downstream search
                                break
                            # is it a confluence (conjunction node)
                            inflownum, inflowcoors = inflow_stream_number(tmprow, tmpcol)
                            if inflownum >= 2:
                                currentID += 1
                            HillslopeMtxTmp[tmprow][tmpcol] = currentID
        stream_data = numpy.copy(HillslopeMtxTmp)
        stream_nodata = DEFAULT_NODATA
        RasterUtilClass.WriteGTiffFile(hillslope_out, nrows, ncols, HillslopeMtxTmp, geotrans, srs,
                                       DEFAULT_NODATA, GDT_Int16)
        currentMaxID = currentID
    # 2. a). find the channel heads and give them unique identifier,
    #    b). assign hillslope code according to the 3*3 neighbour of stream cells
    HillslopeMtx = numpy.copy(stream_data)
    HillslopeMtx[stream_data == stream_nodata] = DEFAULT_NODATA
    headstreamCoors = []
    headstreamNoInflowCoors = []
    for row in range(nrows):
        for col in range(ncols):
            if stream_data[row][col] <= 0:
                continue
            inflownum, inflowcoors = inflow_stream_number(row, col)
            currentID = stream_data[row][col]
            # set hillslope IDs
            head_hillslope_id = currentID * currentMaxID + 1
            right_hillslope_id = currentID * currentMaxID + 2
            left_hillslope_id = currentID * currentMaxID + 3
            if inflownum == 0:
                # it is a one-order stream head
                HillslopeMtx[row][col] = head_hillslope_id
                headstreamCoors.append((row, col))
                continue
            elif inflownum >= 2:
                # it is a confluence, assign hillslope code to the non-stream inflow cells
                non_stream_inflow_count = 0
                for (tmprow, tmpcol) in inflowcoors:
                    if stream_data[tmprow][tmpcol] <= 0:
                        HillslopeMtx[tmprow][tmpcol] = head_hillslope_id
                        non_stream_inflow_count += 1
                if non_stream_inflow_count > 0:
                    HillslopeMtx[row][col] = head_hillslope_id
                    headstreamCoors.append((row, col))
                else:
                    # the first stream cell without inflow will be assigned according to its
                    # downslope stream cell, it maybe right hillslope id or left, or NODATA.
                    headstreamNoInflowCoors.append((row, col))
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
                tmprow = row + CCW_DROW[d - 1]
                tmpcol = col + CCW_DCOL[d - 1]
                tmpstream = stream_data[tmprow][tmpcol]
                if tmpstream <= 0:
                    state = True
                    # see if it flows into the stream cell at (row, col)
                    if int(flowd8_data[tmprow][tmpcol]) == D8DIR_TD_INFLOW_VALUES[d - 1]:
                        HillslopeMtx[tmprow][tmpcol] = right_hillslope_id
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
                tmprow = row + CCW_DROW[d - 1]
                tmpcol = col + CCW_DCOL[d - 1]
                tmpstream = stream_data[tmprow][tmpcol]
                if tmpstream <= 0:
                    # see if it flows into the stream cell at (row, col)
                    if int(flowd8_data[tmprow][tmpcol]) == D8DIR_TD_INFLOW_VALUES[d - 1] and \
                                    HillslopeMtx[tmprow][tmpcol] <= 0:
                        HillslopeMtx[tmprow][tmpcol] = left_hillslope_id
                k += 1
                if k == 7:
                    flag = True

    # 3. From each cell, search downstream for not assigned hillslope
    for row in range(nrows):
        for col in range(ncols):
            if HillslopeMtx[row][col] > 0 or flowd8_data[row][col] == flowd8_nodata:
                continue
            flag = False
            tmprow = row
            tmpcol = col
            tmpcoors = []
            streamID = DEFAULT_NODATA
            while not flag:
                # find it's downslope neighbour
                curflowdir = flowd8_data[tmprow][tmpcol]
                if curflowdir > 0:
                    tmprow, tmpcol = D8Util.downstream_index(curflowdir, tmprow, tmpcol)
                    # if the new cell already has a hillslope value, use that
                    if HillslopeMtx[tmprow][tmpcol] > 0:
                        streamID = HillslopeMtx[tmprow][tmpcol]
                        flag = True
                else:
                    flag = True
                if not flag:
                    tmpcoors.append((tmprow, tmpcol))
            HillslopeMtx[row][col] = streamID
            for (crow, ccol) in tmpcoors:
                HillslopeMtx[crow][ccol] = streamID

    # 4. reassign stream cell's value according to stream_value_method
    if stream_value_method != 0:
        for row in range(nrows):
            for col in range(ncols):
                if HillslopeMtx[row][col] == stream_data[row][col] > 0:
                    right_hillslope_id = stream_data[row][col] * currentMaxID + 2
                    left_hillslope_id = stream_data[row][col] * currentMaxID + 3
                    if stream_value_method == 1:  # right hillslope
                        HillslopeMtx[row][col] = right_hillslope_id
                    elif stream_value_method == 2:  # left hillslope
                        HillslopeMtx[row][col] = left_hillslope_id
                    else:
                        HillslopeMtx[row][col] = DEFAULT_NODATA
    else:
        for (tmprow, tmpcol) in headstreamCoors:
            HillslopeMtx[tmprow][tmpcol] = (HillslopeMtx[tmprow][tmpcol] - 1) / 3
    if stream_value_method == 3:
        for (tmprow, tmpcol) in headstreamCoors:
            HillslopeMtx[tmprow][tmpcol] = DEFAULT_NODATA
    else:
        for (tmprow, tmpcol) in headstreamNoInflowCoors:
            # find it's downslope neighbour
            curflowdir = flowd8_data[tmprow][tmpcol]
            if curflowdir > 0:
                tmprow2, tmpcol2 = D8Util.downstream_index(curflowdir, tmprow, tmpcol)
                HillslopeMtx[tmprow][tmpcol] = HillslopeMtx[tmprow2][tmpcol2]
            else:
                HillslopeMtx[tmprow][tmpcol] = DEFAULT_NODATA

    # 6. Output to raster file
    if 'asc' in hillslope_out.lower():
        RasterUtilClass.WriteAscFile(hillslope_out, HillslopeMtx, ncols, nrows, geotrans, DEFAULT_NODATA)
    else:
        RasterUtilClass.WriteGTiffFile(hillslope_out, nrows, ncols, HillslopeMtx, geotrans, srs, DEFAULT_NODATA,
                                       GDT_Int16)


if __name__ == '__main__':
    streamf = r'C:\z_data_m\SEIMS2017\dianbu2_30m_omp\spatial_raster\stream_link.tif'
    flowdirf = r'C:\z_data_m\SEIMS2017\dianbu2_30m_omp\taudem_delineated\flowDirTauM.tif'
    hillslpf = r'C:\z_data_m\SEIMS2017\dianbu2_30m_omp\spatial_raster\hillslope.tif'
    delineate_hillslopes_downstream_method(streamf, flowdirf, hillslpf, 1)
