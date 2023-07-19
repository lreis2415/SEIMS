import datetime
import logging
import os
import sys


def configure_logging(log_path, log_file_prefix):
    log_file_name = datetime.datetime.now().strftime(f'{log_path}/{log_file_prefix}_%Y.%m.%d-%H.%M.%S.log')
    os.makedirs(os.path.dirname(log_file_name), exist_ok=True)
    logging.basicConfig(
        format='[%(asctime)s][%(filename)s:%(lineno)d][%(levelname)s]%(message)s',
        handlers=[
            logging.FileHandler(log_file_name, 'w', 'utf8'),
            logging.StreamHandler(sys.stdout)
        ],
        level=logging.DEBUG
    )
