#! /usr/bin/env python
# coding=utf-8
# Used for adjusting partitioned tasks by Metis.
# This function is actived when forCluster is True.
# Author: Junzhi Liu
#


def GetMaxWeight(groupWeightDic, groupDic):
    maxId = -1
    maxWeight = -1
    for id in groupWeightDic:
        if len(groupDic[id]) > 1 and maxWeight < groupWeightDic[id]:
            maxWeight = groupWeightDic[id]
            maxId = id
    return maxId, maxWeight


def AdjustGroupResult(g, weightDic, groupList, nGroups):
    groupDic = {}  # group id from 0
    groupWeightDic = {}  # subbasin id form 1
    n = len(weightDic.keys())
    for subId in range(1, n + 1):
        groupId = groupList[subId - 1]
        groupDic.setdefault(groupId, []).append(subId)
        groupWeightDic[groupId] = groupWeightDic.setdefault(
            groupId, 0) + weightDic[subId]
    # get average weight for each group, which is the ideal aim of task
    # scheduling
    aveWeight = 0
    for id in groupWeightDic:
        aveWeight += groupWeightDic[id]
    aveWeight /= nGroups

    for iGroup in range(nGroups):
        if not iGroup in groupDic:
            maxId, maxWeight = GetMaxWeight(groupWeightDic, groupDic)
            if maxId < 0:
                continue
            subList = groupDic[maxId]
            subId = subList[0]
            groupDic[iGroup] = [subId, ]
            groupList[subId - 1] = iGroup
            groupDic[maxId].remove(subId)
