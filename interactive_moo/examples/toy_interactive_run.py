"""A small actual optimization run with no SEIMS dependency.

It demonstrates the exact core lifecycle: initialise -> simulate -> select ->
crossover/mutate/repair -> simulate -> select. Results are written to the
path given by --output.
"""

import argparse
import json
import random
from pathlib import Path

from deap import base, creator

from interactive_moo import (
    DomainOperations, EliteAggregationSelector, Evaluation, InteractiveNSGA2,
    Preference, Stakeholder,
)


if not hasattr(creator, "ToyInteractiveFitness"):
    creator.create("ToyInteractiveFitness", base.Fitness, weights=(-1.0, 1.0))
    creator.create("ToyInteractiveIndividual", list, fitness=creator.ToyInteractiveFitness)


def make_population(size):
    return [creator.ToyInteractiveIndividual([random.randint(0, 10)]) for _ in range(size)]


def evaluate(individual):
    x = individual[0]
    economy = float(x * 10)
    environment = float(10 - abs(x - 7))
    return Evaluation(
        objectives={"economy": economy, "environment": environment},
        indicators={"economy": economy, "environment": environment},
    )


def mate(left, right):
    left[0], right[0] = right[0], left[0]


def mutate(individual):
    individual[0] += random.choice((-1, 1))


def repair(individual):
    individual[0] = max(0, min(10, individual[0]))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="interactive_moo/results/toy_pareto.json")
    args = parser.parse_args()
    random.seed(7)
    stakeholders = [
        Stakeholder("budget", {"economy": Preference(30, "less", 30)}),
        Stakeholder("environment", {"environment": Preference(8, "greater", 3)}),
    ]
    operations = DomainOperations(make_population, evaluate, mate, mutate, repair)
    optimizer = InteractiveNSGA2(
        operations, EliteAggregationSelector(), stakeholders,
        population_size=12, generations=6, interaction_interval=3,
    )
    population = optimizer.run()
    result = [
        {"gene": individual[0], "objectives": individual.indicator_values,
         "preference_score": getattr(individual.fitness, "preference_score", 0.0)}
        for individual in population
    ]
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(f"Saved {len(result)} selected solutions to {output}")


if __name__ == "__main__":
    main()
