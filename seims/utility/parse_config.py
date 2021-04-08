"""Parse common used value or class from the configuration (*.ini) file.

    @author   : Liangjun Zhu

    @changelog:
    - 18-10-29  - lj - Extract from other packages.
"""
from __future__ import absolute_import, unicode_literals

import os
import argparse
from configparser import ConfigParser
from datetime import datetime

from typing import Optional, AnyStr
from pygeoc.utils import FileClass, StringClass, UtilClass


def get_optimization_config(desc='The help information is supposed not be empty.'):
    # type: (AnyStr) -> (ConfigParser, AnyStr)
    """Parse arguments.
    Returns:
        cf: ConfigParse object of *.ini file
        mtd: Method name, e.g., 'nsga2' for optimization, 'morris' for sensitivity analysis.
    """
    # define input arguments
    parser = argparse.ArgumentParser(description=desc)
    parser.add_argument('-ini', type=str, help="Full path of configuration file")
    # add mutually group
    psa_group = parser.add_mutually_exclusive_group()
    psa_group.add_argument('-nsga2', action='store_true', help='Run NSGA-II method')
    # parse arguments
    args = parser.parse_args()
    ini_file = args.ini
    psa_mtd = 'nsga2'  # Default
    if args.nsga2:
        psa_mtd = 'nsga2'
    if not FileClass.is_file_exists(ini_file):
        raise ImportError('Configuration file is not existed: %s' % ini_file)
    cf = ConfigParser()
    cf.read(ini_file)
    return cf, psa_mtd


def check_config_option(cf, section_name, option_name, print_warn=False):
    # type: (ConfigParser, AnyStr, AnyStr, bool) -> bool
    if not isinstance(cf, ConfigParser):
        raise IOError('ErrorInput: The first argument cf MUST be the object of `ConfigParser`!')
    if section_name not in cf.sections():
        print('Warning: %s is NOT defined!' % section_name)
        return False
    if not (cf.has_option(section_name, option_name)):
        if print_warn:
            print('Warning: %s is NOT defined in %s!' % (option_name, section_name))
        return False
    return True


def parse_datetime_from_ini(cf, section_name, option_name):
    # type: (ConfigParser, AnyStr, AnyStr) -> Optional[datetime]
    """Parse datetime from the `ConfigParser` object."""
    if not check_config_option(cf, section_name, option_name):
        return None
    time_str = cf.get(section_name, option_name)
    try:  # UTCTIME
        return StringClass.get_datetime(time_str)
    except ValueError:
        print('Warning: The time string %s is invalid, which MUST be '
              '"YYYY-MM-DD" or "YYYY-MM-DD HH:MM:SS".' % time_str)
        return None


class ParseNSGA2Config(object):
    """NSGA-II related parameters"""

    def __init__(self, cf, wp, dir_template='NSGA2_Gen_%d_Pop_%d'):
        # type: (ConfigParser, AnyStr, AnyStr) -> None
        """Initialization."""
        self.ngens = cf.getint('NSGA2', 'generationsnum') if \
            cf.has_option('NSGA2', 'generationsnum') else 1
        self.npop = cf.getint('NSGA2', 'populationsize') if \
            cf.has_option('NSGA2', 'populationsize') else 4
        self.rsel = cf.getfloat('NSGA2', 'selectrate') if \
            cf.has_option('NSGA2', 'selectrate') else 1.
        self.rcross = cf.getfloat('NSGA2', 'crossoverrate') if \
            cf.has_option('NSGA2', 'crossoverrate') else 0.8
        self.pmut = cf.getfloat('NSGA2', 'maxmutateperc') if \
            cf.has_option('NSGA2', 'maxmutateperc') else 0.2
        self.rmut = cf.getfloat('NSGA2', 'mutaterate') if \
            cf.has_option('NSGA2', 'mutaterate') else 0.1

        if self.npop % 4 != 0:
            raise ValueError('PopulationSize must be a multiple of 4.')

        if '%d' not in dir_template:
            dir_template += '_Gen_%d_Pop_%d'
        elif dir_template.count('%d') == 1:
            dir_template += '_Pop_%d'
        elif dir_template.count('%d') > 2:
            dir_template = 'NSGA2_Gen_%d_Pop_%d'
        self.dirname = dir_template % (self.ngens, self.npop)
        self.out_dir = wp + os.path.sep + self.dirname
        UtilClass.rmmkdir(self.out_dir)

        self.hypervlog = self.out_dir + os.path.sep + 'hypervolume.txt'
        self.logfile = self.out_dir + os.path.sep + 'runtime.log'
        self.logbookfile = self.out_dir + os.path.sep + 'logbook.txt'
        self.simdata_dir = self.out_dir + os.path.sep + 'simulated_data'
        UtilClass.rmmkdir(self.simdata_dir)
