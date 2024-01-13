import os
import subprocess
from pathlib import Path

from fastapi import FastAPI

app = FastAPI()


@app.get("/calibration")
def calibration(ini_path: str):
    # get current file path
    cur_path = Path(os.path.abspath(os.path.dirname(__file__)))
    # get calibration path
    cali_path = cur_path.parent / 'calibration' / 'main_nsga2.py'
    process = subprocess.Popen(
        # executable=r"",
        args = [],
        # args=['python','-m', cali_path, '-ini', r'C:\src\SEIMS\data\youwuzhen\model_configs_conceptual-completed\calibration.ini'],
        # args=['python','-m','scoop','-n','4', cali_path, '-ini', r'C:\src\SEIMS\data\youwuzhen\model_configs_conceptual-completed\calibration.ini'],
        # args=r'conda run -n pyseims python C:/src/auto-context-hydro-model/test/a.py'.split(' '),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        shell=True,
        encoding="gb2312",
    )
    out,err = process.communicate(['python','-m', cali_path, '-ini', r'C:\src\SEIMS\data\youwuzhen\model_configs_conceptual-completed\calibration.ini'])
    print('output:')
    print(process.stdout)
    print('error:')
    print(process.stderr)
    print('done')
