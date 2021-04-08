"""Generate parameters of reaches.

    This script should be updated with the following files/code simultaneously.\n
      1. seims/preprocess/database/model_param_ini.csv: Emend initial parameters;\n
      2. seims/src/seims_main/base/data/clsReach.h(.cpp): Update the reading and checking of data.

    @author   : Liangjun Zhu, Junzhi Liu
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-23  lj - reorganize as basic class
                18-02-08  lj - compatible with Python3.\n
                18-08-13  lj - add erosion related parameters according to readrte.f of SWAT.\n
"""
from __future__ import absolute_import, unicode_literals

import shutil
import os
import sys
from io import open
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

import numpy
from typing import Union, Dict, AnyStr
import networkx as nx
from osgeo.ogr import FieldDefn as ogr_FieldDefn
from osgeo.ogr import OFTInteger
from osgeo.ogr import Open as ogr_Open
from pygeoc.raster import RasterUtilClass
from pygeoc.utils import UtilClass, FileClass
from pymongo import ASCENDING

from utility import UTIL_ZERO, MINI_SLOPE


class ImportReaches2Mongo(object):
    """Import reaches related parameters to MongoDB."""
    # Spatial parameters
    _TAB_REACH = 'REACHES'
    _LINKNO = 'LINKNO'
    _DSLINKNO = 'DSLINKNO'  # Fields in _TAB_REACH
    _SUBBASIN = 'SUBBASINID'
    _NUMCELLS = 'NUM_CELLS'
    _GROUP = 'GROUP'  # group divided, e.g., '1,2,3,4'
    _KMETIS = 'KMETIS'  # group id corresponding to group by kmetis, e.g., '0,0,2,1'.
    _PMETIS = 'PMETIS'  # group id corresponding to group by pmetis, e.g., '1,1,0,3'
    _DOWNSTREAM = 'DOWNSTREAM'
    _UPDOWN_ORDER = 'UP_DOWN_ORDER'
    _DOWNUP_ORDER = 'DOWN_UP_ORDER'
    _WIDTH = 'CH_WIDTH'
    _LENGTH = 'CH_LEN'
    _DEPTH = 'CH_DEPTH'
    _WDRATIO = 'CH_WDRATIO'
    _AREA = 'CH_AREA'
    _SIDESLP = 'CH_SSLP'  # H/V. 0: vertical band, 5: bank with gentle side slope. chside in SWAT
    _SLOPE = 'CH_SLP'
    # Hydrological related parameters
    _MANNING = 'CH_N'  # Manning's "n" value
    _BEDK = 'CH_BED_K'  # effective hydraulic conductivity, mm/hr. In SWAT, only ch_k are provided.
    _BNKK = 'CH_BNK_K'
    # Erosion related parameters
    _BEDBD = 'CH_BED_BD'  # bulk density of channel bed sediment (1.1-1.9), g/cm^3
    _BNKBD = 'CH_BNK_BD'  # bulk density of channel bank sediment (1.1-1.9), g/cm^3
    _BEDCOV = 'CH_BED_COV'  # Channel bed cover factor (0-1), ch_cov2 in SWAT.
    _BNKCOV = 'CH_BNK_COV'  # Channel bank cover factor, ch_cov1 in SWAT.
    _BEDEROD = 'CH_BED_EROD'  # Erodibility of channel bed sediment, ch_bed_kd in SWAT(0.001 - 3.75)
    _BNKEROD = 'CH_BNK_EROD'  # Erodibility of channel bank sediment, ch_bnk_kd in SWAT (cm3/N-s)
    _BEDD50 = 'CH_BED_D50'  # D50(median) particle size diameter of channel bed sediment, micrometer
    _BNKD50 = 'CH_BNK_D50'  # D50(median) particle size diameter of channel bank sediment, 0.001-20
    # Nutrient routing related parameters
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
    _DISOX = 'DISOX'  # 0-50 mg/L
    _BOD = 'BOD'  # 0-1000 mg/L
    _ALGAE = 'ALGAE'  # 0-200 mg/L
    _ORGN = 'ORGN'  # 0-100 mg/L, ch_onco in SWAT
    _NH4 = 'NH4'  # 0-50 mg/L
    _NO2 = 'NO2'  # 0-100 mg/L
    _NO3 = 'NO3'  # 0-50 mg/L
    _ORGP = 'ORGP'  # 0-25 mg/L, ch_opco in SWAT
    _SOLP = 'SOLP'  # 0-25 mg/L
    # Groundwater nutrient related parameters
    _GWNO3 = 'GWNO3'  # 0-1000 mg/L
    _GWSOLP = 'GWSOLP'  # 0-1000 mg/L

    @staticmethod
    def get_subbasin_cell_count(subbsn_file, subdict=None):
        """Get cell number of each subbasin.
        Args:
            subbsn_file: subbasin raster file.
            subdict: default is None

        Returns:
            subbasin cell count dict and cell width
        """
        wtsd_raster = RasterUtilClass.read_raster(subbsn_file)

        values, counts = numpy.unique(wtsd_raster.data, return_counts=True)
        if not subdict:
            subdict = dict()
        for v, c in zip(values, counts):
            if abs(v - wtsd_raster.noDataValue) < UTIL_ZERO:
                continue
            subdict[int(v)][ImportReaches2Mongo._NUMCELLS] = int(c)
            subdict[int(v)][ImportReaches2Mongo._AREA] = int(c) * wtsd_raster.dx ** 2
        return subdict

    @staticmethod
    def construct_flow_graph(downstream_dict):
        g = nx.DiGraph()
        for from_id, info in downstream_dict.items():
            if info['downstream'] > 0:
                g.add_edge(from_id, info['downstream'])
        return g

    @staticmethod
    def prepare_node_with_weight_for_metis(graph, weight, wp):
        # construct the METIS input file
        UtilClass.mkdir(wp)
        metis_input = '%s/metis.txt' % wp
        ns = list(graph.nodes())
        ns.sort()
        with open(metis_input, 'w', encoding='utf-8') as f:
            f.write('%s\t%s\t010\t1\n' % (repr(len(ns)), repr(len(graph.edges()))))
            for node in ns:
                if node <= 0:
                    continue
                tmp_line = '%s\t' % repr(weight[node][ImportReaches2Mongo._NUMCELLS])
                for e in graph.out_edges(node):
                    if e[1] > 0:
                        tmp_line += '%s\t' % repr(e[1])
                for e in graph.in_edges(node):
                    if e[0] > 0:
                        tmp_line += '%s\t' % repr(e[0])
                f.write('%s\n' % tmp_line)
        return metis_input

    @staticmethod
    def metis_partition(g, weight, wp, bin_dir):
        """Partition subbasins into multiple groups by METIS

        Args:
            g: `NetworkX.DiGraph` object
            weight: weight of each node, e.g., area of subbasin, {subbasinID: weight}
            wp: output directory
            bin_dir: directory of METIS package
        Returns:
            group_dict: {subbasinID: {'group': group_number_list,
                                      'kmetis': group_ids_list_by_kmetis,
                                      'pmetis': group_ids_list_by_pmetis}
                        }
        """
        group_dict = dict()
        for subbsn_id in g.nodes():
            group_dict[subbsn_id] = {'group': list(), 'kmetis': list(), 'pmetis': list()}

        metis_input = ImportReaches2Mongo.prepare_node_with_weight_for_metis(g, weight, wp)
        # Creating group divided numbers
        nlist = list(range(1, 129))
        nlist.extend([192, 256, 384, 512, 768, 1536])
        # nlist should be less than the number of subbasin, otherwise it will make nonsense.
        ns = g.nodes()
        nlist = [x for x in nlist if x <= max(ns)]
        # Make directories for KMETIS and PMETIS
        UtilClass.mkdir(wp + os.path.sep + 'kmetis')
        UtilClass.mkdir(wp + os.path.sep + 'pmetis')
        for n in nlist:
            print('divide number: %d' % n)
            if n <= 1:
                for subbsn_id in g.nodes():
                    group_dict[subbsn_id]['group'].append(1)
                    group_dict[subbsn_id]['kmetis'].append(0)
                    group_dict[subbsn_id]['pmetis'].append(0)
                continue
            # kmetis, -ptype=kway, direct k-way partitioning (default)
            str_command = '"%s/gpmetis" %s %d' % (bin_dir, metis_input, n)
            result = UtilClass.run_command(str_command)
            kmetis_file = '%s/kmetis/kmetisResult%d.txt' % (wp, n)
            with open(kmetis_file, 'w', encoding='utf-8') as f_metis_output:
                f_metis_output.write('\n'.join(result))
            metis_output = '%s.part.%d' % (metis_input, n)
            with open(metis_output, 'r', encoding='utf-8') as f:
                lines = f.readlines()
            group_kmetis = [int(item) for item in lines]
            adjust_group_result(weight, group_kmetis, n)
            shutil.move(metis_output, '%s/kmetis/metis.part.%d' % (wp, n))

            # pmetis, -ptype=rb, recursive bisectioning
            str_command = '"%s/gpmetis" -ptype=rb %s %d' % (bin_dir, metis_input, n)
            result = UtilClass.run_command(str_command)
            pmetis_file = '%s/pmetis/pmetisResult%d.txt' % (wp, n)
            with open(pmetis_file, 'w', encoding='utf-8') as f_metis_output:
                f_metis_output.write('\n'.join(result))
            with open(metis_output, 'r', encoding='utf-8') as f:
                lines = f.readlines()
            group_pmetis = [int(item) for item in lines]
            adjust_group_result(weight, group_pmetis, n)
            shutil.move(metis_output, '%s/pmetis/metis.part.%d' % (wp, n))

            for i, (gk, gp) in enumerate(zip(group_kmetis, group_pmetis)):
                group_dict[i + 1]['group'].append(n)
                group_dict[i + 1]['kmetis'].append(gk)
                group_dict[i + 1]['pmetis'].append(gp)
        return group_dict

    @staticmethod
    def read_reach_downstream_info(reach_shp, is_taudem=True):
        # type: (AnyStr, bool) -> Dict[int, Dict[AnyStr, Union[int, float]]]
        """Read information of subbasin.
        Args:
            reach_shp: reach ESRI shapefile.
            is_taudem: is TauDEM or not, true is default.

        Returns:
            rch_dict: {stream ID: {'downstream': downstreamID,
                                   'depth': depth value,
                                   'slope': slope value,
                                   'width': width value,
                                   'length': length value}
                                  }
        """
        rch_dict = dict()

        ds_reach = ogr_Open(reach_shp)
        layer_reach = ds_reach.GetLayer(0)
        layer_def = layer_reach.GetLayerDefn()
        if not is_taudem:  # For ArcSWAT
            ImportReaches2Mongo._LINKNO = 'FROM_NODE'
            ImportReaches2Mongo._DSLINKNO = 'TO_NODE'
            ImportReaches2Mongo._SLOPE = 'Slo2'  # TauDEM: Slope (tan); ArcSWAT: Slo2 (100*tan)
            ImportReaches2Mongo._LENGTH = 'Len2'  # TauDEM: Length; ArcSWAT: Len2
        ifrom = layer_def.GetFieldIndex(str(ImportReaches2Mongo._LINKNO))
        ito = layer_def.GetFieldIndex(str(ImportReaches2Mongo._DSLINKNO))
        idph = layer_def.GetFieldIndex(str(ImportReaches2Mongo._DEPTH))
        islp = layer_def.GetFieldIndex(str('Slope'))
        iwth = layer_def.GetFieldIndex(str(ImportReaches2Mongo._WIDTH))
        ilen = layer_def.GetFieldIndex(str('Length'))

        ft = layer_reach.GetNextFeature()
        while ft is not None:
            nfrom = ft.GetFieldAsInteger(ifrom)
            nto = ft.GetFieldAsInteger(ito)
            rch_dict[nfrom] = {'downstream': nto,
                               'depth': ft.GetFieldAsDouble(idph) if idph > 0. else 1.5,
                               'slope': ft.GetFieldAsDouble(islp)
                               if islp > -1 and ft.GetFieldAsDouble(islp) > MINI_SLOPE
                               else MINI_SLOPE,
                               'width': ft.GetFieldAsDouble(iwth) if iwth > 0. else 5.,
                               'length': ft.GetFieldAsDouble(ilen)}

            ft = layer_reach.GetNextFeature()

        return rch_dict

    @staticmethod
    def construct_downup_order(g):
        """

        Returns:
            downstream_up_order_dic: from outlet up stream dict
            upstream_down_order_dic: from source down stream dict
        """
        # find outlet subbasin
        outlet = -1
        for node in g.nodes():
            if g.out_degree(node) == 0:
                outlet = node
        if outlet < 0:
            raise ValueError('Cannot find outlet subbasin ID, please check the '
                             'threshold for stream extraction!')
        print('outlet subbasin:%d' % outlet)

        # assign order from outlet to upstream subbasins from 1
        downstream_up_order_dic = dict()
        start_node = [outlet]
        order_num = 0
        while start_node:
            tmp = list()
            order_num += 1
            for snode in start_node:
                downstream_up_order_dic[snode] = order_num
                for in_nodes in g.in_edges(snode):
                    tmp.append(in_nodes[0])
            start_node = tmp[:]
        # order_num now is the maximum order number from outlet
        # reserve the order number
        for k, v in downstream_up_order_dic.items():
            downstream_up_order_dic[k] = order_num - v + 1
        return downstream_up_order_dic

    @staticmethod
    def construct_updown_order(graph):
        # assign order from the source subbasins
        g = graph.copy()
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

        return upstream_down_order_dic

    @staticmethod
    def add_group_field(shp_file, subbasin_field_name, group_metis_dict):
        """add group information to subbasin ESRI shapefile

        Args:
            shp_file: Subbasin Shapefile
            subbasin_field_name: field name of subbasin
            group_metis_dict: returned by func`metis_partition`
        """
        if not group_metis_dict:
            return
        ds_reach = ogr_Open(shp_file, update=True)
        layer_reach = ds_reach.GetLayer(0)
        layer_def = layer_reach.GetLayerDefn()
        icode = layer_def.GetFieldIndex(str(subbasin_field_name))
        igrp = layer_def.GetFieldIndex(str(ImportReaches2Mongo._GROUP))
        ikgrp = layer_def.GetFieldIndex(str(ImportReaches2Mongo._KMETIS))
        ipgrp = layer_def.GetFieldIndex(str(ImportReaches2Mongo._PMETIS))

        if igrp < 0:
            new_field = ogr_FieldDefn(str(ImportReaches2Mongo._GROUP), OFTInteger)
            layer_reach.CreateField(new_field)
        if ikgrp < 0:
            new_field = ogr_FieldDefn(str(ImportReaches2Mongo._KMETIS), OFTInteger)
            layer_reach.CreateField(new_field)
        if ipgrp < 0:
            new_field = ogr_FieldDefn(str(ImportReaches2Mongo._PMETIS), OFTInteger)
            layer_reach.CreateField(new_field)

        ftmap = dict()
        layer_reach.ResetReading()
        ft = layer_reach.GetNextFeature()
        while ft is not None:
            tmpid = ft.GetFieldAsInteger(icode)
            ftmap[tmpid] = ft
            ft = layer_reach.GetNextFeature()

        groups = group_metis_dict[1]['group']
        for i, n in enumerate(groups):
            for node, d in group_metis_dict.items():
                ftmap[node].SetField(str(ImportReaches2Mongo._GROUP), n)
                ftmap[node].SetField(str(ImportReaches2Mongo._KMETIS), d['kmetis'][i])
                ftmap[node].SetField(str(ImportReaches2Mongo._PMETIS), d['pmetis'][i])
                layer_reach.SetFeature(ftmap[node])
            # copy the reach file to new file
            prefix = os.path.splitext(shp_file)[0]
            dstfile = prefix + "_" + str(n) + ".shp"
            FileClass.copy_files(shp_file, dstfile)

        layer_reach.SyncToDisk()
        ds_reach.Destroy()
        del ds_reach

    @staticmethod
    def generate_reach_table(cfg, maindb):
        """Generate reaches table and import to MongoDB

        Args:
            cfg: configuration object
            maindb: database object of MongoDB
        """
        reach_dict = ImportReaches2Mongo.read_reach_downstream_info(cfg.vecs.reach)
        ImportReaches2Mongo.get_subbasin_cell_count(cfg.spatials.subbsn, reach_dict)

        g = ImportReaches2Mongo.construct_flow_graph(reach_dict)
        downup_order = ImportReaches2Mongo.construct_downup_order(g)
        updown_order = ImportReaches2Mongo.construct_updown_order(g)

        # interpolation among different stream orders
        min_manning = 0.035
        max_manning = 0.075

        rch_orders = list(updown_order.values())
        min_order = min(rch_orders)
        max_order = max(rch_orders)

        a = (max_manning - min_manning) / (max_order - min_order)
        for tmpid in list(reach_dict.keys()):
            reach_dict[tmpid]['manning'] = max_manning - a * (updown_order[tmpid] - min_order)

        group_metis = ImportReaches2Mongo.metis_partition(g, reach_dict,
                                                          cfg.dirs.metis, cfg.seims_bin)
        # add group fields to Shapefile for visualization
        ImportReaches2Mongo.add_group_field(cfg.vecs.reach, ImportReaches2Mongo._LINKNO,
                                            group_metis)
        ImportReaches2Mongo.add_group_field(cfg.vecs.subbsn, ImportReaches2Mongo._SUBBASIN,
                                            group_metis)
        # import to MongoDB
        ImportReaches2Mongo.import_reach_info(maindb, reach_dict, updown_order,
                                              downup_order, group_metis)

    @staticmethod
    def import_reach_info(maindb, rch, updown, downup, metis):
        """import reach info"""
        # remove the older reaches collection if existed
        maindb.drop_collection(ImportReaches2Mongo._TAB_REACH)

        for subbsn_id, rchdata in rch.items():
            dic = dict()
            dic[ImportReaches2Mongo._SUBBASIN] = subbsn_id
            dic[ImportReaches2Mongo._NUMCELLS] = rchdata[ImportReaches2Mongo._NUMCELLS]
            dic[ImportReaches2Mongo._GROUP] = ','.join(str(v) for v in metis[subbsn_id]['group'])
            dic[ImportReaches2Mongo._KMETIS] = ','.join(str(v) for v in metis[subbsn_id]['kmetis'])
            dic[ImportReaches2Mongo._PMETIS] = ','.join(str(v) for v in metis[subbsn_id]['pmetis'])
            dic[ImportReaches2Mongo._DOWNSTREAM] = rchdata['downstream']
            dic[ImportReaches2Mongo._UPDOWN_ORDER] = updown[subbsn_id]
            dic[ImportReaches2Mongo._DOWNUP_ORDER] = downup[subbsn_id]
            dic[ImportReaches2Mongo._WIDTH] = rchdata['width']
            dic[ImportReaches2Mongo._LENGTH] = rchdata['length']
            dic[ImportReaches2Mongo._DEPTH] = rchdata['depth']
            dic[ImportReaches2Mongo._WDRATIO] = 3.5
            dic[ImportReaches2Mongo._AREA] = rchdata[ImportReaches2Mongo._AREA]
            dic[ImportReaches2Mongo._SIDESLP] = 2.
            dic[ImportReaches2Mongo._SLOPE] = rchdata['slope']
            # Hydrological related
            dic[ImportReaches2Mongo._MANNING] = rchdata['manning']
            dic[ImportReaches2Mongo._BEDK] = 0.
            dic[ImportReaches2Mongo._BNKK] = 0.
            # Erosion related
            dic[ImportReaches2Mongo._BEDBD] = 1.5  # sandy loam bed
            dic[ImportReaches2Mongo._BNKBD] = 1.4  # silty loam bank
            dic[ImportReaches2Mongo._BEDCOV] = 0.
            dic[ImportReaches2Mongo._BNKCOV] = 0.
            dic[ImportReaches2Mongo._BEDEROD] = 0.
            dic[ImportReaches2Mongo._BNKEROD] = 0.
            dic[ImportReaches2Mongo._BEDD50] = 500.  # micrometer, sand type particle
            dic[ImportReaches2Mongo._BNKD50] = 50.  # micrometer, silt type particle
            # Nutrient related
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
            dic[ImportReaches2Mongo._DISOX] = 10
            dic[ImportReaches2Mongo._BOD] = 10
            dic[ImportReaches2Mongo._ALGAE] = 0
            dic[ImportReaches2Mongo._ORGN] = 0
            dic[ImportReaches2Mongo._NH4] = 0
            dic[ImportReaches2Mongo._NO2] = 0
            dic[ImportReaches2Mongo._NO3] = 0
            dic[ImportReaches2Mongo._ORGP] = 0
            dic[ImportReaches2Mongo._SOLP] = 0
            # Groundwater nutrient related
            dic[ImportReaches2Mongo._GWNO3] = 0
            dic[ImportReaches2Mongo._GWSOLP] = 0

            cur_filter = {ImportReaches2Mongo._SUBBASIN: subbsn_id}
            maindb[ImportReaches2Mongo._TAB_REACH].find_one_and_replace(cur_filter, dic,
                                                                        upsert=True)

        maindb[ImportReaches2Mongo._TAB_REACH].create_index([(ImportReaches2Mongo._SUBBASIN,
                                                              ASCENDING)])


