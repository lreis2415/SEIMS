#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Plot simulated discharge, observed discharge, and observed precipitation."""
from __future__ import absolute_import, unicode_literals

import argparse
import os

import matplotlib

matplotlib.use("Agg")
import matplotlib.dates as mdates
import matplotlib.pyplot as plt
import pandas as pd


REPO_ROOT = os.path.abspath(os.path.dirname(__file__))
DEFAULT_MODEL_DIR = os.path.join(
    REPO_ROOT, "data", "AndrewsForest", "andrews_forest_model")
DEFAULT_OUTPUT_DIR = os.path.join(
    DEFAULT_MODEL_DIR, "storm", "OUTPUT_D8_DOWNUP--")
DEFAULT_SIM_Q = os.path.join(DEFAULT_OUTPUT_DIR, "Q.txt")
DEFAULT_OBS_Q = os.path.join(
    REPO_ROOT, "data", "AndrewsForest", "data_prepare", "observed",
    "observed_Q_storm_outlet.csv")
DEFAULT_PCP = os.path.join(
    REPO_ROOT, "data", "AndrewsForest", "data_prepare", "climate",
    "pcp_Intensity_5min.csv")
DEFAULT_FIG = os.path.join(DEFAULT_OUTPUT_DIR, "q_pcp_comparison.png")


def parse_args():
    parser = argparse.ArgumentParser(
        description="Plot precipitation bars above simulated/observed discharge lines.")
    parser.add_argument("--sim-q", default=DEFAULT_SIM_Q,
                        help="Path to simulated discharge Q.txt.")
    parser.add_argument("--obs-q", default=DEFAULT_OBS_Q,
                        help="Path to observed outlet discharge CSV.")
    parser.add_argument("--pcp", default=DEFAULT_PCP,
                        help="Path to precipitation CSV.")
    parser.add_argument("--output", default=DEFAULT_FIG,
                        help="Output figure path, e.g. q_pcp_comparison.png.")
    parser.add_argument("--pcp-station", default="mean",
                        help="Precipitation station column, or 'mean' for all stations.")
    parser.add_argument("--start", default=None,
                        help="Optional start datetime, e.g. '2015-02-05 07:00:00'.")
    parser.add_argument("--end", default=None,
                        help="Optional end datetime, e.g. '2015-02-08 13:55:00'.")
    parser.add_argument("--dpi", default=300, type=int, help="Figure DPI.")
    parser.add_argument("--allow-duplicate-times", action="store_true",
                        help="Keep the last value when Q.txt contains duplicate datetimes.")
    return parser.parse_args()


def parse_datetime(value):
    if value is None:
        return None
    return pd.to_datetime(value)


def read_simulated_q(path, allow_duplicate_times=False):
    rows = []
    with open(path, "r") as fobj:
        for line in fobj:
            line = line.strip()
            if not line or line.startswith("#") or line.startswith("Subbasin"):
                continue
            parts = line.split()
            if len(parts) < 3:
                continue
            rows.append((pd.to_datetime("%s %s" % (parts[0], parts[1])), float(parts[2])))
    if not rows:
        raise RuntimeError("No simulated discharge data found in %s" % path)
    df = pd.DataFrame(rows, columns=["DATETIME", "Q_SIM"])
    duplicate_count = int(df.duplicated(subset=["DATETIME"]).sum())
    if duplicate_count > 0:
        if not allow_duplicate_times:
            raise RuntimeError(
                "Found %d duplicate simulated Q timestamps in %s. "
                "Clean the output directory before rerunning, or pass "
                "--allow-duplicate-times to keep the last value." %
                (duplicate_count, path))
        df = df.drop_duplicates(subset=["DATETIME"], keep="last")
    return df.set_index("DATETIME")


def read_observed_q(path):
    df = pd.read_csv(path, comment="#")
    df = df[df["Type"].str.upper() == "Q"].copy()
    df["DATETIME"] = pd.to_datetime(df["DATETIME"])
    df["VALUE"] = pd.to_numeric(df["VALUE"], errors="coerce")
    df = df.dropna(subset=["VALUE"])
    if df.empty:
        raise RuntimeError("No observed discharge data found in %s" % path)
    return df.set_index("DATETIME")[["VALUE"]].rename(columns={"VALUE": "Q_OBS"})


def read_precipitation(path, station):
    df = pd.read_csv(path, comment="#")
    if "DATETIME" not in df.columns:
        raise RuntimeError("Precipitation file must contain DATETIME column: %s" % path)
    df["DATETIME"] = pd.to_datetime(df["DATETIME"])
    value_cols = [col for col in df.columns if col != "DATETIME"]
    if not value_cols:
        raise RuntimeError("No precipitation station columns found in %s" % path)
    for col in value_cols:
        df[col] = pd.to_numeric(df[col], errors="coerce")
    if station.lower() == "mean":
        df["PCP"] = df[value_cols].mean(axis=1)
    else:
        if station not in value_cols:
            raise RuntimeError(
                "Precipitation station %s not found. Available: %s" %
                (station, ", ".join(value_cols)))
        df["PCP"] = df[station]
    return df.set_index("DATETIME")[["PCP"]].dropna()


def subset_time(df, start, end):
    if start is not None:
        df = df[df.index >= start]
    if end is not None:
        df = df[df.index <= end]
    return df


