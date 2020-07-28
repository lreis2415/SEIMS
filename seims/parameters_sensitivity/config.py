"""Base configuration of Parameters Sensitivity Analysis.

    @author   : Liangjun Zhu

    @changelog:
    - 17-12-22  - lj - initial implementation.
    - 18-01-11  - lj - integration of screening method and variant-based method.
    - 18-02-09  - lj - compatible with Python3.
    - 18-07-10  - lj - Extract a common parse class for SEIMS model, `ParseSEIMSConfig`.
"""
from __future__ import absolute_import, unicode_literals

import os
import sys
import argparse

from configparser import ConfigParser
from pygeoc.utils import FileClass, StringClass, UtilClass

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from run_seims import ParseSEIMSConfig
from utility import PlotConfig


def get_psa_config():
    """Parse arguments.
    Returns:
        cf: ConfigParse object of *.ini file
        mtd: Parameters sensitivity method name, currently, 'morris' and 'fast' are supported.
    """
    # define input arguments
    parser = argparse.ArgumentParser(description="Execute parameters sensitivity analysis.")
    parser.add_argument('-ini', type=str, help="Full path of configuration file")
    # add mutually group
    psa_group = parser.add_mutually_exclusive_group()
    psa_group.add_argument('-morris', action='store_true', help='Run Morris Screening method')
    psa_group.add_argument('-fast', action='store_true', help='Run FAST variant-based method')
    # parse arguments
    args = parser.parse_args()
    ini_file = args.ini
    psa_mtd = 'morris'  # Default
    if args.fast:
        psa_mtd = 'fast'
    elif args.morris:
        psa_mtd = 'morris'
    if not FileClass.is_file_exists(ini_file):
        raise ImportError('Configuration file is not existed: %s' % ini_file)
    cf = ConfigParser()
    cf.read(ini_file)
    return cf, psa_mtd


class MorrisConfig(object):
    """Configuration for Morris screening method."""

    def __init__(self, cf):
        # type: (ConfigParser) -> None
        """Get parameters from ConfigParser object."""
        self.N = 100
        self.num_levels = 10
        self.optimal_t = None
        self.local_opt = True
        section_name = 'Morris_Method'
        if section_name not in cf.sections():
            raise ValueError('[%s] section MUST be existed in *.ini file.' % section_name)

        if cf.has_option(section_name, 'n'):
            self.N = cf.getint(section_name, 'n')
        if cf.has_option(section_name, 'num_levels'):
            self.num_levels = cf.getint(section_name, 'num_levels')
        if cf.has_option(section_name, 'optimal_trajectories'):
            tmp_opt_t = cf.get(section_name, 'optimal_trajectories')
            if not StringClass.string_match(tmp_opt_t, 'none'):
                self.optimal_t = cf.getint(section_name, 'optimal_trajectories')
                if self.optimal_t > self.N or self.optimal_t < 2:
                    self.optimal_t = None
        if cf.has_option(section_name, 'local_optimization'):
            self.local_opt = cf.getboolean(section_name, 'local_optimization')


class FASTConfig(object):
    """Configuration for FAST variant-based method."""

    def __init__(self, cf):
        """Get parameters from ConfigParser object."""
        self.param_range_def = 'fast_param_rng.def'
        self.N = 64
        self.M = 4
        section_name = 'FAST_Method'
        if section_name not in cf.sections():
            raise ValueError('[%s] section MUST be existed in *.ini file.' % section_name)

        self.param_range_def = cf.get(section_name, 'paramrngdef')
        if cf.has_option(section_name, 'n'):
            self.N = cf.getint(section_name, 'n')
        if cf.has_option(section_name, 'm'):
            self.M = cf.getint(section_name, 'm')

        if self.N <= 4 * self.M ** 2:
            raise ValueError('Sample size N > 4M^2 is required for FAST method. M=4 by default.')


