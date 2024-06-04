import os
import sys
from pathlib import Path

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from calibration.config import CaliConfig, get_optimization_config
from calibration.calibrate import Calibration


def main(
    perf_file_path: Path,
):
    cf, method, processors, workers, processors_per_worker = get_optimization_config()
    cali_cfg = CaliConfig(cf, method=method)
    cali_obj = Calibration(cali_cfg)

    # perf header:
    # generation	calibrationID	Cali-Q-NSE	Cali-Q-RSR	Cali-Q-abs(PBIAS)	Vali-Q-NSE	Vali-Q-RSR	Vali-Q-abs(PBIAS)	gene_values
    # open perf file, starting from line 3, find calibrationID with the best Cali-Q-NSE
    gene = ''
    best_cali_q_nse = -1
    with open(perf_file_path, 'r') as f:
        lines = f.readlines()
        for line in lines[2:]:
            parts = line.strip().split('\t')
            cali_q_nse = float(parts[2])
            if cali_q_nse > best_cali_q_nse:
                best_cali_q_nse = cali_q_nse
                gene = parts[-1]

    # clear the param-1.cali files
    # TODO

    # gene: Individual('d', [-0.21427568069926856, ...])
    # extract gene values as list
    gene_values = gene[17:-2].split(', ')
    for i,subs in enumerate(cali_obj.impact_subbasins):
        for sub in subs:
            param_file_name = f'param-{sub}.cali'
            param_file_path = perf_file_path.parent / param_file_name
            if param_file_path.exists():
                with open(param_file_path, 'w') as f:
                    f.write(f'{sub}\n')
            with open(param_file_name, 'a') as f:
                f.write(f'{cali_obj.param_names[i]},{gene_values[i]}\n')



if __name__ == '__main__':
    perf_file_path = Path(
        r'C:\data\SEIMS\NorthAmerica\SEIMS\camels_11383500\models\auto\camels_11383500_modelauto_svo_test\gen50_perf_from_qm.txt'
    )
    main(perf_file_path)
