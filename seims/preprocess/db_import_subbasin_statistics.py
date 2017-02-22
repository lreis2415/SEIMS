import numpy
from pygeoc.hydro import FlowDirectionCode
from pygeoc.raster import RasterUtilClass

from config import *
from utility import LoadConfiguration
from utility import DEFAULT_NODATA
# for test main
from db_mongodb import ConnectMongoDB


def ImportSubbasinStatistics(db):
    """
    Import subbasin numbers, outlet ID, etc. to MongoDB.
    :return:
    """
    streamlinkR = WORKING_DIR + os.sep + DIR_NAME_GEODATA2DB + os.sep + streamLinkOut
    flowdirR = WORKING_DIR + os.sep + DIR_NAME_GEODATA2DB + os.sep + flowDirOut
    DIR_ITEMS = dict()
    if isTauDEM:
        DIR_ITEMS = FlowDirectionCode("TauDEM").get_cell_shift()
    else:
        DIR_ITEMS = FlowDirectionCode("ArcGIS").get_cell_shift()
    streamlinkD = RasterUtilClass.ReadRaster(streamlinkR)
    nodata = streamlinkD.noDataValue
    nrows = streamlinkD.nRows
    ncols = streamlinkD.nCols
    streamlinkData = streamlinkD.data
    maxSubbasinID = int(streamlinkD.GetMax())
    minSubbasinID = int(streamlinkD.GetMin())
    subbasinNum = numpy.unique(streamlinkData).size - 1
    # print maxSubbasinID, minSubbasinID, subbasinNum
    flowdirD = RasterUtilClass.ReadRaster(flowdirR)
    flowdirData = flowdirD.data
    iRow = -1
    iCol = -1
    for row in range(nrows):
        for col in range(ncols):
            if streamlinkData[row][col] != nodata:
                iRow = row
                iCol = col
                # print row, col
                break
        else:
            continue
        break
    if iRow == -1 or iCol == -1:
        raise ValueError("Stream link data invalid, please check and retry.")

    def flow_down_stream_idx(dirValue, i, j):
        drow, dcol = DIR_ITEMS[int(dirValue)]
        return i + drow, j + dcol

    def findOutletIndex(r, c):
        flag = True
        while flag:
            fdir = flowdirData[r][c]
            newr, newc = flow_down_stream_idx(fdir, r, c)
            if newr < 0 or newc < 0 or newr >= nrows or newc >= ncols or streamlinkData[newr][newc] == nodata:
                flag = False
            else:
                # print newr, newc, streamlinkData[newr][newc]
                r = newr
                c = newc
        return r, c

    oRow, oCol = findOutletIndex(iRow, iCol)
    outletBsnID = int(streamlinkData[oRow][oCol])
    importStatsDict = {PARAM_NAME_OUTLETID : outletBsnID, PARAM_NAME_OUTLETROW: oRow, PARAM_NAME_OUTLETCOL: oCol,
                       PARAM_NAME_SUBBSNMAX: maxSubbasinID, PARAM_NAME_SUBBSNMIN: minSubbasinID,
                       PARAM_NAME_SUBBSNNUM: subbasinNum}
    for stat in importStatsDict.keys():
        dic = {PARAM_FLD_NAME.upper()  : stat, PARAM_FLD_DESC.upper(): stat, PARAM_FLD_UNIT.upper(): "NONE",
               PARAM_FLD_MODS.upper()  : "ALL", PARAM_FLD_VALUE.upper(): importStatsDict[stat],
               PARAM_FLD_IMPACT.upper(): DEFAULT_NODATA, PARAM_FLD_CHANGE.upper(): PARAM_CHANGE_NC,
               PARAM_FLD_MAX.upper()   : DEFAULT_NODATA, PARAM_FLD_MIN.upper(): DEFAULT_NODATA,
               PARAM_FLD_USE.upper()   : PARAM_USE_Y, Tag_DT_Type.upper(): "WATERSHED"}
        curfilter = {PARAM_FLD_NAME.upper(): dic[PARAM_FLD_NAME.upper()]}
        # print (dic, curfilter)
        db[DB_TAB_PARAMETERS.upper()].find_one_and_replace(curfilter, dic, upsert = True)
    db[DB_TAB_PARAMETERS.upper()].create_index(PARAM_FLD_NAME.upper())


# test code
if __name__ == "__main__":
    LoadConfiguration(getconfigfile())
    client = ConnectMongoDB(HOSTNAME, PORT)
    conn = client.get_conn()
    db = conn[SpatialDBName]
    ImportSubbasinStatistics(db)
    client.close()
