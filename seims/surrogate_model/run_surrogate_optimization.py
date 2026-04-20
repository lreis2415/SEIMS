"""Run surrogate-based optimization - Simplified without scalers."""
import sys
import os
import time
import pickle
import numpy as np
from datetime import datetime
from deap import base, creator, tools
import random

print("=" * 80)
print("Surrogate-based Spatial Optimization: 80 x 300")
print("=" * 80)
print(f"Start time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

# Configuration
model_file = "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_training_slppos_500/models/surrogate_model.pkl"
output_dir = "D:/EGC/SEIMS-dev/data/youwuzhen/surrogate_optimization_test"
os.makedirs(output_dir, exist_ok=True)

n_pop = 80
n_gen = 300
n_genes = 105

print(f"\nConfiguration:")
print(f"  Population: {n_pop}")
print(f"  Generations: {n_gen}")
print(f"  Total evaluations: {n_pop * n_gen}")

# Load model
print(f"\nLoading surrogate model...")
with open(model_file, 'rb') as f:
    model = pickle.load(f)
print("  Model loaded")

# Evaluation function (without scalers)
def evaluate_surrogate(individual):
    X = np.array(individual, dtype=float).reshape(1, -1)
    sed = model.predict(X)[0]
    cost = np.sum(np.array(individual) > 0)
    return sed, cost

# Setup DEAP
if hasattr(creator, "FitnessMin"):
    del creator.FitnessMin
if hasattr(creator, "Individual"):
    del creator.Individual

creator.create("FitnessMin", base.Fitness, weights=(-1.0, -1.0))
creator.create("Individual", list, fitness=creator.FitnessMin)

toolbox = base.Toolbox()
toolbox.register("attr_gene", random.randint, 0, 4)
toolbox.register("individual", tools.initRepeat, creator.Individual, toolbox.attr_gene, n=n_genes)
toolbox.register("population", tools.initRepeat, list, toolbox.individual)
toolbox.register("evaluate", evaluate_surrogate)
toolbox.register("mate", tools.cxTwoPoint)
toolbox.register("mutate", tools.mutUniformInt, low=0, up=4, indpb=0.1)
toolbox.register("select", tools.selNSGA2)

# Run optimization
print(f"\nStarting NSGA-II optimization...")
random.seed(42)
np.random.seed(42)

start_time = time.time()

# Initialize
pop = toolbox.population(n=n_pop)
fitnesses = list(map(toolbox.evaluate, pop))
for ind, fit in zip(pop, fitnesses):
    ind.fitness.values = fit

print(f"  Gen 0/{n_gen} completed")

# Evolution
for gen in range(1, n_gen + 1):
    offspring = toolbox.select(pop, len(pop))
    offspring = list(map(toolbox.clone, offspring))

    for child1, child2 in zip(offspring[::2], offspring[1::2]):
        if random.random() < 0.75:
            toolbox.mate(child1, child2)
            del child1.fitness.values
            del child2.fitness.values

    for mutant in offspring:
        if random.random() < 0.1:
            toolbox.mutate(mutant)
            del mutant.fitness.values

    invalid_ind = [ind for ind in offspring if not ind.fitness.valid]
    fitnesses = map(toolbox.evaluate, invalid_ind)
    for ind, fit in zip(invalid_ind, fitnesses):
        ind.fitness.values = fit

    pop[:] = offspring

    if gen % 50 == 0 or gen == n_gen:
        elapsed = time.time() - start_time
        print(f"  Gen {gen}/{n_gen} - Elapsed: {elapsed:.2f}s")

end_time = time.time()
total_time = end_time - start_time

# Extract Pareto front
pareto_front = tools.sortNondominated(pop, len(pop), first_front_only=True)[0]

print("\n" + "=" * 80)
print("Results")
print("=" * 80)
print(f"Total time: {total_time:.2f} seconds = {total_time/60:.2f} minutes")
print(f"Total evaluations: {n_pop * n_gen}")
print(f"Time per evaluation: {total_time/(n_pop*n_gen)*1000:.3f} ms")
print(f"Pareto front size: {len(pareto_front)}")

# Save Pareto front
results_file = os.path.join(output_dir, 'pareto_front.csv')
with open(results_file, 'w') as f:
    f.write("SED,Cost,Genes\n")
    for ind in pareto_front:
        genes_str = ','.join(map(str, ind))
        f.write(f"{ind.fitness.values[0]},{ind.fitness.values[1]},{genes_str}\n")
print(f"Saved to {results_file}")

# Comparison
seims_time = n_pop * n_gen * 6.4 / 8
print("\n" + "=" * 80)
print("Comparison with SEIMS")
print("=" * 80)
print(f"SEIMS (8 parallel): {seims_time/60:.1f} minutes")
print(f"Surrogate: {total_time:.2f} seconds")
print(f"Speedup: {seims_time/total_time:.0f}x")

print("\n" + "=" * 80)
print(f"End time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
print("=" * 80)
