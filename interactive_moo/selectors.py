"""Selectable preference-aware NSGA-II strategies; the research core."""

from abc import ABC, abstractmethod
from collections import OrderedDict
from math import ceil
from typing import List, Sequence

from deap import tools

from .aggregation import aggregate_preferences
from .model import Stakeholder
from .preference import PreferenceModel


class PopulationSelector(ABC):
    """Explicit selection strategy contract for the optimization workflow."""

    @abstractmethod
    def select(self, population: Sequence, size: int, stakeholders: Sequence[Stakeholder]) -> List:
        raise NotImplementedError


class NSGA2Selector(PopulationSelector):
    """Baseline non-interactive NSGA-II selection."""

    def select(self, population, size, stakeholders=()):
        return tools.selNSGA2(list(population), size)


class FunctionAggregationSelector(PopulationSelector):
    """Func_agg: fuse preference functions, then select one shared population."""

    def __init__(self, preference_model=None):
        self.preference_model = preference_model or PreferenceModel()

    def select(self, population, size, stakeholders):
        return self._select_with_preferences(population, size, aggregate_preferences(stakeholders))

    def _select_with_preferences(self, population, size, preferences):
        fronts = tools.sortNondominated(list(population), len(population))
        selected = []
        for front_id, front in enumerate(fronts, start=1):
            tools.emo.assignCrowdingDist(front)
            for individual in front:
                individual.fitness.preference_score = self.preference_model.score(individual, preferences)
                individual.fitness.front_id = front_id
            if len(selected) + len(front) <= size:
                selected.extend(front)
                continue
            selected.extend(sorted(
                front,
                key=lambda item: (item.fitness.preference_score, item.fitness.crowding_dist),
                reverse=True,
            )[: size - len(selected)])
            break
        return selected


class EliteAggregationSelector(FunctionAggregationSelector):
    """Elite_agg: select each stakeholder's elite candidates before merging."""

    def select(self, population, size, stakeholders):
        if not stakeholders:
            return NSGA2Selector().select(population, size)
        quota = max(1, ceil(size / len(stakeholders)))
        merged = OrderedDict()
        for stakeholder in stakeholders:
            for individual in self._select_with_preferences(population, quota, stakeholder.preferences):
                merged.setdefault(id(individual), individual)
        selected = list(merged.values())
        for individual in tools.selNSGA2(list(population), len(population)):
            if len(selected) >= size:
                break
            if id(individual) not in merged:
                selected.append(individual)
                merged[id(individual)] = individual
        return selected[:size]


def selector_for(mode: str, preference_model=None) -> PopulationSelector:
    if mode == "Func_agg":
        return FunctionAggregationSelector(preference_model)
    if mode == "Elite_agg":
        return EliteAggregationSelector(preference_model)
    raise ValueError("mode must be 'Func_agg' or 'Elite_agg'")
