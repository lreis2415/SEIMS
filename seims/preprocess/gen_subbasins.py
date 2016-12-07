#! /usr/bin/env python
# coding=utf-8
# Generation masked subbasin spatial data
# Author: Junzhi Liu
# Revised: Liang-Jun Zhu
# Note: Improve calculation efficiency by numpy
#
import platform

from pymongo import MongoClient
from pymongo.errors import ConnectionFailure

from post_process_taudem import *
from util import GetExecutableFullPath, RunExternalCmd

def GetMaskFromRaster(rasterFile, dstdir):
    rasterR = ReadRaster(rasterFile)
    xsize = rasterR.nCols
    ysize = rasterR.nRows
    noDataValue = rasterR.noDataValue
    srs = rasterR.srs
    xMin = rasterR.xMin
    yMax = rasterR.yMax
    dx = rasterR.dx
    data = rasterR.data

    iMin = ysize - 1
    iMax = 0
    jMin = xsize - 1
    jMax = 0

    for i in range(ysize):
        for j in range(xsize):
            if abs(data[i][j] - noDataValue) > UTIL_ZERO:
                iMin = min(i, iMin)
                iMax = max(i, iMax)
                jMin = min(j, jMin)
                jMax = max(j, jMax)

    # print iMin, iMax, jMin, jMax
    ySizeMask = iMax - iMin + 1
    xSizeMask = jMax - jMin + 1
    xMinMask = xMin + jMin * dx
    yMaxMask = yMax - iMin * dx
    print ("%dx%d -> %dx%d" % (xsize, ysize, xSizeMask, ySizeMask))

    mask = numpy.zeros((ySizeMask, xSizeMask))

    for i in range(ySizeMask):
        for j in range(xSizeMask):
            if abs(data[i + iMin][j + jMin] - noDataValue) > UTIL_ZERO:
                mask[i][j] = 1
            else:
                mask[i][j] = DEFAULT_NODATA

    outputFile = dstdir + os.sep + mask_to_ext
    maskGeotrans = [xMinMask, dx, 0, yMaxMask, 0, -dx]
    WriteGTiffFile(outputFile, ySizeMask, xSizeMask, mask,
                   maskGeotrans, srs, DEFAULT_NODATA, gdal.GDT_Int32)
    return outputFile, Raster(ySizeMask, xSizeMask, mask, DEFAULT_NODATA, maskGeotrans, srs)


def MaskDEMFiles(workingDir, exeDir=None):
    tauDir = workingDir + os.sep + DIR_NAME_TAUDEM
    subbasinTauFile = tauDir + os.sep + subbasin
    originalFiles = [subbasin, flowDir, streamRaster,
                     slope, filledDem, acc, streamOrder,
                     flowDirDinf, dirCodeDinf, slopeDinf, weightDinf, cellLat, daylMin, dormhr, dist2StreamD8]
    originalFiles = [(tauDir + os.sep + item) for item in originalFiles]
    maskedFiles = [subbasinM, flowDirM, streamRasterM]
    maskedFiles = [(tauDir + os.sep + item) for item in maskedFiles]
    outputList = [slopeM, filldemM, accM, streamOrderM, flowDirDinfM,
                  dirCodeDinfM, slopeDinfM, weightDinfM, cellLatM, daylMinM, dormhrM, dist2StreamD8M]

    if mgtFieldFile is not None:
        originalFiles.append(mgtFieldFile)
        outputList.append(mgtFieldMFile)

    for output in outputList:
        maskedFiles.append(workingDir + os.sep + output)

    maskFile, mask = GetMaskFromRaster(subbasinTauFile, workingDir)

    print ("Mask files...")
    n = len(originalFiles)
    # write mask config file
    configFile = "%s%s%s" % (workingDir, os.sep, FN_STATUS_MASKRASTERS)
    f = open(configFile, 'w')
    f.write(maskFile + "\n")
    f.write("%d\n" % (n,))
    for i in range(n):
        s = "%s\t%d\t%s\n" % (originalFiles[i], DEFAULT_NODATA, maskedFiles[i])
        f.write(s)
    f.close()

    # s = '"%s/mask_raster" %s' % (exeDir, configFile)
    # os.system(s)
    RunExternalCmd('"%s/mask_raster" %s' % (exeDir, configFile))


