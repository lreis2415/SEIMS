"""Constant strings used in SEIMS, both in preprocessing and SEIMS modules (C++)

    @author   : Liangjun Zhu

    @changelog:
    - 16-12-07  - lj - rewrite for version 2.0
    - 17-06-23  - lj - reorganize as basic class other than Global variables
    - 18-02-08  - lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals

from os import sep as SEP


# This is intended to be deprecated, since one database is desirable
#   for both storm and daily models. By lj. 2018-8-23
# class ModelNameUtils(object):
#     """Simulation Model related tags"""
#     Model = 'model'
#     # Cluster = 'cluster'
#     Mode = 'MODE'
#     Storm = 'STORM'
#     Daily = 'DAILY'
#     StormClimateDBSuffix = 'storm'
#
#     @staticmethod
#     def standardize_climate_dbname(climatedbname):
#         """standardize climate database name"""
#         if climatedbname is not None:
#             climatedbname = climatedbname + '_' + ModelNameUtils.StormClimateDBSuffix
#         return climatedbname


class ModelCfgUtils(object):
    """Model configuration utility names"""
    _FILE_IN = 'file.in'
    _FILE_OUT = 'file.out'
    _FILE_CFG = 'config.fig'
    _FILE_CALI = 'param.cali'

    def __init__(self, model_dir):
        """assign model config file paths"""
        self.filein = model_dir + SEP + ModelCfgUtils._FILE_IN
        self.fileout = model_dir + SEP + ModelCfgUtils._FILE_OUT
        self.filecfg = model_dir + SEP + ModelCfgUtils._FILE_CFG
        self.filecali = model_dir + SEP + ModelCfgUtils._FILE_CALI


class DirNameUtils(object):
    """Names for folders in output workspace for Spatial data preprocessing"""
    _Log = 'runtime_log'
    _TauDEM = 'taudem_delineated'
    _GeoData2DB = 'spatial_raster'
    _GeoShp = 'spatial_shp'
    _LayerInfo = 'layering_info'
    _Metis = 'metis_output'
    _Import2DB = 'imported_monogodb'
    _Lookup = 'lookup_tables'

    def __init__(self, pre_dir):
        """prepare output directories"""
        self.log = pre_dir + SEP + DirNameUtils._Log
        self.taudem = pre_dir + SEP + DirNameUtils._TauDEM
        self.geodata2db = pre_dir + SEP + DirNameUtils._GeoData2DB
        self.geoshp = pre_dir + SEP + DirNameUtils._GeoShp
        self.layerinfo = pre_dir + SEP + DirNameUtils._LayerInfo
        self.metis = pre_dir + SEP + DirNameUtils._Metis
        self.import2db = pre_dir + SEP + DirNameUtils._Import2DB
        self.lookup = pre_dir + SEP + DirNameUtils._Lookup


class ModelParamDataUtils(object):
    """Model parameters data file related.
    """
    _INIT_PARAM_NAME = 'model_param_ini.csv'
    _INIT_OUTPUTS_NAME = 'AvailableOutputs.csv'
    Tag_Params = 'param'
    Tag_Lookup = 'lookup'
    _LOOKUP_DICT = {'SoilLookup': 'SoilLookup.csv',
                    'LanduseLookup': 'LanduseLookup.csv',
                    'TillageLookup': 'TillageLookup.csv',
                    'UrbanLookup': 'UrbanLookup.csv',
                    'CropLookup': 'CropLookup.csv',
                    'FertilizerLookup': 'FertilizerLookup.csv'}

    # CROP, LANDUSE attributes(fields)
    # Match to the new lookup table of SWAT 2012 rev.637. lj
    crop_fields = ['IDC', 'BIO_E', 'HVSTI', 'BLAI', 'FRGRW1', 'LAIMX1', 'FRGRW2',
                   'LAIMX2', 'DLAI', 'CHTMX', 'RDMX', 'T_OPT', 'T_BASE', 'CNYLD',
                   'CPYLD', 'BN1', 'BN2', 'BN3', 'BP1', 'BP2', 'BP3', 'WSYF',
                   'USLE_C', 'GSI', 'VPDFR', 'FRGMAX', 'WAVP', 'CO2HI', 'BIOEHI',
                   'RSDCO_PL', 'OV_N', 'CN2A', 'CN2B', 'CN2C', 'CN2D', 'FERTFIELD',
                   'ALAI_MIN', 'BIO_LEAF', 'MAT_YRS', 'BMX_TREES', 'EXT_COEF', 'BM_DIEOFF']
    landuse_fields = ['LANDUSE_ID', 'CN2A', 'CN2B', 'CN2C', 'CN2D', 'ROOTDEPTH', 'MANNING',
                      'INTERC_MAX', 'INTERC_MIN', 'SHC', 'SOIL_T10',
                      'PET_FR', 'PRC_ST1', 'PRC_ST2', 'PRC_ST3', 'PRC_ST4',
                      'PRC_ST5', 'PRC_ST6', 'PRC_ST7', 'PRC_ST8', 'PRC_ST9',
                      'PRC_ST10', 'PRC_ST11', 'PRC_ST12', 'SC_ST1', 'SC_ST2',
                      'SC_ST3', 'SC_ST4', 'SC_ST5', 'SC_ST6', 'SC_ST7', 'SC_ST8',
                      'SC_ST9', 'SC_ST10', 'SC_ST11', 'SC_ST12', 'DSC_ST1', 'DSC_ST2',
                      'DSC_ST3', 'DSC_ST4', 'DSC_ST5', 'DSC_ST6', 'DSC_ST7', 'DSC_ST8',
                      'DSC_ST9', 'DSC_ST10', 'DSC_ST11', 'DSC_ST12']
    # Metadata field names for lookup gridfs
    item_count = 'ITEM_COUNT'
    field_count = 'FIELD_COUNT'

    def __init__(self, in_dir):
        """assign text file path"""
        self.crop_file = in_dir + SEP + ModelParamDataUtils._LOOKUP_DICT.get('CropLookup')
        self.init_params_file = in_dir + SEP + ModelParamDataUtils._INIT_PARAM_NAME
        self.init_outputs_file = in_dir + SEP + ModelParamDataUtils._INIT_OUTPUTS_NAME
        self.lookup_tabs_dict = dict()
        for k, v in list(ModelParamDataUtils._LOOKUP_DICT.items()):
            self.lookup_tabs_dict[k] = in_dir + SEP + v


class DataType(object):
    """Climate datatype tags, MUST BE coincident with text.h in SEIMS.
    """
    p = 'P'  # DataType_Precipitation, 1, Suffix of precipitation data
    m = 'M'  # Meteorological
    mean_tmp = 'TMEAN'  # DataType_MeanTemperature, 2
    min_tmp = 'TMIN'  # DataType_MinimumTemperature, 3
    max_tmp = 'TMAX'  # DataType_MaximumTemperature, 4
    pet = 'PET'  # DataType_PotentialEvapotranspiration, 5
    sr = 'SR'  # DataType_SolarRadiation, 6
    ws = 'WS'  # DataType_WindSpeed, 7
    rm = 'RM'  # DataType_RelativeAirMoisture, 8
    ssd = 'SSD'  # DataType_SunDurationHour, 9
    phu_tot = 'PHUTOT'  # DataType_YearlyHeatUnit
    phu0 = 'PHU0'  # Datatype_PHU0
    mean_tmp0 = 'TMEAN0'  # DataType_MeanTemperature0, multiply annuals


class ModelParamFields(object):
    """Model parameters fields."""
    # field names in parameter table of MongoDB
    name = 'NAME'
    desc = 'DESCRIPTION'
    unit = 'UNIT'
    module = 'MODULE'
    value = 'VALUE'
    impact = 'IMPACT'
    change = 'CHANGE'
    max = 'MAX'
    min = 'MIN'
    type = 'TYPE'
    # available values
    change_vc = 'VC'
    change_rc = 'RC'
    change_ac = 'AC'
    change_nc = 'NC'


class ModelCfgFields(object):
    """Model configuration fields.
        field in Model Configuration Collections, FILE_IN and FILE_OUT
    """
    tag = 'TAG'
    value = 'VALUE'
    mod_cls = 'MODULE_CLASS'
    output_id = 'OUTPUTID'
    desc = 'DESCRIPTION'
    unit = 'UNIT'
    type = 'TYPE'
    stime = 'STARTTIME'
    etime = 'ENDTIME'
    interval = 'INTERVAL'
    interval_unit = 'INTERVAL_UNIT'
    subbsn = 'SUBBASIN'
    filename = 'FILENAME'
    use = 'USE'


class SubbsnStatsName(object):
    """Variable name of subbasin statistics"""
    outlet = 'OUTLET_ID'
    o_row = 'OUTLET_ROW'
    o_col = 'OUTLET_COL'
    subbsn_max = 'SUBBASINID_MAX'
    subbsn_min = 'SUBBASINID_MIN'
    subbsn_num = 'SUBBASINID_NUM'


class DataValueFields(object):
    """DATA_VALUES collection"""
    id = 'STATIONID'
    dt = 'DATETIME'
    y = 'Y'
    m = 'M'
    d = 'D'
    hour = 'HH'
    minute = 'MM'
    second = 'SS'
    type = 'TYPE'
    local_time = 'LOCALDATETIME'
    time_zone = 'UTCOFFSET'
    utc = 'UTCDATETIME'
    value = 'VALUE'


class VariableDesc(object):
    """Variable description"""
    id = 'ID'
    type = 'TYPE'
    unit = 'UNIT'
    is_regular = 'ISREGULAR'
    time = 'TIMESUPPORT'


class StationFields(object):
    """Hydro-climate station sites."""
    id = 'STATIONID'
    subbsn = 'SUBBASINID'
    name = 'NAME'
    x = 'LOCALX'
    y = 'LOCALY'
    prj = 'LOCALPRJID'
    lon = 'LON'
    lat = 'LAT'
    datum = 'DATUMID'
    elev = 'ELEVATION'
    type = 'TYPE'
    outlet = 'ISOUTLET'
    unit = 'UNIT'


class FieldNames(object):
    """Field name used in MongoDB, Shapefile, etc."""
    # Field in SiteList table or other tables, such as subbasin.shp
    subbasin_id = 'SUBBASINID'
    basin = 'BASIN'
    mode = 'MODE'
    db = 'DB'
    # sitelist table of main database
    site_p = 'SITELISTP'
    site_m = 'SITELISTM'
    site_pet = 'SITELISTPET'


class SpatialNamesUtils(object):
    """predefined raster file names which are ready for importing to database"""
    _MASK_TO_EXT = 'mask.tif'
    # output to mongoDB file names
    _SUBBASINOUT = 'subbasin.tif'
    _FLOWDIROUT = 'flow_dir.tif'
    _STREAMLINKOUT = 'stream_link.tif'
    _HILLSLOPEOUT = 'hillslope.tif'
    # masked and output to mongoDB file names
    _SLOPEM = 'slope.tif'
    _FILLEDDEMM = 'dem.tif'
    _ACCM = 'acc.tif'
    _STREAMORDERM = 'stream_order.tif'
    _FLOWDIRDINFM = 'flow_dir_angle_dinf.tif'
    _DIRCODEDINFM = 'flow_dir_dinf.tif'
    _WEIGHTDINFM = 'weight_dinf.tif'
    _SLOPEDINFM = 'slope_dinf.tif'
    _CELLLAT = 'celllat.tif'
    _DAYLMIN = 'dayLenMin.tif'
    _DORMHR = 'dormhr.tif'
    _DIST2STREAMD8M = 'dist2Stream.tif'
    _CHWIDTH = 'ch_width.tif'
    _CHDEPTH = 'ch_depth.tif'
    _LANDUSEMFILE = 'landuse.tif'
    _CROPMFILE = 'LANDCOVER.tif'  # added by LJ.
    _SOILTYPEMFILE = 'soiltype.tif'
    # _MGTFIELDMFILE = 'mgt_fields.tif'
    _SOILTEXTURE = 'SOIL_TEXTURE.tif'
    _HYDROGROUP = 'HYDRO_GROUP.tif'
    _USLEK = 'USLE_K.tif'
    _INITSOILMOIST = 'moist_in.tif'
    _DEPRESSIONFILE = 'depression.tif'
    _CN2FILE = 'CN2.tif'
    _RADIUSFILE = 'radius.tif'
    _MANNINGFILE = 'MANNING.tif'
    _VELOCITYFILE = 'velocity.tif'
    # flow time to the main river from each grid cell
    _T0_SFILE = 't0_s.tif'
    # standard deviation of t0_s
    _DELTA_SFILE = 'delta_s.tif'
    # potential runoff coefficient
    _RUNOFF_COEFFILE = 'runoff_co.tif'

    def __init__(self, spa_dir):
        """assign spatial data file paths"""
        self.mask = spa_dir + SEP + self._MASK_TO_EXT
        self.subbsn = spa_dir + SEP + self._SUBBASINOUT
        self.d8flow = spa_dir + SEP + self._FLOWDIROUT
        self.stream_link = spa_dir + SEP + self._STREAMLINKOUT
        self.hillslope = spa_dir + SEP + self._HILLSLOPEOUT
        self.slope = spa_dir + SEP + self._SLOPEM
        self.filldem = spa_dir + SEP + self._FILLEDDEMM
        self.d8acc = spa_dir + SEP + self._ACCM
        self.stream_order = spa_dir + SEP + self._STREAMORDERM
        self.dinf = spa_dir + SEP + self._FLOWDIRDINFM
        self.dinf_d8dir = spa_dir + SEP + self._DIRCODEDINFM
        self.dinf_weight = spa_dir + SEP + self._WEIGHTDINFM
        self.dinf_slp = spa_dir + SEP + self._SLOPEDINFM
        self.cell_lat = spa_dir + SEP + self._CELLLAT
        self.dayl_min = spa_dir + SEP + self._DAYLMIN
        self.dorm_hr = spa_dir + SEP + self._DORMHR
        self.dist2stream_d8 = spa_dir + SEP + self._DIST2STREAMD8M
        self.chwidth = spa_dir + SEP + self._CHWIDTH
        self.chdepth = spa_dir + SEP + self._CHDEPTH
        self.landuse = spa_dir + SEP + self._LANDUSEMFILE
        self.crop = spa_dir + SEP + self._CROPMFILE
        self.soil_type = spa_dir + SEP + self._SOILTYPEMFILE
        # self.mgt_field = spa_dir + SEP + self._MGTFIELDMFILE
        self.soil_texture = spa_dir + SEP + self._SOILTEXTURE
        self.hydro_group = spa_dir + SEP + self._HYDROGROUP
        self.usle_k = spa_dir + SEP + self._USLEK
        self.init_somo = spa_dir + SEP + self._INITSOILMOIST
        self.depression = spa_dir + SEP + self._DEPRESSIONFILE
        self.cn2 = spa_dir + SEP + self._CN2FILE
        self.radius = spa_dir + SEP + self._RADIUSFILE
        self.manning = spa_dir + SEP + self._MANNINGFILE
        self.velocity = spa_dir + SEP + self._VELOCITYFILE
        self.t0_s = spa_dir + SEP + self._T0_SFILE
        self.delta_s = spa_dir + SEP + self._DELTA_SFILE
        self.runoff_coef = spa_dir + SEP + self._RUNOFF_COEFFILE


class VectorNameUtils(object):
    """predefined vector(shp and geojson) file names"""
    # GeoJson format files (WGS84 coordinate)
    _GEOJSON_REACH = 'river.json'
    _GEOJSON_SUBBSN = 'subbasin.json'
    _GEOJSON_BSN = 'basin.json'
    _GEOJSON_OUTLET = 'outlet.json'
    _SHP_OUTLET = 'outlet.shp'
    _SHP_SUBBSN = 'subbasin.shp'
    _SHP_BSN = 'basin.shp'
    _SHP_REACH = 'reach.shp'

    def __init__(self, shp_dir):
        """assign vector files path"""
        self.json_reach = shp_dir + SEP + VectorNameUtils._GEOJSON_REACH
        self.json_subbsn = shp_dir + SEP + VectorNameUtils._GEOJSON_SUBBSN
        self.json_bsn = shp_dir + SEP + VectorNameUtils._GEOJSON_BSN
        self.json_outlet = shp_dir + SEP + VectorNameUtils._GEOJSON_OUTLET
        self.reach = shp_dir + SEP + VectorNameUtils._SHP_REACH
        self.subbsn = shp_dir + SEP + VectorNameUtils._SHP_SUBBSN
        self.bsn = shp_dir + SEP + VectorNameUtils._SHP_BSN
        self.outlet = shp_dir + SEP + VectorNameUtils._SHP_OUTLET


class LogNameUtils(object):
    """predefined log file names"""
    _STATUS_DELINEATION = 'status_SubbasinDelineation.txt'
    _STATUS_EXTRACTSOILPARAM = 'status_ExtractSoilParameters.txt'
    _STATUS_EXTRACTLANDUSEPARAM = 'status_ExtractLanduseParameters.txt'
    _STATUS_EXTRACTTERRAINPARAM = 'status_ExtractTerrainParameters.txt'
    _STATUS_MONGO = 'status_BuildMongoDB.txt'
    _CONFIG_MASKRASTERS = 'config_maskDelineatedRaster.txt'
    _CONFIG_RECLASSIFYLU = 'config_reclassLanduseConfig.txt'

    def __init__(self, log_dir):
        """assign log file path"""
        self.delineation = log_dir + SEP + LogNameUtils._STATUS_DELINEATION
        self.extract_soil = log_dir + SEP + LogNameUtils._STATUS_EXTRACTSOILPARAM
        self.extract_lu = log_dir + SEP + LogNameUtils._STATUS_EXTRACTLANDUSEPARAM
        self.extract_terrain = log_dir + SEP + LogNameUtils._STATUS_EXTRACTTERRAINPARAM
        self.build_mongo = log_dir + SEP + LogNameUtils._STATUS_MONGO
        self.mask_cfg = log_dir + SEP + LogNameUtils._CONFIG_MASKRASTERS
        self.reclasslu_cfg = log_dir + SEP + LogNameUtils._CONFIG_RECLASSIFYLU


class RasterMetadata(object):
    """Header information of raster data (Derived from Mask.tif)"""
    tab_name = 'Header'
    nodata = 'NODATA_VALUE'
    xll = 'XLLCENTER'
    yll = 'YLLCENTER'
    nrows = 'NROWS'
    ncols = 'NCOLS'
    cellsize = 'CELLSIZE'
    subbasin = 'SUBBASIN'
    cellnum = 'NUM_CELLS'
    # for weight data
    site_num = 'NUM_SITES'
    srs = 'SRS'


class DBTableNames(object):
    """Predefined MongoDB database collection names."""
    # Main model database
    gridfs_spatial = 'SPATIAL'
    gridfs_output = 'OUTPUT'
    main_sitelist = 'SITELIST'
    main_parameter = 'PARAMETERS'
    main_filein = 'FILE_IN'
    main_fileout = 'FILE_OUT'
    main_scenario = 'BMPDATABASE'
    # hydro-climate database
    data_values = 'DATA_VALUES'
    annual_stats = 'ANNUAL_STATS'
    observes = 'MEASUREMENT'
    var_desc = 'VARIABLES'
    sites = 'SITES'
    # BMPs scenario database
    scenarios = 'BMP_SCENARIOS'
