# -*- coding: utf-8 -*-
# """ BMPs location and order optimization based on slope position units.
#
#     @author   : Tong Wu
#
# """
from __future__ import absolute_import, unicode_literals

import array
import json
import os
import sys
import random
import time
import pickle
import copy
import logging
from io import open

import matplotlib

if os.name != 'nt':  # Force matplotlib to not use any Xwindows backend.
    matplotlib.use('Agg')

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
    initRepeatWithCfgFromList, initIterateWithCfgIndvInput, initIterateWithCfg, initRepeatWithCfg, \
    initIterateWithCfgWithInput
from scenario_analysis.visualization import read_pareto_solutions_from_txt
from scenario_analysis.spatialunits.config import SASlpPosConfig, SAConnFieldConfig, \
    SACommUnitConfig
from scenario_analysis.spatialunits.scenario import SUScenario
from scenario_analysis.spatialunits.scenario import initialize_scenario, scenario_effectiveness, \
    initialize_scenario_with_bmps_order, initialize_scenario_s_t, scenario_effectiveness_with_bmps_order, get_key_bmps
from scenario_analysis.spatialunits.userdef import check_individual_diff, mutate_with_bmps_order, crossover_slppos, \
    mutate_rule_s_t, mutate_rule_s
from scenario_analysis.spatialunits.userdef import check_individual_diff, \
    crossover_rdm, crossover_slppos, crossover_updown, mutate_rule, mutate_rdm
from scenario_analysis.deap_tool import selNSGA2_prefer, calculate_preference_score, interactive_selection, \
    update_preference_params, merge_multiuser_pops, merge_multiuser_prefs
from scenario_analysis.interactive_algorithm import InteractiveAlgorithm
from scenario_analysis.async_interactive_algorithm import AsyncInteractiveAlgorithm

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
# toolbox.register('gene_values', initialize_scenario_with_bmps_order)
toolbox.register('gene_values', initialize_scenario_s_t)
# toolbox.register('individual', initIterateWithCfgIndv, creator.Individual, toolbox.gene_values)
toolbox.register('individual', initIterateWithCfg, creator.Individual, toolbox.gene_values)
# toolbox.register('population', initRepeatWithCfgIndv, list, toolbox.individual)
toolbox.register('population', initRepeatWithCfg, list, toolbox.individual)

# register functions by inputs
# toolbox.register('individual_byinput', initIterateWithCfgIndvInput, creator.Individual,
#                  toolbox.gene_values)
toolbox.register('individual_byinput', initIterateWithCfgWithInput, creator.Individual,
                 toolbox.gene_values)
toolbox.register('population_byinputs', initRepeatWithCfgFromList, list, toolbox.individual_byinput)

toolbox.register('evaluate', scenario_effectiveness_with_bmps_order)
toolbox.register('crossover', tools.cxTwoPoint)
# toolbox.register('mutate', mutate_with_bmps_order)
toolbox.register('mutate_s_t', mutate_rule_s_t)
toolbox.register('mutate_s', mutate_rule_s)
toolbox.register('select', tools.selNSGA2)
toolbox.register('select_prefer', selNSGA2_prefer)
ref_points = tools.uniform_reference_points(nobj=2, p=12)
toolbox.register('select3', tools.selNSGA3, ref_points=ref_points)
toolbox.register('mate_slppos', crossover_slppos)
toolbox.register('mate_updown', crossover_updown)
toolbox.register('mate_rdm', crossover_rdm)


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


def run_base_scenario(sceobj):
    """Run base scenario to get the environment effectiveness value."""
    copyed_sceobj = copy.deepcopy(sceobj)
    base_ind = creator.Individual(initialize_scenario_s_t(copyed_sceobj.cfg))
    for i in list(range(len(base_ind))):
        base_ind[i] = 0
    base_ind = scenario_effectiveness_with_bmps_order(copyed_sceobj.cfg, base_ind)
    sceobj.cfg.eval_info['BASE_ENV'] = base_ind.sed_sum
    scoop_log('Benchmark scenario economy: %f, environment %f, sed_sum: %f, sed_per_period: %s ' %
              (base_ind.fitness.values[0], base_ind.fitness.values[1], base_ind.sed_sum,
               base_ind.sed_per_period))


