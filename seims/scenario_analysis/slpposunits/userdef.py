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

def crossover_slppos(cfg, ind1, ind2):
    """Crossover operator based on slope position units.
    Args:
        cfg: Configuration instance of `SASPUConfig`.
        ind1: The first individual participating in the crossover.
        ind2: The second individual participating in the crossover.

    Returns:
        A tuple of two individuals.
    """
    tools.cxTwoPoint(ind1, ind2)

    # Check the validation according to spatial configuration rules.

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
    if perc > 0.5 or perc < 0.01:
        perc = 0.02
    try:
        mut_num = random.randint(1, int(len(individual) * perc))
    except ValueError or Exception:
        mut_num = 1
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
    if perc > 0.5 or perc < 0.01:
        perc = 0.02
    if 0 not in bmps_mut_target:
        bmps_mut_target.append(0)
    mut_num = random.randint(1, int(len(individual) * perc))
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
