#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Calibration by NSGA-II algorithm.
    @author   : Liangjun Zhu
    @changelog: 18-1-22  lj - initial implementation.\n
"""
import array
import os
import random
import time

import matplotlib

if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    matplotlib.use('Agg', warn=False)
import numpy
from deap import base
from deap import creator
from deap import tools
from deap.benchmarks.tools import hypervolume
from pygeoc.utils import UtilClass

from config import CaliConfig, get_cali_config
from calibrate import Calibration, initialize_calibrations, calibration_objectives
from scenario_analysis.utility import print_message
from scenario_analysis.userdef import initIterateWithCfg, initRepeatWithCfg
from scenario_analysis.visualization import plot_pareto_front
from userdef import write_param_values_to_mongodb

# Definitions, assignments, operations, etc. that will be executed by each worker
#    when paralleled by SCOOP.
# Thus, DEAP related operations (initialize, register, etc.) are better defined here.

# Multiobjects: Maximum Nash-Sutcliffe coefficients of discharge and sediment
multi_weight = (1., 1.)
creator.create('FitnessMulti', base.Fitness, weights=multi_weight)
# The FitnessMulti class equals to:
# class FitnessMulti(base.Fitness):
#     weights = (1., 1.)
creator.create('Individual', array.array, typecode='d', fitness=creator.FitnessMulti,
               gen=-1, id=-1)
# The Individual class equals to:
# class Individual(array.array):
#     gen = -1  # Generation No.
#     id = -1   # Calibration index of current generation
#     def __init__(self):
#         self.fitness = FitnessMulti()

# Register NSGA-II related operations
toolbox = base.Toolbox()
toolbox.register('gene_values', initialize_calibrations)
toolbox.register('individual', initIterateWithCfg, creator.Individual, toolbox.gene_values)
toolbox.register('population', initRepeatWithCfg, list, toolbox.individual)
toolbox.register('evaluate', calibration_objectives)

# mate and mutate
toolbox.register('mate', tools.cxSimulatedBinaryBounded)
toolbox.register('mutate', tools.mutPolynomialBounded)

toolbox.register('select', tools.selNSGA2)


def main(cfg):
    """Main workflow of NSGA-II based Scenario analysis."""
    random.seed()
    print_message('Population: %d, Generation: %d' % (cfg.opt.npop, cfg.opt.ngens))

    # create reference point for hypervolume
    ref_pt = numpy.array([-1., -1.]) * multi_weight * -1

    stats = tools.Statistics(lambda sind: sind.fitness.values)
    stats.register('min', numpy.min, axis=0)
    stats.register('max', numpy.max, axis=0)
    stats.register('avg', numpy.mean, axis=0)
    stats.register('std', numpy.std, axis=0)

    logbook = tools.Logbook()
    logbook.header = 'gen', 'evals', 'min', 'max', 'avg', 'std'

    # Initialize population
    # pop = toolbox.population(cfg, n=cfg.opt.npop) # Deprecated method!
    cali_obj = Calibration(cfg)
    param_values = cali_obj.initialize(cfg.opt.npop)
    pop = list()
    for i in range(cfg.opt.npop):
        ind = creator.Individual(param_values[i])
        ind.gen = 0
        ind.id = i
        pop.append(ind)
    param_values = numpy.array(param_values)
    write_param_values_to_mongodb(cfg.hostname, cfg.port, cfg.spatial_db,
                                  cali_obj.ParamDefs, param_values)
    # get the low and up bound of calibrated parameters
    bounds = numpy.array(cali_obj.ParamDefs['bounds'])
    low = bounds[:, 0]
    up = bounds[:, 1]
    low = low.tolist()
    up = up.tolist()
    # Evaluate the individuals with an invalid fitness
    invalid_ind = [ind for ind in pop if not ind.fitness.valid]

    try:
        # parallel on multiprocesor or clusters using SCOOP
        from scoop import futures
        fitnesses = futures.map(toolbox.evaluate, [cali_obj] * len(invalid_ind), invalid_ind)
        # print ('parallel-fitnesses: ', fitnesses)
    except ImportError or ImportWarning:
        # serial
        fitnesses = toolbox.map(toolbox.evaluate, [cali_obj] * len(invalid_ind), invalid_ind)
        # print ('serial-fitnesses: ', fitnesses)

    for ind, fit in zip(invalid_ind, fitnesses):
        ind.fitness.values = [fit[0], fit[5]]

    pop = toolbox.select(pop, int(cfg.opt.npop * cfg.opt.rsel))
    record = stats.compile(pop)
    logbook.record(gen=0, evals=len(pop), **record)
    print_message(logbook.stream)

    # Begin the generational process
    output_str = '### Generation number: %d, Population size: %d ###\n' % (cfg.opt.ngens,
                                                                           cfg.opt.npop)
    print_message(output_str)
    UtilClass.writelog(cfg.opt.logfile, output_str, mode='replace')

    for gen in range(1, cfg.opt.ngens + 1):
        output_str = '###### Generation: %d ######\n' % gen
        print_message(output_str)

        offspring = [toolbox.clone(ind) for ind in pop]
        # method1: use crowding distance (normalized as 0~1) as eta
        # tools.emo.assignCrowdingDist(offspring)
        # method2: use the index of individual at the sorted offspring list as eta
        if len(offspring) >= 2:  # when offspring size greater than 2, mate can be done
            for i, ind1, ind2 in zip(range(len(offspring) / 2), offspring[::2], offspring[1::2]):
                if random.random() > cfg.opt.rcross:
                    continue
                # if ind1.fitness.crowding_dist == float('inf') and \
                #         ind2.fitness.crowding_dist == float('inf'):
                #     eta = 1.
                # elif ind1.fitness.crowding_dist == float('inf') and \
                #         ind2.fitness.crowding_dist != float('inf'):
                #     eta = ind2.fitness.crowding_dist
                # elif ind1.fitness.crowding_dist != float('inf') and \
                #         ind2.fitness.crowding_dist == float('inf'):
                #     eta = ind1.fitness.crowding_dist
                # else:
                #     eta = (ind1.fitness.crowding_dist + ind2.fitness.crowding_dist) / 2.
                eta = i  # method2
                toolbox.mate(ind1, ind2, eta, low, up)
                toolbox.mutate(ind1, eta, low, up, cfg.opt.rmut)
                toolbox.mutate(ind2, eta, low, up, cfg.opt.rmut)
                del ind1.fitness.values, ind2.fitness.values
        else:
            toolbox.mutate(offspring[0], 1., low, up, cfg.opt.rmut)
            del offspring[0].fitness.values

        # Evaluate the individuals with an invalid fitness
        invalid_ind = [ind for ind in offspring if not ind.fitness.valid]
        invalid_ind_size = len(invalid_ind)
        # Write new calibrated parameters to MongoDB
        param_values = list()
        for idx, ind in enumerate(invalid_ind):
            ind.gen = gen
            ind.id = idx
            param_values.append(ind[:])
        param_values = numpy.array(param_values)
        write_param_values_to_mongodb(cfg.hostname, cfg.port, cfg.spatial_db,
                                      cali_obj.ParamDefs, param_values)
        # print_message('Evaluate pop size: %d' % invalid_ind_size)
        try:
            from scoop import futures
            fitnesses = futures.map(toolbox.evaluate, [cali_obj] * invalid_ind_size, invalid_ind)
        except ImportError or ImportWarning:
            fitnesses = toolbox.map(toolbox.evaluate, [cali_obj] * invalid_ind_size, invalid_ind)

        for ind, fit in zip(invalid_ind, fitnesses):
            ind.fitness.values = [fit[0], fit[5]]

        # Select the next generation population
        pop = toolbox.select(pop + offspring, int(cfg.opt.npop * cfg.opt.rsel))

        hyper_str = 'Gen: %d, hypervolume: %f\n' % (gen, hypervolume(pop, ref_pt))
        print_message(hyper_str)
        UtilClass.writelog(cfg.opt.hypervlog, hyper_str, mode='append')

        record = stats.compile(pop)
        logbook.record(gen=gen, evals=len(invalid_ind), **record)
        print_message(logbook.stream)

        # Create plot
        plot_pareto_front(pop, cfg.opt.out_dir, gen, 'Pareto frontier of Calibration',
                          'NSE of discharge',
                          'NSE of sediment')
        # save in file
        output_str += 'generation-calibrationID\tNSE-Q\tNSE-SED\tgene_values\n'
        for indi in pop:
            output_str += '%d-%d\t%f\t%f\t%s\n' % (indi.gen, indi.id, indi.fitness.values[0],
                                                   indi.fitness.values[1], str(indi))
        UtilClass.writelog(cfg.opt.logfile, output_str, mode='append')

    return pop, logbook


if __name__ == "__main__":
    cf, method = get_cali_config()
    cfg = CaliConfig(cf, method=method)

    print (cfg)

    print_message('### START TO CALIBRATION OPTIMIZING ###')
    startT = time.time()
    fpop, fstats = main(cfg)
    fpop.sort(key=lambda x: x.fitness.values)
    print_message(fstats)
    with open(cfg.opt.logbookfile, 'w') as f:
        f.write(fstats.__str__())

    endT = time.time()
    print_message('Running time: %.2fs' % (endT - startT))