def main(scenario_obj):
    # type: (SUScenario, Individual) -> ()
    """Main workflow of NSGA-II based Spatio-temporal Scenario analysis."""
    # The Base scenario maintains the same evaluation method as the original one.

    if scenario_obj.cfg.eval_info['BASE_ENV'] < 0:
        # run_benchmark_scenario(scenario_obj)
        run_base_scenario(scenario_obj)
        print('The environment effectiveness value of the '
              'base scenario is %.2f' % scenario_obj.cfg.eval_info['BASE_ENV'])

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

    if not initialize_byinputs:
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
            pop = toolbox.population(scenario_obj.cfg, n=pop_size)  # type: List
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
        new_ind.costs_per_period = list()
        new_ind.incomes_per_period = list()

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

    #pop = toolbox.select_prefer(pop, pop_select_num, preference_params=scenario_obj.cfg.preference_param)
    record = stats.compile(pop)
    logbook.record(gen=0, evals=len(pop), **record)
    scoop_log(logbook.stream)
    front = numpy.array([ind.fitness.values for ind in pop])
    # save front for further possible use
    # FIXED: Unified output file naming
    # OLD: 'pareto_front_s_t_gen0.txt'
    numpy.savetxt(scenario_obj.scenario_dir + os.sep + 'pareto_front_gen0.txt',
                  front, delimiter=str(' '), fmt=str('%.4f'))

    # Begin the generational process
    output_str = '### Generation number: %d, Population size: %d ###\n' % (gen_num, pop_size)
    scoop_log(output_str)
    UtilClass.writelog(scenario_obj.cfg.opt.logfile, output_str, mode='replace')

    modelsel_count = {0: len(pop)}  # type: Dict[int, int] # newly added Pareto fronts

    fixed_positions = scenario_obj.cfg.key_bmps
    # 使用 InteractiveAlgorithm 封装所有交互逻辑，替代原先内联的 enable_interactive/merged_prefs 代码
    # 根据 enable_async 选择同步或异步交互算法
    enable_async = getattr(scenario_obj.cfg, 'enable_async', False)
    if enable_async:
        ia = AsyncInteractiveAlgorithm(
            enable_interactive=getattr(scenario_obj.cfg, 'enable_interactive', False),
            interactive_interval=getattr(scenario_obj.cfg, 'interactive_interval', 10),
            users=getattr(scenario_obj.cfg, 'users', {}),
            logger=logging.getLogger(__name__),
            enable_async=True,
            task_id=getattr(scenario_obj.cfg, 'async_task_id', 'default_task'),
            signal_dir=getattr(scenario_obj.cfg, 'async_signal_dir', '/data/config'),
            checkpoint_dir=getattr(scenario_obj.cfg, 'async_checkpoint_dir', '/data/checkpoints')
        )
        scoop_log('Async Interactive mode enabled: task_id=%s, interval=%d generations' %
                  (getattr(scenario_obj.cfg, 'async_task_id', 'default_task'),
                   getattr(scenario_obj.cfg, 'interactive_interval', 10)))
        ia.register_to_toolbox(toolbox)
    else:
        ia = InteractiveAlgorithm(
            enable_interactive=getattr(scenario_obj.cfg, 'enable_interactive', False),
            interactive_interval=getattr(scenario_obj.cfg, 'interactive_interval', 10),
            users=getattr(scenario_obj.cfg, 'users', {}),
            logger=logging.getLogger(__name__)
        )
        ia.register_to_toolbox(toolbox)
        if ia.enable_interactive:
            scoop_log('Interactive mode enabled: %d users, interval=%d generations' %
                      (len(ia.users), ia.interactive_interval))

    # 异步模式下：检查是否有checkpoint，有则从checkpoint加载（ia必须先初始化）
    if enable_async and hasattr(ia, 'has_checkpoint'):
        latest_gen = ia.get_latest_checkpoint_gen()
        if latest_gen is not None and latest_gen >= 0:
            scoop_log(f'Loading checkpoint from generation {latest_gen}...')
            pop = ia.load_from_checkpoint(latest_gen)
            if pop is not None:
                initialize_byinputs = True
                scoop_log(f'Checkpoint loaded successfully: {len(pop)} individuals')

    for gen in range(1, gen_num + 1):
        output_str = '###### Generation: %d ######\n' % gen
        scoop_log(output_str)
        offspring = [toolbox.clone(ind) for ind in pop]
        if len(offspring) >= 2:  # when offspring size greater than 2, mate can be done
            for ind1, ind2 in zip(offspring[::2], offspring[1::2]):
                old_ind1 = toolbox.clone(ind1)
                old_ind2 = toolbox.clone(ind2)
                if random.random() <= cx_rate:
                    # toolbox.crossover(ind1, ind2)
                    if cfg_method == BMPS_CFG_METHODS[3]:  # SLPPOS method
                        toolbox.mate_slppos(ind1, ind2, scenario_obj.cfg.hillslp_genes_num, fixed_positions)
                    elif cfg_method == BMPS_CFG_METHODS[2]:  # UPDOWN method
                        toolbox.mate_updown(updown_units, gene_to_unit, unit_to_gene, ind1, ind2)
                    else:
                        toolbox.mate_rdm(ind1, ind2)

                # toolbox.mutate(ind1, 1, scenario_obj.cfg.change_times, mut_rate, mut_perc)
                # toolbox.mutate(ind2, 1, scenario_obj.cfg.change_times, mut_rate, mut_perc)

                # Mutation Gene values for rule-based BMP configuration strategies with implementation order.
                # Note: cfg_method != BMPS_CFG_METHODS[0]
                if cfg_method != BMPS_CFG_METHODS[0]:
                    tagnames = None
                    if scenario_obj.cfg.bmps_cfg_unit == BMPS_CFG_UNITS[3]:
                        tagnames = scenario_obj.cfg.slppos_tagnames
                    if scenario_obj.cfg.enable_implementation_order:
                        toolbox.mutate_s_t(units_info, gene_to_unit, unit_to_gene,
                                           suit_bmps, ind1, fixed_positions=fixed_positions,
                                           perc=mut_perc, indpb=mut_rate,
                                           unit=cfg_unit, method=cfg_method,
                                           tagnames=tagnames,
                                           low=1,
                                           up=scenario_obj.cfg.change_times)
                        toolbox.mutate_s_t(units_info, gene_to_unit, unit_to_gene,
                                           suit_bmps, ind2, fixed_positions=fixed_positions,
                                           perc=mut_perc, indpb=mut_rate,
                                           unit=cfg_unit, method=cfg_method,
                                           tagnames=tagnames,
                                           low=1,
                                           up=scenario_obj.cfg.change_times)
                    else:
                        toolbox.mutate_s(units_info, gene_to_unit, unit_to_gene,
                                         suit_bmps, ind1, fixed_positions=fixed_positions,
                                         perc=mut_perc, indpb=mut_rate,
                                         unit=cfg_unit, method=cfg_method,
                                         tagnames=tagnames)
                        toolbox.mutate_s(units_info, gene_to_unit, unit_to_gene,
                                         suit_bmps, ind2, fixed_positions=fixed_positions,
                                         perc=mut_perc, indpb=mut_rate,
                                         unit=cfg_unit, method=cfg_method,
                                         tagnames=tagnames)

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
        # 使用 InteractiveAlgorithm 统一处理种群选择（自动区分偏好/标准选择）
        pop = ia.select_population(toolbox, pop, pop_select_num)
        # 若当前代需要交互，执行用户交互流程并更新偏好参数
        ia.run_interaction(pop, gen)

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
        scenarios = numpy.array([ind.id for ind in pop])

        # NEW: Only save preference_score/front_id when interactive mode is enabled
        # OLD: Always tried to access ind.fitness.preference_score which doesn't exist in non-interactive mode
        if enable_interactive and merged_prefs:
            front_array = numpy.array([
                numpy.concatenate([
                    ind.fitness.values,  # 浮点数组
                    [getattr(ind.fitness, 'preference_score', 0.0)],  # 浮点标量
                    [getattr(ind.fitness, 'front_id', 0)],  # 整型标量（存储为float）
                    [ind.id]  # 整型标量（存储为float）
                ]) for ind in pop
            ])
            n_float_columns = len(pop[0].fitness.values) + 1  # values列数 + preference_score
            n_int_columns = 2  # front_id + id
            fmt_str = ' '.join(['%.4f'] * n_float_columns + ['%d'] * n_int_columns)

            # save front with preference info
            # FIXED: Unified naming - interactive mode saves extra preference data as separate file
            # OLD: pareto_front_with_s_t_gen{gen}.txt
            numpy.savetxt(scenario_obj.scenario_dir + os.sep + f'pareto_front_prefer_gen{gen}.txt',
                          front_array, delimiter=str(' '), fmt=fmt_str)
        else:
            # Non-interactive: no extra preference file needed
            pass
        # FIXED: Unified output file naming for all modes
        # OLD: pareto_front_with_s_t_gen{gen}.txt / pareto_front_values_with_s_t_gen{gen}.txt
        numpy.savetxt(scenario_obj.scenario_dir + os.sep + f'pareto_front_gen{gen}.txt',
                      front, delimiter=str(' '), fmt=str('%.4f'))
        numpy.savetxt(
            scenario_obj.scenario_dir + os.sep + f'pareto_front_scenarios_gen{gen}.txt',
            scenarios, delimiter=' ', fmt='%.4f')

        '''front_array_user = numpy.array([
            numpy.concatenate([
                ind.fitness.values,  # 浮点数组（例如[0.85, 0.92]）
                [ind.fitness.preference_score],  # 浮点标量（例如0.78）
                [ind.fitness.front_id],  # 显式转换为浮点（例如3.0）
                [ind.id]  # 显式转换为浮点（例如1024.0）
            ])
            # 修正循环顺序：先遍历每个用户的种群，再遍历种群中的个体
            for user_pop in user_pops  # 外层循环遍历用户种群列表
            for ind in user_pop  # 内层循环遍历当前种群中的个体
        ])
        numpy.savetxt(scenario_obj.scenario_dir + os.sep + 'pareto_front_with_s_t_user_gen%d.txt' % gen,
                      front_array_user, delimiter=str(' '), fmt=fmt_str)'''
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
        output_str += 'generation\tscenario\teconomy\tenvironment\tsed_sum\tsed_pp\tnet_cost_pp\tcosts_pp\tincomes_pp\tgene_values\n'
        for indi in pop:
            output_str += '%d\t%d\t%f\t%f\t%f\t%s\t%s\t%s\t%s\t%s\n' % (indi.gen, indi.id, indi.fitness.values[0],
                                                                        indi.fitness.values[1], indi.sed_sum,
                                                                        str(indi.sed_per_period),
                                                                        str(indi.net_costs_per_period),
                                                                        str(indi.costs_per_period),
                                                                        str(indi.incomes_per_period), str(indi))
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
    sa_cfg = SASlpPosConfig(in_cf)
    sa_cfg.construct_indexes_units_gene()
    get_key_bmps(sa_cfg)

    sce = SUScenario(sa_cfg)
    key_bmp = {9: 1, 16: 2, 43: 2, 60: 1, 61: 3, 66: 1, 67: 2, 69: 1}
    sce.cfg.key_bmps=key_bmp
    # 定义偏好参数
    ori_preference_params = {
        'economy': (78, 'equal', 2),
        'environment': (12.5, 'equal', 0.0),
        'abandon_possibility': (3.04764, 'equal', 0.0),
        'cost_variation': (148.91808, 'equal', 20),
        'return_on_invest': (0.08936, 'equal', 0.018),
        'env_on_invest': (0.16374, 'equal', 0.01257),
        'bmp_rules': {
            "position_specific_rules": {9: 1, 16: 2, 43: 2},
            "type_distribution_rules": {}
        }

    }
    users = {
        "user2": {
            "history_good": [],
            "history_bad": [],
            "history_bad_reasons": [],
            "preference_param": {
                'economy': (66.58, 'greater', 5),
                'environment': (14.33, 'greater', 2),
                'abandon_possibility': (2.6, 'less', 0.8),
                'cost_variation': (135, 'equal', 120),
                'return_on_invest': (0.08, 'equal', 0.015),
                'env_on_invest': (0.215, 'greater', 0.012),
                'bmp_rules': {
                    "position_specific_rules": {},
                    "type_distribution_rules": {}
                }
            },
        },"user1": {
            "history_good": [],
            "history_bad": [],
            "history_bad_reasons": [],
            "preference_param": {
                'economy': (63.85, 'less', 4.5),
                'environment': (8.89, 'equal', 0.0),
                'abandon_possibility': (3.1, 'equal', 0.0),
                'cost_variation': (123, 'less', 125),
                'return_on_invest': (0.06, 'equal', 0.02),
                'env_on_invest': (0.15, 'equal', 0.02),
                'bmp_rules': {
                    "position_specific_rules": {},
                    "type_distribution_rules": {}
                }
            },
        }

    }
    '''
,
        # 用户1的配置

        '''
    sce.cfg.users = users
    sce.cfg.preference_param = ori_preference_params
    # selectedScenarioFile = sa_cfg.model.model_dir + os.sep + sa_cfg.selected_scenario_file
    # with open(selectedScenarioFile) as fp:
    #     for line in fp.readlines():
    #         items = line.split(':')
    #         if items[0] == 'Scenario ID':
    #             sceid = int(items[1])
    #         elif items[0] == 'Gene number':
    #             geneNum = int(items[1])
    #         elif items[0] == 'Gene values':
    #             gvalues = [float(v.strip()) for v in items[1].split(',')]
    #         else:
    #             pass
    #
    # sce.set_unique_id(sceid)
    # sce.initialize(input_genes=gvalues)
    # print('The ID of the selected scenario that provided spatial configuration: ' + str(sce.ID))
    # print('The genes of the selected scenario: ' + str(gvalues))

    scoop_log('### START TO SCENARIOS OPTIMIZING ###')
    startT = time.time()

    s_t_pareto_pop, s_t_pareto_stats = main(sce)

    s_t_pareto_pop.sort(key=lambda x: x.fitness.values)
    scoop_log(s_t_pareto_stats)
    with open(sa_cfg.opt.logbookfile, 'w', encoding='utf-8') as f:
        # In case of 'TypeError: write() argument 1 must be unicode, not str' in Python2.7
        #   when using unicode_literals, please use '%s' to concatenate string!
        f.write('%s' % s_t_pareto_stats.__str__())

    endT = time.time()
    scoop_log('Running time: %.2fs' % (endT - startT))
