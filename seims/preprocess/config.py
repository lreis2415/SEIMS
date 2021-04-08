"""Configuration of SEIMS project.

    @author   : Liangjun Zhu

    @changelog:
    - 16-12-07  lj - rewrite for version 2.0
    - 17-06-23  lj - reorganize as basic class
    - 17-12-18  lj - add field partition parameters
    - 18-02-08  lj - combine serial and cluster versions and compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

import json
import os
import sys

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from configparser import ConfigParser

from pygeoc.TauDEM import TauDEMFilesUtils
from pygeoc.utils import FileClass, StringClass, UtilClass, get_config_file, is_string

from preprocess.text import ModelCfgUtils, DirNameUtils, LogNameUtils
from preprocess.text import VectorNameUtils, SpatialNamesUtils, ModelParamDataUtils


class PreprocessConfig(object):
    """Parse SEIMS project configuration."""

    def __init__(self, cf):
        """Initialization."""
        # 1. Directories
        self.base_dir = None
        self.clim_dir = None
        self.spatial_dir = None
        self.observe_dir = None
        self.scenario_dir = None
        self.model_dir = None
        self.txt_db_dir = None
        self.preproc_script_dir = None
        self.seims_bin = None
        self.mpi_bin = None
        self.workspace = None
        # 1.1. Directory determined flags
        self.use_observed = True
        self.use_scernario = True
        # 2. MongoDB configuration and database, collation, GridFS names
        self.hostname = '127.0.0.1'  # localhost by default
        self.port = 27017
        self.climate_db = ''
        self.bmp_scenario_db = ''
        self.spatial_db = ''
        # 3. Climate inputs
        self.hydro_climate_vars = None
        self.prec_sites = None
        self.prec_data = None
        self.Meteo_sites = None
        self.Meteo_data = None
        self.thiessen_field = 'ID'
        # 4. Spatial inputs
        self.prec_sites_thiessen = None
        self.meteo_sites_thiessen = None
        self.dem = None
        self.outlet_file = None
        self.landuse = None
        self.landcover_init_param = None
        self.soil = None
        self.soil_property = None
        self.fields_partition = False
        self.fields_partition_thresh = list()
        self.additional_rs = dict()
        # 5. Option parameters
        self.d8acc_threshold = 0
        self.np = 4
        self.d8down_method = 's'
        self.dorm_hr = -1.
        self.temp_base = 0.
        self.imper_perc_in_urban = 0.
        self.default_landuse = -1
        self.default_soil = -1
        # 1. Directories
        if 'PATH' in cf.sections():
            self.base_dir = cf.get('PATH', 'base_data_dir')
            self.clim_dir = cf.get('PATH', 'climate_data_dir')
            self.spatial_dir = cf.get('PATH', 'spatial_data_dir')
            self.observe_dir = cf.get('PATH', 'measurement_data_dir')
            self.scenario_dir = cf.get('PATH', 'bmp_data_dir')
            self.model_dir = cf.get('PATH', 'model_dir')
            self.txt_db_dir = cf.get('PATH', 'txt_db_dir')
            self.preproc_script_dir = cf.get('PATH', 'preproc_script_dir')
            self.seims_bin = cf.get('PATH', 'cpp_program_dir')
            self.mpi_bin = cf.get('PATH', 'mpiexec_dir')
            self.workspace = cf.get('PATH', 'working_dir')
        else:
            raise ValueError('[PATH] section MUST be existed in *.ini file.')
        if not (FileClass.is_dir_exists(self.base_dir)
                and FileClass.is_dir_exists(self.model_dir)
                and FileClass.is_dir_exists(self.txt_db_dir)
                and FileClass.is_dir_exists(self.preproc_script_dir)
                and FileClass.is_dir_exists(self.seims_bin)):
            raise IOError('Please Check Directories defined in [PATH]. '
                          'BASE_DATA_DIR, MODEL_DIR, TXT_DB_DIR, PREPROC_SCRIPT_DIR, '
                          'and CPP_PROGRAM_DIR are required!')
        if not FileClass.is_dir_exists(self.mpi_bin):
            self.mpi_bin = None
        if not FileClass.is_dir_exists(self.workspace):
            try:  # first try to make dirs
                UtilClass.mkdir(self.workspace)
                # os.mkdir(self.workspace)
            except OSError as exc:
                self.workspace = self.model_dir + os.path.sep + 'preprocess_output'
                print('WARNING: Make WORKING_DIR failed! Use the default: %s' % self.workspace)
                if not os.path.exists(self.workspace):
                    UtilClass.mkdir(self.workspace)

        self.dirs = DirNameUtils(self.workspace)
        self.logs = LogNameUtils(self.dirs.log)
        self.vecs = VectorNameUtils(self.dirs.geoshp)
        self.taudems = TauDEMFilesUtils(self.dirs.taudem)
        self.spatials = SpatialNamesUtils(self.dirs.geodata2db)
        self.modelcfgs = ModelCfgUtils(self.model_dir)
        self.paramcfgs = ModelParamDataUtils(self.preproc_script_dir + os.path.sep + 'database')

        if not FileClass.is_dir_exists(self.clim_dir):
            print('The CLIMATE_DATA_DIR is not existed, try the default folder name "climate".')
            self.clim_dir = self.base_dir + os.path.sep + 'climate'
            if not FileClass.is_dir_exists(self.clim_dir):
                raise IOError('Directories named "climate" MUST BE located in [base_dir]!')

        if not FileClass.is_dir_exists(self.spatial_dir):
            print('The SPATIAL_DATA_DIR is not existed, try the default folder name "spatial".')
            self.spatial_dir = self.base_dir + os.path.sep + 'spatial'
            raise IOError('Directories named "spatial" MUST BE located in [base_dir]!')

        if not FileClass.is_dir_exists(self.observe_dir):
            self.observe_dir = None
            self.use_observed = False

        if not FileClass.is_dir_exists(self.scenario_dir):
            self.scenario_dir = None
            self.use_scernario = False

        # 2. MongoDB related
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

        # 3. Climate Input
        if 'CLIMATE' in cf.sections():
            self.hydro_climate_vars = self.clim_dir + os.path.sep + cf.get('CLIMATE',
                                                                           'hydroclimatevarfile')
            self.prec_sites = self.clim_dir + os.path.sep + cf.get('CLIMATE', 'precsitefile')
            self.prec_data = self.clim_dir + os.path.sep + cf.get('CLIMATE', 'precdatafile')
            self.Meteo_sites = self.clim_dir + os.path.sep + cf.get('CLIMATE', 'meteositefile')
            self.Meteo_data = self.clim_dir + os.path.sep + cf.get('CLIMATE', 'meteodatafile')
            self.thiessen_field = cf.get('CLIMATE', 'thiessenidfield')
        else:
            raise ValueError('Climate input file names MUST be provided in [CLIMATE]!')

        # 4. Spatial Input
        if 'SPATIAL' in cf.sections():
            self.prec_sites_thiessen = self.spatial_dir + os.path.sep + cf.get('SPATIAL',
                                                                               'precsitesthiessen')
            self.meteo_sites_thiessen = self.spatial_dir + os.path.sep + cf.get('SPATIAL',
                                                                                'meteositesthiessen')
            self.dem = self.spatial_dir + os.path.sep + cf.get('SPATIAL', 'dem')
            self.outlet_file = self.spatial_dir + os.path.sep + cf.get('SPATIAL', 'outlet_file')
            if not os.path.exists(self.outlet_file):
                self.outlet_file = None
            self.landuse = self.spatial_dir + os.path.sep + cf.get('SPATIAL', 'landusefile')
            self.landcover_init_param = self.txt_db_dir + os.path.sep + cf.get('SPATIAL',
                                                                               'landcoverinitfile')
            self.soil = self.spatial_dir + os.path.sep + cf.get('SPATIAL', 'soilseqnfile')
            self.soil_property = self.txt_db_dir + os.path.sep + cf.get('SPATIAL', 'soilseqntext')
            if cf.has_option('SPATIAL', 'additionalfile'):
                additional_dict_str = cf.get('SPATIAL', 'additionalfile')
                tmpdict = json.loads(additional_dict_str)
                tmpdict = {str(k): (str(v) if is_string(v) else v) for k, v in
                           list(tmpdict.items())}
                for k, v in list(tmpdict.items()):
                    # Existence check has been moved to mask_origin_delineated_data()
                    #  in sp_delineation.py
                    self.additional_rs[k] = v
            # Field partition
            if cf.has_option('SPATIAL', 'field_partition_thresh'):
                ths = cf.get('SPATIAL', 'field_partition_thresh')
                thsv = StringClass.extract_numeric_values_from_string(ths)
                if thsv is not None:
                    self.fields_partition_thresh = [int(v) for v in thsv]
                    self.fields_partition = True
        else:
            raise ValueError('Spatial input file names MUST be provided in [SPATIAL]!')

        # 5. Optional parameters
        if 'OPTIONAL_PARAMETERS' in cf.sections():
            self.d8acc_threshold = cf.getfloat('OPTIONAL_PARAMETERS', 'd8accthreshold')
            self.np = cf.getint('OPTIONAL_PARAMETERS', 'np')
            self.d8down_method = cf.get('OPTIONAL_PARAMETERS', 'd8downmethod')
            if StringClass.string_match(self.d8down_method, 'surface'):
                self.d8down_method = 's'
            elif StringClass.string_match(self.d8down_method, 'horizontal'):
                self.d8down_method = 'h'
            elif StringClass.string_match(self.d8down_method, 'pythagoras'):
                self.d8down_method = 'p'
            elif StringClass.string_match(self.d8down_method, 'vertical'):
                self.d8down_method = 'v'
            else:
                self.d8down_method = self.d8down_method.lower()
                if self.d8down_method not in ['s', 'h', 'p', 'v']:
                    self.d8down_method = 's'
            self.dorm_hr = cf.getfloat('OPTIONAL_PARAMETERS', 'dorm_hr')
            self.temp_base = cf.getfloat('OPTIONAL_PARAMETERS', 't_base')
            self.imper_perc_in_urban = cf.getfloat('OPTIONAL_PARAMETERS',
                                                   'imperviouspercinurbancell')
            self.default_landuse = cf.getint('OPTIONAL_PARAMETERS', 'defaultlanduse')
            self.default_soil = cf.getint('OPTIONAL_PARAMETERS', 'defaultsoil')


def parse_ini_configuration():
    """Load model configuration from *.ini file"""
    cf = ConfigParser()
    ini_file = get_config_file()
    cf.read(ini_file)
    return PreprocessConfig(cf)


if __name__ == '__main__':
    seims_cfg = parse_ini_configuration()
    print(seims_cfg.meteo_sites_thiessen)
