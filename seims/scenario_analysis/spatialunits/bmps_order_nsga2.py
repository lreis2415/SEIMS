# -*- coding: utf-8 -*-
# """ BMPs order optimization based on slope position units.
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
import copy
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
from scenario_analysis.userdef import initIterateWithCfgIndv, initRepeatWithCfgIndv, \
    initRepeatWithCfgFromList, initIterateWithCfgIndvInput
from scenario_analysis.visualization import read_pareto_solutions_from_txt
from scenario_analysis.spatialunits.config import SASlpPosConfig, SAConnFieldConfig, \
    SACommUnitConfig
from scenario_analysis.spatialunits.scenario import SUScenario
from scenario_analysis.spatialunits.scenario import initialize_scenario, scenario_effectiveness, \
    initialize_scenario_with_bmps_order, scenario_effectiveness_with_bmps_order
from scenario_analysis.spatialunits.userdef import check_individual_diff, mutate_with_bmps_order

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
toolbox.register('gene_values', initialize_scenario_with_bmps_order)
toolbox.register('individual', initIterateWithCfgIndv, creator.Individual, toolbox.gene_values)
toolbox.register('population', initRepeatWithCfgIndv, list, toolbox.individual)

# register functions by inputs
toolbox.register('individual_byinput', initIterateWithCfgIndvInput, creator.Individual,
                 toolbox.gene_values)
toolbox.register('population_byinputs', initRepeatWithCfgFromList, list, toolbox.individual_byinput)

toolbox.register('evaluate', scenario_effectiveness_with_bmps_order)
toolbox.register('crossover', tools.cxTwoPoint)
toolbox.register('mutate', mutate_with_bmps_order)
toolbox.register('select', tools.selNSGA2)


def run_benchmark_scenario(sceobj):
    """ Base scenario: Set the gene values of the current scenario to 1.
    This means that all BMPs are implemented in the first year.
    Then run base scenario to get the environment effectiveness value."""
    new_gene_values = []
    for v in sceobj.gene_values:
        if numpy.isclose(v, 0.0):
            new_v = v
        else:
            new_v = int('{0}1'.format(int(v)))
        new_gene_values.append(new_v)

    copyed_sceobj = copy.deepcopy(sceobj)
    # benchmark scenario donot consider investment quota
    copyed_sceobj.cfg.enable_investment_quota = False
    benchmark_indv = creator.Individual(initialize_scenario_with_bmps_order(copyed_sceobj.cfg, new_gene_values, True))
    benchmark_indv = scenario_effectiveness_with_bmps_order(copyed_sceobj.cfg, benchmark_indv)
    sceobj.cfg.eval_info['BASE_ENV'] = benchmark_indv.fitness.values[1]
    scoop_log('Benchmark scenario economy: %f, environment %f, sed_sum: %f, sed_per_period: %s ' %
              (benchmark_indv.fitness.values[0], benchmark_indv.fitness.values[1], benchmark_indv.sed_sum,
               benchmark_indv.sed_per_period))


