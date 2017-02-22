import argparse
from utils import FileClass


class C(object):
    pass


def getconfigfile():
    # Get model configuration file name
    c = C()
    parser = argparse.ArgumentParser(
        description="Read configuration file.")
    parser.add_argument('-ini', help="Full path of configuration file")
    args = parser.parse_args(namespace=c)
    ini_file = args.ini
    FileClass.checkfileexists(ini_file)
    return ini_file
