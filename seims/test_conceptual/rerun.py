import shutil

from preprocess.db_import_model_parameters import ImportParam2Mongo
from test_conceptual.demo_config import *
from run_seims import MainSEIMS
from postprocess.plot_timeseries import TimeSeriesPlots
from pathlib import Path
from datetime import datetime
import argparse


def rerun(config_files_dir):
    config_files_dir = Path(config_files_dir)
    preprocess_config_file = config_files_dir / 'preprocess.ini'
    run_config_file = config_files_dir / 'runmodel.ini'
    postprocess_config_file = config_files_dir / 'postprocess.ini'

    cf = ConfigParser()
    cf.read(preprocess_config_file)
    preprocess_config = PreprocessConfig(cf)
    ImportParam2Mongo.workflow(preprocess_config)

    cf = ConfigParser()
    cf.read(run_config_file)
    run_config = ParseSEIMSConfig(cf)
    seims_obj = MainSEIMS(args_dict=run_config.ConfigDict)

    time = datetime.now().strftime('%Y.%m.%d-%H.%M.%S')

    seims_obj.run()

    cf = ConfigParser()
    cf.read(postprocess_config_file)
    postprocess_config = PostConfig(cf)

    plt = TimeSeriesPlots(postprocess_config).generate_plots()

    out_dir = Path(seims_obj.output_dir)
    shutil.move(out_dir, out_dir.parent / f'{out_dir.name}_{time}')

    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', type=str, help='Path to the directory containing the config files', )
    args = parser.parse_args()
    rerun(args.d)
