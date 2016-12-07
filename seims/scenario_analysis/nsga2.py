import os
import array
import matplotlib
if os.name != 'nt':
    # Force matplotlib to not use any Xwindows backend.
    matplotlib.use('Agg')
import matplotlib.pyplot as plt
import scoop
import platform
from deap import base
from deap import benchmarks
from deap import creator
from deap import tools
from deap.benchmarks.tools import hypervolume
from scenario import *
from userdef import *

creator.create("FitnessMin", base.Fitness, weights=(-1.0, -1.0))
creator.create("Individuals", array.array, typecode='d', fitness=creator.FitnessMin)
toolbox = base.Toolbox()
#
def iniPops():
    bmpSce = Scenario()
    bmpSce.create()
    return bmpSce.attributes

toolbox.register("attr_float", iniPops)
toolbox.register("individual", tools.initIterate, creator.Individuals, toolbox.attr_float)
toolbox.register("population", tools.initRepeat, list, toolbox.individual)
toolbox.register("evaluate", calBenefitandCost)
toolbox.register("mate", tools.cxOnePoint)
toolbox.register("mutate", mutModel, indpb=MutateRate)
toolbox.register("select", tools.selNSGA2)


def main(num_Gens, size_Pops, cx, seed=None):

    # toolbox.register("attr_float", iniPops)
    # toolbox.register("individual", tools.initIterate, creator.Individuals, toolbox.attr_float)
    # toolbox.register("population", tools.initRepeat, list, toolbox.individual)
    # toolbox.register("evaluate", calBenefitandCost)
    # toolbox.register("mate", tools.cxOnePoint)
    # toolbox.register("mutate", mutModel, indpb=MutateRate)
    # toolbox.register("select", tools.selNSGA2)

    random.seed(seed)
    stats = tools.Statistics(lambda ind: ind.fitness.values)
    stats.register("min", numpy.min, axis=0)
    stats.register("max", numpy.max, axis=0)
    # stats.register("avg", numpy.mean, axis=0)
    # stats.register("std", numpy.std, axis=0)

    logbook = tools.Logbook()
    logbook.header = "gen", "evals", "min", "max"
    
    pop = toolbox.population(n=size_Pops)
    # Evaluate the individuals with an invalid fitness
    invalid_ind = [ind for ind in pop if not ind.fitness.valid]

    try:
        # parallel on multiprocesor or clusters using SCOOP
        from scoop import futures
        fitnesses = futures.map(toolbox.evaluate, invalid_ind)
        # print "parallel-fitnesses: ",fitnesses
    except ImportError or ImportWarning:
        # serial
        fitnesses = toolbox.map(toolbox.evaluate, invalid_ind)
        # print "serial-fitnesses: ",fitnesses

    for ind, fit in zip(invalid_ind, fitnesses):
        ind.fitness.values = fit

    # This is just to assign the crowding distance to the individuals
    # no actual selection is done
    pop = toolbox.select(pop, int(len(pop) * SelectRate))
    record = stats.compile(pop)
    logbook.record(gen=0, evals=len(invalid_ind), **record)
    print(logbook.stream)

    # Begin the generational process
    for gen in range(1, num_Gens):
        printInfo("###### Iteration: %d ######" % gen)
        # Vary the population
        offspring = tools.selTournamentDCD(pop, int(len(pop) * SelectRate))
        offspring = [toolbox.clone(ind) for ind in offspring]
        for ind1, ind2 in zip(offspring[::2], offspring[1::2]):
            if random.random() <= cx:
                toolbox.mate(ind1, ind2)
            toolbox.mutate(ind1)
            toolbox.mutate(ind2)
            del ind1.fitness.values, ind2.fitness.values
        
        # Evaluate the individuals with an invalid fitness
        invalid_ind = [ind for ind in offspring if not ind.fitness.valid]
        try:
            # parallel on multiprocesor or clusters using SCOOP
            from scoop import futures
            fitnesses = futures.map(toolbox.evaluate, invalid_ind)
        except ImportError or ImportWarning:
            # serial
            fitnesses = toolbox.map(toolbox.evaluate, invalid_ind)

        # invalid_ind = [ind for ind in offspring if not ind.fitness.valid]
        # fitnesses = toolbox.map(toolbox.evaluate, invalid_ind)
        for ind, fit in zip(invalid_ind, fitnesses):
            ind.fitness.values = fit

        # Select the next generation population
        pop = toolbox.select(pop + offspring, size_Pops)
        record = stats.compile(pop)
        logbook.record(gen=gen, evals=len(invalid_ind), **record)
        # print "\nlogbook.stream: ", logbook.stream

        if gen % 1 == 0:
            # Create plot
            createPlot(pop, model_Workdir, num_Gens, size_Pops, gen)
            # save in file
            outputStr = "### Generation number: %d, Population size: %d ###" % (num_Gens, size_Pops) + LF
            outputStr += "### Generation_%d ###" % gen + LF
            outputStr += "cost\tbenefit\tscenario" + LF
            for indi in pop:
                outputStr += str(indi) + LF
            outfilename = model_Workdir + os.sep + "NSGAII_OUTPUT" + os.sep + "Gen_" \
                           + str(GenerationsNum) + "_Pop_" + str(PopulationSize) + os.sep + "Gen_" \
                           + str(GenerationsNum) + "_Pop_" + str(PopulationSize) + "_resultLog.txt"
            WriteLog(outfilename, outputStr, MODE='append')
    printInfo("Final population hypervolume is %f" % hypervolume(pop, [11.0, 11.0]))
    return pop, logbook
        
