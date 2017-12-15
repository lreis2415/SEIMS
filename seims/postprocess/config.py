#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Configuration of Postprocess for SEIMS.
    @author   : Liangjun Zhu, Huiran Gao
    @changelog: 17-08-17  lj - reorganize as basic class
"""
import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.append(os.path.abspath(os.path.join(sys.path[0], '..')))

try:
    from ConfigParser import ConfigParser  # py2
except ImportError:
    from configparser import ConfigParser  # py3

from pygeoc.utils import FileClass, StringClass, get_config_file


class PostConfig(object):
    """Parse postprocess configuration of SEIMS project."""

    def __init__(self, cf):
        """Initialization."""
        # 1. Directories
        self.model_dir = None
        self.scenario_id = -1
        if 'PATH' in cf.sections():
            self.model_dir = cf.get('PATH', 'model_dir')
            self.scenario_id = cf.getint('PATH', 'scenarioid')
            if self.scenario_id < 0:
                self.model_dir = self.model_dir + os.sep + 'OUTPUT'
            else:
                self.model_dir = self.model_dir + os.sep + 'OUTPUT' + str(self.scenario_id)
        else:
            raise ValueError("[PATH] section MUST be existed in *.ini file.")
        if not FileClass.is_dir_exists(self.model_dir):
            raise ValueError("Please Check Directories defined in [PATH]")

        # 2. MongoDB configuration and database, collation, GridFS names
        self.hostname = '127.0.0.1'  # localhost by default
        self.port = 27017
        self.climate_db = ''
        self.bmp_scenario_db = ''
        self.spatial_db = ''
        if 'MONGODB' in cf.sections():
            self.hostname = cf.get('MONGODB', 'hostname')
            self.port = cf.getint('MONGODB', 'port')
            self.climate_db = cf.get('MONGODB', 'climatedbname')
            self.bmp_scenario_db = cf.get('MONGODB', 'bmpscenariodbname')
            self.spatial_db = cf.get('MONGODB', 'spatialdbname')
        else:
            raise ValueError('[MONGODB] section MUST be existed in *.ini file.')
        if not StringClass.is_valid_ip_addr(self.hostname):
            raise ValueError('HOSTNAME illegal defined in [MONGODB]!')

        # 3. Parameters
        self.plt_subbsnid = -1
        self.plt_vars = list()
        if 'PARAMETERS' in cf.sections():
            self.plt_subbsnid = cf.getint('PARAMETERS', 'plot_subbasinid')
            plt_vars_str = cf.get('PARAMETERS', 'plot_variables')
        else:
            raise ValueError("[PARAMETERS] section MUST be existed in *.ini file.")
        if self.plt_subbsnid < 0:
            raise ValueError("PLOT_SUBBASINID must be greater or equal than 0.")
        if plt_vars_str != '':
            self.plt_vars = StringClass.split_string(plt_vars_str)
        else:
            raise ValueError("PLOT_VARIABLES illegal defined in [PARAMETERS]!")

        # 4. Optional_Parameters
        if 'OPTIONAL_PARAMETERS' in cf.sections():
            tstart = cf.get('OPTIONAL_PARAMETERS', 'time_start')
            tend = cf.get('OPTIONAL_PARAMETERS', 'time_end')
        else:
            raise ValueError("[OPTIONAL_PARAMETERS] section MUST be existed in *.ini file.")
        try:
            # UTCTIME
            self.time_start = StringClass.get_datetime(tstart)
            self.time_end = StringClass.get_datetime(tend)
        except ValueError:
            raise ValueError('The time format MUST be "YYYY-MM-DD" or "YYYY-MM-DD HH:MM:SS".')
        if self.time_start >= self.time_end:
            raise ValueError("Wrong time setted in [OPTIONAL_PARAMETERS]!")
        # 5. Switches
        self.lang_cn = False
        if 'SWITCH' in cf.sections():
            self.lang_cn = cf.getboolean('SWITCH', 'lang_cn')


def parse_ini_configuration():
    """Load model configuration from *.ini file"""
    cf = ConfigParser()
    ini_file = get_config_file()
    cf.read(ini_file)
    return PostConfig(cf)


if __name__ == '__main__':
    cfg = parse_ini_configuration()
