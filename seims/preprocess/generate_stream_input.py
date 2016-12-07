#! /usr/bin/env python
# coding=utf-8
# Gernerate parameters of reaches
# Author: Junzhi Liu
# Revised: Liang-Jun Zhu
#

from osgeo import ogr

import networkx as nx
import pymongo
from pymongo import MongoClient
from pymongo.errors import ConnectionFailure

from adjust_groups import *
from config import *
from util import *

sys.setrecursionlimit(10000)


def gridNumber(watershedFile):
    numDic = {}
    ds = gdal.Open(watershedFile)
    band = ds.GetRasterBand(1)
    data = band.ReadAsArray()
    xsize = band.XSize
    ysize = band.YSize
    noDataValue = band.GetNoDataValue()
    for i in range(ysize):
        for j in range(xsize):
            k = int(data[i][j])
            if (abs(k - noDataValue) > UTIL_ZERO):
                numDic[k] = numDic.setdefault(k, 0) + 1
    return numDic, ds.GetGeoTransform()[1]


def DownstreamUpOrder(orderDic, g, node, orderNum):
    orderDic[node] = orderNum
    for inNode in g.in_edges(node):
        DownstreamUpOrder(orderDic, g, inNode[0], orderNum + 1)


def downStream(reachFile):
    downStreamDic = {}
    depthDic = {}
    slopeDic = {}
    widthDic = {}
    lenDic = {}
    dsReach = ogr.Open(reachFile)
    layerReach = dsReach.GetLayer(0)
    layerDef = layerReach.GetLayerDefn()
    # TauDEM: LINKNO; ArcSWAT: FROM_NODE
    iFrom = layerDef.GetFieldIndex(FLD_LINKNO)
    # TauDEM: DSLINKNO; ArcSWAT: TO_NODE
    iTo = layerDef.GetFieldIndex(FLD_DSLINKNO)
    iDepth = layerDef.GetFieldIndex(REACH_DEPTH)
    # TauDEM: Slope (tan); ArcSWAT: Slo2 (100*tan)
    iSlope = layerDef.GetFieldIndex(REACH_SLOPE)
    iWidth = layerDef.GetFieldIndex(REACH_WIDTH)
    # TauDEM: Length; ArcSWAT: Len2
    iLen = layerDef.GetFieldIndex(REACH_LENGTH)

    g = nx.DiGraph()
    ft = layerReach.GetNextFeature()
    while ft is not None:
        nodeFrom = ft.GetFieldAsInteger(iFrom)
        nodeTo = ft.GetFieldAsInteger(iTo)
        if iDepth > -1:
            depthDic[nodeFrom] = ft.GetFieldAsDouble(iDepth)
        else:
            depthDic[nodeFrom] = 1

        if iDepth > -1:
            slopeDic[nodeFrom] = ft.GetFieldAsDouble(iSlope)
            if slopeDic[nodeFrom] < MINI_SLOPE:
                slopeDic[nodeFrom] = MINI_SLOPE
        else:
            slopeDic[nodeFrom] = MINI_SLOPE

        if iWidth > -1:
            widthDic[nodeFrom] = ft.GetFieldAsDouble(iWidth)
        else:
            widthDic[nodeFrom] = 10

        lenDic[nodeFrom] = ft.GetFieldAsDouble(iLen)
        downStreamDic[nodeFrom] = nodeTo
        if nodeTo > 0:
            # print nodeFrom, nodeTo
            g.add_edge(nodeFrom, nodeTo)
        ft = layerReach.GetNextFeature()

    # find outlet subbasin
    outlet = -1
    for node in g.nodes():
        if g.out_degree(node) == 0:
            outlet = node
    if outlet < 0:
        raise ValueError("Can't find outlet subbasin ID, please check!")
    print 'outlet subbasin:%d' % outlet

    # assign order from outlet to upstream subbasins
    downstreamUpOrderDic = {}
    DownstreamUpOrder(downstreamUpOrderDic, g, outlet, 1)
    # find the maximum order nubmer
    maxOrder = 0
    for k in downstreamUpOrderDic.keys():
        if (downstreamUpOrderDic[k] > maxOrder):
            maxOrder = downstreamUpOrderDic[k]
    # reserve the order number
    for k in downstreamUpOrderDic.keys():
        downstreamUpOrderDic[k] = maxOrder - downstreamUpOrderDic[k] + 1

    # assign order from the source subbasins
    upstreamDownOrderDic = {}
    orderNum = 1
    nodelist = g.nodes()
    while (len(nodelist) != 0):
        nodelist = g.nodes()
        delList = []
        for node in nodelist:
            if (g.in_degree(node) == 0):
                upstreamDownOrderDic[node] = orderNum
                delList.append(node)
        for item in delList:
            g.remove_node(item)
        orderNum = orderNum + 1

    return downStreamDic, downstreamUpOrderDic, upstreamDownOrderDic, \
        depthDic, slopeDic, widthDic, lenDic


