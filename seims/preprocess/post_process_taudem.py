#! /usr/bin/env python
# coding=utf-8
# Post process of TauDEM
#   1. convert subbasin raster to polygon shapefile
#   2. add width and default depth to reach.shp
# @Author: Junzhi Liu, 2012-4-12
# @Revised: Liang-Jun Zhu, 2016-7-7
#
import platform
from osgeo import ogr

from chwidth import chwidth
from config import *
from util import *

def GenerateSubbasinVector(subbasinRaster, subbasinVector, layerName, fieldName):
    RemoveShpFile(subbasinVector)
    # raster to polygon vector
    if platform.system() == 'Windows':
        exepath = '"%s/Scripts/gdal_polygonize.py"' % sys.exec_prefix
    else:
        exepath = GetExecutableFullPath("gdal_polygonize.py")
    strCmd = '%s -f "ESRI Shapefile" %s %s %s %s' % (exepath, subbasinRaster, subbasinVector, layerName, fieldName)
    print strCmd
    # os.system(strCmd)
    process = subprocess.Popen(strCmd, shell=True, stdout=subprocess.PIPE)
    print process.stdout.readlines()


def SerializeStreamNet(streamNetFile, outputReachFile):
    CopyShpFile(streamNetFile, outputReachFile)

    dsReach = ogr.Open(outputReachFile, update=True)
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
            if reachLen < UTIL_ZERO:
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
        if not id in idMap.keys():
            layerReach.DeleteFeature(ft.GetFID())
            ft = layerReach.GetNextFeature()
            continue

        dsId = ft.GetFieldAsInteger(iLinkDownSlope)
        dsId = outputDic.get(dsId, dsId)
        dsId = outputDic.get(dsId, dsId)

        ft.SetField(FLD_LINKNO, idMap[id])
        if dsId in idMap.keys():
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


def SerializeSubbasin(subbasinFile, streamRasterFile, idMap,
                      outputSubbasinFile, outputStreamLinkFile):
    subbasin = ReadRaster(subbasinFile)
    nRows = subbasin.nRows
    nCols = subbasin.nCols
    noDataValue = subbasin.noDataValue
    data = subbasin.data

    streamRaster = ReadRaster(streamRasterFile)
    dataStream = streamRaster.data
    noDataValueStream = streamRaster.noDataValue
    # print noDataValueStream

    outputSubbasin = numpy.zeros((nRows, nCols))
    outputStream = numpy.zeros((nRows, nCols))
    n = len(idMap)
    print "number of reaches: ", n
    for i in range(nRows):
        for j in range(nCols):
            if abs(data[i][j] - noDataValue) < UTIL_ZERO:
                outputSubbasin[i][j] = noDataValue
            else:
                # error if the outlet subbasin contains only one grid, i.e.,
                # there is no reach for this subbasin
                outputSubbasin[i][j] = idMap[int(data[i][j])]
            if dataStream[i][j] < UTIL_ZERO:
                outputStream[i][j] = noDataValueStream
            else:
                outputStream[i][j] = outputSubbasin[i][j]

    WriteGTiffFile(outputSubbasinFile, nRows, nCols, outputSubbasin,
                   subbasin.geotrans, subbasin.srs, noDataValue, gdal.GDT_Int32)
    WriteGTiffFile(outputStreamLinkFile, nRows, nCols, outputStream,
                   streamRaster.geotrans, streamRaster.srs, noDataValue, gdal.GDT_Int32)


def ChangeFlowDir(flowDirFileTau, flowDirFileEsri):
    # flowDirFileTau is float
    dirMap = {1.: 1.,
              2.: 128.,
              3.: 64.,
              4.: 32.,
              5.: 16.,
              6.: 8.,
              7.: 4.,
              8.: 2.}
    replaceByDict(flowDirFileTau, dirMap, flowDirFileEsri)