def get_max_weight(group_weight_dic, group_dic):
    """Get max. weight"""
    max_id = -1
    max_weight = -1
    for cur_id in group_weight_dic:
        if len(group_dic[cur_id]) > 1 and max_weight < group_weight_dic[cur_id]:
            max_weight = group_weight_dic[cur_id]
            max_id = cur_id
    return max_id, max_weight


def adjust_group_result(weight_dic, group_list, n_groups):
    """Adjust group result"""
    group_dic = dict()  # group tmpid from 0
    group_weight_dic = dict()  # subbasin tmpid form 1
    n = len(weight_dic.keys())
    for sub_id in range(1, n + 1):
        group_id = group_list[sub_id - 1]
        group_dic.setdefault(group_id, []).append(sub_id)
        if group_id not in group_weight_dic:
            group_weight_dic[group_id] = 0
        group_weight_dic[group_id] += weight_dic[sub_id][ImportReaches2Mongo._NUMCELLS]
    # get average weight for each group, which is the ideal aim of task scheduling
    ave_eight = 0
    for tmpid in group_weight_dic:
        ave_eight += group_weight_dic[tmpid]
    ave_eight /= n_groups

    for iGroup in range(n_groups):
        if iGroup not in group_dic:
            max_id, max_weight = get_max_weight(group_weight_dic, group_dic)
            if max_id < 0:
                continue
            sub_list = group_dic[max_id]
            sub_id = sub_list[0]
            group_dic[iGroup] = [sub_id, ]
            group_list[sub_id - 1] = iGroup
            group_dic[max_id].remove(sub_id)


def main():
    """TEST CODE"""
    from preprocess.config import parse_ini_configuration
    from preprocess.db_mongodb import ConnectMongoDB
    seims_cfg = parse_ini_configuration()
    client = ConnectMongoDB(seims_cfg.hostname, seims_cfg.port)
    conn = client.get_conn()
    maindb = conn[seims_cfg.spatial_db]

    ImportReaches2Mongo.generate_reach_table(seims_cfg, maindb)

    client.close()


if __name__ == "__main__":
    main()
