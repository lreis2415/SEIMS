# Architecture

## Responsibilities

```text
interactive-moo                     optional integration packages
-------------------------------     --------------------------------------
preference aggregation              seims-bmp-adapter
preference-aware selection          - initialise feasible BMP populations
DEAP-oriented MOO primitives        - domain crossover, mutation and repair
async protocol (future)             - physical constraints and cost logic
                                    surrogate-seims
                                    - encode BMP/time genes
                                    - load model and scalers
                                    - evaluate sediment/water-quality results
```

The core accepts individuals with DEAP fitness and domain indicator attributes
such as `economy`, `environment`, or `cost_variation`. Adapters now publish
them as `individual.indicator_values`; legacy attributes are only a temporary
backward-compatible read path. It does not interpret their gene representation.

## Core files

| File | Responsibility |
| --- | --- |
| `indicators.py` | Explicit indicator definitions and derived-indicator calculations. |
| `preference.py` | Preference satisfaction quantification and explanation. |
| `selectors.py` | `NSGA2Selector`, `FunctionAggregationSelector`, `EliteAggregationSelector`. |
| `workflow.py` | Main lifecycle: initialise → evaluate → select → variation/repair → evaluate → select. |
| `seims_adapter.py` | The single optional integration boundary for SEIMS-BMP and surrogate-SEIMS callbacks. |

The selector classes are the research-focused components. Domain crossover and
mutation deliberately remain in the adapter because only it can preserve BMP
and temporal physical constraints.

## Integration contract

The domain adapter must guarantee that every individual produced by
initialisation, crossover, mutation, and repair is physically meaningful. The
surrogate evaluator must validate its expected feature count before predicting.
For the current 10 m SEIMS spatio-temporal model, raw BMP/time genes are
encoded from 210 values to 630 features before XGBoost evaluation.

## Configuration mapping

`interactive.preference_fusion_strategy` selects `Func_agg` or `Elite_agg`.
Legacy `merge_preferences` and `select_then_merge` remain accepted aliases.
`evaluation.backend` selects `surrogate` or `seims`; the old
`surrogate.use_surrogate` setting remains backward compatible.

## Known limits

The package offers selection, not a complete optimizer lifecycle yet. A
public release should add a versioned checkpoint schema and an integration test
using real pickleable DEAP individuals. Do not pickle arbitrary domain objects
without testing restoration in a fresh process.
