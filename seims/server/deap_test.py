import os
import subprocess
from pathlib import Path

cur_path = Path(os.path.abspath(os.path.dirname(__file__)))
# get calibration path
cali_path = cur_path.parent
process = subprocess.Popen(
    # executable=r"",
    # args=['conda','env','list'],
    # args=['python','-m', r'calibration.main_nsga2', '-ini', r'C:\src\SEIMS\data\youwuzhen\model_configs_conceptual-completed\calibration.ini'],
    args=['python','-m','scoop','-n','4', 'calibration/main_nsga2.py', '-ini', r'C:\src\SEIMS\data\youwuzhen\model_configs_conceptual-completed\calibration.ini'],
    # args=r'conda run -n pyseims python C:/src/auto-context-hydro-model/test/a.py'.split(' '),
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    # shell=True,
    encoding="utf-8",
    universal_newlines=True,
    cwd=r'C:\src\SEIMS\seims'
)
out, err = process.communicate(
    # r'python -m C:\src\SEIMS\seims\calibration\main_nsga2.py -ini C:\src\SEIMS\data\youwuzhen\model_configs_conceptual-completed\calibration.ini'
)
print('output:')
print(out)
print('error:')
print(err)
print('done')
