"""Explicit domain-neutral definitions for preference indicators.

An indicator is not stored as algorithm logic on a SEIMS individual.  Instead
an adapter publishes raw values and this registry defines how preference values
are obtained or derived from them.
"""

from dataclasses import dataclass
from typing import Callable, Dict, Mapping, Optional


@dataclass(frozen=True)
class PreferenceIndicator:
    """Definition of one quantity a stakeholder can express a preference for."""

    name: str
    description: str
    unit: str = ""
    calculator: Optional[Callable[[Mapping[str, float]], float]] = None

    def evaluate(self, raw_values: Mapping[str, float]) -> float:
        if self.calculator is not None:
            return float(self.calculator(raw_values))
        return float(raw_values[self.name])


class IndicatorRegistry:
    """Named preference indicators, extensible without changing selectors."""

    def __init__(self, indicators=()):
        self._items: Dict[str, PreferenceIndicator] = {}
        for indicator in indicators:
            self.register(indicator)

    def register(self, indicator: PreferenceIndicator) -> None:
        if indicator.name in self._items:
            raise ValueError(f"indicator already registered: {indicator.name}")
        self._items[indicator.name] = indicator

    def quantify(self, name: str, raw_values: Mapping[str, float]) -> float:
        if name not in self._items:
            # A raw adapter-published indicator is also a valid extension
            # point; no SEIMS class modification is needed.
            return float(raw_values[name])
        return self._items[name].evaluate(raw_values)


DEFAULT_INDICATORS = IndicatorRegistry([
    PreferenceIndicator("economy", "Discounted economic cost", "currency"),
    PreferenceIndicator("environment", "Environmental benefit", "ratio"),
    PreferenceIndicator("env_on_invest", "Environmental benefit per investment", "ratio",
                        lambda values: values["environment"] / values["economy"] if values["economy"] else 0.0),
    PreferenceIndicator("return_on_invest", "Return on investment", "ratio"),
    PreferenceIndicator("abandon_possibility", "Implementation abandonment risk", "ratio"),
    PreferenceIndicator("cost_variation", "Inter-period cost variation", "currency"),
])
