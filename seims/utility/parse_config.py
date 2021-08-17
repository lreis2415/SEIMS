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

from typing import Optional, List, AnyStr
from pygeoc.utils import FileClass, StringClass, UtilClass, MathClass, is_integer


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


def get_option_value_exactly(cf, secname, optname, valtyp=str):
    # type: (ConfigParser, AnyStr, Optional[AnyStr, List[AnyStr]], type) -> Optional[AnyStr, int, float]
    if valtyp == int:
        return cf.getint(secname, optname)
    elif valtyp == float:
        return cf.getfloat(secname, optname)
    elif valtyp == bool:
        return cf.getboolean(secname, optname)
    else:
        return cf.get(secname, optname)


def get_option_value(cf, secname, optnames, valtyp=str, defvalue=''):
    # type: (ConfigParser, AnyStr, Optional[AnyStr, List[AnyStr]], type) -> Optional[AnyStr, int, float]
    if type(optnames) is not list:
        optnames = [optnames]

    value = ''
    found = False
    for optname in optnames:  # For backward compatibility
        if secname in cf.sections() and cf.has_option(secname, optname):
            value = get_option_value_exactly(cf, secname, optname, valtyp=valtyp)
            found = True
            break
        elif cf.has_option('', optname):  # May be in [DEFAULT] section
            value = get_option_value_exactly(cf, '', optname, valtyp=valtyp)
            found = True
            break

    if not found:
        if valtyp == int and defvalue == '':  # int type and not set default value
            defvalue = -9999
        value = defvalue
    return value


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
    # type: (ConfigParser, AnyStr, Optional[AnyStr, List[AnyStr]]) -> Optional[datetime]
    """Parse datetime from the `ConfigParser` object."""
    time_str = get_option_value(cf, section_name, option_name)
    if not time_str:
        return None
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


class ParseResourceConfig(object):
    """Configuration of computing resources for model-level parallel computing."""

    def __init__(self, cf=None):
        # type: (Optional[ConfigParser]) -> None
        """Get parameters from ConfigParser object."""
        self.workload = 'scoop'  # type: AnyStr # available: scoop, slurm
        self.partition = ''  # type: AnyStr
        self.nnodes = -1  # type: int  # computing nodes required
        self.ntasks_pernode = -1  # type: int  # maximum tasks (process of mpi or task of scoop)
        self.ncores_pernode = -1  # type: int  # maximum cores/processors of each node

        res_sec = 'Computing_Resources'
        self.workload = get_option_value(cf, res_sec, 'workload')
        if self.workload is '':
            self.workload = 'scoop'
        self.partition = get_option_value(cf, res_sec, 'partition')
        self.nnodes = get_option_value(cf, res_sec, 'nnodes', valtyp=int)
        self.ntasks_pernode = get_option_value(cf, res_sec, 'ntasks_pernode', valtyp=int)
        self.ncores_pernode = get_option_value(cf, res_sec, 'ncores_pernode', valtyp=int)


