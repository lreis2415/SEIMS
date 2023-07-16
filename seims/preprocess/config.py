"""Configuration of SEIMS project.

    @author   : Liangjun Zhu

    @changelog:
    - 16-12-07  lj - rewrite for version 2.0
    - 17-06-23  lj - reorganize as basic class
    - 17-12-18  lj - add field partition parameters
    - 18-02-08  lj - combine serial and cluster versions and compatible with Python3.
    - 23-03-30  lj - improve code robust when reading configuration options
"""
from __future__ import absolute_import, unicode_literals

import json
import sys
import os
from os.path import join as pjoin

from pymongo import InsertOne

if os.path.abspath(os.path.join(sys.path[0], '..')) not in sys.path:
    sys.path.insert(0, os.path.abspath(os.path.join(sys.path[0], '..')))

from configparser import ConfigParser

from pymongo.mongo_client import MongoClient
from pymongo.database import Database
from pygeoc.utils import FileClass, StringClass, UtilClass, get_config_file, is_string

from preprocess.text import ModelCfgUtils, DirNameUtils, LogNameUtils, DBTableNames, ModelCfgFields
from preprocess.text import TauDEMbasedNames, VectorNameUtils, \
    SpatialNamesUtils, ModelParamDataUtils
from preprocess.db_mongodb import ConnectMongoDB, MongoUtil
from utility.parse_config import get_option_value


