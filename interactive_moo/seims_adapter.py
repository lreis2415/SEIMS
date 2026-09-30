"""The single optional bridge for SEIMS-BMP and surrogate-SEIMS integration.

This module is intentionally a thin adapter: existing SEIMS initialisation,
simulation, crossover, mutation, and repair functions are passed in by the
host application.  No SEIMS imports are made by the interactive-moo core.
"""

from dataclasses import dataclass
from typing import Callable

from .workflow import DomainOperations


@dataclass
class SEIMSBMPAdapter:
    """Bundle legacy SEIMS callbacks behind one explicit integration object."""

    make_population: Callable
    simulate_seims: Callable
    simulate_surrogate: Callable
    crossover: Callable
    mutation: Callable
    repair_individual: Callable = None
    backend: str = "surrogate"

    def operations(self) -> DomainOperations:
        if self.backend not in ("surrogate", "seims"):
            raise ValueError("backend must be 'surrogate' or 'seims'")
        evaluator = self.simulate_surrogate if self.backend == "surrogate" else self.simulate_seims
        return DomainOperations(
            population=self.make_population,
            evaluate=evaluator,
            mate=self.crossover,
            mutate=self.mutation,
            repair=self.repair_individual,
        )
