import datetime
import inspect
import logging
import os
import sys
from pathlib import Path

logging_format = '[%(asctime)s][%(filename)s:%(lineno)d][%(process)d-%(threadName)s][%(levelname)s] %(message)s'
# 2 handlers for the same logger:
h1 = logging.StreamHandler(sys.stdout)
h1.setLevel(logging.DEBUG)
# filter out everything that is above INFO level (WARN, ERROR, ...)
h1.addFilter(lambda record: record.levelno <= logging.INFO)

h2 = logging.StreamHandler(sys.stderr)
# take only warnings and error logs
h2.setLevel(logging.WARNING)
h2.addFilter(lambda record: record.levelno > logging.INFO)

LOCK = False


def set_logging_level_warning():
    logging.getLogger().setLevel(logging.WARNING)


def configure_logging(log_path='.', log_file_prefix=None, logging_level=logging.INFO, lock=False):
    if lock:
        global LOCK
        if LOCK:
            return
        LOCK = True
    if log_file_prefix is None:
        log_file_prefix = Path(inspect.stack()[1].filename).name
    log_root = Path(__file__).parent.parent / 'logs'
    if Path(log_path).is_absolute():
        log_root = Path(log_path)
    datetime_str = datetime.datetime.now().strftime('%Y.%m.%d-%H.%M.%S')
    log_file_name = log_root / f'{log_file_prefix}_{datetime_str}.log'
    Path(log_file_name).parent.mkdir(parents=True, exist_ok=True)

    # https://stackoverflow.com/questions/12158048/changing-loggings-basicconfig-which-is-already-set
    for handler in logging.root.handlers[:]:
        logging.root.removeHandler(handler)

    logging.basicConfig(
        format=logging_format,
        handlers=[
            logging.FileHandler(log_file_name, 'w', 'utf8'),
            h1, h2
        ],
        level=logging_level
    )
    logging.info(f'Log file: {log_file_name}')


def unlock_logging():
    global LOCK
    LOCK = False


if __name__ == '__main__':
    configure_logging('logs', 'test')
    logging.info('info')
    logging.warning('warning')
    set_logging_level_warning()
    logging.info('info2')
    logging.warning('warning2')
