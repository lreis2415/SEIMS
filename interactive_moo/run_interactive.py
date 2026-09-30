"""Wizard entry point for configuring an interactive SEIMS optimization run."""

import argparse
import json
import subprocess
import sys
from pathlib import Path


def _ask(prompt, default):
    value = input(f"{prompt} [{default}]: ").strip()
    return value or str(default)


def configure(config):
    interactive = config.setdefault("interactive", {})
    existing = interactive.get("users", [])
    people = int(_ask("Number of participating stakeholders", len(existing) or 1))
    interval = int(_ask("Interaction interval (generations)", interactive.get("interval_generations", 10)))
    backend = _ask("Evaluation backend (surrogate/seims)", config.get("evaluation", {}).get("backend", "surrogate"))
    if backend not in ("surrogate", "seims"):
        raise ValueError("Evaluation backend must be 'surrogate' or 'seims'")
    mode = "Func_agg"
    if people > 1:
        mode = _ask("Preference aggregation (Func_agg/Elite_agg)", interactive.get("preference_fusion_strategy", "Func_agg"))
        if mode not in ("Func_agg", "Elite_agg"):
            raise ValueError("Preference aggregation must be 'Func_agg' or 'Elite_agg'")

    users = existing[:people]
    while len(users) < people:
        number = len(users) + 1
        users.append({"user_id": f"user{number}", "preference_params": {}})
    interactive.update({
        "enable": True,
        "interval_generations": interval,
        "preference_fusion_strategy": mode,
        "users": users,
    })
    config["evaluation"] = {**config.get("evaluation", {}), "backend": backend}
    return config


def main():
    parser = argparse.ArgumentParser(description="Configure and launch an interactive MOO SEIMS run.")
    parser.add_argument("--config", required=True, help="base unified JSON configuration")
    parser.add_argument("--output-config", default="interactive_moo/generated_interactive_config.json")
    parser.add_argument("--run", action="store_true", help="launch the SEIMS unified entry after confirmation")
    parser.add_argument("--dry-run", action="store_true", help="validate through the unified entry without optimizing")
    args = parser.parse_args()

    with open(args.config, encoding="utf-8") as stream:
        config = configure(json.load(stream))
    output = Path(args.output_config)
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(config, indent=2, ensure_ascii=False), encoding="utf-8")
    print(f"Interactive configuration written to {output}")
    print(f"  stakeholders: {len(config['interactive']['users'])}")
    print(f"  interval: {config['interactive']['interval_generations']}")
    print(f"  backend: {config['evaluation']['backend']}")
    print(f"  aggregation: {config['interactive']['preference_fusion_strategy']}")

    if args.run or args.dry_run:
        root = Path(__file__).resolve().parents[1]
        command = [sys.executable, str(root / "seims" / "scenario_analysis" / "run_unified_v2.py"), "--config", str(output)]
        if args.dry_run:
            command.append("--dry-run")
        raise SystemExit(subprocess.call(command, cwd=root))


if __name__ == "__main__":
    main()
