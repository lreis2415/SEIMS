import logging
import os
import pickle
import argparse
import sys
from configparser import ConfigParser
from pathlib import Path
import pandas as pd
import datetime

from tqdm import trange

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))
from utility.logger import configure_logging
from postprocess.plot_timeseries import TimeSeriesPlots
from postprocess.config import parse_ini_configuration, PostConfig

configure_logging(logging_level=logging.INFO)


def plot_best_simulation_hydrograph(cfg, pickle_path, index=None, time_difference=0):
    """Find the best individual from the pickle file."""
    if not Path(pickle_path).exists():
        raise FileNotFoundError(f"Pickle file not found: {pickle_path}")
    with open(pickle_path, 'rb') as f:
        data = pickle.load(f)
        best_nse = 0
        best_plt = None
        if index is not None:
            # if index is specified, plot the individual
            best_ind = index
            best_plt = TimeSeriesPlots(cfg)
            best_plt.init_plot_vals(data[index])
        else:
            # otherwise, find the best individual
            for i in range(len(data)):
                plt = TimeSeriesPlots(cfg)
                dic = data[i]
                try:
                    plt.init_plot_vals(dic)
                except Exception as e:
                    logging.error(f"Error plotting individual {best_ind}: {e}")
                    continue
                nse = plt.sim_obs_dict['Q']['NSE']
                logging.info(f"Individual {i}: NSE = {nse}")
                if nse > best_nse:
                    best_nse = nse
                    best_ind = i
                    best_plt = plt
    best_plt.generate_plots(image_file_suffix=f'ind_{best_ind}')

    res_list = list()
    # iter datetime in obs, find the corresponding time of pcp and sim, output to csv
    for dt, pcp in best_plt.pcp_date_value:
        t = dt + datetime.timedelta(hours=time_difference)
        obs, sim = [None], [None]
        if t in best_plt.obs_data_dict:
            obs = best_plt.obs_data_dict[t]
        if t in best_plt.sim_data_dict:
            sim = best_plt.sim_data_dict[t]
        res_list.append([dt, pcp] + obs + sim)
    obs_columns = ['obs'+o for o in best_plt.obs_vars]
    sim_columns = ['sim'+o for o in best_plt.obs_vars]
    res_df = pd.DataFrame(res_list, columns=['datetime', 'pcp'] + obs_columns + sim_columns)
    res_df.to_csv(Path(pickle_path).parent / 'best_simulation_hydrograph.csv', index=False)


if __name__ == '__main__':
    """
    args:
        -ini: path of postprocess ini file
        -p: path of pickle file
        -i (optional): index of individual
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('-ini', '--ini', type=str, help='path of postprocess ini file')
    parser.add_argument('-p', '--path', type=str, help='path of pickle file')
    parser.add_argument('-i', '--index', type=int, help='index of individual')
    parser.add_argument('-t', '--time_difference', type=int, default=0, required=False,
                        help='time difference in hour, pcp + time_difference = obs')
    args = parser.parse_args()

    cf = ConfigParser()
    cf.read(args.ini)
    cfg = PostConfig(cf)

    plot_best_simulation_hydrograph(cfg, args.path, index=args.index, time_difference=args.time_difference)
