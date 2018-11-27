#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""BMPs optimization based on slope position units.

    @author   : Liangjun Zhu, Huiran Gao

    @changelog:
    - 16-12-30  hr - initial implementation.
    - 17-08-18  lj - reorganize.
    - 18-02-09  lj - compatible with Python3.
    - 18-11-02  lj - Optimization.
"""
from __future__ import absolute_import, unicode_literals

import array
import os
import sys
import random
import time
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
from pygeoc.utils import UtilClass, MathClass, get_config_parser

if os.path.abspath(os.path.join(sys.path[0], '../..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '../..')))

from typing import List
from utility.scoop_func import scoop_log
from scenario_analysis import BMPS_CFG_UNITS, BMPS_CFG_METHODS
from scenario_analysis.config import SAConfig
from scenario_analysis.userdef import initIterateWithCfg, initRepeatWithCfg
from scenario_analysis.slpposunits.config import SASlpPosConfig, SAConnFieldConfig, SACommUnitConfig
from scenario_analysis.slpposunits.scenario import SUScenario
from scenario_analysis.slpposunits.scenario import initialize_scenario, scenario_effectiveness
from scenario_analysis.slpposunits.userdef import crossover_slppos, mutate_rule, \
    crossover_rdm, mutate_rdm

# Definitions, assignments, operations, etc. that will be executed by each worker
#    when paralleled by SCOOP.
# Thus, DEAP related operations (initialize, register, etc.) are better defined here.

# Multiobjects: Minimum the economical cost, and maximum reduction rate of soil erosion
multi_weight = (-1., 1.)
filter_ind = False  # type: bool # Filter for valid population for the next generation
# Specific conditions for multiple objectives, None means no rule.
conditions = [None, '>0.']

creator.create('FitnessMulti', base.Fitness, weights=multi_weight)
creator.create('Individual', array.array, typecode='d', fitness=creator.FitnessMulti,
               gen=-1, id=-1,
               io_time=0., comp_time=0., simu_time=0., runtime=0.)

# Register NSGA-II related operations
toolbox = base.Toolbox()
toolbox.register('gene_values', initialize_scenario)
toolbox.register('individual', initIterateWithCfg, creator.Individual, toolbox.gene_values)
toolbox.register('population', initRepeatWithCfg, list, toolbox.individual)
toolbox.register('evaluate', scenario_effectiveness)

# knowledge-rule based mate and mutate
toolbox.register('mate_slppos', crossover_slppos)
toolbox.register('mutate_rule', mutate_rule)
# random-based mate and mutate
toolbox.register('mate_rdm', crossover_rdm)
toolbox.register('mutate_rdm', mutate_rdm)

toolbox.register('select', tools.selNSGA2)


def main(sceobj):
    # type: (SUScenario) -> ()
    """Main workflow of NSGA-II based Scenario analysis."""
    random.seed()

    # Initial timespan variables
    stime = time.time()
    plot_time = 0.
    allmodels_exect = list()  # execute time of all model runs

    pop_size = sceobj.cfg.opt.npop
    gen_num = sceobj.cfg.opt.ngens
    cx_rate = sceobj.cfg.opt.rcross
    mut_perc = sceobj.cfg.opt.pmut
    mut_rate = sceobj.cfg.opt.rmut
    sel_rate = sceobj.cfg.opt.rsel
    pop_select_num = int(pop_size * sel_rate)

    ws = sceobj.cfg.opt.out_dir
    cfg_unit = sceobj.cfg.bmps_cfg_unit
    cfg_method = sceobj.cfg.bmps_cfg_method
    worst_econ = sceobj.worst_econ
    worst_env = sceobj.worst_env
    # available gene value list
    possible_gene_values = list(sceobj.bmps_params.keys())
    if 0 not in possible_gene_values:
        possible_gene_values.append(0)
    units_info = sceobj.cfg.units_infos
    suit_bmps = sceobj.suit_bmps
    gene_to_unit = sceobj.cfg.gene_to_unit
    unit_to_gene = sceobj.cfg.unit_to_gene

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
    pop = toolbox.population(sceobj.cfg, n=pop_size)  # type: List
    init_time = time.time() - stime

    def check_individual_diff(old_ind, new_ind):
        """Check the gene values of two individuals."""
        diff = False
        for i in range(len(old_ind)):
            if not MathClass.floatequal(old_ind[i], new_ind[i]):
                diff = True
                break
        return diff

    def delete_fitness(new_ind):
        """Delete the fitness and other information of new individual."""
        del new_ind.fitness.values
        new_ind.gen = -1
        new_ind.id = -1
        new_ind.io_time = 0.
        new_ind.comp_time = 0.
        new_ind.simu_time = 0.
        new_ind.runtime = 0.

    def check_validation(fitvalues):
        """Check the validation of the fitness values of an individual."""
        flag = True
        for condidx, condstr in enumerate(conditions):
            if condstr is None:
                continue
            if not eval('%f%s' % (fitvalues[condidx], condstr)):
                flag = False
        return flag

    def evaluate_parallel(invalid_pops):
        """Evaluate model by SCOOP or map, and get fitness of individuals."""
        popnum = len(invalid_pops)
        try:
            # parallel on multiprocesor or clusters using SCOOP
            from scoop import futures
            invalid_pops = list(futures.map(toolbox.evaluate, [sceobj.cfg] * popnum, invalid_pops))
        except ImportError or ImportWarning:
            # serial
            invalid_pops = list(toolbox.map(toolbox.evaluate, [sceobj.cfg] * popnum, invalid_pops))

        # Filter for a valid solution
        if filter_ind:
            invalid_pops = [tmpind for tmpind in invalid_pops
                            if check_validation(tmpind.fitness.values)]
            if len(invalid_pops) < 2:
                print('The initial population should be greater or equal than 2. '
                      'Please check the parameters ranges or change the sampling strategy!')
                exit(2)
        return invalid_pops  # Currently, `invalid_pops` contains evaluated individuals

    # Record the count and execute timespan of model runs during the optimization
    modelruns_count = {0: len(pop)}
    modelruns_time = {0: 0.}  # Total time counted according to evaluate_parallel()
    modelruns_time_sum = {0: 0.}  # Summarize time of every model runs according to pop

    # Generation 0 before optimization
    stime = time.time()
    pop = evaluate_parallel(pop)
    modelruns_time[0] = time.time() - stime
    for ind in pop:
        ind.gen = 0
        allmodels_exect.append([ind.io_time, ind.comp_time, ind.simu_time, ind.runtime])
        modelruns_time_sum[0] += ind.runtime

    # Currently, len(pop) may less than pop_select_num
    pop = toolbox.select(pop, pop_select_num)
    record = stats.compile(pop)
    logbook.record(gen=0, evals=len(pop), **record)
    scoop_log(logbook.stream)
    front = numpy.array([ind.fitness.values for ind in pop])
    # save front for further possible use
    numpy.savetxt(sceobj.scenario_dir + os.sep + 'pareto_front_gen0.txt',
                  front, delimiter=str(' '), fmt=str('%.4f'))

    # Begin the generational process
    output_str = '### Generation number: %d, Population size: %d ###\n' % (gen_num, pop_size)
    scoop_log(output_str)
    UtilClass.writelog(sceobj.cfg.opt.logfile, output_str, mode='replace')

    modelsel_count = {0: len(pop)}  # type: Dict[int, int] # newly added Pareto fronts

    for gen in range(1, gen_num + 1):
        output_str = '###### Generation: %d ######\n' % gen
        scoop_log(output_str)
        offspring = [toolbox.clone(ind) for ind in pop]
        if len(offspring) >= 2:  # when offspring size greater than 2, mate can be done
            for ind1, ind2 in zip(offspring[::2], offspring[1::2]):
                old_ind1 = toolbox.clone(ind1)
                old_ind2 = toolbox.clone(ind2)
                if random.random() <= cx_rate:
                    if sceobj.cfg.bmps_cfg_unit == BMPS_CFG_UNITS[3]:
                        toolbox.mate_slppos(sceobj.cfg.slppos_tagnames, ind1, ind2)
                    else:
                        toolbox.mate_rdm(ind1, ind2)

                if cfg_method == BMPS_CFG_METHODS[0]:
                    toolbox.mutate_rdm(possible_gene_values, ind1, perc=mut_perc, indpb=mut_rate)
                    toolbox.mutate_rdm(possible_gene_values, ind2, perc=mut_perc, indpb=mut_rate)
                else:
                    tagnames = None
                    if sceobj.cfg.bmps_cfg_unit == BMPS_CFG_UNITS[3]:
                        tagnames = sceobj.cfg.slppos_tagnames
                    toolbox.mutate_rule(units_info, gene_to_unit, unit_to_gene,
                                        suit_bmps, ind1,
                                        perc=mut_perc, indpb=mut_rate,
                                        unit=cfg_unit, method=cfg_method,
                                        tagnames=tagnames)
                    toolbox.mutate_rule(units_info, gene_to_unit, unit_to_gene,
                                        suit_bmps, ind2,
                                        perc=mut_perc, indpb=mut_rate,
                                        unit=cfg_unit, method=cfg_method,
                                        tagnames=tagnames)
                if check_individual_diff(old_ind1, ind1):
                    delete_fitness(ind1)
                if check_individual_diff(old_ind2, ind2):
                    delete_fitness(ind2)

        # Evaluate the individuals with an invalid fitness
        invalid_inds = [ind for ind in offspring if not ind.fitness.valid]
        valid_inds = [ind for ind in offspring if ind.fitness.valid]
        invalid_ind_size = len(invalid_inds)
        if invalid_ind_size == 0:  # No need to continue
            scoop_log('Note: No invalid individuals available, the NSGA2 will be terminated!')
            break
        modelruns_count.setdefault(gen, invalid_ind_size)
        stime = time.time()
        invalid_inds = evaluate_parallel(invalid_inds)
        curtimespan = time.time() - stime
        modelruns_time.setdefault(gen, curtimespan)
        modelruns_time_sum.setdefault(gen, 0.)
        for ind in invalid_inds:
            ind.gen = gen
            allmodels_exect.append([ind.io_time, ind.comp_time, ind.simu_time, ind.runtime])
            modelruns_time_sum[gen] += ind.runtime

        # Select the next generation population
        pop = toolbox.select(pop + valid_inds + invalid_inds, pop_select_num)
        hyper_str = 'Gen: %d, New model runs: %d, ' \
                    'Execute timespan: %.4f, Sum of model run timespan: %.4f, ' \
                    'Hypervolume: %.4f\n' % (gen, invalid_ind_size,
                                             curtimespan, modelruns_time_sum[gen],
                                             hypervolume(pop, ref_pt))
        scoop_log(hyper_str)
        UtilClass.writelog(sceobj.cfg.opt.hypervlog, hyper_str, mode='append')

        record = stats.compile(pop)
        logbook.record(gen=gen, evals=len(invalid_inds), **record)
        scoop_log(logbook.stream)

        # Count the newly generated near Pareto fronts
        new_count = 0
        for ind in pop:
            if ind.gen == gen:
                new_count += 1
        modelsel_count.setdefault(gen, new_count)

        # Plot 2D near optimal pareto front graphs
        stime = time.time()
        front = numpy.array([ind.fitness.values for ind in pop])
        # save front for further possible use
        numpy.savetxt(sceobj.scenario_dir + os.sep + 'pareto_front_gen%d.txt' % gen,
                      front, delimiter=str(' '), fmt=str('%.4f'))
        # Comment out since matplotlib is quite often not working.
        # try:
        #     from concurrent.futures import ThreadPoolExecutor, TimeoutError
        #     from scenario_analysis.visualization import plot_pareto_front_single
        #     p = ThreadPoolExecutor(1)
        #     func = p.submit(plot_pareto_front_single, front, ['Economy', 'Environment'],
        #                     ws, gen, 'Near Pareto optimal solutions')
        #     func.result(timeout=10)
        # except TimeoutError:
        #     scoop_log('Plot pareto front timeout for generation %d!' % gen)
        #     pass
        # except Exception as e:
        #     scoop_log('Exception caught: %s' % str(e))
        # except:
        #     scoop_log('Exception caught: %s' % sys.exc_info()[0])
        plot_time += time.time() - stime

        # save in file
        output_str += 'generation\tscenario\teconomy\tenvironment\tgene_values\n'
        for indi in pop:
            output_str += '%d\t%d\t%f\t%f\t%s\n' % (indi.gen, indi.id, indi.fitness.values[0],
                                                    indi.fitness.values[1], str(indi))
        UtilClass.writelog(sceobj.cfg.opt.logfile, output_str, mode='append')

    # Plot hypervolume and newly executed model count
    # Comment out since matplotlib is quite often not working.
    # try:
    #     from scenario_analysis.visualization import plot_hypervolume_single
    #     p = ThreadPoolExecutor(1)
    #     func = p.submit(plot_hypervolume_single, sceobj.cfg.opt.hypervlog, ws)
    #     func.result(timeout=5)
    # except TimeoutError:
    #     scoop_log('Plot hypervolume timeout!')
    #     pass

    # Save newly added Pareto fronts of each generations
    new_fronts_count = numpy.array(list(modelsel_count.items()))
    numpy.savetxt('%s/new_pareto_fronts_count.txt' % ws,
                  new_fronts_count, delimiter=str(','), fmt=str('%d'))

    # Save and print timespan information
    allmodels_exect = numpy.array(allmodels_exect)
    numpy.savetxt('%s/exec_time_allmodelruns.txt' % ws, allmodels_exect,
                  delimiter=str(' '), fmt=str('%.4f'))
    scoop_log('Running time of all SEIMS models:\n'
              '\tIO\tCOMP\tSIMU\tRUNTIME\n'
              'MAX\t%s\n'
              'MIN\t%s\n'
              'AVG\t%s\n'
              'SUM\t%s\n' % ('\t'.join('%.3f' % v for v in allmodels_exect.max(0)),
                             '\t'.join('%.3f' % v for v in allmodels_exect.min(0)),
                             '\t'.join('%.3f' % v for v in allmodels_exect.mean(0)),
                             '\t'.join('%.3f' % v for v in allmodels_exect.sum(0))))

    exec_time = 0.
    for genid, tmptime in list(modelruns_time.items()):
        exec_time += tmptime
    exec_time_sum = 0.
    for genid, tmptime in list(modelruns_time_sum.items()):
        exec_time_sum += tmptime
    allcount = 0
    for genid, tmpcount in list(modelruns_count.items()):
        allcount += tmpcount

    scoop_log('Initialization timespan: %.4f\n'
              'Model execution timespan: %.4f\n'
              'Sum of model runs timespan: %.4f\n'
              'Plot Pareto graphs timespan: %.4f' % (init_time, exec_time,
                                                     exec_time_sum, plot_time))

    return pop, logbook


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

    sce = SUScenario(sa_cfg)

    scoop_log('### START TO SCENARIOS OPTIMIZING ###')
    startT = time.time()

    fpop, fstats = main(sce)
    fpop.sort(key=lambda x: x.fitness.values)
    scoop_log(fstats)
    with open(sa_cfg.opt.logbookfile, 'w', encoding='utf-8') as f:
        # In case of 'TypeError: write() argument 1 must be unicode, not str' in Python2.7
        #   when using unicode_literals, please use '%s' to concatenate string!
        f.write('%s' % fstats.__str__())

    endT = time.time()
    scoop_log('Running time: %.2fs' % (endT - startT))
