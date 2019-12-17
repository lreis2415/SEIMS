# #! /usr/bin/env python
# # -*- coding: utf-8 -*-
# """Time extened BMPs optimization based on slope position units.
#
#     @author   : Shen Shen
#
# """
from __future__ import absolute_import, unicode_literals

import array
import os
import sys
import random
import time
import pickle
from io import open

import matplotlib

if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    matplotlib.use('Agg', warn=False)

from typing import Dict
import numpy
from deap import base
from deap import creator
from deap import tools
from deap.benchmarks.tools import hypervolume
from pygeoc.utils import UtilClass, get_config_parser

if os.path.abspath(os.path.join(sys.path[0], '../..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '../..')))

from typing import List
from utility.scoop_func import scoop_log
from scenario_analysis import BMPS_CFG_UNITS, BMPS_CFG_METHODS
from scenario_analysis.config import SAConfig
from scenario_analysis.userdef import initIterateWithCfgIndv, initRepeatWithCfgIndv,\
    initRepeatWithCfgFromList, initIterateWithCfgWithInput
from scenario_analysis.visualization import read_pareto_solutions_from_txt
from scenario_analysis.spatialunits.config import SASlpPosConfig, SAConnFieldConfig,\
    SACommUnitConfig
from scenario_analysis.spatialunits.scenario import SUScenario
from scenario_analysis.spatialunits.scenario import initialize_time_extended_scenario, scenario_effectiveness
from scenario_analysis.spatialunits.userdef import check_individual_diff,\
    crossover_rdm, crossover_slppos, crossover_updown, mutate_rule, mutate_rdm

# Multiobjects: Minimum the economical cost, and maximum reduction rate of soil erosion
multi_weight = (-1., 1.)
filter_ind = False  # type: bool # Filter for valid population for the next generation
# Specific conditions for multiple objectives, None means no rule.
conditions = [None, '>0.']

creator.create('FitnessMulti', base.Fitness, weights=multi_weight)
# NOTE that to maintain the compatibility with Python2 and Python3,
#      the typecode=str('d') MUST NOT changed to typecode='d', since
#      the latter will raise TypeError that 'must be char, not unicode'!
creator.create('Individual', array.array, typecode=str('d'), fitness=creator.FitnessMulti,
               gen=-1, id=-1,
               io_time=0., comp_time=0., simu_time=0., runtime=0.)

# Register NSGA-II related operations
toolbox = base.Toolbox()
toolbox.register('gene_values', initialize_time_extended_scenario)
toolbox.register('individual', initIterateWithCfgIndv, creator.Individual, toolbox.gene_values)
toolbox.register('population', initRepeatWithCfgIndv, list, toolbox.individual)

# toolbox.register('individual_byinput', initIterateWithCfgWithInput, creator.Individual,
#                  toolbox.gene_values)
# toolbox.register('population_byinputs', initRepeatWithCfgFromList, list, toolbox.individual_byinput)

toolbox.register('evaluate', scenario_effectiveness)

# knowledge-rule based mate and mutate
toolbox.register('mate_slppos', crossover_slppos)
toolbox.register('mate_updown', crossover_updown)
toolbox.register('mutate_rule', mutate_rule)
# random-based mate and mutate
toolbox.register('mate_rdm', crossover_rdm)
toolbox.register('mutate_rdm', mutate_rdm)

toolbox.register('select', tools.selNSGA2)


def main(scenario_obj, pareto_indv_obj):
    # type: (SUScenario, Individual) -> ()
    """Main workflow of NSGA-II based Time Extended Scenario analysis."""
    # no need to run base scenario first
    random.seed()

    # Initial timespan variables
    stime = time.time()
    plot_time = 0.
    allmodels_exect = list()  # execute time of all model runs

    pop_size = scenario_obj.cfg.opt.npop
    gen_num = scenario_obj.cfg.opt.ngens
    cx_rate = scenario_obj.cfg.opt.rcross
    mut_perc = scenario_obj.cfg.opt.pmut
    mut_rate = scenario_obj.cfg.opt.rmut
    sel_rate = scenario_obj.cfg.opt.rsel
    pop_select_num = int(pop_size * sel_rate)

    ws = scenario_obj.cfg.opt.out_dir
    cfg_unit = scenario_obj.cfg.bmps_cfg_unit
    cfg_method = scenario_obj.cfg.bmps_cfg_method
    worst_econ = scenario_obj.worst_econ
    worst_env = scenario_obj.worst_env

    # available gene value list
    possible_gene_values = list(scenario_obj.bmps_params.keys())
    if 0 not in possible_gene_values: # 0不配置BMP
        possible_gene_values.append(0)
    units_info = scenario_obj.cfg.units_infos
    suit_bmps = scenario_obj.suit_bmps
    gene_to_unit = scenario_obj.cfg.gene_to_unit
    unit_to_gene = scenario_obj.cfg.unit_to_gene
    updown_units = scenario_obj.cfg.updown_units

    scoop_log('Population: %d, Generation: %d' % (pop_size, gen_num))
    scoop_log('BMPs configure unit: %s, configuration method: %s' % (cfg_unit, cfg_method))

    # create reference point for hypervolume
    ref_pt = numpy.array([worst_econ, worst_env]) * multi_weight * -1

    stats = tools.Statistics(lambda sind: sind.fitness.values)
    stats.register('min', numpy.min, axis=0)
    stats.register('max', numpy.max, axis=0)
    stats.register('avg', numpy.mean, axis=0)
    stats.register('std', numpy.std, axis=0)

    logbook = tools.Logbook()
    logbook.header = 'gen', 'evals', 'min', 'max', 'avg', 'std'

    # Initialize population
    # 通过指定文件初始化种群，避免优化过程中途失败需再重新开始
    initialize_byinputs = False
    # if sceobj.cfg.initial_byinput and sceobj.cfg.input_pareto_file is not None and \
    #     sceobj.cfg.input_pareto_gen > 0:  # Initial by input Pareto solutions
    #     inpareto_file = sceobj.modelcfg.model_dir + os.sep + sceobj.cfg.input_pareto_file
    #     if os.path.isfile(inpareto_file):
    #         inpareto_solutions = read_pareto_solutions_from_txt(inpareto_file,
    #                                                             sce_name='scenario',
    #                                                             field_name='gene_values')
    #         if sceobj.cfg.input_pareto_gen in inpareto_solutions:
    #             pareto_solutions = inpareto_solutions[sceobj.cfg.input_pareto_gen]
    #             pop = toolbox.population_byinputs(sceobj.cfg, pareto_solutions)  # type: List
    #             initialize_byinputs = True

    if not initialize_byinputs:
        pop = toolbox.population(scenario_obj.cfg, pareto_indv_obj, n=pop_size)  # type: List
        print(pop)

    init_time = time.time() - stime


if __name__ == "__main__":
    in_cf = get_config_parser()
    base_cfg = SAConfig(in_cf)  # type: SAConfig

    if base_cfg.bmps_cfg_unit == BMPS_CFG_UNITS[3]:  # SLPPOS
        sa_cfg = SASlpPosConfig(in_cf)
    elif base_cfg.bmps_cfg_unit == BMPS_CFG_UNITS[2]:  # CONNFIELD
        sa_cfg = SAConnFieldConfig(in_cf)
    else:  # Common spatial units, e.g., HRU and EXPLICITHRU
        sa_cfg = SACommUnitConfig(in_cf)
    sa_cfg.construct_indexes_units_gene()

    with open(sa_cfg.model.model_dir + os.path.sep + 'pop_out', 'rb') as fp:
        pareto_pop = pickle.load(fp)
        print(type(pareto_pop))
        print(pareto_pop)
    sce = SUScenario(sa_cfg)

    # scoop_log('### START TO SCENARIOS OPTIMIZING ###')
    # startT = time.time()

    time_pareto_pop, time_pareto_stats = main(sce, pareto_pop[0])
    # fpop.sort(key=lambda x: x.fitness.values)
    # scoop_log(fstats)
    # with open(sa_cfg.opt.logbookfile, 'w', encoding='utf-8') as f:
    #     # In case of 'TypeError: write() argument 1 must be unicode, not str' in Python2.7
    #     #   when using unicode_literals, please use '%s' to concatenate string!
    #     f.write('%s' % fstats.__str__())
    #
    # endT = time.time()
    # scoop_log('Running time: %.2fs' % (endT - startT))


