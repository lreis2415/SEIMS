import logging
import os
import random
import re
import shlex
import subprocess
import sys
from pathlib import Path

from fastapi import FastAPI
from pydantic import BaseModel

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from configparser import ConfigParser
from calibration.config import CaliConfig
from utility.logger import configure_logging

configure_logging()
app = FastAPI()


class CaliParams(BaseModel):
    ini_path: str
    total_cpus: int = 1
    workers: int = 1
    cpus_per_worker: int = 1

@app.get("/health")
def health():
    return 1

@app.post("/seims/calibration")
def calibration(cali_params: CaliParams):
    cur_path = Path(os.path.abspath(os.path.dirname(__file__)))
    cwd = cur_path.parent
    cali_script = 'calibration/main_nsga2.py'
    cmd_str = (f'python {cali_script} -ini {cali_params.ini_path} -p {cali_params.total_cpus} '
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
    with open(logbook.as_posix(), 'r', encoding='utf-8') as f:
        lines = f.readlines()
        last_line = lines[-1]
        max_str = last_line.split('\t')[4]
        max_str = re.sub(r'\[|\]', '', max_str)
        nse = float(max_str.split()[0])
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
