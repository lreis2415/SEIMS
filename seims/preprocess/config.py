#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Configuration of SEIMS project.
    @author   : Liangjun Zhu
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-23  lj - reorganize as basic class
"""
import os

from configparser import ConfigParser

from seims.preprocess.text import ModelNameUtils, ModelCfgUtils, DirNameUtils, LogNameUtils, \
    VectorNameUtils, SpatialNamesUtils, ModelParamDataUtils
from seims.pygeoc.pygeoc.hydro.TauDEM import TauDEMFilesUtils
from seims.pygeoc.pygeoc.utils.utils import FileClass, StringClass, get_config_file


class SEIMSConfig(object):
    """Parse SEIMS project configuration."""

    def __init__(self, cf):
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
        # 3. Switch for building SEIMS
        self.cluster = False
        self.storm_mode = False
        self.gen_cn = True
        self.gen_runoff_coef = True
        self.gen_crop = True
        self.gen_iuh = True
        # 4. Climate inputs
        self.hydro_climate_vars = None
        self.prec_sites = None
        self.prec_data = None
        self.Meteo_sites = None
        self.Meteo_data = None
        self.thiessen_field = 'ID'
        # 5. Spatial inputs
        self.prec_sites_thiessen = None
        self.meteo_sites_thiessen = None
        self.dem = None
        self.outlet_file = None
        self.landuse = None
        self.landcover_init_param = None
        self.soil = None
        self.soil_property = None
        self.mgt_field = None
        # 6. Option parameters
        self.is_TauDEM = True
        self.d8acc_threshold = 0
        self.np = 4
        self.d8down_method = 's'
        self.dorm_hr = -1.
        self.temp_base = 0.
        self.imper_perc_in_urban = 0.3
        self.default_reach_depth = 5.
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
                          'BASE_DIR, MODEL_DIR, TXT_DB_DIR, PREPROC_SCRIPT_DIR, '
                          'and CPP_PROGRAM_DIR are required!')
        if not FileClass.is_dir_exists(self.mpi_bin):
            self.mpi_bin = None
        if not FileClass.is_dir_exists(self.workspace):
            try:  # first try to make dirs
                os.mkdir(self.workspace)
            except OSError as exc:
                self.workspace = self.model_dir + os.sep + 'preprocess_output'
                print ('WARNING: Make WORKING_DIR failed: %s. Use the default: %s' % (
                    exc.message, self.workspace))
                if not os.path.exists(self.workspace):
                    os.mkdir(self.workspace)

        self.dirs = DirNameUtils(self.workspace)
        self.logs = LogNameUtils(self.dirs.log)
        self.vecs = VectorNameUtils(self.dirs.geoshp)
        self.taudems = TauDEMFilesUtils(self.dirs.taudem)
        self.spatials = SpatialNamesUtils(self.dirs.geodata2db)
        self.modelcfgs = ModelCfgUtils(self.model_dir)
        self.paramcfgs = ModelParamDataUtils(self.preproc_script_dir + os.sep + 'database')

        if not FileClass.is_dir_exists(self.clim_dir):
            print ('The CLIMATE_DATA_DIR is not existed, try the default folder name "climate".')
            self.clim_dir = self.base_dir + os.sep + 'climate'
            if not FileClass.is_dir_exists(self.clim_dir):
                raise IOError('Directories named "climate" MUST BE located in [base_dir]!')

        if not FileClass.is_dir_exists(self.spatial_dir):
            print ('The SPATIAL_DATA_DIR is not existed, try the default folder name "spatial".')
            self.spatial_dir = self.base_dir + os.sep + 'spatial'
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
            self.bmp_scenario_db = cf.get('MONGODB', 'BMPScenarioDBName')
            self.spatial_db = cf.get('MONGODB', 'SpatialDBName')
        else:
            raise ValueError('[MONGODB] section MUST be existed in *.ini file.')
        if not StringClass.is_valid_ip_addr(self.hostname):
            raise ValueError('HOSTNAME illegal defined in [MONGODB]!')

        # 3. Model related switch
        # by default, OpenMP version and daily (longterm) mode will be built
        if 'SWITCH' in cf.sections():
            self.cluster = cf.getboolean('SWITCH', 'forCluster')
            self.storm_mode = cf.getboolean('SWITCH', 'stormMode')
            self.gen_cn = cf.getboolean('SWITCH', 'genCN')
            self.gen_runoff_coef = cf.getboolean('SWITCH', 'genRunoffCoef')
            self.gen_crop = cf.getboolean('SWITCH', 'genCrop')

        if self.storm_mode:
            self.gen_iuh = False
            self.climate_db = ModelNameUtils.standardize_climate_dbname(self.climate_db)

        self.spatial_db = ModelNameUtils.standardize_spatial_dbname(self.cluster, self.storm_mode,
                                                                    self.spatial_db)

        # 4. Climate Input
        if 'CLIMATE' in cf.sections():
            self.hydro_climate_vars = self.clim_dir + os.sep + cf.get('CLIMATE',
                                                                      'hydroclimatevarfile')
            self.prec_sites = self.clim_dir + os.sep + cf.get('CLIMATE', 'precsitefile')
            self.prec_data = self.clim_dir + os.sep + cf.get('CLIMATE', 'precdatafile')
            self.Meteo_sites = self.clim_dir + os.sep + cf.get('CLIMATE', 'meteositefile')
            self.Meteo_data = self.clim_dir + os.sep + cf.get('CLIMATE', 'meteodatafile')
            self.thiessen_field = cf.get('CLIMATE', 'thiessenidfield')
        else:
            raise ValueError('Climate input file names MUST be provided in [CLIMATE]!')

        # 5. Spatial Input
        if 'SPATIAL' in cf.sections():
            self.prec_sites_thiessen = self.spatial_dir + os.sep + cf.get('SPATIAL',
                                                                          'precsitesthiessen')
            self.meteo_sites_thiessen = self.spatial_dir + os.sep + cf.get('SPATIAL',
                                                                           'meteositesthiessen')
            self.dem = self.spatial_dir + os.sep + cf.get('SPATIAL', 'dem')
            self.outlet_file = self.spatial_dir + os.sep + cf.get('SPATIAL', 'outlet_file')
            if not os.path.exists(self.outlet_file):
                self.outlet_file = None
            self.landuse = self.spatial_dir + os.sep + cf.get('SPATIAL', 'landusefile')
            self.landcover_init_param = self.txt_db_dir + os.sep \
                                        + cf.get('SPATIAL', 'landcoverinitfile')
            self.soil = self.spatial_dir + os.sep + cf.get('SPATIAL', 'soilseqnfile')
            self.soil_property = self.txt_db_dir + os.sep + cf.get('SPATIAL', 'soilseqntext')
            self.mgt_field = self.spatial_dir + os.sep + cf.get('SPATIAL', 'mgtfieldfile')
            if not os.path.exists(self.mgt_field) or \
                    StringClass.string_match(self.mgt_field, 'none'):
                self.mgt_field = None
        else:
            raise ValueError('Spatial input file names MUST be provided in [SPATIAL]!')

        # 6. Option parameters
        if 'OPTIONAL_PARAMETERS' in cf.sections():
            self.is_TauDEM = cf.getboolean('OPTIONAL_PARAMETERS', 'istaudemd8')
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
                    self.d8down_method = 'h'
            self.dorm_hr = cf.getfloat('OPTIONAL_PARAMETERS', 'dorm_hr')
            self.temp_base = cf.getfloat('OPTIONAL_PARAMETERS', 't_base')
            self.imper_perc_in_urban = cf.getfloat('OPTIONAL_PARAMETERS',
                                                   'imperviouspercinurbancell')
            self.default_reach_depth = cf.getfloat('OPTIONAL_PARAMETERS', 'default_reach_depth')
            self.default_landuse = cf.getint('OPTIONAL_PARAMETERS', 'defaultlanduse')
            self.default_soil = cf.getint('OPTIONAL_PARAMETERS', 'defaultsoil')


def parse_ini_configuration():
    """Load model configuration from *.ini file"""
    cf = ConfigParser()
    ini_file = get_config_file()
    cf.read(ini_file)
    return SEIMSConfig(cf)


if __name__ == '__main__':
    seims_cfg = parse_ini_configuration()
    print seims_cfg.meteo_sites_thiessen
