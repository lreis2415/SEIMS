# interactive-moo

`interactive-moo` is a small, domain-independent DEAP extension for
multi-stakeholder interactive multi-objective optimization. It contains no
SEIMS model, MongoDB, GDAL, watershed data, or surrogate artifact.

The repository-local package is intentionally separated from `seims/` so it
can be published as its own repository later without extracting a heavy model
framework.

## Aggregation modes

`Func_agg` is the default. It combines compatible stakeholder preference
functions into one weighted preference function and uses it to rank a shared
non-dominated population.

`Elite_agg` first selects preference-biased elites for every stakeholder,
merges the elite populations with duplicate removal, and fills remaining
places with ordinary NSGA-II. It is useful where representation of every
stakeholder is more important than one consensus score.

## SEIMS integration

SEIMS remains responsible for creating physically meaningful BMP individuals,
performing domain-specific crossover/mutation/repair, and evaluating the
individuals. The unified JSON configuration uses these explicit fields:

```json
{
  "evaluation": {"backend": "surrogate"},
  "interactive": {
    "enable": true,
    "interval_generations": 30,
    "preference_fusion_strategy": "Func_agg",
    "users": [{"user_id": "user1", "preference_params": {}}]
  }
}
```

`evaluation.backend` is `surrogate` or `seims`. A surrogate adapter owns gene
encoding, scaler use, and feature-dimension validation. It must not be
implemented inside the MOO core.

## Entrypoints

Existing SEIMS workflow:

```powershell
python seims/scenario_analysis/run_unified_v2.py --config test_configs/10m_spatio_temporal_interactive_surrogate.json --fusion-mode Elite_agg --evaluation-backend surrogate
```

Guided workflow; it asks for number of stakeholders, interval, evaluator, and
the aggregation mode when more than one person participates:

```powershell
python -m interactive_moo.run_interactive --config test_configs/10m_spatio_temporal_interactive_surrogate.json --dry-run
```

The wizard preserves existing per-user preferences. For newly added users it
creates empty preference placeholders; supply their `preference_params` in the
generated JSON before a real optimization run.

## Publishing boundary

For a standalone release publish this directory as the package root. A future
`seims-bmp-adapter` package should implement `DomainAdapter` from
`interactive_moo.adapters`; it is an optional integration, not a dependency.

See [ARCHITECTURE.md](ARCHITECTURE.md) for the detailed design and
[USAGE.md](USAGE.md) for the configuration reference.
