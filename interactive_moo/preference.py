"""Preference expression and quantification, independent of a domain model."""

from typing import Mapping, Sequence

from .aggregation import _as_preference, satisfaction
from .indicators import DEFAULT_INDICATORS, IndicatorRegistry
from .model import Preference, Stakeholder


class PreferenceModel:
    """Quantifies stakeholder satisfaction from a declared indicator registry."""

    def __init__(self, registry: IndicatorRegistry = DEFAULT_INDICATORS):
        self.registry = registry

    @staticmethod
    def raw_values(individual) -> Mapping[str, float]:
        """Read adapter-published values, falling back to legacy attributes."""
        values = getattr(individual, "indicator_values", None)
        if values is not None:
            return values
        return vars(individual)

    def score(self, individual, preferences: Mapping[str, Preference]) -> float:
        weighted = []
        raw = self.raw_values(individual)
        for name, raw_preference in preferences.items():
            if name not in raw and name not in self.registry._items:
                continue
            preference = _as_preference(raw_preference)
            try:
                value = self.registry.quantify(name, raw)
            except KeyError:
                continue
            weighted.append((satisfaction(value, preference), preference.weight))
        if not weighted:
            return 0.0
        total_weight = sum(weight for _, weight in weighted)
        return sum(score * weight for score, weight in weighted) / total_weight if total_weight else 0.0

    def explain(self, individual, preferences: Mapping[str, Preference]):
        """Return each quantified indicator and satisfaction for UI/reporting."""
        raw = self.raw_values(individual)
        result = {}
        for name, raw_preference in preferences.items():
            preference = _as_preference(raw_preference)
            try:
                value = self.registry.quantify(name, raw)
            except KeyError:
                continue
            result[name] = {"value": value, "satisfaction": satisfaction(value, preference)}
        return result
