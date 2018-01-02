#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Generate parameters of reaches
    @author   : Liangjun Zhu, Junzhi Liu
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-23  lj - reorganize as basic class
"""
import os
import sys
from math import sqrt

import networkx as nx
from osgeo.ogr import FieldDefn as ogr_FieldDefn
from osgeo.ogr import OFTInteger
from osgeo.ogr import Open as ogr_Open
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import UtilClass, FileClass
from pymongo import ASCENDING

from mpi_adjust_groups import adjust_group_result
from utility import UTIL_ZERO, MINI_SLOPE

sys.setrecursionlimit(10000)


class ImportReaches2Mongo(object):
    """Import reaches related parameters to MongoDB."""
    _TAB_REACH = 'REACHES'
    _LINKNO = 'LINKNO'
    _DSLINKNO = 'DSLINKNO'
    # Fields in _TAB_REACH
    _SUBBASIN = 'SUBBASINID'
    _NUMCELLS = 'NUM_CELLS'
    _GROUP = 'GROUP'
    _GROUPDIVIDED = 'GROUP_DIVIDE'
    _KMETIS = 'KMETIS'  # GROUP_KMETIS is too long for ArcGIS Shapefile
    _PMETIS = 'PMETIS'  # the same reason
    _DOWNSTREAM = 'DOWNSTREAM'
    _UPDOWN_ORDER = 'UP_DOWN_ORDER'
    _DOWNUP_ORDER = 'DOWN_UP_ORDER'
    _WIDTH = 'CH_WIDTH'
    _LENGTH = 'CH_LEN'
    _DEPTH = 'CH_DEPTH'
    _V0 = 'CH_V0'
    _AREA = 'CH_AREA'
    _SIDESLP = 'CH_SSLP'
    _MANNING = 'CH_N'
    _SLOPE = 'CH_SLP'
    _KBANK = 'CH_K_BANK'
    _KBED = 'CH_K_BED'
    _BC1 = 'BC1'
    _BC2 = 'BC2'
    _BC3 = 'BC3'
    _BC4 = 'BC4'
    _RK1 = 'RK1'
    _RK2 = 'RK2'
    _RK3 = 'RK3'
    _RK4 = 'RK4'
    _RS1 = 'RS1'
    _RS2 = 'RS2'
    _RS3 = 'RS3'
    _RS4 = 'RS4'
    _RS5 = 'RS5'
    # reach erosion related parameters, 2016-8-16, LJ
    _COVER = 'CH_COVER'  # -0.05 - 0.6
    _EROD = 'CH_EROD'  # -0.001 - 1
    # nutrient routing related parameters
    _DISOX = 'DISOX'  # 0-50 mg/L
    _BOD = 'BOD'  # 0-1000 mg/L
    _ALGAE = 'ALGAE'  # 0-200 mg/L
    _ORGN = 'ORGN'  # 0-100 mg/L
    _NH4 = 'NH4'  # 0-50 mg/L
    _NO2 = 'NO2'  # 0-100 mg/L
    _NO3 = 'NO3'  # 0-50 mg/L
    _ORGP = 'ORGP'  # 0-25 mg/L
    _SOLP = 'SOLP'  # 0-25 mg/L
    # groundwater related parameters
    _GWNO3 = 'GWNO3'  # 0-1000 mg/L
    _GWSOLP = 'GWSOLP'  # 0-1000 mg/L

    @staticmethod
    def get_subbasin_cell_count(subbsn_file):
        """Get cell number of each subbasin.
        Args:
            subbsn_file: subbasin raster file.

        Returns:
            subbasin cell count dict and cell width
        """
        num_dic = dict()
        wtsd_raster = RasterUtilClass.read_raster(subbsn_file)
        data = wtsd_raster.data
        xsize = wtsd_raster.nCols
        ysize = wtsd_raster.nRows
        dx = wtsd_raster.dx
        nodata_value = wtsd_raster.noDataValue
        for i in range(ysize):
            for j in range(xsize):
                k = int(data[i][j])
                if abs(k - nodata_value) > UTIL_ZERO:
                    num_dic[k] = num_dic.setdefault(k, 0) + 1
        return num_dic, dx

    @staticmethod
    def stream_orders_from_outlet_up(order_dic, g, node, order_num):
        """assign order from outlet to upstream subbasins
        Args:
            order_dic: result dict
            g: directed graphs object
            node: graph node
            order_num: stream order number
        """
        order_dic[node] = order_num
        for inNode in g.in_edges(node):
            ImportReaches2Mongo.stream_orders_from_outlet_up(order_dic, g, inNode[0], order_num + 1)

    @staticmethod
    def down_stream(reach_shp, is_taudem=True):
        """Construct stream order layers etc.
        Args:
            reach_shp: reach ESRI shapefile.
            is_taudem: is TauDEM or not, true is default.

        Returns:
            down_stream_dic: the key is stream id, and value is its downstream id
            downstream_up_order_dic: from outlet up stream dict
            upstream_down_order_dic: from source down stream dict
            depth_dic: stream depth dict
            slope_dic: stream slope dict
            width_dic: stream width dict
            len_dic: stream length dict
        """
        down_stream_dic = dict()
        depth_dic = dict()
        slope_dic = dict()
        width_dic = dict()
        len_dic = dict()
        ds_reach = ogr_Open(reach_shp)
        layer_reach = ds_reach.GetLayer(0)
        layer_def = layer_reach.GetLayerDefn()
        if not is_taudem:  # For ArcSWAT
            ImportReaches2Mongo._LINKNO = 'FROM_NODE'
            ImportReaches2Mongo._DSLINKNO = 'TO_NODE'
            ImportReaches2Mongo._SLOPE = 'Slo2'  # TauDEM: Slope (tan); ArcSWAT: Slo2 (100*tan)
            ImportReaches2Mongo._LENGTH = 'Len2'  # TauDEM: Length; ArcSWAT: Len2
        i_from = layer_def.GetFieldIndex(ImportReaches2Mongo._LINKNO)
        i_to = layer_def.GetFieldIndex(ImportReaches2Mongo._DSLINKNO)
        i_depth = layer_def.GetFieldIndex(ImportReaches2Mongo._DEPTH)
        i_slope = layer_def.GetFieldIndex('Slope')
        i_width = layer_def.GetFieldIndex(ImportReaches2Mongo._WIDTH)
        i_len = layer_def.GetFieldIndex('Length')

        g = nx.DiGraph()
        ft = layer_reach.GetNextFeature()
        while ft is not None:
            node_from = ft.GetFieldAsInteger(i_from)
            node_to = ft.GetFieldAsInteger(i_to)
            if i_depth > -1:
                depth_dic[node_from] = ft.GetFieldAsDouble(i_depth)
            else:
                depth_dic[node_from] = 1

            if i_depth > -1:
                slope_dic[node_from] = ft.GetFieldAsDouble(i_slope)
                if slope_dic[node_from] < MINI_SLOPE:
                    slope_dic[node_from] = MINI_SLOPE
            else:
                slope_dic[node_from] = MINI_SLOPE

            if i_width > -1:
                width_dic[node_from] = ft.GetFieldAsDouble(i_width)
            else:
                width_dic[node_from] = 10

            len_dic[node_from] = ft.GetFieldAsDouble(i_len)
            down_stream_dic[node_from] = node_to
            if node_to > 0:
                # print node_from, node_to
                g.add_edge(node_from, node_to)
            ft = layer_reach.GetNextFeature()

        # find outlet subbasin
        outlet = -1
        for node in g.nodes():
            if g.out_degree(node) == 0:
                outlet = node
        if outlet < 0:
            raise ValueError('Cannot find outlet subbasin ID, please check the '
                             'threshold for stream extraction!')
        print ('outlet subbasin:%d' % outlet)

        # assign order from outlet to upstream subbasins
        downstream_up_order_dic = dict()
        ImportReaches2Mongo.stream_orders_from_outlet_up(downstream_up_order_dic, g, outlet, 1)
        # find the maximum order number
        max_order = 0
        for k, v in downstream_up_order_dic.items():
            if v > max_order:
                max_order = v
        # reserve the order number
        for k, v in downstream_up_order_dic.items():
            downstream_up_order_dic[k] = max_order - v + 1

        # assign order from the source subbasins
        upstream_down_order_dic = dict()
        order_num = 1
        nodelist = g.nodes()
        while len(nodelist) != 0:
            nodelist = g.nodes()
            del_list = list()
            for node in nodelist:
                if g.in_degree(node) == 0:
                    upstream_down_order_dic[node] = order_num
                    del_list.append(node)
            for item in del_list:
                g.remove_node(item)
            order_num += 1

        return (down_stream_dic, downstream_up_order_dic, upstream_down_order_dic,
                depth_dic, slope_dic, width_dic, len_dic)

    @staticmethod
    def add_group_field(shp_file, subbasin_field_name, n, group_kmetis, group_pmetis, ns):
        """add group information to subbasin ESRI shapefile
        Args:
            shp_file: Subbasin Shapefile
            subbasin_field_name: field name of subbasin
            n: divide number
            group_kmetis: kmetis
            group_pmetis: pmetis
            ns: a list of the nodes in the graph

        Returns:
            group_dic: group dict
            group_dic_pmetis: pmetis dict
        """
        ds_reach = ogr_Open(shp_file, update=True)
        layer_reach = ds_reach.GetLayer(0)
        layer_def = layer_reach.GetLayerDefn()
        i_code = layer_def.GetFieldIndex(subbasin_field_name)
        i_group = layer_def.GetFieldIndex(ImportReaches2Mongo._GROUP)
        i_group_pmetis = layer_def.GetFieldIndex(ImportReaches2Mongo._PMETIS)
        if i_group < 0:
            new_field = ogr_FieldDefn(ImportReaches2Mongo._GROUP, OFTInteger)
            layer_reach.CreateField(new_field)
        if i_group_pmetis < 0:
            new_field = ogr_FieldDefn(ImportReaches2Mongo._PMETIS, OFTInteger)
            layer_reach.CreateField(new_field)
            # grid_code:feature map
        ftmap = dict()
        layer_reach.ResetReading()
        ft = layer_reach.GetNextFeature()
        while ft is not None:
            tmpid = ft.GetFieldAsInteger(i_code)
            ftmap[tmpid] = ft
            ft = layer_reach.GetNextFeature()

        group_dic = dict()
        group_dic_pmetis = dict()
        i = 0
        for node in ns:
            group_dic[node] = group_kmetis[i]
            group_dic_pmetis[node] = group_pmetis[i]
            ftmap[node].SetField(ImportReaches2Mongo._GROUP, group_kmetis[i])
            ftmap[node].SetField(ImportReaches2Mongo._PMETIS, group_pmetis[i])
            layer_reach.SetFeature(ftmap[node])
            i += 1

        layer_reach.SyncToDisk()
        ds_reach.Destroy()
        del ds_reach

        # copy the reach file to new file
        prefix = os.path.splitext(shp_file)[0]
        dstfile = prefix + "_" + str(n) + ".shp"
        FileClass.copy_files(shp_file, dstfile)
        return group_dic, group_dic_pmetis

    @staticmethod
    def generate_reach_table(cfg, maindb):
        """generate reaches table"""
        # remove the older reaches collection if existed
        maindb.drop_collection(ImportReaches2Mongo._TAB_REACH)

        area_dic, dx = ImportReaches2Mongo.get_subbasin_cell_count(cfg.spatials.subbsn)
        (downStreamDic, downstreamUpOrderDic, upstreamDownOrderDic, depthDic,
         slopeDic, widthDic, lenDic) = ImportReaches2Mongo.down_stream(cfg.vecs.reach)
        # for k in downStreamDic:
        #     print (k, downStreamDic[k])

        g = nx.DiGraph()
        for k in downStreamDic:
            if downStreamDic[k] > 0:
                g.add_edge(k, downStreamDic[k])

        ns = g.nodes()

        # construct the METIS input file
        UtilClass.mkdir(cfg.dirs.metis)
        metis_input = r'%s/metis.txt' % cfg.dirs.metis
        f = open(metis_input, 'w')
        f.write(str(len(ns)) + '\t' + str(len(g.edges())) + '\t' + '010\t1\n')
        for node in ns:
            if node <= 0:
                continue
            f.write(str(area_dic[node]) + '\t')
            for e in g.out_edges(node):
                if e[1] > 0:
                    f.write(str(e[1]) + '\t')
            for e in g.in_edges(node):
                if e[0] > 0:
                    f.write(str(e[0]) + '\t')
            f.write('\n')
        f.close()

        # execute metis
        nlist = [1, ]
        if cfg.cluster:
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
        # nlist should be less than the number of subbasin, otherwise it will make nonsense.
        # by LJ
        nlist = [x for x in nlist if x <= max(ns)]

        # interpolation among different stream orders
        min_manning = 0.035
        max_manning = 0.075

        min_order = 1
        max_order = 1
        for k, up_order in upstreamDownOrderDic.items():
            if up_order > max_order:
                max_order = up_order

        dic_manning = dict()
        a = (max_manning - min_manning) / (max_order - min_order)
        for tmpid in downStreamDic.keys():
            dic_manning[tmpid] = max_manning - a * (upstreamDownOrderDic[tmpid] - min_order)

        def import_reach_info(n, down_stream_dic, gdic_k, gdic_p):
            """import reach info"""
            for tmpid2 in down_stream_dic:
                dic = dict()
                dic[ImportReaches2Mongo._SUBBASIN] = tmpid2
                dic[ImportReaches2Mongo._DOWNSTREAM] = down_stream_dic[tmpid2]
                dic[ImportReaches2Mongo._UPDOWN_ORDER] = upstreamDownOrderDic[tmpid2]
                dic[ImportReaches2Mongo._DOWNUP_ORDER] = downstreamUpOrderDic[tmpid2]
                dic[ImportReaches2Mongo._MANNING] = dic_manning[tmpid2]
                dic[ImportReaches2Mongo._SLOPE] = slopeDic[tmpid2]
                dic[ImportReaches2Mongo._V0] = sqrt(slopeDic[tmpid2]) * \
                                               pow(depthDic[tmpid2], 2. / 3.) / \
                                               dic[ImportReaches2Mongo._MANNING]
                dic[ImportReaches2Mongo._NUMCELLS] = area_dic[tmpid2]
                if n == 1:
                    dic[ImportReaches2Mongo._GROUP] = n
                else:
                    dic[ImportReaches2Mongo._KMETIS] = gdic_k[tmpid2]
                    dic[ImportReaches2Mongo._PMETIS] = gdic_p[tmpid2]
                dic[ImportReaches2Mongo._GROUPDIVIDED] = n
                dic[ImportReaches2Mongo._WIDTH] = widthDic[tmpid2]
                dic[ImportReaches2Mongo._LENGTH] = lenDic[tmpid2]
                dic[ImportReaches2Mongo._DEPTH] = depthDic[tmpid2]
                dic[ImportReaches2Mongo._AREA] = area_dic[tmpid2] * dx * dx
                dic[ImportReaches2Mongo._SIDESLP] = 2.
                dic[ImportReaches2Mongo._KBANK] = 20.
                dic[ImportReaches2Mongo._KBED] = 0.5
                dic[ImportReaches2Mongo._BC1] = 0.55
                dic[ImportReaches2Mongo._BC2] = 1.1
                dic[ImportReaches2Mongo._BC3] = 0.21
                dic[ImportReaches2Mongo._BC4] = 0.35
                dic[ImportReaches2Mongo._RK1] = 1.71
                dic[ImportReaches2Mongo._RK2] = 50
                dic[ImportReaches2Mongo._RK3] = 0.36
                dic[ImportReaches2Mongo._RK4] = 2
                dic[ImportReaches2Mongo._RS1] = 1
                dic[ImportReaches2Mongo._RS2] = 0.05
                dic[ImportReaches2Mongo._RS3] = 0.5
                dic[ImportReaches2Mongo._RS4] = 0.05
                dic[ImportReaches2Mongo._RS5] = 0.05
                dic[ImportReaches2Mongo._COVER] = 0.1
                dic[ImportReaches2Mongo._EROD] = 0.1
                dic[ImportReaches2Mongo._DISOX] = 10
                dic[ImportReaches2Mongo._BOD] = 10
                dic[ImportReaches2Mongo._ALGAE] = 0
                dic[ImportReaches2Mongo._ORGN] = 0
                dic[ImportReaches2Mongo._NH4] = 0
                dic[ImportReaches2Mongo._NO2] = 0
                dic[ImportReaches2Mongo._NO3] = 0
                dic[ImportReaches2Mongo._ORGP] = 0
                dic[ImportReaches2Mongo._SOLP] = 0
                dic[ImportReaches2Mongo._GWNO3] = 0
                dic[ImportReaches2Mongo._GWSOLP] = 0

                cur_filter = {ImportReaches2Mongo._SUBBASIN: tmpid2}
                maindb[ImportReaches2Mongo._TAB_REACH].find_one_and_replace(cur_filter, dic,
                                                                            upsert=True)

        for n in nlist:
            print ('divide number: ', n)
            if n == 1:
                import_reach_info(n, downStreamDic, {}, {})
                continue

            # for cluster, based on kmetis
            str_command = '"%s/gpmetis" %s %d' % (cfg.seims_bin, metis_input, n)
            result = UtilClass.run_command(str_command)
            f_metis_output = open('%s/kmetisResult%d.txt' % (cfg.dirs.metis, n), 'w')
            for line in result:
                f_metis_output.write(line)
            f_metis_output.close()

            metis_output = '%s.part.%d' % (metis_input, n)
            f = open(metis_output)
            lines = f.readlines()
            group_kmetis = [int(item) for item in lines]
            f.close()
            adjust_group_result(g, area_dic, group_kmetis, n)

            # pmetis
            str_command = '"%s/gpmetis" -ptype=rb %s %d' % (cfg.seims_bin, metis_input, n)
            result = UtilClass.run_command(str_command)
            f_metis_output = open('%s/pmetisResult%d.txt' % (cfg.dirs.metis, n), 'w')
            for line in result:
                f_metis_output.write(line)
            f_metis_output.close()

            f = open(metis_output)
            lines = f.readlines()
            group_pmetis = [int(item) for item in lines]
            f.close()
            adjust_group_result(g, area_dic, group_pmetis, n)

            group_dic_k, group_dic_p = \
                ImportReaches2Mongo.add_group_field(cfg.vecs.reach, ImportReaches2Mongo._LINKNO,
                                                    n, group_kmetis, group_pmetis, ns)
            group_dic_k, group_dic_p = \
                ImportReaches2Mongo.add_group_field(cfg.vecs.subbsn, ImportReaches2Mongo._SUBBASIN,
                                                    n, group_kmetis, group_pmetis, ns)

            import_reach_info(n, downStreamDic, group_dic_k, group_dic_p)
        maindb[ImportReaches2Mongo._TAB_REACH].create_index([(ImportReaches2Mongo._SUBBASIN,
                                                              ASCENDING),
                                                             (ImportReaches2Mongo._GROUPDIVIDED,
                                                              ASCENDING)])


def main():
    """TEST CODE"""
    from config import parse_ini_configuration
    from db_mongodb import ConnectMongoDB
    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    maindb = conn[seims_cfg.spatial_db]

    ImportReaches2Mongo.generate_reach_table(seims_cfg, maindb)

    client.close()


if __name__ == "__main__":
    main()
