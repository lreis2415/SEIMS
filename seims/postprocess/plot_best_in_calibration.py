import pickle
import argparse
from configparser import ConfigParser
from pathlib import Path

from tqdm import trange

from postprocess.plot_timeseries import TimeSeriesPlots
from postprocess.config import parse_ini_configuration, PostConfig


def plot_best_simulation_hydrograph(cfg, pickle_path, index=None):
    """Find the best individual from the pickle file."""
    if not Path(pickle_path).exists():
        raise FileNotFoundError(f"Pickle file not found: {pickle_path}")
    with open(pickle_path, 'rb') as f:
        data = pickle.load(f)
    best_nse = 0
    best_plt = None
    if index is not None:
        best_ind = index
        best_plt = TimeSeriesPlots(cfg)
        best_plt.init_plot_vals(data[index])
    else:
        for i in trange(len(data)):
            plt = TimeSeriesPlots(cfg)
            dic = data[i]
            plt.init_plot_vals(dic)
            nse = plt.sim_obs_dict['Q']['NSE']
            if nse > best_nse:
                best_nse = nse
                best_ind = i
                best_plt = plt
    best_plt.generate_plots(image_file_suffix=f'ind_{best_ind}')




if __name__ == '__main__':
    """
    args:
        -ini: path of postprocess ini file
        -p: path of pickle file
        -i: index of individual
    """
    parser = argparse.ArgumentParser()
    parser.add_argument('-ini', '--ini', type=str, help='path of postprocess ini file')
    parser.add_argument('-p', '--path', type=str, help='path of pickle file')
    parser.add_argument('-i', '--index', type=int, help='index of individual')
    args = parser.parse_args()

    cf = ConfigParser()
    cf.read(args.ini)
    cfg = PostConfig(cf)

    plot_best_simulation_hydrograph(cfg, args.path, args.index)
