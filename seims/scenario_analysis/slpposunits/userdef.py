#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""User defined operation for optimizing BMPs based on slope position units.
    @author   : Liangjun Zhu, Huiran Gao

    @changelog:

    - 16-11-08  hr - initial implementation.
    - 17-08-18  lj - reorganization.
    - 18-02-09  lj - compatible with Python3.
    - 18-11-07  lj - support multiple BMPs configuration methods.
    - 18-12-04  lj - add func:`crossover_updown` according to Wu et al. (2018).
"""
from __future__ import absolute_import, unicode_literals

import array
from collections import OrderedDict
import os
import sys
import random

from pygeoc.utils import get_config_parser, MathClass
from typing import List, Tuple, Dict, Union, Any, Optional, AnyStr

if os.path.abspath(os.path.join(sys.path[0], '../..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '../..')))

from scenario_analysis import _DEBUG
from scenario_analysis.slpposunits.scenario import select_potential_bmps


def check_individual_diff(old_ind,  # type: Union[array.array, List[int], Tuple[int]]
                          new_ind  # type: Union[array.array, List[int], Tuple[int]]
                          ):
    # type: (...) -> bool
    """Check the gene values of two individuals."""
    diff = False
    for i in range(len(old_ind)):
        if not MathClass.floatequal(old_ind[i], new_ind[i]):
            diff = True
            break
    return diff


#                                       #
#          Crossover (Mate)             #
#                                       #

def crossover_slppos(tagnames,  # type: List[Tuple[int, str]]
                     ind1,  # type: Union[array.array, List[int], Tuple[int]]
                     ind2  # type: Union[array.array, List[int], Tuple[int]]
                     ):
    # type: (...) -> (Union[array.array, List[int], Tuple[int]], Union[array.array, List[int], Tuple[int]])
    """Crossover operator based on slope position units.
    Each individual can keep the domain knowledge based rules after crossover operation.

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


def crossover_updown(updownunits,  # type: Dict[int, Dict[AnyStr, List[int]]]
                     gene2unit,  # type: Dict[int, int]
                     unit2gene,  # type: OrderedDict[int, int]
                     ind1,  # type: Union[array.array, List[int], Tuple[int]]
                     ind2  # type: Union[array.array, List[int], Tuple[int]]
                     ):
    """Crossover operator based on hydrologically connected fields with
    upstream-downstream relationships.

    A subtree exchange method introduced in Wu et al. (2018) is adapted and extended.

    - 1. A node (gene) is chosen randomly.
    - 2. If the pattern of 'downslope-current' of each individual after
         crossover is accord with the UPDOWN rules, then exchange the subtrees with
         the selected gene as root node.
    - 3. If not, check the downslope gene one by one, until a eligible gene is reached.
    - 4. If no eligible gene is found until the last gene is reached:
      - 4.1. If the number of the subtree with the last gene as root equals to all genes, return;
      - 4.2. Else, exchange the subtree like step 2.
    """

    def check_validation(geneidx, genevalue, ind):
        """Check if the gene value is valid according to UPDOWN method."""
        if geneidx <= 0:
            return True
        unitid = gene2unit[geneidx]
        downids = updownunits[unitid]['downslope']
        valid = True
        for downid in downids:
            if downid <= 0:
                continue
            downgeneidx = unit2gene[downid]
            downgenevalue = ind[downgeneidx]
            if downgenevalue > 0 and genevalue > 0:
                # Only when the planned gene and its downslope gene are both configured BMP,
                #   is considered invalid.
                valid = False
                break
        return valid

    size = min(len(ind1), len(ind2))
    # 1. Chose a node (gene) randomly.
    cxpoint = random.randint(0, size - 1)
    if _DEBUG:
        print('-- Randomly selected crossover point index: %d' % cxpoint)
    # 2. Whether it is valid after crossover
    untested = [cxpoint]
    while len(untested) > 0:
        cxp = untested[0]
        if check_validation(cxp, ind2[cxp], ind1) and check_validation(cxp, ind1[cxp], ind2):
            cxpoint = cxp
            break
        else:
            untested.remove(cxp)
            for i in updownunits[gene2unit[cxp]]['downslope']:
                if i > 0:
                    untested.append(unit2gene[i])
    if cxpoint < 0:
        return ind1, ind2
    upids = updownunits[gene2unit[cxpoint]]['all_upslope']
    upids.insert(0, gene2unit[cxpoint])  # contain the crossover point itself

    if len(upids) == 1 and ind1[unit2gene[upids[0]]] == ind2[unit2gene[upids[0]]]:
        if _DEBUG:
            print('-- No need to crossover since only one gene with the same value is selected!')
        return ind1, ind2
    if len(upids) >= len(gene2unit):  # avoid exchange the entire genes
        if _DEBUG:
            print('-- No need to crossover since the entire genes is selected!')
        return ind1, ind2
    if _DEBUG:
        print('-- Adjusted crossover point index: %d, exchanged length: %d' % (cxpoint,
                                                                               len(upids)))
    for upunitid in upids:
        upgeneidx = unit2gene[upunitid]
        tmpvalue = ind1[upgeneidx]
        ind1[upgeneidx] = ind2[upgeneidx]
        ind2[upgeneidx] = tmpvalue
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


