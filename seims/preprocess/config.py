#! /usr/bin/env python
# coding=utf-8
# @Configuration of Preprocessing for SEIMS
# @Author Liang-Jun Zhu
#

import ConfigParser
import errno
from text import *
from util import *

# Load model configuration from *.ini file
cf = ConfigParser.ConfigParser()
cf.read(GetINIfile())
# 1. Directories
CLIMATE_DATA_DIR = None
SPATIAL_DATA_DIR = None
BMP_DATA_DIR = None
MEASUREMENT_DATA_DIR = None
if 'PATH' in cf.sections():
    BASE_DATA_DIR = cf.get('PATH', 'BASE_DATA_DIR'.lower())
    CLIMATE_DATA_DIR = cf.get('PATH', 'CLIMATE_DATA_DIR'.lower())
    SPATIAL_DATA_DIR = cf.get('PATH', 'SPATIAL_DATA_DIR'.lower())
    MEASUREMENT_DATA_DIR = cf.get('PATH', 'MEASUREMENT_DATA_DIR'.lower())
    BMP_DATA_DIR = cf.get('PATH', 'BMP_DATA_DIR'.lower())
    MODEL_DIR = cf.get('PATH', 'MODEL_DIR'.lower())
    TXT_DB_DIR = cf.get('PATH', 'TXT_DB_DIR'.lower())
    PREPROC_SCRIPT_DIR = cf.get('PATH', 'PREPROC_SCRIPT_DIR'.lower())
    CPP_PROGRAM_DIR = cf.get('PATH', 'CPP_PROGRAM_DIR'.lower())
    METIS_DIR = cf.get('PATH', 'METIS_DIR'.lower())
    MPIEXEC_DIR = cf.get('PATH', 'MPIEXEC_DIR'.lower())
    WORKING_DIR = cf.get('PATH', 'WORKING_DIR'.lower())
else:
    raise ValueError("[PATH] section MUST be existed in *.ini file.")
if not (isPathExists(BASE_DATA_DIR) and isPathExists(MODEL_DIR) and isPathExists(TXT_DB_DIR)
        and isPathExists(PREPROC_SCRIPT_DIR) and isPathExists(CPP_PROGRAM_DIR)):
    raise IOError("Please Check Directories defined in [PATH]")
if not isPathExists(MPIEXEC_DIR):
    MPIEXEC_DIR = None
if not os.path.isdir(WORKING_DIR):
    try: # first try to make dirs
        os.mkdir(WORKING_DIR)
    except OSError as exc:
        WORKING_DIR = MODEL_DIR + os.sep + 'preprocess_output'
        if not os.path.exists(WORKING_DIR):
            os.mkdir(WORKING_DIR)
        pass

if not (isPathExists(CLIMATE_DATA_DIR) and isPathExists(SPATIAL_DATA_DIR)):
    raise IOError(
        "Directories named 'climate' and 'spatial' MUST BE located in [BASE_DATA_DIR]!")
useObserved = True
if not isPathExists(MEASUREMENT_DATA_DIR):
    MEASUREMENT_DATA_DIR = None
    useObserved = False
useScernario = True
if not isPathExists(BMP_DATA_DIR):
    BMP_DATA_DIR = None
    useScernario = False

# 2. MongoDB related
if 'MONGODB' in cf.sections():
    HOSTNAME = cf.get('MONGODB', 'HOSTNAME'.lower())
    PORT = cf.getint('MONGODB', 'PORT'.lower())
    ClimateDBName = cf.get('MONGODB', 'ClimateDBName'.lower())
    BMPScenarioDBName = cf.get('MONGODB', 'BMPScenarioDBName'.lower())
    SpatialDBName = cf.get('MONGODB', 'SpatialDBName'.lower())
else:
    raise ValueError("[MONGODB] section MUST be existed in *.ini file.")
if not isIPValid(HOSTNAME):
    raise ValueError("HOSTNAME illegal defined in [MONGODB]!")

# 3. Model related switch
# by default, OMP version and daily mode will be built
forCluster = False
stormMode = False
genCN = True
genRunoffCoef = True
genCrop = True
if 'SWITCH' in cf.sections():
    forCluster = cf.getboolean('SWITCH', 'forCluster'.lower())
    stormMode = cf.getboolean('SWITCH', 'stormMode'.lower())
    genCN = cf.getboolean('SWITCH', 'genCN'.lower())
    genRunoffCoef = cf.getboolean('SWITCH', 'genRunoffCoef'.lower())
    genCrop = cf.getboolean('SWITCH', 'genCrop'.lower())

genIUH = True
simuMode = Tag_Mode_Daily
if forCluster and Tag_Cluster not in SpatialDBName.lower():
    SpatialDBName = Tag_Cluster + "_" + SpatialDBName