def main(scenario_obj, indv_obj_benchmark):
    # type: (SUScenario, Individual) -> ()
    """Main workflow of NSGA-II based Time Extended Scenario analysis."""
    # The Base scenario maintains the same evaluation method as the original one.
    # if scenario_obj.cfg.eval_info['BASE_ENV'] < 0:
    #     run_benchmark_scenario(scenario_obj)
    #     print('The environment effectiveness value of the '
    #           'base scenario is %.2f' % scenario_obj.cfg.eval_info['BASE_ENV'])

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
    if 0 not in possible_gene_values:  # 0 means no BMP is configured
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

    # Initialize population by specifying the file to avoid failure in the optimization process
    # and thus have to start from beginning again
    # PopulationSize must be the same as the original
    initialize_byinputs = False
    if scenario_obj.cfg.initial_byinput and scenario_obj.cfg.input_pareto_file is not None and \
        scenario_obj.cfg.input_pareto_gen > 0:  # Initial by input Pareto solutions
        inpareto_file = scenario_obj.modelcfg.model_dir + os.sep + scenario_obj.cfg.input_pareto_file
        if os.path.isfile(inpareto_file):
            inpareto_solutions = read_pareto_solutions_from_txt(inpareto_file,
                                                                sce_name='scenario',
                                                                field_name='gene_values')
            if scenario_obj.cfg.input_pareto_gen in inpareto_solutions:
                pareto_solutions = inpareto_solutions[scenario_obj.cfg.input_pareto_gen]
                pop = toolbox.population_byinputs(scenario_obj.cfg, pareto_solutions)  # type: List
                initialize_byinputs = True

    if not initialize_byinputs:
        pop = toolbox.population(scenario_obj.cfg, indv_obj_benchmark, n=pop_size)  # type: List
        print(pop)

    init_time = time.time() - stime

    def delete_fitness(new_ind):
        """Delete the fitness and other information of new individual."""
        del new_ind.fitness.values
        new_ind.gen = -1
        new_ind.id = -1
        new_ind.io_time = 0.
        new_ind.comp_time = 0.
        new_ind.simu_time = 0.
        new_ind.runtime = 0.
        new_ind.sed_sum = 0.
        new_ind.sed_per_period = list()
        new_ind.net_costs_per_period = list()

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
            invalid_pops = list(futures.map(toolbox.evaluate, [scenario_obj.cfg] * popnum, invalid_pops))
        except ImportError or ImportWarning:
            # serial
            invalid_pops = list(toolbox.map(toolbox.evaluate, [scenario_obj.cfg] * popnum, invalid_pops))

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
    numpy.savetxt(scenario_obj.scenario_dir + os.sep + 'pareto_front_with_bmps_order_gen0.txt',
                  front, delimiter=str(' '), fmt=str('%.4f'))

    # Begin the generational process
    output_str = '### Generation number: %d, Population size: %d ###\n' % (gen_num, pop_size)
    scoop_log(output_str)
    UtilClass.writelog(scenario_obj.cfg.opt.logfile, output_str, mode='replace')

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
                    toolbox.crossover(ind1, ind2)

                toolbox.mutate(ind1, 1, scenario_obj.cfg.change_times, mut_rate)
                toolbox.mutate(ind2, 1, scenario_obj.cfg.change_times, mut_rate)

                if check_individual_diff(old_ind1, ind1):
                    delete_fitness(ind1)  # delete fitness, valid will be false
                if check_individual_diff(old_ind2, ind2):
                    delete_fitness(ind2)

        # only evaluate the individuals with invalid fitness
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
        # Previous version may result in duplications of the same scenario in one Pareto front,
        #   thus, I decided to check and remove the duplications first.
        # pop = toolbox.select(pop + valid_inds + invalid_inds, pop_select_num)
        # remove individuals with duplicated gen and id
        tmppop = pop + valid_inds + invalid_inds
        pop = list()
        unique_sces = dict()
        for tmpind in tmppop:
            if tmpind.gen in unique_sces and tmpind.id in unique_sces[tmpind.gen]:
                continue
            if tmpind.gen not in unique_sces:
                unique_sces.setdefault(tmpind.gen, [tmpind.id])
            elif tmpind.id not in unique_sces[tmpind.gen]:
                unique_sces[tmpind.gen].append(tmpind.id)
            pop.append(tmpind)
        pop = toolbox.select(pop, pop_select_num)

        hyper_str = 'Gen: %d, New model runs: %d, ' \
                    'Execute timespan: %.4f, Sum of model run timespan: %.4f, ' \
                    'Hypervolume: %.4f\n' % (gen, invalid_ind_size,
                                             curtimespan, modelruns_time_sum[gen],
                                             hypervolume(pop, ref_pt))
        scoop_log(hyper_str)
        UtilClass.writelog(scenario_obj.cfg.opt.hypervlog, hyper_str, mode='append')

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
        numpy.savetxt(scenario_obj.scenario_dir + os.sep + 'pareto_front_with_bmps_order_gen%d.txt' % gen,
                      front, delimiter=str(' '), fmt=str('%.4f'))

        # Comment out the following plot code if matplotlib does not work.
        try:
            from scenario_analysis.visualization import plot_pareto_front_single
            pareto_title = 'Near Pareto optimal solutions'
            xlabel = 'Economy'
            ylabel = 'Environment'
            if scenario_obj.cfg.plot_cfg.plot_cn:
                xlabel = r'经济净投入'
                ylabel = r'环境效益'
                pareto_title = r'近似最优Pareto解集'
            plot_pareto_front_single(front, [xlabel, ylabel],
                                     ws, gen, pareto_title,
                                     plot_cfg=scenario_obj.cfg.plot_cfg)
        except Exception as e:
            scoop_log('Exception caught: %s' % str(e))
        plot_time += time.time() - stime

        # save in file
        output_str += 'generation\tscenario\teconomy\tenvironment\tsed_sum\tsed_pp\tnet_cost_pp\tgene_values\n'
        for indi in pop:
            output_str += '%d\t%d\t%f\t%f\t%f\t%s\t%s\t%s\n' % (indi.gen, indi.id, indi.fitness.values[0],
                indi.fitness.values[1], indi.sed_sum, str(indi.sed_per_period), str(indi.net_costs_per_period), str(indi))
        UtilClass.writelog(scenario_obj.cfg.opt.logfile, output_str, mode='append')

        pklfile_str = 'gen%d.pickle' % (gen,)
        with open(scenario_obj.cfg.opt.simdata_dir + os.path.sep + pklfile_str, 'wb') as pklfp:
            pickle.dump(pop, pklfp)

    # Plot hypervolume and newly executed model count
    # Comment out the following plot code if matplotlib does not work.
    try:
        from scenario_analysis.visualization import plot_hypervolume_single
        plot_hypervolume_single(scenario_obj.cfg.opt.hypervlog, ws, plot_cfg=scenario_obj.cfg.plot_cfg)
    except Exception as e:
        scoop_log('Exception caught: %s' % str(e))

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

    with open(sa_cfg.model.model_dir + os.path.sep + 'gen63.pickle', 'rb') as fp:
        pareto_pop = pickle.load(fp)
        # print(type(pareto_pop))
        # print(pareto_pop)
    sce = SUScenario(sa_cfg)

    scoop_log('### START TO SCENARIOS OPTIMIZING ###')
    startT = time.time()

    # Select an individual from the pareto front as the benchmark scenario
    target_indv_id = 156278373  # modify to an input parameter later
    for indv in pareto_pop:
        if indv.id == target_indv_id:
            selected_indv = indv
            break

    print('The ID of the selected scenario that provided spatial configuration: ' + str(selected_indv.id))
    print('The genes of the selected scenario: ' + str(selected_indv.tolist()))
    sce.set_unique_id(selected_indv.id)
    sce.gene_values = selected_indv.tolist()
    sce.economy = selected_indv.fitness.values[0]
    sce.environment = selected_indv.fitness.values[1]
    sce.export_scenario_to_txt()
    # sce.export_scenario_to_gtiff()

    time_pareto_pop, time_pareto_stats = main(sce, selected_indv)

    time_pareto_pop.sort(key=lambda x: x.fitness.values)
    scoop_log(time_pareto_stats)
    with open(sa_cfg.opt.logbookfile, 'w', encoding='utf-8') as f:
        # In case of 'TypeError: write() argument 1 must be unicode, not str' in Python2.7
        #   when using unicode_literals, please use '%s' to concatenate string!
        f.write('%s' % time_pareto_stats.__str__())

    endT = time.time()
    scoop_log('Running time: %.2fs' % (endT - startT))