class PSAOutputs(object):
    """Predefined output files for parameters sensitivity analysis."""

    def __init__(self, wp):
        """Initialization."""
        self.param_defs_json = wp + os.path.sep + 'param_defs.json'
        self.param_values_txt = wp + os.path.sep + 'param_values.txt'
        self.output_values_dir = wp + os.path.sep + 'temp_output_values'
        self.output_values_txt = wp + os.path.sep + 'output_values.txt'
        self.psa_si_json = wp + os.path.sep + 'psa_si.json'
        self.psa_si_sort_txt = wp + os.path.sep + 'psa_si_sorted.csv'
        UtilClass.mkdir(self.output_values_dir)


class PSAConfig(object):
    """Parse parameters sensitivity analysis configuration of SEIMS project."""

    def __init__(self, cf, method='morris'):
        """Initialization."""
        self.method = method
        # 1. SEIMS model related
        self.model = ParseSEIMSConfig(cf)
        # 2. Common settings of parameters sensitivity analysis
        if 'PSA_Settings' not in cf.sections():
            raise ValueError("[PSA_Settings] section MUST be existed in *.ini file.")

        self.evaluate_params = list()
        if cf.has_option('PSA_Settings', 'evaluateparam'):
            eva_str = cf.get('PSA_Settings', 'evaluateparam')
            self.evaluate_params = StringClass.split_string(eva_str, ',')
        else:
            self.evaluate_params = ['Q']  # Default

        self.param_range_def = 'morris_param_rng.def'  # Default
        if cf.has_option('PSA_Settings', 'paramrngdef'):
            self.param_range_def = cf.get('PSA_Settings', 'paramrngdef')
        self.param_range_def = self.model.model_dir + os.path.sep + self.param_range_def
        if not FileClass.is_file_exists(self.param_range_def):
            raise IOError('Ranges of parameters MUST be provided!')

        if not (cf.has_option('PSA_Settings', 'psa_time_start') and
                cf.has_option('PSA_Settings', 'psa_time_end')):
            raise ValueError("Start and end time of PSA MUST be specified in [PSA_Settings].")
        try:
            # UTCTIME
            tstart = cf.get('PSA_Settings', 'psa_time_start')
            tend = cf.get('PSA_Settings', 'psa_time_end')
            self.psa_stime = StringClass.get_datetime(tstart)
            self.psa_etime = StringClass.get_datetime(tend)
        except ValueError:
            raise ValueError('The time format MUST be"YYYY-MM-DD HH:MM:SS".')
        if self.psa_stime >= self.psa_etime:
            raise ValueError("Wrong time settings in [PSA_Settings]!")

        # 3. Parameters settings for specific sensitivity analysis methods
        self.morris = None
        self.fast = None
        if self.method == 'fast':
            self.fast = FASTConfig(cf)
            self.psa_outpath = '%s/PSA_FAST_N%dM%d' % (self.model.model_dir,
                                                       self.fast.N, self.fast.M)
        elif self.method == 'morris':
            self.morris = MorrisConfig(cf)
            self.psa_outpath = '%s/PSA_Morris_N%dL%d' % (self.model.model_dir,
                                                         self.morris.N,
                                                         self.morris.num_levels)
        # 4. (Optional) Plot settings for matplotlib
        self.plot_cfg = PlotConfig(cf)

        # Do not remove psa_outpath if already existed
        UtilClass.mkdir(self.psa_outpath)
        self.outfiles = PSAOutputs(self.psa_outpath)


if __name__ == '__main__':
    cf, method = get_psa_config()
    cfg = PSAConfig(cf, method=method)

    print(cfg.param_range_def)
    if cfg.method == 'morris':
        print('Morris sceening method:')
        print('  N: %d, num_levels: %d' % (cfg.morris.N, cfg.morris.num_levels))
    elif cfg.method == 'fast':
        print('FAST variant-based method')
        print('  N: %d, M: %d' % (cfg.fast.N, cfg.fast.M))
