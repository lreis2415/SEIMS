#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Run Andrews Forest storm model with clean outputs and a comparison plot."""
from __future__ import absolute_import, unicode_literals

import argparse
import os
import subprocess
import sys


REPO_ROOT = os.path.abspath(os.path.dirname(__file__))
DEFAULT_MODEL_DIR = os.path.join(
    REPO_ROOT, "data", "AndrewsForest", "andrews_forest_model")
DEFAULT_BIN_DIR = os.path.join(REPO_ROOT, "build", "bin")
DEFAULT_START = "2015-02-05 07:00:00"
DEFAULT_END = "2015-02-08 20:00:00"
DEFAULT_DYLD_LIBRARY_PATH = ":".join([
    "/Users/flora/miniconda3/envs/pyseims/lib",
    "/opt/homebrew/opt/llvm/lib",
    "/opt/homebrew/opt/mongo-c-driver@1/lib",
])


def parse_args():
    """Parse command line arguments."""
    parser = argparse.ArgumentParser(
        description="Import, clean, run, and plot AndrewsForest storm simulation.")
    parser.add_argument("--host", default="127.0.0.1", help="MongoDB host.")
    parser.add_argument("--port", default=27017, type=int, help="MongoDB port.")
    parser.add_argument("--bin-dir", default=DEFAULT_BIN_DIR, help="SEIMS binary directory.")
    parser.add_argument("--model-dir", default=DEFAULT_MODEL_DIR, help="SEIMS model directory.")
    parser.add_argument("--db-name", default="andrews_forest_model", help="MongoDB database name.")
    parser.add_argument("--cfg-name", default="storm", help="Model config/sub-model name.")
    parser.add_argument("--task-name", default="SingleRun", help="Model task name.")
    parser.add_argument("--threads", default=4, type=int, help="OpenMP thread count.")
    parser.add_argument("--start", default=DEFAULT_START, help="Simulation start datetime.")
    parser.add_argument("--end", default=DEFAULT_END, help="Simulation end datetime.")
    parser.add_argument("--no-plot", action="store_true", help="Do not create comparison plot.")
    parser.add_argument("--figure-name", default="q_pcp_comparison.png",
                        help="Figure filename under the model output directory.")
    parser.add_argument("--dyld-library-path", default=DEFAULT_DYLD_LIBRARY_PATH,
                        help="DYLD_LIBRARY_PATH used by the SEIMS C++ executable.")
    return parser.parse_args()


def build_model(args):
    """Create a MainSEIMS object for AndrewsForest storm runs."""
    seims_pkg = os.path.join(REPO_ROOT, "seims")
    if seims_pkg not in sys.path:
        sys.path.insert(0, seims_pkg)

    from run_seims import MainSEIMS

    return MainSEIMS(args_dict={
        "host": args.host,
        "port": args.port,
        "bin_dir": args.bin_dir,
        "model_dir": args.model_dir,
        "db_name": args.db_name,
        "cfg_name": args.cfg_name,
        "task_name": args.task_name,
        "simu_mode": 1,
        "timestep": 300,
        "version": "OMP",
        "nthread": args.threads,
        "fdirmtd": 0,
        "lyrmtd": 1,
        "scenario_id": -1,
        "calibration_id": -1,
        "subbasin_id": 0,
        "filein_mongo": 1,
        "simu_stime": args.start,
        "simu_etime": args.end,
        "out_stime": args.start,
        "out_etime": args.end,
    })