def add_group_field(shpFile, subbasinFieldName, n, groupKmetis, groupPmetis, ns):
    dsReach = ogr.Open(shpFile, update=True)
    layerReach = dsReach.GetLayer(0)
    layerDef = layerReach.GetLayerDefn()
    iCode = layerDef.GetFieldIndex(subbasinFieldName)
    iGroup = layerDef.GetFieldIndex(REACH_GROUP)
    iGroupPmetis = layerDef.GetFieldIndex(REACH_PMETIS)
    if (iGroup < 0):
        new_field = ogr.FieldDefn(REACH_GROUP, ogr.OFTInteger)
        layerReach.CreateField(new_field)
    if (iGroupPmetis < 0):
        new_field = ogr.FieldDefn(REACH_PMETIS, ogr.OFTInteger)
        layerReach.CreateField(new_field)
        # grid_code:feature map
    ftmap = {}
    layerReach.ResetReading()
    ft = layerReach.GetNextFeature()
    while ft is not None:
        id = ft.GetFieldAsInteger(iCode)
        ftmap[id] = ft
        ft = layerReach.GetNextFeature()

    groupDic = {}
    groupDicPmetis = {}
    i = 0
    for node in ns:
        groupDic[node] = groupKmetis[i]
        groupDicPmetis[node] = groupPmetis[i]
        ftmap[node].SetField(REACH_GROUP, groupKmetis[i])
        ftmap[node].SetField(REACH_PMETIS, groupPmetis[i])
        layerReach.SetFeature(ftmap[node])
        i = i + 1

    layerReach.SyncToDisk()
    dsReach.Destroy()
    del dsReach

    # copy the reach file to new file
    for ext in shp_ext_list:
        prefix = os.path.splitext(shpFile)[0]
        src = prefix + ext
        if os.path.isfile(src): # Is the ArcGIS Shapefile with the extension existed
            dst = prefix + "_" + str(n) + ext
            if os.path.exists(dst):
                os.remove(dst)
            shutil.copy(src, dst)

    return groupDic, groupDicPmetis


