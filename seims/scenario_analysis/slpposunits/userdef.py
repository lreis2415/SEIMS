#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""User defined operation for optimizing BMPs based on slope position units.
    @author   : Liangjun Zhu, Huiran Gao

    @changelog:

    - 16-11-08  hr - initial implementation.
    - 17-08-18  lj - reorganization.
    - 18-02-09  lj - compatible with Python3.
    - 18-11-07  lj - support multiple BMPs configuration methods.
"""
from __future__ import absolute_import, unicode_literals

import array
from collections import OrderedDict
import os
import sys
import random

from pygeoc.utils import get_config_parser
from typing import List, Tuple, Dict, Union, Any, Optional, AnyStr

if os.path.abspath(os.path.join(sys.path[0], '../..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '../..')))

from scenario_analysis import _DEBUG
from scenario_analysis.slpposunits.scenario import select_potential_bmps


#                                       #
#          Crossover (Mate)             #
#                                       #

def crossover_slppos(tagnames,  # type: List[Tuple[int, str]]
                     ind1,  # type: Union[array.array, List[int], Tuple[int]]
                     ind2  # type: Union[array.array, List[int], Tuple[int]]
                     ):
    # type: (...) -> (Union[array.array, List[int], Tuple[int]], Union[array.array, List[int], Tuple[int]])
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
        # print(cxpoint1, cxpoint2)
        cs1 = cxpoint1 // sp_num
        cs2 = cxpoint2 // sp_num
        # print(cs1, cs2)
        cxpoint1 = cs1 * sp_num
        if cxpoint2 % sp_num != 0:
            cxpoint2 = sp_num * (cs2 + 1)
        if cxpoint1 == cxpoint2:
            cxpoint2 += sp_num
        # print(cxpoint1, cxpoint2)
        if not (cxpoint1 == 0 and cxpoint2 == size):
            break  # avoid change the entire genes
    ind1[cxpoint1:cxpoint2], ind2[cxpoint1:cxpoint2] \
        = ind2[cxpoint1:cxpoint2], ind1[cxpoint1:cxpoint2]

    return ind1, ind2


def crossover_rdm(ind1,  # type: Union[array.array, List[int], Tuple[int]]
                  ind2  # type: Union[array.array, List[int], Tuple[int]]
                  ):
    # type: (...) -> (Union[array.array, List[int], Tuple[int]], Union[array.array, List[int], Tuple[int]])
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

def mutate_rule(unitsinfo,  # type: Dict[Union[str, int], Any]
                gene2unit,  # type: Dict[int, int]
                unit2gene,  # type: OrderedDict[int, int]
                suitbmps,  # type: Dict[int, List[int]]
                individual,  # type: Union[array.array, List[int], Tuple[int]]
                perc,  # type: float
                indpb,  # type: float
                unit='SLPPOS',  # type: AnyStr
                method='SUIT',  # type: AnyStr
                bmpgrades=None,  # type: Optional[Dict[int, int]]
                tagnames=None  # type: Optional[List[Tuple[int, AnyStr]]] # For slope position units
                ):
    # type: (...) -> Union[array.array, List[int], Tuple[int]]
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
        unit(str): BMPs configuration unit type.
        method(str): Domain knowledge-based rule method.
        bmpgrades(dict): (Optional) Effectiveness grades of BMPs.

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
    # print('Max mutate num: %d' % mut_num)
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
        # get the potential BMP IDs
        bmps = select_potential_bmps(unitid, suitbmps, unitsinfo, unit2gene, individual,
                                     unit=unit, method=method,
                                     bmpgrades=bmpgrades, tagnames=tagnames)
        if bmps is None or len(bmps) == 0:
            continue
        # Get new BMP ID for current unit.
        if 0 not in bmps:
            bmps.append(0)
        if oldgenev in bmps:
            bmps.remove(oldgenev)
        if len(bmps) > 0:
            individual[mpoint] = bmps[random.randint(0, len(bmps) - 1)]
            if _DEBUG:
                print('  Mutate on unit: %d, oldgene: %d, potBMPs: %s, new gene: %d' %
                      (unitid, oldgenev, bmps.__str__(), individual[mpoint]))
            muted.append(mpoint)
        else:  # No available BMP
            pass
    return individual


def mutate_rdm(bmps_mut_target,  # type: Union[List[int], Tuple[int]]
               individual,  # type: Union[array.array, List[int], Tuple[int]]
               perc,  # type: float
               indpb  # type: float
               ):
    # type: (...) -> (Union[array.array, List[int], Tuple[int]], Union[array.array, List[int], Tuple[int]])
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


def main_test_mutate(perc, indpb, unit, mtd):
    # type: (float, float, AnyStr, AnyStr) -> None
    """Test mutate function."""
    from scenario_analysis import BMPS_CFG_UNITS
    from scenario_analysis.config import SAConfig
    from scenario_analysis.slpposunits.config import SASlpPosConfig, SAConnFieldConfig, \
        SACommUnitConfig
    from scenario_analysis.slpposunits.scenario import SUScenario
    cf = get_config_parser()

    base_cfg = SAConfig(cf)  # type: SAConfig
    if base_cfg.bmps_cfg_unit == BMPS_CFG_UNITS[3]:  # SLPPOS
        cfg = SASlpPosConfig(cf)
    elif base_cfg.bmps_cfg_unit == BMPS_CFG_UNITS[2]:  # CONNFIELD
        cfg = SAConnFieldConfig(cf)
    else:  # Common spatial units, e.g., HRU and UNIQHRU
        cfg = SACommUnitConfig(cf)
    cfg.construct_indexes_units_gene()

    sce = SUScenario(cfg)

    # Initialize gene values for one individual population
    init_gene_values = sce.initialize()
    sceid = sce.set_unique_id()
    print('ScenarioID: %d: initial genes: %s' % (sceid, init_gene_values.__str__()))

    # Calculate initial economic benefit
    inicost = sce.calculate_economy()

    # Mutate
    mutate_rule(sce.cfg.units_infos, sce.cfg.gene_to_unit, sce.cfg.unit_to_gene,
                sce.suit_bmps, init_gene_values, perc, indpb, unit=unit,
                method=mtd, bmpgrades=sce.bmps_grade, tagnames=sce.cfg.slppos_tagnames)
    print('Mutated genes: %s' % init_gene_values.__str__())

    # Calculate economic benefit after mutate
    setattr(sce, 'gene_values', init_gene_values)
    mutcost = sce.calculate_economy()
    print('Initial cost: %.3f, after mutation: %.3f' % (inicost, mutcost))


if __name__ == '__main__':
    main_test_mutate(0.2, 0.3, 'SLPPOS', 'SLPPOS')