def infer_bar_width_days(pcp):
    if len(pcp.index) < 2:
        return 5.0 / (24.0 * 60.0)
    seconds = pcp.index.to_series().diff().dropna().dt.total_seconds()
    seconds = seconds[seconds > 0]
    if seconds.empty:
        return 5.0 / (24.0 * 60.0)
    return float(seconds.median()) / 86400.0 * 0.9


def calc_stats(sim_q, obs_q):
    joined = sim_q.join(obs_q, how="inner").dropna()
    if joined.empty:
        raise RuntimeError("No overlapping simulated and observed discharge data.")
    sim = joined["Q_SIM"]
    obs = joined["Q_OBS"]
    denominator = ((obs - obs.mean()) ** 2).sum()
    nse = float("nan") if denominator == 0 else 1.0 - float(((sim - obs) ** 2).sum()) / denominator
    pbias = float("nan") if obs.sum() == 0 else 100.0 * float((sim - obs).sum()) / float(obs.sum())
    sim_peak = sim.max()
    obs_peak = obs.max()
    peak_error = float("nan") if obs_peak == 0 else 100.0 * float(sim_peak - obs_peak) / float(obs_peak)
    sim_peak_time = sim.idxmax()
    obs_peak_time = obs.idxmax()
    ttp_error_h = (sim_peak_time - obs_peak_time).total_seconds() / 3600.0
    return {
        "nse": nse,
        "pbias": pbias,
        "peak_error": peak_error,
        "ttp_error_h": ttp_error_h,
        "sim_peak": float(sim_peak),
        "obs_peak": float(obs_peak),
        "sim_peak_time": sim_peak_time,
        "obs_peak_time": obs_peak_time,
        "n_pairs": len(joined),
    }


def plot(sim_q, obs_q, pcp, output, dpi):
    fig, (ax_pcp, ax_q) = plt.subplots(
        2, 1, figsize=(12, 6), sharex=True,
        gridspec_kw={"height_ratios": [1, 2.6], "hspace": 0.05})

    bar_width = infer_bar_width_days(pcp)
    ax_pcp.bar(pcp.index, pcp["PCP"], width=bar_width, color="#4a90c2",
               edgecolor="#4a90c2", linewidth=0.0, align="center")
    ax_pcp.set_ylabel("Precip.\n(mm/step)")
    ax_pcp.invert_yaxis()
    ax_pcp.grid(True, axis="y", linestyle="--", linewidth=0.5, alpha=0.35)
    ax_pcp.tick_params(labelbottom=False)

    ax_q.plot(obs_q.index, obs_q["Q_OBS"], color="#222222", linewidth=1.8,
              label="Observed Q")
    ax_q.plot(sim_q.index, sim_q["Q_SIM"], color="#c73e3a", linewidth=1.6,
              label="Simulated Q")
    ax_q.set_ylabel("Discharge (m3/s)")
    ax_q.set_xlabel("Datetime")
    ax_q.grid(True, linestyle="--", linewidth=0.5, alpha=0.35)
    ax_q.legend(loc="upper right", frameon=False)
    stats = calc_stats(sim_q, obs_q)
    stats_text = (
        "NSE = %.3f\nPBIAS = %.2f%%\nPeak err. = %.2f%%\nTTP err. = %.2f h\nn = %d" %
        (stats["nse"], stats["pbias"], stats["peak_error"],
         stats["ttp_error_h"], stats["n_pairs"]))
    ax_q.text(
        0.02, 0.96, stats_text, transform=ax_q.transAxes,
        ha="left", va="top", fontsize=10,
        bbox={"boxstyle": "round,pad=0.3", "facecolor": "white",
              "edgecolor": "#bbbbbb", "alpha": 0.85})

    ax_q.xaxis.set_major_locator(mdates.AutoDateLocator(minticks=5, maxticks=9))
    ax_q.xaxis.set_major_formatter(mdates.DateFormatter("%Y-%m-%d\n%H:%M"))
    fig.autofmt_xdate(rotation=0, ha="center")

    os.makedirs(os.path.dirname(os.path.abspath(output)), exist_ok=True)
    fig.savefig(output, dpi=dpi, bbox_inches="tight")
    plt.close(fig)


def main():
    args = parse_args()
    sim_q = read_simulated_q(args.sim_q, args.allow_duplicate_times)
    start = parse_datetime(args.start) or sim_q.index.min()
    end = parse_datetime(args.end) or sim_q.index.max()
    sim_q = subset_time(sim_q, start, end)
    obs_q = subset_time(read_observed_q(args.obs_q), start, end)
    pcp = subset_time(read_precipitation(args.pcp, args.pcp_station), start, end)
    if sim_q.empty or obs_q.empty or pcp.empty:
        raise RuntimeError("One or more datasets are empty after time filtering.")
    plot(sim_q, obs_q, pcp, args.output, args.dpi)
    stats = calc_stats(sim_q, obs_q)
    print("NSE: %.4f" % stats["nse"])
    print("PBIAS: %.2f%%" % stats["pbias"])
    print("PeakError: %.2f%% (sim %.3f, obs %.3f)" %
          (stats["peak_error"], stats["sim_peak"], stats["obs_peak"]))
    print("TimeToPeakError: %.2f h (sim %s, obs %s)" %
          (stats["ttp_error_h"], stats["sim_peak_time"], stats["obs_peak_time"]))
    print("Saved figure: %s" % args.output)


if __name__ == "__main__":
    main()