class PreprocessConfig(object):
    """Parse SEIMS project configuration."""

    def __init__(self, cf):  # type: (ConfigParser) -> None
        """Initialization."""
        # 1. Directories
        self.base_dir = None
        self.clim_dir = None
        self.spatial_dir = None
        self.observe_dir = None
        self.scenario_dir = None
        self.model_dir = None
        self.txt_db_dir = None
        self.prepscript_dir = None
        self.seims_bin = None
        self.mpi_bin = None
        self.workspace = None
        # 1.1. Directory determined flags
        self.use_observed = True
        self.use_scenario = True
        # 2. MongoDB configuration and database, collation, GridFS names
        self.hostname = '127.0.0.1'  # localhost by default
        self.port = 27017
        self.climate_db = ''
        self.scenario_db = ''
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
        self.soil_property_physical = None
        self.soil_property_conceptual = None
        self.fields_partition = False
        self.fields_partition_thresh = list()
        self.additional_rs = dict()
        self.has_conceptual_subbasin = False
        self.is_lumped = False
        self.hru_property_names = list()
        self.hru_property_files = list()
        self.conceptual_mask_file = None
        # 5. Option parameters
        self.acc_thresh = 0
        self.np = 4
        self.distdown_method = 's'
        self.dorm_hr = -1.
        self.temp_base = 0.
        self.imper_perc_in_urban = 0.
        self.default_landuse = -1
        self.default_soil = -1
        # 1. Directories
        if 'PATH' in cf.sections():
            self.base_dir = get_option_value(cf, 'PATH', 'base_data_dir', required=True)
            self.model_dir = get_option_value(cf, 'PATH', 'model_dir', required=True)
            self.prepscript_dir = get_option_value(cf, 'PATH', 'preproc_script_dir', required=True)
            self.seims_bin = get_option_value(cf, 'PATH', 'cpp_program_dir', required=True)
            # Optional paths
            self.clim_dir = get_option_value(cf, 'PATH', 'climate_data_dir')
            self.spatial_dir = get_option_value(cf, 'PATH', 'spatial_data_dir')
            self.txt_db_dir = get_option_value(cf, 'PATH', 'txt_db_dir')
            self.observe_dir = get_option_value(cf, 'PATH', 'measurement_data_dir')
            self.scenario_dir = get_option_value(cf, 'PATH', 'bmp_data_dir')
            self.mpi_bin = get_option_value(cf, 'PATH', 'mpiexec_dir')
            self.workspace = get_option_value(cf, 'PATH', 'working_dir')
        else:
            raise ValueError('[PATH] section MUST be existed in *.ini file.')

        if not FileClass.is_dir_exists(self.base_dir):
            raise IOError('BASE_DATA_DIR is required in PATH section!')

        if not FileClass.is_dir_exists(self.model_dir):
            raise IOError('MODEL_DIR is required in PATH section!')

        if not FileClass.is_dir_exists(self.prepscript_dir):
            raise IOError('PREPROC_SCRIPT_DIR is required in PATH section!')

        if not FileClass.is_dir_exists(self.seims_bin):
            raise IOError('CPP_PROGRAM_DIR is required in PATH section!')

        if not self.mpi_bin or not FileClass.is_dir_exists(self.mpi_bin):
            self.mpi_bin = None
        if not self.workspace or not FileClass.is_dir_exists(self.workspace):
            try:  # first try to make dirs
                UtilClass.mkdir(self.workspace)
            except OSError as exc:
                self.workspace = self.model_dir + os.path.sep + 'preprocess_output'
                print('WARNING: Make WORKING_DIR failed! Use the default: %s' % self.workspace)
                UtilClass.mkdir(self.workspace)

        self.dirs = DirNameUtils(self.workspace)
        self.logs = LogNameUtils(self.dirs.log)
        self.vecs = VectorNameUtils(self.dirs.geoshp)
        self.taudems = TauDEMbasedNames(self.dirs.taudem)
        self.spatials = SpatialNamesUtils(self.dirs.geodata2db)
        self.modelcfgs = ModelCfgUtils(self.model_dir)
        self.paramcfgs = ModelParamDataUtils(self.prepscript_dir + os.path.sep + 'database')

        if not self.clim_dir or not FileClass.is_dir_exists(self.clim_dir):
            print('The CLIMATE_DATA_DIR is not specified or does not exist. Try the default folder name "climate".')
            self.clim_dir = self.base_dir + os.path.sep + 'climate'
            if not FileClass.is_dir_exists(self.clim_dir):
                raise IOError(
                    'Preprocess configuration file error! Nor is the CLIMATE_DATA_DIR specified, or the default folder named "climate" exist in BASE_DATA_DIR.')

        if not self.spatial_dir or not FileClass.is_dir_exists(self.spatial_dir):
            print('The SPATIAL_DATA_DIR is not specified or does not exist. Try the default folder name "spatial".')
            self.spatial_dir = self.base_dir + os.path.sep + 'spatial'
            if not FileClass.is_dir_exists(self.spatial_dir):
                raise IOError(
                    'Preprocess configuration file error! Nor is the SPATIAL_DATA_DIR specified, or the default folder named "spatial" exist in BASE_DATA_DIR.')

        if not self.txt_db_dir or not FileClass.is_dir_exists(self.txt_db_dir):
            print('The TXT_DB_DIR is not specified or does not exist. Try the default folder name "lookup".')
            self.txt_db_dir = self.base_dir + os.path.sep + 'lookup'
            if not FileClass.is_dir_exists(self.txt_db_dir):
                self.txt_db_dir = None

        if not self.observe_dir or not FileClass.is_dir_exists(self.observe_dir):
            print('The MEASUREMENT_DATA_DIR is not specified or does not exist. '
                  'Try the default folder name "observed".')
            self.observe_dir = self.base_dir + os.path.sep + 'observed'
            if not FileClass.is_dir_exists(self.observe_dir):
                self.observe_dir = None
                self.use_observed = False

        if not self.scenario_dir or not FileClass.is_dir_exists(self.scenario_dir):
            print('The BMP_DATA_DIR is not specified or does not exist. Try the default folder name "scenario".')
            self.scenario_dir = self.base_dir + os.path.sep + 'scenario'
            if not FileClass.is_dir_exists(self.scenario_dir):
                self.scenario_dir = None
                self.use_scenario = False

        # 2. MongoDB related
        if 'MONGODB' in cf.sections():
            self.hostname = get_option_value(cf, 'MONGODB', ['hostname', 'ip'], required=True)
            self.port = get_option_value(cf, 'MONGODB', 'port', int, 27017)
            self.spatial_db = get_option_value(cf, 'MONGODB', 'spatialdbname', required=True)
            self.climate_db = get_option_value(cf, 'MONGODB', 'climatedbname', required=True)
            self.scenario_db = get_option_value(cf, 'MONGODB',
                                                ['bmpscenariodbname', 'scenariodbname'])
        else:
            raise ValueError('[MONGODB] section MUST be specified in *.ini file.')
        # build a global connection to mongodb database
        self.client = ConnectMongoDB(self.hostname, self.port)
        self.conn = self.client.get_conn()  # type: MongoClient
        self.maindb = self.conn[self.spatial_db]  # type: Database
        self.climatedb = self.conn[self.climate_db]
        self.scenariodb = None
        if self.use_scenario and self.scenario_db:
            self.scenariodb = self.conn[self.scenario_db]

        # 3. Climate Input
        if 'CLIMATE' in cf.sections():
            self.hydro_climate_vars = pjoin(self.clim_dir,
                                            get_option_value(cf, 'CLIMATE', 'hydroclimatevarfile',
                                                             required=True))
            self.prec_sites = pjoin(self.clim_dir,
                                    get_option_value(cf, 'CLIMATE', 'precsitefile', required=True))
            self.prec_data = pjoin(self.clim_dir,
                                   get_option_value(cf, 'CLIMATE', 'precdatafile', required=True))
            self.Meteo_sites = pjoin(self.clim_dir,
                                     get_option_value(cf, 'CLIMATE',
                                                      'meteositefile', required=True))
            self.Meteo_data = pjoin(self.clim_dir,
                                    get_option_value(cf, 'CLIMATE',
                                                     'meteodatafile', required=True))
            self.thiessen_field = get_option_value(cf, 'CLIMATE', 'thiessenidfield', str, 'ID')
        else:
            raise ValueError('Climate input file names MUST be specified in [CLIMATE]!')

        # 4. Spatial Input
        if 'SPATIAL' in cf.sections():
            self.prec_sites_thiessen = pjoin(self.spatial_dir,
                                             get_option_value(cf, 'SPATIAL',
                                                              'precsitesthiessen', required=True))
            self.meteo_sites_thiessen = pjoin(self.spatial_dir,
                                              get_option_value(cf, 'SPATIAL',
                                                               'meteositesthiessen', required=True))
            self.dem = pjoin(self.spatial_dir, get_option_value(cf, 'SPATIAL', 'dem', required=True))
            self.outlet_file = pjoin(self.spatial_dir, get_option_value(cf, 'SPATIAL', 'outlet_file'))
            if not FileClass.is_file_exists(self.outlet_file):
                self.outlet_file = None
            self.landuse = pjoin(self.spatial_dir,
                                 get_option_value(cf, 'SPATIAL', 'landusefile', required=True))
            self.landcover_init_param = pjoin(self.txt_db_dir,
                                              get_option_value(cf, 'SPATIAL', 'landcoverinitfile'))
            self.soil = pjoin(self.spatial_dir,
                              get_option_value(cf, 'SPATIAL', 'soilseqnfile', required=True))

            if_at_least_has_one = False
            if cf.has_option('SPATIAL', 'soilseqntext'):
                self.soil_property_physical = pjoin(self.txt_db_dir, get_option_value(cf, 'SPATIAL', 'soilseqntext'))
                if_at_least_has_one = True
            elif cf.has_option('SPATIAL', 'soilseqntextphysical'):
                self.soil_property_physical = pjoin(self.txt_db_dir,
                                                    get_option_value(cf, 'SPATIAL', 'soilseqntextphysical'))
                if_at_least_has_one = True
            if cf.has_option('SPATIAL', 'soilseqntextconceptual'):
                self.soil_property_conceptual = pjoin(self.txt_db_dir,
                                                      get_option_value(cf, 'SPATIAL', 'soilseqntextconceptual'))
                if_at_least_has_one = True
            if not if_at_least_has_one:
                raise ValueError(
                    'At least one of the soil property files MUST be specified in [SPATIAL]! (soilSEQNTextPhysical or soilSEQNTextConceptual)')

            if cf.has_option('SPATIAL', 'additionalfile'):
                additional_dict_str = get_option_value(cf, 'SPATIAL', 'additionalfile')
                tmpdict = json.loads(additional_dict_str)
                tmpdict = {str(k): (str(v) if is_string(v) else v) for k, v in
                           list(tmpdict.items())}
                for k, v in list(tmpdict.items()):
                    # Existence check has been moved to mask_origin_delineated_data()
                    #  in sp_delineation.py
                    self.additional_rs[k] = v
            # Field partition
            if cf.has_option('SPATIAL', 'field_partition_thresh'):
                ths = get_option_value(cf, 'SPATIAL', 'field_partition_thresh')
                thsv = StringClass.extract_numeric_values_from_string(ths)
                if thsv is not None:
                    self.fields_partition_thresh = [int(v) for v in thsv]
                    self.fields_partition = True

            # Conceptual subbasins
            if cf.has_option('SPATIAL', 'hasConceptualSubbasin'):
                value = get_option_value(cf, 'SPATIAL', 'hasConceptualSubbasin', int)
                if value == 1:
                    self.has_conceptual_subbasin = True
            if cf.has_option('SPATIAL', 'isLumped'):
                value = get_option_value(cf, 'SPATIAL', 'isLumped', int)
                if value == 1:
                    self.is_lumped = True

            # HRU property files
            if cf.has_option('SPATIAL', 'HRU_properties'):
                if self.is_lumped or self.has_conceptual_subbasin:
                    raise ValueError('Warning: [SPATIAL] `HRU_properties` field is not allowed when `isLumped` or `hasConceptualSubbasin` is set to 1.')
                hru_properties = get_option_value(cf, 'SPATIAL', 'HRU_properties', str)
                # split and strip
                hru_properties = [x.strip() for x in hru_properties.split(',')]
                for prop in hru_properties:
                    if prop == SpatialNamesUtils._SOILTYPEMFILE:
                        self.hru_property_names.append(SpatialNamesUtils._SOILTYPEMFILE)
                        self.hru_property_files.append(self.soil_property_conceptual)
                    elif prop == SpatialNamesUtils._LANDUSEMFILE:
                        self.hru_property_names.append(SpatialNamesUtils._LANDUSEMFILE)
                        self.hru_property_files.append(self.landuse)
                    else:
                        raise ValueError('Warning: [SPATIAL] `HRU_properties` field may be incorrectly written.\n'
                                         'Correct example: HRU_properties = LANDUSE, SOILTYPE')
            self._check_conceptual_setting()
            if self.is_lumped: # TODO: lump may use a single raster cell to represent the whole basin, using hru_subbasin_id. --wyj
                self.conceptual_mask_file = self.spatials.mask
            elif self.has_conceptual_subbasin:
                self.conceptual_mask_file = self.spatials.hru_subbasin_id



        else:
            raise ValueError('Spatial input file names MUST be specified in [SPATIAL]!')

        # 5. Optional parameters
        if 'OPTIONAL_PARAMETERS' in cf.sections():
            self.np = get_option_value(cf, 'OPTIONAL_PARAMETERS', 'np', int, 2)
            self.acc_thresh = get_option_value(cf, 'OPTIONAL_PARAMETERS', 'accthreshold', float, 10)
            self.min_flowfrac = get_option_value(cf, 'OPTIONAL_PARAMETERS', 'minflowfraction',
                                                 float, 0.01)  # default min flow frac to downstream
            self.distdown_method = get_option_value(cf, 'OPTIONAL_PARAMETERS', 'distancedownmethod')
            if StringClass.string_match(self.distdown_method, 'surface'):
                self.distdown_method = 's'
            elif StringClass.string_match(self.distdown_method, 'horizontal'):
                self.distdown_method = 'h'
            elif StringClass.string_match(self.distdown_method, 'pythagoras'):
                self.distdown_method = 'p'
            elif StringClass.string_match(self.distdown_method, 'vertical'):
                self.distdown_method = 'v'
            else:
                self.distdown_method = self.distdown_method.lower()
                if self.distdown_method not in ['s', 'h', 'p', 'v']:
                    self.distdown_method = 's'
            self.dorm_hr = get_option_value(cf, 'OPTIONAL_PARAMETERS', 'dorm_hr', float, -1.)
            self.temp_base = get_option_value(cf, 'OPTIONAL_PARAMETERS', 't_base', float, 0.)
            self.imper_perc_in_urban = get_option_value(cf, 'OPTIONAL_PARAMETERS',
                                                        'imperviouspercinurbancell', float, 0.)
            self.default_landuse = get_option_value(cf, 'OPTIONAL_PARAMETERS', 'defaultlanduse', int)
            self.default_soil = get_option_value(cf, 'OPTIONAL_PARAMETERS', 'defaultsoil', int)

    def _check_conceptual_setting(self):
        if not self.has_conceptual_subbasin and self.is_lumped:
            raise ValueError('Warning: [SPATIAL] Error if isLumped=1 and hasConceptualSubbasin=0.')

def parse_ini_configuration():
    """Load model configuration from *.ini file"""
    cf = ConfigParser()
    ini_file = get_config_file()
    cf.read(ini_file)
    return PreprocessConfig(cf)


if __name__ == '__main__':
    seims_cfg = parse_ini_configuration()
    print(seims_cfg.meteo_sites_thiessen)
