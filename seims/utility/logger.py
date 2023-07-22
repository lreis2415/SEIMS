import datetime
import logging
import os
import sys


def log_subprocess_output(pipe):
    for line in iter(pipe.readline, b''):  # b'\n'-separated lines
        logging.info('%r', line)


def configure_logging(log_path, log_file_prefix):
    log_file_name = datetime.datetime.now().strftime(f'{log_path}/{log_file_prefix}_%Y.%m.%d-%H.%M.%S.log')
    os.makedirs(os.path.dirname(log_file_name), exist_ok=True)

    # https://stackoverflow.com/questions/12158048/changing-loggings-basicconfig-which-is-already-set
    for handler in logging.root.handlers[:]:
        logging.root.removeHandler(handler)

    # 2 handlers for the same logger:
    h1 = logging.StreamHandler(sys.stdout)
    h1.setLevel(logging.DEBUG)
    # filter out everything that is above INFO level (WARN, ERROR, ...)
    h1.addFilter(lambda record: record.levelno <= logging.INFO)

    h2 = logging.StreamHandler(sys.stderr)
    # take only warnings and error logs
    h2.setLevel(logging.WARNING)
    h2.addFilter(lambda record: record.levelno > logging.INFO)

    logging.basicConfig(
        format='[%(asctime)s][%(filename)s:%(lineno)d][%(process)d-%(threadName)s][%(levelname)s] %(message)s',
        handlers=[
            logging.FileHandler(log_file_name, 'w', 'utf8'),
            h1, h2
        ],
        level=logging.INFO
    )