def AddWidthToReach(reachFile, stramLinkFile, width):
    streamLink = ReadRaster(stramLinkFile)
    nRows = streamLink.nRows
    nCols = streamLink.nCols
    noDataValue = streamLink.noDataValue
    dataStream = streamLink.data

    chWidthDic = {}
    chNumDic = {}

    for i in range(nRows):
        for j in range(nCols):
            if abs(dataStream[i][j] - noDataValue) > UTIL_ZERO:
                id = int(dataStream[i][j])
                chNumDic.setdefault(id, 0)
                chWidthDic.setdefault(id, 0)
                chNumDic[id] = chNumDic[id] + 1
                chWidthDic[id] = chWidthDic[id] + width[i][j]

    for k in chNumDic.keys():
        chWidthDic[k] = chWidthDic[k] / chNumDic[k]

    # add channel width field to reach shp file
    dsReach = ogr.Open(reachFile, update=True)
    layerReach = dsReach.GetLayer(0)
    layerDef = layerReach.GetLayerDefn()
    iLink = layerDef.GetFieldIndex(FLD_LINKNO)
    iWidth = layerDef.GetFieldIndex(REACH_WIDTH)
    iDepth = layerDef.GetFieldIndex(REACH_DEPTH)
    if (iWidth < 0):
        new_field = ogr.FieldDefn(REACH_WIDTH, ogr.OFTReal)
        layerReach.CreateField(new_field)
    if (iDepth < 0):
        new_field = ogr.FieldDefn(REACH_DEPTH, ogr.OFTReal)
        layerReach.CreateField(new_field)
        # grid_code:feature map
    # ftmap = {}
    layerReach.ResetReading()
    ft = layerReach.GetNextFeature()
    while ft is not None:
        id = ft.GetFieldAsInteger(iLink)
        w = 1
        if id in chWidthDic.keys():
            w = chWidthDic[id]
        ft.SetField(REACH_WIDTH, w)
        ft.SetField(REACH_DEPTH, default_reach_depth)
        layerReach.SetFeature(ft)
        ft = layerReach.GetNextFeature()

    layerReach.SyncToDisk()
    dsReach.Destroy()
    del dsReach


def PostProcessTauDEM(dstdir):
    tauDir = dstdir + os.sep + DIR_NAME_TAUDEM
    streamNetFile = tauDir + os.sep + streamNet
    subbasinFile = tauDir + os.sep + subbasinM
    flowDirFileTau = tauDir + os.sep + flowDirM
    streamRasterFile = tauDir + os.sep + streamRasterM

    reachDir = dstdir + os.sep + DIR_NAME_REACH
    if not os.path.exists(reachDir):
        os.mkdir(reachDir)

    outputReachFile = reachDir + os.sep + reachesOut
    outputSubbasinFile = dstdir + os.sep + subbasinOut
    outputFlowDirFile = dstdir + os.sep + flowDirOut
    outputStreamLinkFile = dstdir + os.sep + streamLinkOut

    subbasinDir = dstdir + os.sep + DIR_NAME_SUBBSN
    if not os.path.exists(subbasinDir):
        os.mkdir(subbasinDir)
    subbasinVectorFile = subbasinDir + os.sep + subbasinVec

    idMap = SerializeStreamNet(streamNetFile, outputReachFile)
    SerializeSubbasin(subbasinFile, streamRasterFile, idMap,
                      outputSubbasinFile, outputStreamLinkFile)
    # Change TauDEM code to ArcGIS. Now, it is deprecated, By LJ.
    if(isTauDEM):
        shutil.copy(flowDirFileTau, outputFlowDirFile)
    else:
        ChangeFlowDir(flowDirFileTau, outputFlowDirFile)

    accFile = dstdir + os.sep + accM
    chwidthFile = dstdir + os.sep + chwidthName
    width = chwidth(accFile, chwidthFile)
    AddWidthToReach(outputReachFile, outputStreamLinkFile, width)

    print "Generating subbasin vector..."
    GenerateSubbasinVector(outputSubbasinFile, subbasinVectorFile, "subbasin", FLD_SUBBASINID)

    maskFile = dstdir + os.sep + mask_to_ext
    basinVector = dstdir + os.sep + basinVec
    print "Generating basin vector..."
    GenerateSubbasinVector(maskFile, basinVector, "basin", FLD_BASINID)


if __name__ == '__main__':
    LoadConfiguration(GetINIfile())
    PostProcessTauDEM(WORKING_DIR)
