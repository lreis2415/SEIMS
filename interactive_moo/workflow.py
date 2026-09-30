"""A reusable NSGA-II lifecycle that delegates all domain operations."""

from dataclasses import dataclass
from random import random
from typing import Callable, Optional, Sequence

from deap import base, tools

from .model import Evaluation, Stakeholder
from .selectors import NSGA2Selector, PopulationSelector


@dataclass
class DomainOperations:
    """The only integration point required by the MOO core."""

    population: Callable[[int], list]
    evaluate: Callable[[object], Evaluation]
    mate: Callable[[object, object], None]
    mutate: Callable[[object], None]
    repair: Optional[Callable[[object], None]] = None


class InteractiveNSGA2:
    """Selection-centred interactive workflow; domain simulation is injected."""

    def __init__(self, operations: DomainOperations, selector: PopulationSelector,
                 stakeholders: Sequence[Stakeholder], population_size: int,
                 generations: int, interaction_interval: int = 10,
                 crossover_rate: float = 0.8, mutation_rate: float = 0.1,
                 on_interaction: Optional[Callable[[int, list, Sequence[Stakeholder]], Sequence[Stakeholder]]] = None):
        self.operations = operations
        self.selector = selector
        self.stakeholders = list(stakeholders)
        self.population_size = population_size
        self.generations = generations
        self.interaction_interval = interaction_interval
        self.crossover_rate = crossover_rate
        self.mutation_rate = mutation_rate
        self.on_interaction = on_interaction
        self.toolbox = base.Toolbox()
        self.toolbox.register("clone", lambda individual: type(individual)(individual))

    def _evaluate(self, individuals):
        for individual in individuals:
            result = self.operations.evaluate(individual)
            if not isinstance(result, Evaluation):
                raise TypeError("domain evaluate() must return interactive_moo.Evaluation")
            individual.indicator_values = dict(result.indicators)
            individual.indicator_values.update(result.objectives)
            individual.fitness.values = tuple(result.objectives.values())

    def run(self):
        population = self.operations.population(self.population_size)
        self._evaluate(population)
        population = NSGA2Selector().select(population, self.population_size)
        for generation in range(1, self.generations + 1):
            offspring = [self.toolbox.clone(item) for item in population]
            for left, right in zip(offspring[::2], offspring[1::2]):
                if random() < self.crossover_rate:
                    self.operations.mate(left, right)
                for item in (left, right):
                    if random() < self.mutation_rate:
                        self.operations.mutate(item)
                    if self.operations.repair:
                        self.operations.repair(item)
            self._evaluate(offspring)
            pool = population + offspring
            population = self.selector.select(pool, self.population_size, self.stakeholders)
            if self.on_interaction and generation % self.interaction_interval == 0:
                self.stakeholders = list(self.on_interaction(generation, population, self.stakeholders))
        return population
