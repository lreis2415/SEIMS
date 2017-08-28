#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""User defined operation for optimizing BMPs based on slope position units.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-11-08  hr - initial implementation.\n
                17-08-18  lj - reorganization.\n
"""
import random

from deap import tools


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

def mutate_slppos(unitsinfo, gene2unit, tagnames, suitbmps, individual, perc, indpb):
    """
    Mutation Gene values based on slope position rules.
    Old gene value is excluded from target values.

    Args:
        unitsinfo(dict): Slope position units information, see more detail on `SASPUConfig`.
        gene2unit(dict): Gene index to slope position unit ID.
        tagnames(list): slope position tags and names, from up to bottom of hillslope.
                        The format is [(tag, name),...].
        suitbmps(dict): key is slope position tag, and value is available BMPs IDs list.
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
    try:
        mut_num = random.randint(1, int(len(individual) * perc))
    except ValueError or Exception:
        return individual
    for m in range(mut_num):
        if random.random() < indpb:
            mpoint = random.randint(0, len(individual) - 1)
            oldgenev = individual[mpoint]
            unitid = gene2unit[mpoint]
            # begin to mutate
            for spid, spdict in unitsinfo.iteritems():
                if unitid not in spdict:
                    continue
                sptag = -1
                for t, n in tagnames:
                    if spid == n:
                        sptag = t
                        break
                if sptag < 0:
                    continue
                bmps = suitbmps[sptag][:]
                bmps = list(set(bmps))
                if 0 not in bmps:
                    bmps.append(0)
                if oldgenev in bmps:
                    bmps.remove(oldgenev)
                individual[mpoint] = bmps[random.randint(0, len(bmps) - 1)]
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
    for m in range(mut_num):
        if random.random() < indpb:
            mpoint = random.randint(0, len(individual) - 1)
            target = bmps_mut_target[:]
            target = list(set(target))
            if individual[mpoint] in target:
                target.remove(individual[mpoint])
            ind = random.randint(0, len(target) - 1)
            individual[mpoint] = target[ind]
    return individual
