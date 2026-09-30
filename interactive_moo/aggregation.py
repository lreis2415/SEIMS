"""Preference-function and elite-population aggregation for DEAP NSGA-II."""

from collections import OrderedDict
from math import ceil
from typing import Dict, Iterable, List, Mapping, Sequence

from deap import tools

from .model import Preference, Stakeholder

FUSION_MODES = ("Func_agg", "Elite_agg")


def _as_preference(value) -> Preference:
    if isinstance(value, Preference):
        return value
    if isinstance(value, (list, tuple)) and len(value) >= 3:
        return Preference(float(value[0]), str(value[1]), float(value[2]))
    if isinstance(value, Mapping):
        return Preference(
            float(value["target"]), str(value["direction"]),
            float(value["tolerance"]), float(value.get("weight", 1.0)),
        )
    raise ValueError("preference must be Preference, [target, direction, tolerance], or a mapping")


def aggregate_preferences(stakeholders: Sequence[Stakeholder]) -> Dict[str, Preference]:
    """Fuse compatible stakeholder preference functions by weighted mean.

    Conflicting directions are intentionally rejected rather than silently
    averaged, because an application must decide how to negotiate a conflict.
    """
    grouped: Dict[str, List[Preference]] = {}
    for stakeholder in stakeholders:
        for name, pref in stakeholder.preferences.items():
            grouped.setdefault(name, []).append(_as_preference(pref))

    result = {}
    for name, preferences in grouped.items():
        directions = {preference.direction for preference in preferences}
        if len(directions) != 1:
            raise ValueError(f"conflicting preference directions for '{name}': {sorted(directions)}")
        total = sum(max(preference.weight, 0.0) for preference in preferences)
        if total == 0:
            total = float(len(preferences))
            weights = [1.0] * len(preferences)
        else:
            weights = [max(preference.weight, 0.0) for preference in preferences]
        result[name] = Preference(
            target=sum(pref.target * weight for pref, weight in zip(preferences, weights)) / total,
            direction=preferences[0].direction,
            tolerance=sum(pref.tolerance * weight for pref, weight in zip(preferences, weights)) / total,
            weight=total,
        )
    return result


def satisfaction(value: float, preference: Preference) -> float:
    """Return a bounded linear satisfaction score in [0, 1]."""
    tolerance = max(preference.tolerance, 1e-12)
    if preference.direction == "less":
        return 1.0 if value <= preference.target else max(0.0, 1.0 - (value - preference.target) / tolerance)
    if preference.direction == "greater":
        return 1.0 if value >= preference.target else max(0.0, 1.0 - (preference.target - value) / tolerance)
    raise ValueError(f"unsupported preference direction: {preference.direction}")


def preference_score(individual, preferences: Mapping[str, Preference]) -> float:
    """Score an individual using domain indicators stored as attributes."""
    if not preferences:
        return 0.0
    weighted_scores = []
    for name, raw_preference in preferences.items():
        if not hasattr(individual, name):
            continue
        preference = _as_preference(raw_preference)
        weighted_scores.append((satisfaction(float(getattr(individual, name)), preference), preference.weight))
    if not weighted_scores:
        return 0.0
    denominator = sum(weight for _, weight in weighted_scores)
    return sum(score * weight for score, weight in weighted_scores) / denominator if denominator else 0.0


def _preference_select(population: Sequence, k: int, preferences: Mapping[str, Preference]) -> List:
    fronts = tools.sortNondominated(population, len(population))
    selected: List = []
    for front_id, front in enumerate(fronts, start=1):
        tools.emo.assignCrowdingDist(front)
        for individual in front:
            individual.fitness.preference_score = preference_score(individual, preferences)
            individual.fitness.front_id = front_id
        if len(selected) + len(front) <= k:
            selected.extend(front)
            continue
        selected.extend(sorted(
            front,
            key=lambda individual: (
                individual.fitness.preference_score,
                individual.fitness.crowding_dist,
            ),
            reverse=True,
        )[: k - len(selected)])
        break
    return selected


def _elite_aggregate(population: Sequence, k: int, stakeholders: Sequence[Stakeholder]) -> List:
    """Select stakeholder elites first, then merge their populations fairly."""
    per_user = max(1, ceil(k / len(stakeholders)))
    merged = OrderedDict()
    for stakeholder in stakeholders:
        for individual in _preference_select(population, per_user, stakeholder.preferences):
            merged.setdefault(id(individual), individual)
    selected = list(merged.values())
    if len(selected) < k:
        for individual in tools.selNSGA2(list(population), len(population)):
            if id(individual) not in merged:
                selected.append(individual)
                merged[id(individual)] = individual
            if len(selected) == k:
                break
    return selected[:k]


def select_population(population: Sequence, k: int, stakeholders: Sequence[Stakeholder], mode: str = "Func_agg") -> List:
    """Apply one of the two explicit multi-stakeholder aggregation modes.

    ``Func_agg`` aggregates preference functions before one shared selection.
    ``Elite_agg`` selects stakeholder-specific elites then merges them.
    """
    if mode not in FUSION_MODES:
        raise ValueError(f"mode must be one of {FUSION_MODES}, got {mode!r}")
    # Kept as a compact backwards-compatible function API.  The actual
    # research selection implementations are explicit classes in selectors.py.
    from .selectors import NSGA2Selector, selector_for
    return (selector_for(mode) if stakeholders else NSGA2Selector()).select(
        population, k, stakeholders
    )