if __name__ == "__main__":
    num_Gens = GenerationsNum
    size_Pops = PopulationSize
    cx = CrossoverRate
    if size_Pops % 4 != 0:
        raise ValueError("'size_Pops' must be a multiple of 4.")

    # Create result forld and file
    resultForld = model_Workdir + os.sep + "NSGAII_OUTPUT" + os.sep + "Gen_" + str(GenerationsNum) \
                + "_Pop_" + str(PopulationSize)
    createForld(resultForld)
    logText = resultForld + os.sep + "Gen_" + str(GenerationsNum) + "_Pop_" \
              + str(PopulationSize) + "_resultLog.txt"
    scenarios_info = model_Workdir + os.sep + "NSGAII_OUTPUT" + os.sep + "scenarios_info.txt"
    if os.path.isfile(logText):
        # If exit, then delete it
        os.remove(logText)
    if os.path.isfile(scenarios_info):
        os.remove(scenarios_info)

    # Run NSGA-II
    printInfo("### START TO SCENARIOS OPTIMIZING ###")
    startT = time.time()
    pop, stats = main(num_Gens, size_Pops, cx)
    pop.sort(key=lambda x: x.fitness.values)
    printInfo(stats)
    endT = time.time()

    # Create plot
    createPlot(pop, model_Workdir, num_Gens, size_Pops, num_Gens)

    # Save log
    outputStr = "### The best ###" + LF
    outputStr += "cost\tbenefit\tscenario" + LF
    for indi in pop:
        printInfo(indi)
        outputStr += str(indi.fitness.values[0]) + "\t" + str(indi.fitness.values[1]) + "\t" \
                     + str(indi) + LF
    printInfo("Running time: %.2fs" % (endT - startT))
    outputStr += "Running time: %.2fs" % (endT - startT)
    # save as file
    outfilename = model_Workdir + os.sep + "NSGAII_OUTPUT" + os.sep + "Gen_" \
                  + str(GenerationsNum) + "_Pop_" + str(PopulationSize)+ os.sep + "Gen_" \
                  + str(GenerationsNum) + "_Pop_" + str(PopulationSize) + "_resultLog.txt"
    WriteLog(outfilename, outputStr, MODE = 'append')
    # outfile = file(model_Workdir + os.sep + "NSGAII_OUTPUT" + os.sep + "Gen_" \
    #               + str(GenerationsNum) + "_Pop_" + str(PopulationSize)+ os.sep + "Gen_" \
    #               + str(GenerationsNum) + "_Pop_" + str(PopulationSize) + "resultLog.txt", 'a')
    # outfile.write(outputStr)
    # outfile.close()

    # Clear
    # Delete SEIMS output files
    delModelOutfile(model_Workdir, scenarios_info, delfile=True)