def GenerateSubbasins():
    statusFile = WORKING_DIR + os.sep + FN_STATUS_GENSUBBSN
    fStatus = open(statusFile, 'w')

    fStatus.write("%d,%s\n" % (10, "Masking subbasin files..."))
    fStatus.flush()
    MaskDEMFiles(WORKING_DIR, exeDir=CPP_PROGRAM_DIR)

    fStatus.write("%d,%s\n" % (50, "Output files..."))
    fStatus.flush()
    PostProcessTauDEM(WORKING_DIR)

    os.chdir(WORKING_DIR)
    src_srs = ReadRaster(dem).srs
    proj_srs = src_srs.ExportToProj4()
    # print proj_srs
    wgs84_srs = "EPSG:4326"

    def convert2GeoJson(jsonFile, src_srs, dst_srs, src_file):
        if os.path.exists(jsonFile):
            os.remove(jsonFile)
        if platform.system() == 'Windows':
            exepath = '"%s/Lib/site-packages/osgeo/ogr2ogr"' % sys.exec_prefix
        else:
            exepath = GetExecutableFullPath("ogr2ogr")
        # os.system(s)
        s = '%s -f GeoJSON -s_srs "%s" -t_srs %s %s %s' % \
            (exepath, src_srs, dst_srs, jsonFile, src_file)
        RunExternalCmd(s)

    geoJson_dict = {GEOJSON_REACH: WORKING_DIR + os.sep + DIR_NAME_REACH + os.sep + reachesOut,
                    GEOJSON_SUBBSN: WORKING_DIR + os.sep + DIR_NAME_SUBBSN + os.sep + subbasinVec,
                    GEOJSON_OUTLET: WORKING_DIR + os.sep + DIR_NAME_TAUDEM + os.sep + modifiedOutlet}
    for jsonName in geoJson_dict.keys():
        convert2GeoJson(jsonName, proj_srs, wgs84_srs,
                        geoJson_dict.get(jsonName))
    fStatus.write("%d,%s\n" % (100, "Finished!"))
    fStatus.close()
    ImportSubbasinStatistics()


def ImportSubbasinStatistics():
    '''
    Import subbasin numbers, outlet ID, etc. to MongoDB.
    :return:
    '''
    streamlinkR = WORKING_DIR + os.sep + streamLinkOut
    flowdirR = WORKING_DIR + os.sep + flowDirOut
    DIR_ITEMS = {}
    if (isTauDEM):
        DIR_ITEMS = {1: (0, 1),
                     8: (1, 1),
                     7: (1, 0),
                     6: (1, -1),
                     5: (0, -1),
                     4: (-1, -1),
                     3: (-1, 0),
                     2: (-1, 1)}
    else:
        DIR_ITEMS = {1: (0, 1),
                     2: (1, 1),
                     4: (1, 0),
                     8: (1, -1),
                     16: (0, -1),
                     32: (-1, -1),
                     64: (-1, 0),
                     128: (-1, 1)}
    streamlinkD = ReadRaster(streamlinkR)
    nodata = streamlinkD.noDataValue
    nrows = streamlinkD.nRows
    ncols = streamlinkD.nCols
    streamlinkData = streamlinkD.data
    maxSubbasinID = int(streamlinkD.GetMax())
    minSubbasinID = int(streamlinkD.GetMin())
    subbasinNum = numpy.unique(streamlinkData).size - 1
    # print maxSubbasinID, minSubbasinID, subbasinNum
    flowdirD = ReadRaster(flowdirR)
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
    # import parameters to MongoDB
    try:
        conn = MongoClient(HOSTNAME, PORT)
    except ConnectionFailure:
        sys.stderr.write("Could not connect to MongoDB: %s" % ConnectionFailure.message)
        sys.exit(1)
    db = conn[SpatialDBName]
    importStatsDict = {PARAM_NAME_OUTLETID: outletBsnID, PARAM_NAME_OUTLETROW: oRow, PARAM_NAME_OUTLETCOL: oCol,
                       PARAM_NAME_SUBBSNMAX: maxSubbasinID, PARAM_NAME_SUBBSNMIN: minSubbasinID,
                       PARAM_NAME_SUBBSNNUM: subbasinNum}
    for stat in importStatsDict.keys():
        dic = {PARAM_FLD_NAME.upper(): stat, PARAM_FLD_DESC.upper(): stat, PARAM_FLD_UNIT.upper(): "NONE",
               PARAM_FLD_MODS.upper(): "ALL", PARAM_FLD_VALUE.upper(): importStatsDict[stat],
               PARAM_FLD_IMPACT.upper(): DEFAULT_NODATA, PARAM_FLD_CHANGE.upper(): PARAM_CHANGE_NC,
               PARAM_FLD_MAX.upper(): DEFAULT_NODATA, PARAM_FLD_MIN.upper(): DEFAULT_NODATA,
               PARAM_FLD_USE.upper(): PARAM_USE_Y, Tag_DT_Type.upper(): "WATERSHED"}
        curfilter = {PARAM_FLD_NAME.upper(): dic[PARAM_FLD_NAME.upper()]}
        # print (dic, curfilter)
        db[DB_TAB_PARAMETERS.upper()].find_one_and_replace(curfilter, dic, upsert=True)
    db[DB_TAB_PARAMETERS.upper()].create_index(PARAM_FLD_NAME.upper())

    print ("Subbasin statistics imported!")


if __name__ == "__main__":
    LoadConfiguration(GetINIfile())
    GenerateSubbasins()