def main_test_crossover_mutate(cx_rate, mut_perc, mut_rate):
    # type: (float, float, float) -> None
    """Test mutate function."""
    from deap import base
    from scenario_analysis import BMPS_CFG_UNITS, BMPS_CFG_METHODS
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
    else:  # Common spatial units, e.g., HRU and EXPLICITHRU
        cfg = SACommUnitConfig(cf)
    cfg.construct_indexes_units_gene()

    # Initialize gene values for individual 1 and 2
    sce1 = SUScenario(cfg)
    ind1 = sce1.initialize()
    sceid1 = sce1.set_unique_id()
    print('Scenario1-ID: %d\n  Initial genes: %s' % (sceid1, ind1.__str__()))
    sce2 = SUScenario(cfg)
    ind2 = sce2.initialize()
    sceid2 = sce2.set_unique_id()
    print('Scenario2-ID: %d\n  Initial genes: %s' % (sceid2, ind2.__str__()))

    # Calculate initial economic benefit
    inicost1 = sce1.calculate_economy()
    inicost2 = sce2.calculate_economy()

    # Clone old individuals
    toolbox = base.Toolbox()

    # Crossover
    print('Crossover:')
    old_ind1 = toolbox.clone(ind1)
    old_ind2 = toolbox.clone(ind2)
    if random.random() <= cx_rate:
        if base_cfg.bmps_cfg_method == BMPS_CFG_METHODS[3]:  # SLPPOS method
            crossover_slppos(sce1.cfg.slppos_tagnames, ind1, ind2)
        elif base_cfg.bmps_cfg_method == BMPS_CFG_METHODS[2]:  # UPDOWN method
            crossover_updown(sce1.cfg.updown_units, sce1.cfg.gene_to_unit,
                             sce1.cfg.unit_to_gene, ind1, ind2)
        else:
            crossover_rdm(ind1, ind2)
    if not check_individual_diff(old_ind1, ind1) and not check_individual_diff(old_ind2, ind2):
        print('  No crossover occurred on Scenario1 and Scenario2!')
    else:
        print('  Crossover genes:\n    Scenario1: %s\n'
              '    Scenario2: %s' % (ind1.__str__(), ind2.__str__()))
    # Calculate economic benefit after crossover
    setattr(sce1, 'gene_values', ind1)
    cxcost1 = sce1.calculate_economy()
    setattr(sce2, 'gene_values', ind2)
    cxcost2 = sce2.calculate_economy()

    # Mutate
    print('Mutate:')
    old_ind1 = toolbox.clone(ind1)
    old_ind2 = toolbox.clone(ind2)
    if base_cfg.bmps_cfg_method == BMPS_CFG_METHODS[0]:
        possible_gene_values = list(sce1.bmps_params.keys())
        if 0 not in possible_gene_values:
            possible_gene_values.append(0)
        mutate_rdm(possible_gene_values, ind1, perc=mut_perc, indpb=mut_rate)
        mutate_rdm(possible_gene_values, ind2, perc=mut_perc, indpb=mut_rate)
    else:
        tagnames = None
        if base_cfg.bmps_cfg_unit == BMPS_CFG_UNITS[3]:
            tagnames = sce1.cfg.slppos_tagnames
        mutate_rule(sce1.cfg.units_infos, sce1.cfg.gene_to_unit, sce1.cfg.unit_to_gene,
                    sce1.suit_bmps['LANDUSE'], ind1, mut_perc, mut_rate,
                    unit=base_cfg.bmps_cfg_unit, method=base_cfg.bmps_cfg_method,
                    bmpgrades=sce1.bmps_grade, tagnames=tagnames)
        mutate_rule(sce2.cfg.units_infos, sce2.cfg.gene_to_unit, sce2.cfg.unit_to_gene,
                    sce2.suit_bmps['LANDUSE'], ind2, mut_perc, mut_rate,
                    unit=base_cfg.bmps_cfg_unit, method=base_cfg.bmps_cfg_method,
                    bmpgrades=sce2.bmps_grade, tagnames=tagnames)
    if not check_individual_diff(old_ind1, ind1):
        print('  No mutation occurred on Scenario1!')
    else:
        print('    Mutated genes of Scenario1: %s' % ind1.__str__())
    if not check_individual_diff(old_ind2, ind2):
        print('  No mutation occurred on Scenario2!')
    else:
        print('    Mutated genes of Scenario2: %s' % ind2.__str__())

    # Calculate economic benefit after mutate
    setattr(sce1, 'gene_values', ind1)
    mutcost1 = sce1.calculate_economy()
    setattr(sce2, 'gene_values', ind2)
    mutcost2 = sce2.calculate_economy()

    print('Initial cost: \n'
          '  Scenario1: %.3f, Scenario2: %.3f\n'
          'Crossover:\n'
          '  Scenario1: %.3f, Scenario2: %.3f\n'
          'Mutation:\n'
          '  Scenario1: %.3f, Scenario2: %.3f\n' % (inicost1, inicost2,
                                                    cxcost1, cxcost2,
                                                    mutcost1, mutcost2))


if __name__ == '__main__':
    main_test_crossover_mutate(0.8, 0.2, 0.1)