def ensure_optional_parameters(model):
    """Ensure new optional scalar parameters exist in the current MongoDB."""
    from preprocess.text import DBTableNames, ModelParamFields

    optional_rows = [
        ("WaterBalance", "INFIL_FACTOR",
         "Infiltration reduction factor for diagnostic runoff experiments",
         "none", "SUR_SGA", 1.0, 1.0, "RC", 1.0, 0.0, "FLT"),
        ("WaterBalance", "ACTIVE_DEPTH_MAX",
         "Maximum active wetting front depth for storm infiltration",
         "mm", "SUR_SGA", 150.0, 1.0, "RC", 1000.0, 10.0, "FLT"),
        ("WaterBalance", "MOIST_IN_REF",
         "Reference for MOIST_IN initialization (0 field capacity; 1 porosity)",
         "none", "SUR_SGA", 0.0, 1.0, "VC", 1.0, 0.0, "FLT"),
        ("WaterBalance", "GA_ACC_RECOVERY_RATE",
         "Dry-period recovery rate of Green-Ampt cumulative infiltration",
         "mm/h", "SUR_SGA", 0.0, 1.0, "RC", 50.0, 0.0, "FLT"),
        ("WaterBalance", "GA_ACC_RECOVERY_DELAY",
         "Continuous dry duration before Green-Ampt cumulative infiltration recovery starts",
         "hr", "SUR_SGA", 0.0, 1.0, "RC", 72.0, 0.0, "FLT"),
        ("WaterBalance", "GA_STATE_RECOVERY_FACTOR",
         "Fraction of Green-Ampt cumulative infiltration memory recovered from current soil water state",
         "none", "SUR_SGA", 0.0, 1.0, "RC", 1.0, 0.0, "FLT"),
        ("WaterBalance", "FC_ADJUST",
         "Field capacity adjustment factor for the percolation threshold",
         "none", "PERCO_DARCY", 1.0, 1.0, "VC", 1.5, 0.5, "FLT"),
        ("Discharge", "FAST_RATIO",
         "Upper bypass fraction for event-mobile lateral interflow",
         "none", "IKW_IF", 0.0, 1.0, "VC", 1.0, 0.0, "FLT"),
        ("Discharge", "ANISOTROPY",
         "Horizontal to vertical saturated conductivity ratio for lateral interflow",
         "none", "IKW_IF", 1.0, 1.0, "VC", 100.0, 1.0, "FLT"),
        ("Discharge", "MACROPORE_FACTOR",
         "Macropore lateral conductivity multiplier for event-mobile interflow",
         "none", "IKW_IF", 1.0, 1.0, "VC", 50.0, 1.0, "FLT"),
        ("Discharge", "IF_SUBSTEPS",
         "Numerical substeps for lateral interflow routing within one hillslope timestep",
         "none", "IKW_IF", 1.0, 1.0, "VC", 20.0, 1.0, "FLT"),
    ]

    model.SetMongoClient()
    try:
        coll = model.mongoclient[model.db_name][DBTableNames.main_parameter]
        inserted = 0
        for row in optional_rows:
            type_, name, desc, unit, module, value, impact, change, max_, min_, dtype = row
            doc = {
                ModelParamFields.name: name,
                ModelParamFields.value: value,
                ModelParamFields.impact: impact,
                ModelParamFields.change: change,
            }
            metadata_doc = {
                ModelParamFields.type: type_,
                ModelParamFields.desc: desc,
                ModelParamFields.unit: unit,
                ModelParamFields.module: module,
                ModelParamFields.max: max_,
                ModelParamFields.min: min_,
                ModelParamFields.dtype: dtype,
            }
            result = coll.update_one({ModelParamFields.name: name},
                                     {"$set": metadata_doc,
                                      "$setOnInsert": doc}, upsert=True)
            if result.upserted_id is not None:
                inserted += 1
        if inserted:
            print("Inserted %d optional parameter defaults into MongoDB." % inserted)
    finally:
        model.UnsetMongoClient()


def run_plot(model, args):
    """Generate the standard Q/precipitation comparison plot."""
    fig_path = os.path.join(model.output_dir, args.figure_name)
    cmd = [
        sys.executable,
        os.path.join(REPO_ROOT, "plot_storm_q_pcp.py"),
        "--sim-q", os.path.join(model.output_dir, "Q.txt"),
        "--output", fig_path,
        "--start", args.start,
        "--end", args.end,
    ]
    subprocess.check_call(cmd)


def main():
    """Run the clean reproducible storm workflow."""
    args = parse_args()
    model = build_model(args)

    print("Importing file.in, file.out, and param.cali...")
    ensure_optional_parameters(model)
    model.ImportModelIOConfiguration()
    model.ImportCalibratedParameters()
    model.ResetSimulationPeriod()
    model.ResetOutputsPeriod(["QSUBBASIN", "SBQG", "SBGS", "SOLST"],
                             model.simu_stime, model.simu_etime)

    print("Cleaning output directory: %s" % model.output_dir)
    model.clean()

    if args.dyld_library_path:
        os.environ["DYLD_LIBRARY_PATH"] = args.dyld_library_path
    print("Running SEIMS: %s" % model.CommandString)
    model.run()
    if not os.path.isfile(os.path.join(model.output_dir, "Q.txt")):
        raise RuntimeError("SEIMS finished but Q.txt was not generated in %s" %
                           model.output_dir)

    if not args.no_plot:
        run_plot(model, args)


if __name__ == "__main__":
    main()
