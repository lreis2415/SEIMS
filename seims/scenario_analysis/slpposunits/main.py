#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""BMPs optimization based on slope position units.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-12-30  hr - initial implementation.\n
                17-08-18  lj - reorganize.\n
"""
import array
import os
import random
import time

import matplotlib
import numpy

from deap import base
from deap import creator
from deap import tools
from deap.benchmarks.tools import hypervolume
from seims.pygeoc.pygeoc.utils.utils import UtilClass, get_config_parser
from seims.scenario_analysis.slpposunits.config import SASPUConfig
from seims.scenario_analysis.slpposunits.scenario import initialize_scenario, scenario_effectiveness
from seims.scenario_analysis.slpposunits.userdef import crossover_slppos
from seims.scenario_analysis.slpposunits.userdef import mutate_rdm, mutate_slppos
from seims.scenario_analysis.userdef import initIterateWithCfg, initRepeatWithCfg
from seims.scenario_analysis.utility import print_message, delete_model_outputs
from seims.scenario_analysis.visualization import plot_pareto_front

# Definitions, assignments, operations, etc. that will be executed by each worker
#    when parallized by SCOOP.
# Thus, DEAP related operations (initialize, register, etc.) are better defined here.
if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    matplotlib.use('Agg')

# Multiobjects: Minimum the economical cost, and maximum reduction rate of soil erosion
creator.create('FitnessMulti', base.Fitness, weights=(-1.0, 1.0))
creator.create('Individual', array.array, typecode='d', fitness=creator.FitnessMulti)

# Register NSGA-II related operations
toolbox = base.Toolbox()
toolbox.register('gene_values', initialize_scenario)
toolbox.register('individual', initIterateWithCfg, creator.Individual, toolbox.gene_values)
toolbox.register('population', initRepeatWithCfg, list, toolbox.individual)
toolbox.register('evaluate', scenario_effectiveness)

# rule-based mate and mutate
toolbox.register('mate_rule', crossover_slppos)
toolbox.register('mutate_rule', mutate_slppos)
# random-based mate and mutate
toolbox.register('mate_random', tools.cxTwoPoint)
toolbox.register('mutate_rdm', mutate_rdm)

toolbox.register('select', tools.selNSGA2)


def main(cfg):
    """Main workflow of NSAG-II based Scenario analysis."""
    random.seed()
    pop_size = cfg.nsga2_npop
    gen_num = cfg.nsga2_ngens
    rule_cfg = cfg.bmps_rule
    cx_rate = cfg.nsga2_rcross
    mut_perc = cfg.nsga2_pmut
    mut_rate = cfg.nsga2_rmut
    sel_rate = cfg.nsga2_rsel
    ws = cfg.nsga2_dir
    # available gene value list
    possible_gene_values = cfg.bmps_params.keys()
    possible_gene_values.append(0)
    units_info = cfg.units_infos
    slppos_tagnames = cfg.slppos_tagnames
    suit_bmps = cfg.slppos_suit_bmps
    gene_to_unit = cfg.gene_to_slppos

    print_message('Population: %d, Generation: %d' % (pop_size, gen_num))
    print_message('BMPs configure method: %s' % ('rule-based' if rule_cfg else 'random-based'))

    stats = tools.Statistics(lambda sind: sind.fitness.values)
    stats.register('min', numpy.min, axis=0)
    stats.register('max', numpy.max, axis=0)
    stats.register('avg', numpy.mean, axis=0)
    stats.register('std', numpy.std, axis=0)

    logbook = tools.Logbook()
    logbook.header = 'gen', 'evals', 'min', 'max', 'avg', 'std'

    pop = toolbox.population(cfg, n=pop_size)
    # Evaluate the individuals with an invalid fitness
    invalid_ind = [ind for ind in pop if not ind.fitness.valid]

    try:
        # parallel on multiprocesor or clusters using SCOOP
        from scoop import futures
        fitnesses = futures.map(toolbox.evaluate, [cfg] * len(invalid_ind), invalid_ind)
        # print ('parallel-fitnesses: ', fitnesses)
    except ImportError or ImportWarning:
        # serial
        fitnesses = toolbox.map(toolbox.evaluate, [cfg] * len(invalid_ind), invalid_ind)
        # print ('serial-fitnesses: ', fitnesses)

    for ind, fit in zip(invalid_ind, fitnesses):
        ind.fitness.values = fit

    # This is just to assign the crowding distance to the individuals
    # no actual selection is done
    pop = toolbox.select(pop, pop_size)
    record = stats.compile(pop)
    logbook.record(gen=0, evals=len(invalid_ind), **record)
    print_message(logbook.stream)

    # Begin the generational process
    output_str = '### Generation number: %d, Population size: %d ###\n' % (gen_num, pop_size)
    print_message(output_str)
    UtilClass.writelog(cfg.logfile, output_str, mode='replace')

    for gen in range(1, gen_num + 1):
        output_str = '###### Generation: %d ######\n' % gen
        print_message(output_str)
        # Vary the population
        offspring = tools.selTournamentDCD(pop, int(pop_size * sel_rate))
        offspring = [toolbox.clone(ind) for ind in offspring]
        print_message('    offspring size: %d' % len(offspring))
        if len(offspring) >= 2:  # when offspring size greater than 2, mate can be done
            for ind1, ind2 in zip(offspring[::2], offspring[1::2]):
                if random.random() <= cx_rate:
                    if rule_cfg:
                        toolbox.mate_rule(cfg, ind1, ind2)
                    else:
                        toolbox.mate_random(ind1, ind2)
                if rule_cfg:
                    toolbox.mutate_rule(units_info, gene_to_unit, slppos_tagnames, suit_bmps, ind1,
                                        perc=mut_perc, indpb=mut_rate)
                    toolbox.mutate_rule(units_info, gene_to_unit, slppos_tagnames, suit_bmps, ind2,
                                        perc=mut_perc, indpb=mut_rate)
                else:
                    toolbox.mutate_random(possible_gene_values, ind1, perc=mut_perc, indpb=mut_rate)
                    toolbox.mutate_random(possible_gene_values, ind2, perc=mut_perc, indpb=mut_rate)
                del ind1.fitness.values, ind2.fitness.values

        # Evaluate the individuals with an invalid fitness
        invalid_ind = [ind for ind in offspring if not ind.fitness.valid]
        invalid_ind_size = len(invalid_ind)
        print_message('Evaluate pop size: %d' % invalid_ind_size)
        try:
            from scoop import futures
            fitnesses = futures.map(toolbox.evaluate, [cfg] * invalid_ind_size, invalid_ind)
        except ImportError or ImportWarning:
            fitnesses = toolbox.map(toolbox.evaluate, [cfg] * invalid_ind_size, invalid_ind)

        for ind, fit in zip(invalid_ind, fitnesses):
            ind.fitness.values = fit

        # Select the next generation population
        pop = toolbox.select(pop + offspring, pop_size)

        hyper_str = 'Gen: %d, hypervolume: %f\n' % (gen, hypervolume(pop))
        print_message(hyper_str)
        UtilClass.writelog(cfg.hypervlog, hyper_str, mode='append')

        record = stats.compile(pop)
        logbook.record(gen=gen, evals=len(invalid_ind), **record)
        print_message(logbook.stream)

        if gen % 1 == 0:
            # Create plot
            plot_pareto_front(pop, ws, len(invalid_ind), gen)
            # save in file
            output_str += 'economy\tenvironmental\tscenario\n'
            for indi in pop:
                output_str += '%f\t%f\t%s\n' % (indi.fitness.values[0], indi.fitness.values[1],
                                                str(indi))
            UtilClass.writelog(cfg.logfile, output_str, mode='append')

        # Delete SEIMS output files, and BMP Scenario database of current generation
        delete_model_outputs(cfg.model_dir, cfg.hostname, cfg.port, cfg.bmp_scenario_db)

    return pop, logbook


if __name__ == "__main__":
    cf = get_config_parser()
    cfg = SASPUConfig(cf)

    # # test the picklable of SASPUConfig class
    # import pickle
    #
    # s = pickle.dumps(cfg)
    # # print (s)
    # new_cfg = pickle.loads(s)
    # print (new_cfg.units_infos)

    print_message('### START TO SCENARIOS OPTIMIZING ###')
    startT = time.time()

    fpop, fstats = main(cfg)
    fpop.sort(key=lambda x: x.fitness.values)
    print_message(fstats)

    endT = time.time()
    print_message('Running time: %.2fs' % (endT - startT))
