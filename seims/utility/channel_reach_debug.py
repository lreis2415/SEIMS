#!/usr/bin/env python3
"""Build reach cell list and plot channel debug traces.

Usage examples:
  python channel_reach_debug.py --reach-id 6 --base-dir "data/youwuzhen/demo_youwuzhen30m_model/workspace/layering_info"
  python channel_reach_debug.py --reach-id 6 --trace-log "OUTPUT_D8_DOWNUP-0-/trace.log"
"""
from __future__ import annotations

import argparse
from pathlib import Path
from typing import Dict, List, Tuple


def parse_reach_cells(layer_file: Path) -> List[int]:
    if not layer_file.exists():
        raise FileNotFoundError(f"Layer file not found: {layer_file}")

    cells: List[int] = []
    seen = set()
    with layer_file.open("r", encoding="utf-8") as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("LayerID"):
                continue
            # Some files start with a single number (count)
            if line.isdigit():
                continue
            parts = line.split()
            if len(parts) < 3:
                continue
            cell_part = "".join(parts[2:])
            for token in cell_part.split(","):
                token = token.strip()
                if not token:
                    continue
                cid = int(token)
                if cid not in seen:
                    seen.add(cid)
                    cells.append(cid)
    return cells


def load_trace(trace_path: Path, reach_id: int) -> Dict[int, List[Tuple[int, Dict[str, float]]]]:
    data: Dict[int, List[Tuple[int, Dict[str, float]]]] = {}
    with trace_path.open("r", encoding="utf-8", errors="ignore") as f:
        for raw in f:
            if "[TRACE_CSV]" not in raw:
                continue
            line = raw.strip()
            if not line:
                continue
            tokens = line.split(",")
            if len(tokens) < 6:
                continue
            if "Module" not in tokens:
                continue

            record: Dict[str, str] = {}
            i = 1  # skip [TRACE_CSV]
            while i + 1 < len(tokens):
                key = tokens[i].strip()
                val = tokens[i + 1].strip()
                record[key] = val
                i += 2

            if record.get("Module") != "ChannelFlow":
                continue
            if int(record.get("Reach", "-1")) != reach_id:
                continue

            try:
                step = int(record.get("Step", "0"))
                cell = int(record.get("Cell", "-1"))
            except ValueError:
                continue

            values: Dict[str, float] = {}
            for key in ("Q_Up", "Q_Lat_Total", "Q_Lat_Rain", "Q_Lat_QS", "Q_Lat_QI", "Q_Lat_QG", "Q_Out", "H_Ch"):
                if key in record:
                    try:
                        values[key] = float(record[key])
                    except ValueError:
                        values[key] = 0.0

            data.setdefault(cell, []).append((step, values))
    return data


def plot_per_cell(data: Dict[int, List[Tuple[int, Dict[str, float]]]], out_dir: Path) -> None:
    try:
        import matplotlib.pyplot as plt
    except ImportError as exc:
        raise RuntimeError("matplotlib is required for plotting") from exc

    out_dir.mkdir(parents=True, exist_ok=True)

    for cell, series in data.items():
        series.sort(key=lambda x: x[0])
        steps = [s for s, _ in series]
        q_up = [v.get("Q_Up", 0.0) for _, v in series]
        q_lat = [v.get("Q_Lat_Total", 0.0) for _, v in series]
        q_out = [v.get("Q_Out", 0.0) for _, v in series]

        plt.figure(figsize=(9, 4))
        plt.plot(steps, q_up, label="Q_Up")
        plt.plot(steps, q_lat, label="Q_Lat_Total")
        plt.plot(steps, q_out, label="Q_Out")
        plt.xlabel("Step")
        plt.ylabel("Flow (m3/s)")
        plt.title(f"Channel Flow - Cell {cell}")
        plt.legend()
        plt.tight_layout()
        plt.savefig(out_dir / f"channel_cell_{cell}.png", dpi=150)
        plt.close()


def main() -> int:
    parser = argparse.ArgumentParser(description="Prepare reach cell list and plot channel debug traces")
    parser.add_argument("--reach-id", type=int, default=6, help="reach/subbasin id")
    parser.add_argument(
        "--base-dir",
        type=Path,
        default=Path("data/youwuzhen/demo_youwuzhen30m_model/workspace/layering_info"),
        help="layering_info directory",
    )
    parser.add_argument("--flowdir", type=str, default="D8", help="flow direction method (D8/DINF/MFDMD)")
    parser.add_argument("--layering", type=str, default="DOWNUP", help="layering method (DOWNUP/UPDOWN/EVEN)")
    parser.add_argument("--trace-log", type=Path, default=None, help="trace log file that includes ChannelFlow TRACE_CSV")
    parser.add_argument("--out-dir", type=Path, default=Path("channel_debug_plots"), help="output plots directory")
    args = parser.parse_args()

    layer_file = args.base_dir / f"{args.reach_id}_ROUTING_LAYERS_{args.layering}_CHANNEL_{args.flowdir}.txt"
    cells = parse_reach_cells(layer_file)

    out_cells = args.base_dir / f"debug_reach_{args.reach_id}_cells.txt"
    out_cells.write_text("\n".join(str(c) for c in cells) + "\n", encoding="utf-8")

    print(f"Reach {args.reach_id} cells: {len(cells)}")
    print(f"Wrote: {out_cells}")
    print("Set env var before running SEIMS:")
    print(f"  SEIMS_DEBUG_CELLS_FILE={out_cells}")

    if args.trace_log:
        trace_data = load_trace(args.trace_log, args.reach_id)
        if not trace_data:
            print("No ChannelFlow traces found. Check your log file.")
            return 0
        plot_per_cell(trace_data, args.out_dir)
        print(f"Plots saved to: {args.out_dir}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
