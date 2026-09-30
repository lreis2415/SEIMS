"""Public, domain-neutral data contracts for interactive MOO."""

from dataclasses import dataclass, field
from typing import Dict, Literal


Direction = Literal["less", "greater"]


@dataclass(frozen=True)
class Preference:
    """A satisfactory target and tolerance for one named indicator."""

    target: float
    direction: Direction
    tolerance: float
    weight: float = 1.0


@dataclass
class Stakeholder:
    """One decision maker and their current preferences."""

    identifier: str
    preferences: Dict[str, Preference] = field(default_factory=dict)


@dataclass
class Evaluation:
    """Returned by a domain evaluator before values are assigned to DEAP."""

    objectives: Dict[str, float]
    indicators: Dict[str, float] = field(default_factory=dict)
