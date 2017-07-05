#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Constant strings used in SEIMS, both in preprocessing and SEIMS modules (C++)
    @author   : Liangjun Zhu
    @changelog: 16-12-07  lj - rewrite for version 2.0
                17-06-23  lj - reorganize as basic class other than Global variables
"""
import os


class ModelNameUtils(object):
    """Simulation Model related tags"""
    Model = "model"
    Cluster = "cluster"
    Mode = "MODE"
    Storm = "STORM"
    Daily = "DAILY"
    StormClimateDBSuffix = "storm_cmorph"

    @staticmethod
    def standardize_spatial_dbname(is4cluster, is4storm, spatialdbname):
        """standardize spatial database name"""
        if is4cluster and ModelNameUtils.Cluster not in spatialdbname.lower():
            spatialdbname = ModelNameUtils.Cluster + "_" + spatialdbname
        if is4storm:
            if not ModelNameUtils.Storm.lower() in spatialdbname.lower():
                spatialdbname = spatialdbname + "_" + ModelNameUtils.Storm.lower()
        if not ModelNameUtils.Model in spatialdbname.lower():
            spatialdbname = ModelNameUtils.Model + "_" + spatialdbname
        if is4cluster and (not ModelNameUtils.Cluster.lower() in spatialdbname.lower()):
            spatialdbname = ModelNameUtils.Cluster.lower() + "_" + spatialdbname
        return spatialdbname

    @staticmethod
    def standardize_climate_dbname(climatedbname):
        """standardize climate database name"""
        if climatedbname is not None:
            climatedbname = climatedbname + "_" + ModelNameUtils.StormClimateDBSuffix
        return climatedbname


class ModelCfgUtils(object):
    """Model configuration utility names"""
    _FILE_IN = "file.in"
    _FILE_OUT = "file.out"
    _FILE_CFG = "config.fig"

    def __init__(self, model_dir):
        """assign model config file paths"""
        self.filein = model_dir + os.sep + ModelCfgUtils._FILE_IN
        self.fileout = model_dir + os.sep + ModelCfgUtils._FILE_OUT
        self.filecfg = model_dir + os.sep + ModelCfgUtils._FILE_CFG


class DirNameUtils(object):
    """Names for folders in output workspace for Spatial data preprocessing"""
    _Log = "runtime_log"
    _TauDEM = "taudem_delineated"
    _GeoData2DB = "spatial_raster"
    _GeoShp = "spatial_shp"
    _LayerInfo = "layering_info"
    _Metis = "metis_output"
    _Import2DB = "imported_monogodb"
    _Lookup = "lookup_tables"

    def __init__(self, pre_dir):
        """prepare output directories"""
        self.log = pre_dir + os.sep + DirNameUtils._Log
        self.taudem = pre_dir + os.sep + DirNameUtils._TauDEM
        self.geodata2db = pre_dir + os.sep + DirNameUtils._GeoData2DB
        self.geoshp = pre_dir + os.sep + DirNameUtils._GeoShp
        self.layerinfo = pre_dir + os.sep + DirNameUtils._LayerInfo
        self.metis = pre_dir + os.sep + DirNameUtils._Metis
        self.import2db = pre_dir + os.sep + DirNameUtils._Import2DB
        self.lookup = pre_dir + os.sep + DirNameUtils._Lookup


class SQLiteParaUtils(object):
    """Regenerated SQLite database of Parameters for SEIMS
    TODO: The intermediate SQLite db should be deprecated in the future version.
    """
    Tag_Params = "param"
    Tag_Lookup = "lookup"
    init_params = 'model_param_ini'
    lookup_tabs = ['SoilLookup', 'LanduseLookup', 'TillageLookup',
                   'UrbanLookup', 'CropLookup', 'FertilizerLookup']
    _CROP_FILE = 'CropLookup.txt'
    _SQLITE_FILE = "Parameter.db3"

    def __init__(self, in_dir, out_dir):
        """assign text file path and output sqlite db file path"""
        self.crop_file = in_dir + os.sep + SQLiteParaUtils._CROP_FILE
        self.sqlite_file = out_dir + os.sep + SQLiteParaUtils._SQLITE_FILE
        self.init_params_file = in_dir + os.sep + SQLiteParaUtils.init_params + '.txt'


class DataType(object):
    """Climate datatype tags, MUST BE coincident with text.h in SEIMS.
    """
    p = "P"  # DataType_Precipitation, 1, Suffix of precipitation data
    m = "M"  # Meterological
    mean_tmp = "TMEAN"  # DataType_MeanTemperature, 2
    min_tmp = "TMIN"  # DataType_MinimumTemperature, 3
    max_tmp = "TMAX"  # DataType_MaximumTemperature, 4
    pet = "PET"  # DataType_PotentialEvapotranspiration, 5
    sr = "SR"  # DataType_SolarRadiation, 6
    ws = "WS"  # DataType_WindSpeed, 7
    rm = "RM"  # DataType_RelativeAirMoisture, 8
    ssd = "SSD"  # DataType_SunDurationHour, 9
    phu_tot = "PHUTOT"  # DataType_YearlyHeatUnit
    phu0 = "PHU0"  # Datatype_PHU0
    mean_tmp0 = "TMEAN0"  # DataType_MeanTemperature0, multiply annuals


class ModelParamFields(object):
    """Model parameters fields."""
    # field names in parameter table of MongoDB
    name = "NAME"
    desc = "DESCRIPTION"
    unit = "UNIT"
    module = "MODULE"
    value = "VALUE"
    impact = "IMPACT"
    change = "CHANGE"
    max = "MAX"
    min = "MIN"
    use = "USE"
    type = "TYPE"
    # available values
    change_rc = "RC"
    change_ac = "AC"
    change_nc = "NC"
    use_y = "Y"
    use_n = "N"


class ModelCfgFields(object):
    """Model configuration fields.
        field in Model Configuration Collections, FILE_IN and FILE_OUT
    """
    tag = "TAG"
    value = "VALUE"
    mod_cls = "MODULE_CLASS"
    output_id = "OUTPUTID"
    desc = "DESCRIPTION"
    unit = "UNIT"
    type = "TYPE"
    stime = "STARTTIME"
    etime = "ENDTIME"
    interval = "INTERVAL"
    interval_unit = "INTERVAL_UNIT"
    subbsn = "SUBBASIN"
    filename = "FILENAME"
    use = "USE"


class SubbsnStatsName(object):
    """Variable name of subbasin statistics"""
    outlet = "OUTLET_ID"
    o_row = "OUTLET_ROW"
    o_col = "OUTLET_COL"
    subbsn_max = "SUBBASINID_MAX"
    subbsn_min = "SUBBASINID_MIN"
    subbsn_num = "SUBBASINID_NUM"


class DataValueFields(object):
    """DATA_VALUES collection"""
    id = 'STATIONID'
    y = 'Y'
    m = 'M'
    d = 'D'
    type = 'TYPE'
    local_time = 'LOCALDATETIME'
    time_zone = 'UTCOFFSET'
    utc = 'UTCDATETIME'
    value = 'VALUE'


class VariableDesc(object):
    """Variable description"""
    id = "ID"
    type = "TYPE"
    unit = "UNIT"
    is_regular = "ISREGULAR"
    time = "TIMESUPPORT"


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


class TauDEMFilesUtils(object):
    """predefined TauDEM resulted file names"""
    # intermediate data
    _FILLEDDEM = "demFilledTau.tif"
    _D8FLOWDIR = "flowDirTauD8.tif"
    _SLOPE = "slopeTau.tif"
    _D8ACC = "accTauD8.tif"
    _D8ACCWITHWEIGHT = "accTauD8WithWeight.tif"
    _STREAMRASTER = "streamRasterTau.tif"
    _FLOWDIRDINF = "flowDirDinfTau.tif"
    _DIRCODEDINF = "dirCodeDinfTau.tif"
    _WEIGHTDINF = "weightDinfTau.tif"
    _SLOPEDINF = "slopeDinfTau.tif"
    _MODIFIEDOUTLET = "outletM.shp"
    _STREAMSKELETON = "streamSkeleton.tif"
    _STREAMORDER = "streamOrderTau.tif"
    _CHNETWORK = "chNetwork.txt"
    _CHCOORD = "chCoord.txt"
    _STREAMNET = "streamNet.shp"
    _DIST2STREAMD8 = "dist2StreamD8Org.tif"
    _SUBBASIN = "subbasinTau.tif"
    # masked file names
    _SUBBASINM = "subbasinTauM.tif"
    _D8FLOWDIRM = "flowDirTauM.tif"
    _STREAMRASTERM = "streamRasterTauM.tif"

    def __init__(self, tau_dir):
        """assign taudem resulted file path"""
        self.filldem = tau_dir + os.sep + self._FILLEDDEM
        self.d8flow = tau_dir + os.sep + self._D8FLOWDIR
        self.slp = tau_dir + os.sep + self._SLOPE
        self.d8acc = tau_dir + os.sep + self._D8ACC
        self.d8acc_weight = tau_dir + os.sep + self._D8ACCWITHWEIGHT
        self.stream_raster = tau_dir + os.sep + self._STREAMRASTER
        self.dinf = tau_dir + os.sep + self._FLOWDIRDINF
        self.dinf_d8dir = tau_dir + os.sep + self._DIRCODEDINF
        self.dinf_weight = tau_dir + os.sep + self._WEIGHTDINF
        self.dinf_slp = tau_dir + os.sep + self._SLOPEDINF
        self.outlet_m = tau_dir + os.sep + self._MODIFIEDOUTLET
        self.stream_pd = tau_dir + os.sep + self._STREAMSKELETON
        self.stream_order = tau_dir + os.sep + self._STREAMORDER
        self.channel_net = tau_dir + os.sep + self._CHNETWORK
        self.channel_coord = tau_dir + os.sep + self._CHCOORD
        self.streamnet_shp = tau_dir + os.sep + self._STREAMNET
        self.dist2stream_d8 = tau_dir + os.sep + self._DIST2STREAMD8
        self.subbsn = tau_dir + os.sep + self._SUBBASIN
        self.subbsn_m = tau_dir + os.sep + self._SUBBASINM
        self.d8flow_m = tau_dir + os.sep + self._D8FLOWDIRM
        self.stream_m = tau_dir + os.sep + self._STREAMRASTERM


class SpatialNamesUtils(object):
    """predefined raster file names which are ready for importing to database"""
    _MASK_TO_EXT = "mask.tif"
    # output to mongoDB file names
    _SUBBASINOUT = "subbasin.tif"
    _FLOWDIROUT = "flow_dir.tif"
    _STREAMLINKOUT = "stream_link.tif"
    _HILLSLOPEOUT = "hillslope.tif"
    # masked and output to mongoDB file names
    _SLOPEM = "slope.tif"
    _FILLEDDEMM = "dem.tif"
    _ACCM = "acc.tif"
    _STREAMORDERM = "stream_order.tif"
    _FLOWDIRDINFM = "flow_dir_angle_dinf.tif"
    _DIRCODEDINFM = "flow_dir_dinf.tif"
    _WEIGHTDINFM = "weight_dinf.tif"
    _SLOPEDINFM = "slope_dinf.tif"
    _CELLLAT = "celllat.tif"
    _DAYLMIN = "dayLenMin.tif"
    _DORMHR = "dormhr.tif"
    _DIST2STREAMD8M = "dist2Stream.tif"
    _CHWIDTH = "chwidth.tif"
    _LANDUSEMFILE = "landuse.tif"
    _CROPMFILE = "LANDCOVER.tif"  # added by LJ.
    _SOILTYPEMFILE = "soiltype.tif"
    _MGTFIELDMFILE = "mgt_fields.tif"
    _SOILTEXTURE = "SOIL_TEXTURE.tif"
    _HYDROGROUP = "HYDRO_GROUP.tif"
    _USLEK = "USLE_K.tif"
    _INITSOILMOIST = "moist_in.tif"
    _DEPRESSIONFILE = "depression.tif"
    _CN2FILE = "CN2.tif"
    _RADIUSFILE = "radius.tif"
    _MANNINGFILE = "MANNING.tif"
    _VELOCITYFILE = "velocity.tif"
    # flow time to the main river from each grid cell
    _T0_SFILE = "t0_s.tif"
    # standard deviation of t0_s
    _DELTA_SFILE = "delta_s.tif"
    # potential runoff coefficient
    _RUNOFF_COEFFILE = "runoff_co.tif"

    def __init__(self, spa_dir):
        """assign spatial data file paths"""
        self.mask = spa_dir + os.sep + self._MASK_TO_EXT
        self.subbsn = spa_dir + os.sep + self._SUBBASINOUT
        self.d8flow = spa_dir + os.sep + self._FLOWDIROUT
        self.stream_link = spa_dir + os.sep + self._STREAMLINKOUT
        self.hillslope = spa_dir + os.sep + self._HILLSLOPEOUT
        self.slope = spa_dir + os.sep + self._SLOPEM
        self.filldem = spa_dir + os.sep + self._FILLEDDEMM
        self.d8acc = spa_dir + os.sep + self._ACCM
        self.stream_order = spa_dir + os.sep + self._STREAMORDERM
        self.dinf = spa_dir + os.sep + self._FLOWDIRDINFM
        self.dinf_d8dir = spa_dir + os.sep + self._DIRCODEDINFM
        self.dinf_weight = spa_dir + os.sep + self._WEIGHTDINFM
        self.dinf_slp = spa_dir + os.sep + self._SLOPEDINFM
        self.cell_lat = spa_dir + os.sep + self._CELLLAT
        self.dayl_min = spa_dir + os.sep + self._DAYLMIN
        self.dorm_hr = spa_dir + os.sep + self._DORMHR
        self.dist2stream_d8 = spa_dir + os.sep + self._DIST2STREAMD8M
        self.chwidth = spa_dir + os.sep + self._CHWIDTH
        self.landuse = spa_dir + os.sep + self._LANDUSEMFILE
        self.crop = spa_dir + os.sep + self._CROPMFILE
        self.soil_type = spa_dir + os.sep + self._SOILTYPEMFILE
        self.mgt_field = spa_dir + os.sep + self._MGTFIELDMFILE
        self.soil_texture = spa_dir + os.sep + self._SOILTEXTURE
        self.hydro_group = spa_dir + os.sep + self._HYDROGROUP
        self.usle_k = spa_dir + os.sep + self._USLEK
        self.init_somo = spa_dir + os.sep + self._INITSOILMOIST
        self.depression = spa_dir + os.sep + self._DEPRESSIONFILE
        self.cn2 = spa_dir + os.sep + self._CN2FILE
        self.radius = spa_dir + os.sep + self._RADIUSFILE
        self.manning = spa_dir + os.sep + self._MANNINGFILE
        self.velocity = spa_dir + os.sep + self._VELOCITYFILE
        self.t0_s = spa_dir + os.sep + self._T0_SFILE
        self.delta_s = spa_dir + os.sep + self._DELTA_SFILE
        self.runoff_coef = spa_dir + os.sep + self._RUNOFF_COEFFILE


class VectorNameUtils(object):
    """predefined vector(shp and geojson) file names"""
    # GeoJson format files (WGS84 coordinate)
    _GEOJSON_REACH = "river.json"
    _GEOJSON_SUBBSN = "subbasin.json"
    _GEOJSON_BSN = "basin.json"
    _GEOJSON_OUTLET = "outlet.json"
    _SHP_OUTLET = "outlet.shp"
    _SHP_SUBBSN = "subbasin.shp"
    _SHP_BSN = "basin.shp"
    _SHP_REACH = "reach.shp"

    def __init__(self, shp_dir):
        """assign vector files path"""
        self.json_reach = shp_dir + os.sep + VectorNameUtils._GEOJSON_REACH
        self.json_subbsn = shp_dir + os.sep + VectorNameUtils._GEOJSON_SUBBSN
        self.json_bsn = shp_dir + os.sep + VectorNameUtils._GEOJSON_BSN
        self.json_outlet = shp_dir + os.sep + VectorNameUtils._GEOJSON_OUTLET
        self.reach = shp_dir + os.sep + VectorNameUtils._SHP_REACH
        self.subbsn = shp_dir + os.sep + VectorNameUtils._SHP_SUBBSN
        self.bsn = shp_dir + os.sep + VectorNameUtils._SHP_BSN
        self.outlet = shp_dir + os.sep + VectorNameUtils._SHP_OUTLET


class LogNameUtils(object):
    """predefined log file names"""
    _STATUS_DELINEATION = "status_SubbasinDelineation.txt"
    _STATUS_EXTRACTSOILPARAM = "status_ExtractSoilParameters.txt"
    _STATUS_EXTRACTLANDUSEPARAM = "status_ExtractLanduseParameters.txt"
    _STATUS_EXTRACTTERRAINPARAM = "status_ExtractTerrainParameters.txt"
    _STATUS_MONGO = "status_BuildMongoDB.txt"
    _CONFIG_MASKRASTERS = "config_maskDelineatedRaster.txt"
    _CONFIG_RECLASSIFYLU = "config_reclassLanduseConfig.txt"

    def __init__(self, log_dir):
        """assign log file path"""
        self.delineation = log_dir + os.sep + LogNameUtils._STATUS_DELINEATION
        self.extract_soil = log_dir + os.sep + LogNameUtils._STATUS_EXTRACTSOILPARAM
        self.extract_lu = log_dir + os.sep + LogNameUtils._STATUS_EXTRACTLANDUSEPARAM
        self.extract_terrain = log_dir + os.sep + LogNameUtils._STATUS_EXTRACTTERRAINPARAM
        self.build_mongo = log_dir + os.sep + LogNameUtils._STATUS_MONGO
        self.mask_cfg = log_dir + os.sep + LogNameUtils._CONFIG_MASKRASTERS
        self.reclasslu_cfg = log_dir + os.sep + LogNameUtils._CONFIG_RECLASSIFYLU


class RasterMetadata(object):
    """Header information of raster data (Derived from Mask.tif)"""
    tab_name = "Header"
    nodata = "NODATA_VALUE"
    xll = "XLLCENTER"
    yll = "YLLCENTER"
    nrows = "NROWS"
    ncols = "NCOLS"
    cellsize = "CELLSIZE"
    subbasin = "SUBBASIN"
    cellnum = "NUM_CELLS"
    # for weight data
    site_num = "NUM_SITES"


class DBTableNames(object):
    """Predefined MongoDB database collection names."""
    # Main model database
    gridfs_spatial = "SPATIAL"
    main_sitelist = "SITELIST"
    main_parameter = "PARAMETERS"
    main_filein = "FILE_IN"
    main_fileout = "FILE_OUT"
    main_scenario = "BMPDATABASE"
    # hydro-climate database
    data_values = "DATA_VALUES"
    annual_stats = 'ANNUAL_STATS'
    observes = 'MEASUREMENT'
    var_desc = 'VARIABLES'
    sites = "SITES"
