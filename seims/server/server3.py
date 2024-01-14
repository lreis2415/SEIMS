import logging
import os
import random
import re
import shlex
import subprocess
import sys
from pathlib import Path

import uvicorn
from fastapi import FastAPI
from pydantic import BaseModel

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from configparser import ConfigParser
from calibration.config import CaliConfig
from utility.logger import configure_logging
from server import sbatch

configure_logging()
app = FastAPI()


class CaliParams(BaseModel):
    """
    for local (ray), use total_cpus, workers, cpus_per_worker
    """
    ini_path: str
    total_cpus: int = 1
    workers: int = 1
    cpus_per_worker: int = 1


class CaliParamsSlurm(BaseModel):
    ini_path: str
    partition: str = 'work'
    nodes: int
    ntasks: int
    tasks_per_node: int
    job_name: str = 'cali'
    cpus_per_task: int = 1


def get_performance(logbook_path):
    logbook = Path(logbook_path)
    if not logbook.exists():
        return -9999
    with open(logbook.as_posix(), 'r', encoding='utf-8') as f:
        lines = f.readlines()
        last_line = lines[-1]
        max_str = last_line.split('\t')[3]
        max_str = re.sub(r'\[|\]', '', max_str)
        nse = float(max_str.split()[0])
    return nse


@app.get("/health")
def health():
    return 1


@app.post("/seims/calibration/local")
def calibration(cali_params: CaliParams):
    cur_path = Path(os.path.abspath(os.path.dirname(__file__)))
    cwd = cur_path.parent
    cali_script = 'calibration/main_nsga2_ray.py'
    cmd_str = (f'python {cali_script} -ini {cali_params.ini_path} '
               f'-w {cali_params.workers} -ppw {cali_params.cpus_per_worker}')
    logging.info(f'Running: {cmd_str}')
    cmd = shlex.split(cmd_str)
    process = subprocess.Popen(
        args=cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        shell=True,
        encoding="utf-8",
        cwd=cwd.as_posix(),
    )
    out, err = process.communicate()
    logging.info(out)
    logging.error(err)
    parser = ConfigParser()
    parser.read(cali_params.ini_path)
    cfg = CaliConfig(parser, method='nsga2')
    logbook = Path(cfg.opt.out_dir, 'logbook.txt')
    nse = get_performance(logbook)
    data = {
        "code": 200,
        "data": {
            'nse': nse,
            'cali_dir': cfg.opt.out_dir,
        },
    }
    return data


@app.post("/seims/calibration/slurm")
def calibration(cali_params: CaliParamsSlurm):
    cur_path = Path(os.path.abspath(os.path.dirname(__file__)))
    seims_py_root = cur_path.parent
    cali_script = 'calibration/main_nsga2.py'
    exp_name = f'cali'
    logging.info(f'{exp_name}: {CaliParams}')
    sbatch.run(
        partition='work',
        nodes=cali_params.nodes,
        ntasks=cali_params.ntasks,
        tasks_per_node=cali_params.tasks_per_node,
        cali_ini=cali_params.ini_path,
        python_script_path=Path(seims_py_root, cali_script),
        job_name=exp_name,
        cpus_per_task=cali_params.cpus_per_task
    )
    parser = ConfigParser()
    parser.read(cali_params.ini_path)
    cfg = CaliConfig(parser, method='nsga2')
    logbook = Path(cfg.opt.out_dir, 'logbook.txt')
    nse = get_performance(logbook)
    data = {
        "code": 200,
        "data": {
            'nse': nse,
            'cali_dir': cfg.opt.out_dir,
        },
    }
    return data


@app.post("/seims/calibration/test")
def calibration(cali_params: CaliParams):
    cali_dir = Path(r'C:/tmp/tmp')
    cali_dir.mkdir(parents=True, exist_ok=True)

    return {
        "code": 200,
        "data": {
            'nse': random.random(),
            'cali_dir': cali_dir,
        },
    }


if __name__ == '__main__':
    uvicorn.run(app, host="0.0.0.0", port=8415)
