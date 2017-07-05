#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Used for adjusting partitioned tasks by Metis.
   This function is activated when forCluster is True.
    @author   : Junzhi Liu
    @changelog: 13-01-10  jz - initial implementation
                17-06-23  lj - reformat according to pylint and google style
"""


def get_max_weight(group_weight_dic, group_dic):
    """Get max. weight"""
    max_id = -1
    max_weight = -1
    for cur_id in group_weight_dic:
        if len(group_dic[cur_id]) > 1 and max_weight < group_weight_dic[cur_id]:
            max_weight = group_weight_dic[cur_id]
            max_id = cur_id
    return max_id, max_weight


def adjust_group_result(g, weight_dic, group_list, n_groups):
    """Adjust group result"""
    group_dic = {}  # group tmpid from 0
    group_weight_dic = {}  # subbasin tmpid form 1
    n = len(weight_dic.keys())
    for sub_id in range(1, n + 1):
        group_id = group_list[sub_id - 1]
        group_dic.setdefault(group_id, []).append(sub_id)
        group_weight_dic[group_id] = group_weight_dic.setdefault(group_id, 0) + weight_dic[sub_id]
    # get average weight for each group, which is the ideal aim of task
    # scheduling
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