def GenerateReachTable(folder, db, forCluster):
    watershedFile = folder + os.sep + subbasinOut
    reachFile = folder + os.sep + DIR_NAME_REACH + os.sep + reachesOut
    subbasinFile = folder + os.sep + DIR_NAME_SUBBSN + os.sep + subbasinVec
    # print reachFile

    areaDic, dx = gridNumber(watershedFile)
    downStreamDic, downstreamUpOrderDic, upstreamDownOrderDic, \
        depthDic, slopeDic, widthDic, lenDic = downStream(reachFile)
    # for k in downStreamDic:
    # print k, downStreamDic[k]

    g = nx.DiGraph()
    for k in downStreamDic:
        if (downStreamDic[k] > 0):
            g.add_edge(k, downStreamDic[k])

    ns = g.nodes()

    # consturct the METIS input file
    metisFolder = folder + os.sep + DIR_NAME_METISOUT
    if not os.path.exists(metisFolder):
        os.mkdir(metisFolder)
    metisInput = r'%s/metis.txt' % (metisFolder)
    f = open(metisInput, 'w')
    f.write(str(len(ns)) + "\t" + str(len(g.edges())) + "\t" + "010\t1\n")
    for node in ns:
        if node <= 0:
            continue
        f.write(str(areaDic[node]) + "\t")
        for e in g.out_edges(node):
            if e[1] > 0:
                f.write(str(e[1]) + "\t")
        for e in g.in_edges(node):
            if e[0] > 0:
                f.write(str(e[0]) + "\t")
        f.write("\n")
    f.close()

    # execute metis
    nlist = [1, ]
    if (forCluster):
        a = [1, 2, 3, 6]
        a2 = [12 * pow(2, i) for i in range(8)]
        a.extend(a2)

        b = [1, 3]
        b2 = [i / 2 for i in a2]
        b.extend(b2)

        c = [1, 2]
        c2 = [i / 3 for i in a2]
        c.extend(c2)

        d = [1]
        d2 = [i / 6 for i in a2]
        d.extend(d2)

        e = [i / 12 for i in a2]

        nlist = a + b + c + d + e
        nlist.extend(range(1, 129))
        # nlist.extend([576, 288, 512, 258, 172])
        nlist = list(set(nlist))
        nlist.sort()
    # nlist should be less than the number of subbasin, otherwise it will make nonsense. by LJ
    nlist = [x for x in nlist if x <= max(ns)]

    # interpolation among different stream orders
    minManning = 0.035
    maxManning = 0.075

    minOrder = 1
    maxOrder = 1
    for k in upstreamDownOrderDic.keys():
        if (upstreamDownOrderDic[k] > maxOrder):
            maxOrder = upstreamDownOrderDic[k]

    dicManning = {}
    a = (maxManning - minManning) / (maxOrder - minOrder)
    for id in downStreamDic.keys():
        dicManning[id] = maxManning - a * (upstreamDownOrderDic[id] - minOrder)

    def importReachInfo(n, downStreamDic, groupDicK, groupDicP):
        for id in downStreamDic:
            dic = {}
            dic[REACH_SUBBASIN.upper()] = id
            dic[REACH_DOWNSTREAM.upper()] = downStreamDic[id]
            dic[REACH_UPDOWN_ORDER.upper()] = upstreamDownOrderDic[id]
            dic[REACH_DOWNUP_ORDER.upper()] = downstreamUpOrderDic[id]
            dic[REACH_MANNING.upper()] = dicManning[id]
            dic[REACH_SLOPE] = slopeDic[id]
            dic[REACH_V0.upper()] = math.sqrt(slopeDic[id]) * math.pow(depthDic[id], 2. / 3) \
                / dic[REACH_MANNING.upper()]
            dic[REACH_NUMCELLS.upper()] = areaDic[id]
            if (n == 1):
                dic[REACH_GROUP.upper()] = n
            else:
                dic[REACH_KMETIS.upper()] = groupDicK[id]
                dic[REACH_PMETIS.upper()] = groupDicP[id]
            dic[REACH_GROUPDIVIDED.upper()] = n
            dic[REACH_WIDTH.upper()] = widthDic[id]
            dic[REACH_LENGTH.upper()] = lenDic[id]
            dic[REACH_DEPTH.upper()] = depthDic[id]
            dic[REACH_AREA.upper()] = areaDic[id] * dx * dx
            dic[REACH_SIDESLP.upper()] = 2.
            dic[REACH_BC1.upper()] = 0.55
            dic[REACH_BC2.upper()] = 1.1
            dic[REACH_BC3.upper()] = 0.21
            dic[REACH_BC4.upper()] = 0.35
            dic[REACH_RK1.upper()] = 1.71
            dic[REACH_RK2.upper()] = 50
            dic[REACH_RK3.upper()] = 0.36
            dic[REACH_RK4.upper()] = 2
            dic[REACH_RS1.upper()] = 1
            dic[REACH_RS2.upper()] = 0.05
            dic[REACH_RS3.upper()] = 0.5
            dic[REACH_RS4.upper()] = 0.05
            dic[REACH_RS5.upper()] = 0.05
            dic[REACH_COVER.upper()] = 0.1
            dic[REACH_EROD.upper()] = 0.1
            dic[REACH_DISOX.upper()] = 10
            dic[REACH_BOD.upper()] = 10
            dic[REACH_ALGAE.upper()] = 0  # 10
            dic[REACH_ORGN.upper()] = 0  # 10
            dic[REACH_NH4.upper()] = 0  # 1 # 8.
            dic[REACH_NO2.upper()] = 0  # 0.
            dic[REACH_NO3.upper()] = 0  # 1 # 8.
            dic[REACH_ORGP.upper()] = 0  # 10.
            dic[REACH_SOLP.upper()] = 0  # 0.1 # 0.5
            dic[REACH_GWNO3.upper()] = 0  # 10.
            dic[REACH_GWSOLP.upper()] = 0  # 10.

            curFilter = {REACH_SUBBASIN.upper(): id}
            db[DB_TAB_REACH.upper()].find_one_and_replace(curFilter, dic, upsert=True)

    for n in nlist:
        print 'divide number: ', n
        if (n == 1):
            importReachInfo(n, downStreamDic, {}, {})
            continue

        # for cluster, based on kmetis
        strCommand = '"%s/gpmetis" %s %d' % (METIS_DIR, metisInput, n)
        # result = os.popen(strCommand)
        result = RunExternalCmd(strCommand)
        fMetisOutput = open('%s/kmetisResult%d.txt' % (metisFolder, n), 'w')
        # for line in result.readlines():
        for line in result:
            fMetisOutput.write(line)
        fMetisOutput.close()

        metisOutput = "%s.part.%d" % (metisInput, n)
        f = open(metisOutput)
        lines = f.readlines()
        groupKmetis = [int(item) for item in lines]
        f.close()
        AdjustGroupResult(g, areaDic, groupKmetis, n)

        # pmetis
        strCommand = '"%s/gpmetis" -ptype=rb %s %d' % (METIS_DIR, metisInput, n)
        result = RunExternalCmd(strCommand)
        # result = os.popen(strCommand)
        fMetisOutput = open('%s/pmetisResult%d.txt' % (metisFolder, n), 'w')
        # for line in result.readlines():
        for line in result:
            fMetisOutput.write(line)
        fMetisOutput.close()

        f = open(metisOutput)
        lines = f.readlines()
        groupPmetis = [int(item) for item in lines]
        f.close()
        AdjustGroupResult(g, areaDic, groupPmetis, n)

        groupDicK, groupDicP = add_group_field(reachFile, FLD_LINKNO.upper(), n, groupKmetis, groupPmetis, ns)
        groupDicK, groupDicP = add_group_field(subbasinFile, REACH_SUBBASIN.upper(), n, groupKmetis, groupPmetis, ns)

        importReachInfo(n, downStreamDic, groupDicK, groupDicP)
    db[DB_TAB_REACH.upper()].create_index([(REACH_SUBBASIN.upper(), pymongo.ASCENDING),
                                           (REACH_GROUPDIVIDED.upper(), pymongo.ASCENDING)])

    print 'The reaches table is generated!'


# TEST CODE
if __name__ == "__main__":
    try:
        conn = MongoClient(host=HOSTNAME, port=PORT)
    except ConnectionFailure, e:
        sys.stderr.write("Could not connect to MongoDB: %s" % e)
        sys.exit(1)
    db = conn[SpatialDBName]
    if forCluster:
        GenerateReachTable(WORKING_DIR, db, True)
    else:
        GenerateReachTable(WORKING_DIR, db, False)
