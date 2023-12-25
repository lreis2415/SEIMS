import logging
import shutil

import os,sys
if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from preprocess.db_import_model_parameters import ImportParam2Mongo
from test_conceptual.demo_config import *
from run_seims import MainSEIMS
from postprocess.plot_timeseries import TimeSeriesPlots
from pathlib import Path
from datetime import datetime
import argparse

from utility import logger


def rerun(config_files_dir):
    config_files_dir = Path(config_files_dir)
    preprocess_config_file = config_files_dir / 'preprocess.ini'
    run_config_file = config_files_dir / 'runmodel.ini'
    postprocess_config_file = config_files_dir / 'postprocess.ini'

    logging.info('Using %s' % preprocess_config_file)
    parser1 = ConfigParser()
    parser1.read(preprocess_config_file)
    preprocess_config = PreprocessConfig(parser1)

    logging.info(f'Using %s' % run_config_file)
    parser2 = ConfigParser()
    parser2.read(run_config_file)
    run_config = ParseSEIMSConfig(parser2)

    logging.info(f'Using %s' % postprocess_config_file)
    parser3 = ConfigParser()
    parser3.read(postprocess_config_file)
    postprocess_config = PostConfig(parser3)

    seims_obj = MainSEIMS(args_dict=run_config.ConfigDict)

    logger.configure_logging(seims_obj.output_dir, "rerun", logging_level=logging.INFO, lock=True)

    ImportParam2Mongo.workflow(preprocess_config)

    time = datetime.now().strftime('%Y.%m.%d-%H.%M.%S')

    seims_obj.run()

    plt = TimeSeriesPlots(postprocess_config).generate_plots()

    out_dir = Path(seims_obj.output_dir)
    logging.shutdown()
    shutil.move(out_dir, out_dir.parent / f'{out_dir.name}_{time}')

    plt.show()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', type=str, help='Path to the directory containing the config files', )
    args = parser.parse_args()
    rerun(args.d)
