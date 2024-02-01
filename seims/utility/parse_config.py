"""Parse common used value or class from the configuration (*.ini) file.

    @author   : Liangjun Zhu

    @changelog:
    - 18-10-29  - lj - Extract from other packages.
    - 23-03-29  - lj - ReWrite check_config_option and get_option_value functions.
"""
from __future__ import absolute_import, unicode_literals

import os
import argparse
from configparser import ConfigParser
from datetime import datetime
from pathlib import Path

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
    parser.add_argument('-p', type=int, help="num of total processors available", default=None)
    parser.add_argument('-w', type=int, help="num of workers")
    parser.add_argument('-ppw', type=int, help="num of processors per worker")
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
    if args.p or args.w or args.ppw:
        return cf, psa_mtd, args.p, args.w, args.ppw
    else:
        return cf, psa_mtd


def get_option_value_exactly(cf, secname, optname, valtyp=str):
    # type: (ConfigParser, AnyStr, AnyStr, type) -> Optional[AnyStr, int, float]
    if valtyp == int:
        return cf.getint(secname, optname)
    elif valtyp == float:
        return cf.getfloat(secname, optname)
    elif valtyp == bool:
        return cf.getboolean(secname, optname)
    else:
        return cf.get(secname, optname)


def check_config_option(cf, secname, optnames, print_warn=False):
    # type: (ConfigParser, AnyStr, Optional[AnyStr, List[AnyStr]], bool) -> (bool, AnyStr, AnyStr)
    if not isinstance(cf, ConfigParser):
        raise IOError('ErrorInput: The first argument cf MUST be the object of `ConfigParser`!')
    if type(optnames) is not list:
        optnames = [optnames]  # type: List[AnyStr]
    if secname not in cf.sections():
        if print_warn:
            print('Warning: Section %s is NOT defined, try to find in DEFAULT section!' % secname)
        for optname in optnames:  # For backward compatibility
            if cf.has_option('', optname):  # May be in [DEFAULT] section
                return True, '', optname
        if print_warn:
            print('Warning: Section %s is NOT defined, '
                  'Option %s is NOT FOUND!' % (secname, ','.join(optnames)))
        return False, '', ''
    else:
        for optname in optnames:  # For backward compatibility
            if cf.has_option(secname, optname):
                return True, secname, optname
        if print_warn:
            print('Warning: Option %s is NOT FOUND in Section %s!' % (','.join(optnames), secname))
        return False, '', ''


def get_option_value(cf,  # type: ConfigParser
                     secname,  # type: AnyStr
                     optnames,  # type: Optional[AnyStr, List[AnyStr]]
                     valtyp=str,  # type: Optional[AnyStr, int, float, bool]
                     defvalue='',  # type: Optional[AnyStr, int, float, bool]
                     required=False,  # type: bool
                     print_warn=False  # type: bool
                     ):  # type: (...) -> Optional[AnyStr, int, float]
    found, sname, oname = check_config_option(cf, secname, optnames, print_warn=print_warn)
    if not found:
        if required:
            raise IOError('Error Input in configuration!')
        else:
            if defvalue == '' and (valtyp == int or valtyp == float):
                return -9999  # int or float value type, but not set default value properly
            return defvalue
    return get_option_value_exactly(cf, sname, oname, valtyp=valtyp)


def parse_datetime_from_ini(cf, section_name, option_name, print_warn=True, required=True):
    # type: (ConfigParser, AnyStr, Optional[AnyStr, List[AnyStr]], bool, bool) -> Optional[datetime]
    """Parse datetime from the `ConfigParser` object."""
    time_str = get_option_value(cf, section_name, option_name,
                                print_warn=print_warn, required=required)
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
        self.ngens = get_option_value(cf, 'NSGA2', 'generationsnum', int, 1)
        self.npop = get_option_value(cf, 'NSGA2', 'populationsize', int, 4)
        self.rsel = get_option_value(cf, 'NSGA2', 'selectrate', float, 1.)
        self.rcross = get_option_value(cf, 'NSGA2', 'crossoverrate', float, 0.8)
        self.pmut = get_option_value(cf, 'NSGA2', 'maxmutateperc', float, 0.2)
        self.rmut = get_option_value(cf, 'NSGA2', 'mutaterate', float, 0.1)

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

        UtilClass.mkdir(self.out_dir)  # Do not remove if already existed

        self.hypervlog = self.out_dir + os.path.sep + 'hypervolume.txt'
        self.logfile = self.out_dir + os.path.sep + 'runtime.log'
        self.logbookfile = self.out_dir + os.path.sep + 'logbook.txt'
        self.simdata_dir = self.out_dir + os.path.sep + 'simulated_data'
        UtilClass.mkdir(self.simdata_dir)


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
        if self.workload == '':
            self.workload = 'scoop'
        self.partition = get_option_value(cf, res_sec, 'partition')
        self.nnodes = get_option_value(cf, res_sec, 'nnodes', int, 1)
        self.ntasks_pernode = get_option_value(cf, res_sec, 'ntasks_pernode', int, 1)
        self.ncores_pernode = get_option_value(cf, res_sec, 'ncores_pernode', int, 1)
