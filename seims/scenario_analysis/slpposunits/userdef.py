#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""User defined operation for optimizing BMPs based on slope position units.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-11-08  hr - initial implementation.\n
                17-08-18  lj - reorganization.\n
"""
import random

from pygeoc.utils import get_config_parser

from config import SASPUConfig
from scenario import SPScenario, initialize_scenario, get_potential_bmps


#                                       #
#          Crossover (Mate)             #
#                                       #

def crossover_slppos(tagnames, ind1, ind2):
    """Crossover operator based on slope position units.
    Args:
        tagnames(list): slope position tags and names, from up to bottom of hillslope.
                        The format is [(tag, name),...].
        ind1: The first individual participating in the crossover.
        ind2: The second individual participating in the crossover.

    Returns:
        A tuple of two individuals.
    """
    sp_num = len(tagnames)
    size = min(len(ind1), len(ind2))
    assert (size > sp_num * 2)

    while True:
        cxpoint1 = random.randint(0, size - 1)
        cxpoint2 = random.randint(1, size)
        if cxpoint2 < cxpoint1:  # Swap the two cx points
            cxpoint1, cxpoint2 = cxpoint2, cxpoint1
        # print (cxpoint1, cxpoint2)
        cs1 = cxpoint1 / sp_num
        cs2 = cxpoint2 / sp_num
        # print (cs1, cs2)
        cxpoint1 = cs1 * sp_num
        if cxpoint2 % sp_num != 0:
            cxpoint2 = sp_num * (cs2 + 1)
        if cxpoint1 == cxpoint2:
            cxpoint2 += sp_num
        # print (cxpoint1, cxpoint2)
        if not (cxpoint1 == 0 and cxpoint2 == size):
            break  # avoid change the entire genes
    ind1[cxpoint1:cxpoint2], ind2[cxpoint1:cxpoint2] \
        = ind2[cxpoint1:cxpoint2], ind1[cxpoint1:cxpoint2]

    # Check the validation according to spatial configuration rules.

    return ind1, ind2


def crossover_rdm(ind1, ind2):
    """Crossover randomly.
        Args:
            ind1: The first individual participating in the crossover.
            ind2: The second individual participating in the crossover.

        Returns:
            A tuple of two individuals.
        """
    size = min(len(ind1), len(ind2))

    while True:
        cxpoint1 = random.randint(0, size - 1)
        cxpoint2 = random.randint(1, size)
        if cxpoint2 < cxpoint1:  # Swap the two cx points
            cxpoint1, cxpoint2 = cxpoint2, cxpoint1
        else:
            cxpoint2 += 1
        if not (cxpoint1 == 0 and cxpoint2 == size):
            break  # avoid change the entire genes
    ind1[cxpoint1:cxpoint2], ind2[cxpoint1:cxpoint2] \
        = ind2[cxpoint1:cxpoint2], ind1[cxpoint1:cxpoint2]

    return ind1, ind2


#                                       #
#               Mutate                  #
#                                       #

def mutate_slppos(unitsinfo, gene2unit, unit2gene, tagnames, suitbmps, individual,
                  perc, indpb, method=1):
    """
    Mutation Gene values based on slope position rules.
    Old gene value is excluded from target values.

    Args:
        unitsinfo(dict): Slope position units information, see more detail on `SASPUConfig`.
        gene2unit(dict): Gene index to slope position unit ID.
        unit2gene(dict): Slope position unit ID to gene index.
        tagnames(list): slope position tags and names, from up to bottom of hillslope.
                        The format is [(tag, name),...].
        suitbmps(dict): key is slope position tag, and value is available BMPs IDs list.
        individual(list or tuple): Individual to be mutated.
        perc(float): percent of gene length for mutate, default is 0.02
        indpb(float): Independent probability for each attribute to be mutated.
        method(int): Domain knowledge based rule method.

    Returns:
        A tuple of one individual.
    """
    if perc > 0.5:
        perc = 0.5
    elif perc < 0.01:
        perc = 0.01
    try:
        mut_num = random.randint(1, int(len(individual) * perc))
    except ValueError or Exception:
        return individual
    # print ('Max mutate num: %d' % mut_num)
    muted = list()
    for m in range(mut_num):
        if random.random() > indpb:
            continue
        # mutate will happen
        mpoint = random.randint(0, len(individual) - 1)
        while mpoint in muted:
            mpoint = random.randint(0, len(individual) - 1)

        oldgenev = individual[mpoint]
        unitid = gene2unit[mpoint]
        # begin to mutate on unitid
        # 1. get slope position tag, and upslope and downslope unit IDs
        sptag = -1
        down_sid = -1  # slppos unit ID
        down_gid = -1  # gene index
        down_gvalue = -1  # gene value
        up_sid = -1
        up_gid = -1
        up_gvalue = -1
        for spid, spdict in unitsinfo.iteritems():
            if unitid not in spdict:
                continue
            for t, n in tagnames:
                if spid == n:
                    sptag = t
                    break
            down_sid = spdict[unitid]['downslope']
            up_sid = spdict[unitid]['upslope']
            if down_sid > 0:
                down_gid = unit2gene[down_sid]
                down_gvalue = individual[down_gid]
            if up_sid > 0:
                up_gid = unit2gene[up_sid]
                up_gvalue = individual[up_gid]
        if sptag < 0:  # this circumstance may not happen, just in case.
            continue

        # print ('  Mutate on slppos: %d (unit: %d, oldgene: %d), upgene: %d, downgene: %d' %
        #        (sptag, unitid, oldgenev, up_gvalue, down_gvalue))
        # get the potential BMP IDs

        bmps = get_potential_bmps(suitbmps, sptag, up_sid, up_gvalue, down_sid, down_gvalue, method)
        # Get new BMP ID for current unit.
        if oldgenev in bmps:
            bmps.remove(oldgenev)
        # print ('    method: %d, potBMPs: %s' % (method, bmps.__str__()))
        if len(bmps) > 0:
            individual[mpoint] = bmps[random.randint(0, len(bmps) - 1)]
            muted.append(mpoint)
        else:  # No available BMP
            pass
    return individual


def mutate_rdm(bmps_mut_target, individual, perc, indpb):
    """
    Mutation Gene values randomly, old gene value is excluded from target values.
    Args:
        bmps_mut_target(list or tuple): All available gene values.
        individual(list or tuple): Individual to be mutated.
        perc(float): percent of gene length for mutate, default is 0.02
        indpb(float): Independent probability for each attribute to be mutated.

    Returns:
        A tuple of one individual.
    """
    if perc > 0.5:
        perc = 0.5
    elif perc < 0.01:
        perc = 0.01
    if 0 not in bmps_mut_target:
        bmps_mut_target.append(0)
    try:
        mut_num = random.randint(1, int(len(individual) * perc))
    except ValueError or Exception:
        return individual
    muted = list()
    for m in range(mut_num):
        if random.random() < indpb:
            mpoint = random.randint(0, len(individual) - 1)
            while mpoint in muted:
                mpoint = random.randint(0, len(individual) - 1)
            muted.append(mpoint)
            target = bmps_mut_target[:]
            target = list(set(target))
            if individual[mpoint] in target:
                target.remove(individual[mpoint])
            ind = random.randint(0, len(target) - 1)
            individual[mpoint] = target[ind]
    return individual


if __name__ == '__main__':
    cf = get_config_parser()
    cfg = SASPUConfig(cf)

    # print (cfg.gene_to_slppos)
    # print (cfg.slppos_suit_bmps)

    units_info = cfg.units_infos
    slppos_tagnames = cfg.slppos_tagnames
    suit_bmps = cfg.slppos_suit_bmps
    gene_to_unit = cfg.gene_to_slppos
    unit_to_gene = cfg.slppos_to_gene
    init_gene_values = initialize_scenario(cfg)
    # print ('Initial genes: %s' % init_gene_values.__str__())sce = SPScenario(cfg)
    sce = SPScenario(cfg)
    curid = sce.set_unique_id()
    setattr(sce, 'gene_values', init_gene_values)
    sce.calculate_economy()
    inicost = sce.economy
    mutate_slppos(units_info, gene_to_unit, unit_to_gene, slppos_tagnames, suit_bmps,
                  init_gene_values, 0.2, 0.3, method=1)
    # print ('Mutated genes: %s' % init_gene_values.__str__())
    setattr(sce, 'gene_values', init_gene_values)
    sce.calculate_economy()
    mutcost = sce.economy
    print ('%.2f, %.2f' % (inicost, mutcost))