if forCluster and not isPathExists(METIS_DIR):
    raise IOError(
        "Please Check METIS executable Directories defined in [PATH]")
if stormMode:
    simuMode = Tag_Mode_Storm
    if not Tag_Mode_Storm.lower() in SpatialDBName.lower():
        SpatialDBName = SpatialDBName + "_" + Tag_Mode_Storm.lower()
    genIUH = False
if not Tag_Model.lower() in SpatialDBName.lower():
    SpatialDBName = Tag_Model.lower() + "_" + SpatialDBName
if forCluster and (not Tag_Cluster.lower() in SpatialDBName.lower()):
    SpatialDBName = Tag_Cluster.lower() + "_" + SpatialDBName
if ClimateDBName is not None and stormMode:
    ClimateDBName = ClimateDBName + "_" + Tag_CLIM_STORM_Suf.lower()

# 4. Climate Input
if 'CLIMATE' in cf.sections():
    HydroClimateVarFile = CLIMATE_DATA_DIR + os.sep + cf.get('CLIMATE', 'HydroClimateVarFile'.lower())
    MetroSiteFile = CLIMATE_DATA_DIR + os.sep + cf.get('CLIMATE', 'MetroSiteFile'.lower())
    PrecSiteFile = CLIMATE_DATA_DIR + os.sep + cf.get('CLIMATE', 'PrecSiteFile'.lower())
    MeteoDailyFile = CLIMATE_DATA_DIR + os.sep + cf.get('CLIMATE', 'MeteoDailyFile'.lower())
    PrecDailyFile = CLIMATE_DATA_DIR + os.sep + cf.get('CLIMATE', 'PrecDailyFile'.lower())
    thiessenIdField = cf.get('CLIMATE', 'thiessenIdField'.lower())
else:
    raise ValueError("Climate input file names MUST be provided in [CLIMATE]!")

# 5. Spatial Input
if 'SPATIAL' in cf.sections():
    PrecSitesThiessen = SPATIAL_DATA_DIR + os.sep + cf.get('SPATIAL', 'PrecSitesThiessen'.lower())
    MeteorSitesThiessen = SPATIAL_DATA_DIR + os.sep + cf.get('SPATIAL', 'MeteorSitesThiessen'.lower())
    dem = SPATIAL_DATA_DIR + os.sep + cf.get('SPATIAL', 'dem'.lower())
    outlet_file = SPATIAL_DATA_DIR + os.sep + cf.get('SPATIAL', 'outlet_file'.lower())
    if not os.path.exists(outlet_file):
        outlet_file = None
    landuseOriginFile = SPATIAL_DATA_DIR + os.sep + cf.get('SPATIAL', 'landuseFile'.lower())
    landcoverInitFile = SPATIAL_DATA_DIR + os.sep + cf.get('SPATIAL', 'landcoverInitFile'.lower())
    soilSEQNFile = SPATIAL_DATA_DIR + os.sep + cf.get('SPATIAL', 'soilSEQNFile'.lower())
    soilSEQNText = SPATIAL_DATA_DIR + os.sep + cf.get('SPATIAL', 'soilSEQNText'.lower())
    mgtFieldFile = SPATIAL_DATA_DIR + os.sep + cf.get('SPATIAL', 'mgtFieldFile'.lower())
    if not os.path.exists(mgtFieldFile) or StringMatch(mgtFieldFile, 'none'):
        mgtFieldFile = None
else:
    raise ValueError("Spatial input file names MUST be provided in [SPATIAL]!")

# 6. Option parameters
isTauDEM = True
D8AccThreshold = 0
np = 4
D8DownMethod = 'Surface'
dorm_hr = -1.
T_base = 0.
imperviousPercInUrbanCell = 0.3
default_reach_depth = 5.
defaultLanduse = 8
if 'SPATIAL' in cf.sections():
    isTauDEM = cf.getboolean('OPTIONAL_PARAMETERS', 'isTauDEMD8'.lower())
    D8AccThreshold = cf.getfloat(
        'OPTIONAL_PARAMETERS', 'D8AccThreshold'.lower())
    np = cf.getint('OPTIONAL_PARAMETERS', 'np')
    D8DownMethod = cf.get('OPTIONAL_PARAMETERS', 'D8DownMethod'.lower())
    dorm_hr = cf.getfloat('OPTIONAL_PARAMETERS', 'dorm_hr'.lower())
    T_base = cf.getfloat('OPTIONAL_PARAMETERS', 'T_base'.lower())
    imperviousPercInUrbanCell = cf.getfloat(
        'OPTIONAL_PARAMETERS', 'imperviousPercInUrbanCell'.lower())
    default_reach_depth = cf.getfloat(
        'OPTIONAL_PARAMETERS', 'default_reach_depth'.lower())
    defaultLanduse = cf.getint('OPTIONAL_PARAMETERS', 'defaultLanduse'.lower())
