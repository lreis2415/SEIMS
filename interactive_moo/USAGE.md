# Usage guide

## Unified configuration

Use this form for a two-stakeholder run with the default surrogate evaluator:

```json
{
  "evaluation": {"backend": "surrogate"},
  "interactive": {
    "enable": true,
    "interval_generations": 30,
    "preference_fusion_strategy": "Elite_agg",
    "users": [
      {"user_id": "planner", "preference_params": {"economy": [150, "less", 50]}},
      {"user_id": "environment", "preference_params": {"environment": [5, "greater", 2]}}
    ]
  }
}
```

Set `evaluation.backend` to `seims` only for the physical-model workflow. The
surrogate path is the default for a guided interactive run but requires a
compatible `surrogate_model_dir` and its encoder/scalers.

## Command-line overrides

```powershell
python seims/scenario_analysis/run_unified_v2.py --config my_run.json `
  --interactive --interactive-interval 20 --fusion-mode Func_agg `
  --evaluation-backend surrogate
```

The command-line flag enables interaction but does not fabricate stakeholder
preference functions. Put user preference parameters in the JSON or create
them with the wizard.

## Guided entry

```powershell
python -m interactive_moo.run_interactive --config my_run.json
python -m interactive_moo.run_interactive --config my_run.json --dry-run
python -m interactive_moo.run_interactive --config my_run.json --run
```

The first command writes a reviewed configuration but does not launch a model.
`--dry-run` forwards the generated configuration to the SEIMS unified entry.

## Actual standalone core run

The following runs a small deterministic DEAP example and writes selected
solutions to `interactive_moo/results/toy_pareto.json`:

```powershell
python -m interactive_moo.examples.toy_interactive_run
```

This validates the core lifecycle without pretending to be a watershed result.
