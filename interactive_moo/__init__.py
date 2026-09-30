"""Domain-independent interactive multi-objective optimization primitives.

``interactive_moo`` deliberately depends only on NumPy and DEAP.  Domain
packages provide feasible population generation and evaluators.
"""

from .aggregation import FUSION_MODES, aggregate_preferences, select_population
from .indicators import DEFAULT_INDICATORS, IndicatorRegistry, PreferenceIndicator
from .model import Evaluation, Preference, Stakeholder
from .preference import PreferenceModel
from .selectors import EliteAggregationSelector, FunctionAggregationSelector, NSGA2Selector
from .workflow import DomainOperations, InteractiveNSGA2

__all__ = [
    "DEFAULT_INDICATORS", "DomainOperations", "EliteAggregationSelector",
    "Evaluation", "FUSION_MODES", "FunctionAggregationSelector", "IndicatorRegistry",
    "InteractiveNSGA2", "NSGA2Selector", "Preference", "PreferenceIndicator",
    "PreferenceModel", "Stakeholder",
    "aggregate_preferences", "select_population",
]
