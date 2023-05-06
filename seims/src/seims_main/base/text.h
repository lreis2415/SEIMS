/*!
 * \file text.h
 * \brief Predefined string constants used in the code
 *        BE CAUTION, constant value must be aligned by SPACE, not TAB!
 * \author Junzhi Liu, LiangJun Zhu, Huiran Gao, Tong Wu
 * \version 2.0
 * \date Jun.2010, Apr.2016, Apr.2018, Jul. 2021
 *
 * Changelog:
 *   - 1. 2021-03-13 - lj - Instead of using a macro to store a constant, use a const variable.
 *
 */
#ifndef SEIMS_TEXT_H
#define SEIMS_TEXT_H

#ifndef CONST_CHARS
#define CONST_CHARS static const char* ///< const string
#endif
#ifndef CONST_CHARS_LIST
#define CONST_CHARS_LIST static const char* const ///< list of const strings
#endif

CONST_CHARS MODEL_NAME =                      "SEIMS";
CONST_CHARS MODEL_FULLNAME =                  "Spatially Explicit Integrated Modeling System";
CONST_CHARS MODEL_VERSION =                   "2021";
CONST_CHARS SEIMS_EMAIL =                     "zlj@lreis.ac.cn";
CONST_CHARS SEIMS_SITE =                      "https://github.com/lreis2415/SEIMS";

// Constant input variables. But do these tags actually used?
CONST_CHARS CONS_IN_ELEV =                          "Elevation";
CONST_CHARS CONS_IN_LAT =                           "Latitude";
CONST_CHARS CONS_IN_XPR =                           "xpr"; // ?
CONST_CHARS CONS_IN_YPR =                           "ypr"; // ?

// Climate data type, used as suffix
CONST_CHARS DataType_Precipitation =                "P";             ///< 1, m_pcp
CONST_CHARS DataType_MeanTemperature =              "TMEAN";         ///< 2, m_meanTemp
CONST_CHARS DataType_MinimumTemperature =           "TMIN";          ///< 3, m_minTemp
CONST_CHARS DataType_MaximumTemperature =           "TMAX";          ///< 4, m_maxTemp
CONST_CHARS DataType_PotentialEvapotranspiration =  "PET";           ///< 5, m_pet
CONST_CHARS DataType_SolarRadiation =               "SR";            ///< 6, m_sr
CONST_CHARS DataType_WindSpeed =                    "WS";            ///< 7, m_ws
CONST_CHARS DataType_RelativeAirMoisture =          "RM";            ///< 8, m_rhd
CONST_CHARS DataType_Meteorology =                  "M";             ///< 9

// Prefix tags of time series and spatial distributed data used in interploate module
CONST_CHARS DataType_Prefix_TS =                    "T";             ///< m_stationData
CONST_CHARS DataType_Prefix_DIS =                   "D";             ///< m_itpOutput

// Tags of climate related data
CONST_CHARS Tag_StationElevation =                  "StationElevation";        ///< m_hStations
CONST_CHARS Tag_Elevation_Meteorology =             "StationElevation_M";      ///< m_hStations
CONST_CHARS Tag_Elevation_PET =                     "StationElevation_PET";    ///< m_hStations
CONST_CHARS Tag_Elevation_Precipitation =           "StationElevation_P";      ///< m_hStations
CONST_CHARS Tag_Elevation_Temperature =             "StationElevation_T";      ///< m_hStations
CONST_CHARS Tag_Latitude_Meteorology =              "Latitude_M";              ///<
CONST_CHARS Tag_LapseRate =                         "LapseRate";               ///< m_lapseRate
CONST_CHARS_LIST Tag_VerticalInterpolation[] = {"VERTICALINTERPOLATION",
                                                "Execute vertical interpolation (1) or not (0),"
                                                "defined in config.fig"};      ///< m_itpVertical
CONST_CHARS_LIST Tag_Weight[] = {"WEIGHT", "Weight used for interpolation"};   ///< m_itpWeights
CONST_CHARS Tag_DataType =                          "DATATYPE";                ///< m_dataType

///////  define parameter calibration related string constants  ///////
CONST_CHARS PARAM_CHANGE_VC =                       "VC";  ///< replace by a value
CONST_CHARS PARAM_CHANGE_RC =                       "RC";  ///< multiply a ratio, which is diff from SWAT: * (1+ratio)
CONST_CHARS PARAM_CHANGE_AC =                       "AC";  ///< add a value
CONST_CHARS PARAM_CHANGE_NC =                       "NC";  ///< no change
CONST_CHARS PARAM_FLD_NAME =                        "NAME"; ///< unique name
CONST_CHARS PARAM_FLD_DESC =                        "DESCRIPTION"; ///< description
CONST_CHARS PARAM_FLD_UNIT =                        "UNIT"; ///< unit
CONST_CHARS PARAM_FLD_MIDS =                        "MODULE"; ///< associated module
CONST_CHARS PARAM_FLD_VALUE =                       "VALUE"; ///< actual parameter value
CONST_CHARS PARAM_FLD_IMPACT =                      "IMPACT"; ///< impact value for change
CONST_CHARS PARAM_FLD_CHANGE =                      "CHANGE"; ///< change type, used with IMPACT
CONST_CHARS PARAM_FLD_MAX =                         "MAX"; ///< maximum allowed actual VALUE
CONST_CHARS PARAM_FLD_MIN =                         "MIN"; ///< minimum allowed actual VALUE
CONST_CHARS PARAM_FLD_DTYPE =                       "DTYPE"; ///< data type of VALUE, can be INT or FLT
CONST_CHARS PARAM_FLD_USE =                         "USE"; ///< use or not
CONST_CHARS PARAM_USE_Y =                           "Y"; ///<
CONST_CHARS PARAM_USE_N =                           "N"; ///<
CONST_CHARS PARAM_CALI_VALUES =                     "CALI_VALUES"; ///< replace Impact for model calibration

////////////  Input and Output Tags   ///////////////
// Fields in Model Configuration Collections //
// Tags in file.in
CONST_CHARS Tag_ConfTag =                           "TAG";
CONST_CHARS Tag_ConfValue =                         "VALUE";
CONST_CHARS Tag_MODCLS =                            "MODULE_CLASS";
// Tags in file.out
CONST_CHARS Tag_OutputUSE =                         "USE";
CONST_CHARS Tag_OutputID =                          "OUTPUTID";
CONST_CHARS Tag_OutputDESC =                        "DESCRIPTION";
CONST_CHARS Tag_OutputUNIT =                        "UNIT";
CONST_CHARS Tag_StartTime =                         "STARTTIME";
CONST_CHARS Tag_EndTime =                           "ENDTIME";
CONST_CHARS Tag_FileName =                          "FILENAME";
CONST_CHARS Tag_AggType =                           "TYPE";
CONST_CHARS Tag_OutputSubbsn =                      "SUBBASIN";
CONST_CHARS Tag_Interval =                          "INTERVAL";
CONST_CHARS Tag_IntervalUnit =                      "INTERVAL_UNIT";

// Available values of Tag_outputSubbsn
CONST_CHARS Tag_AllSubbsn =                         "ALL";
CONST_CHARS Tag_Outlet =                            "OUTLET";

CONST_CHARS Tag_SiteCount =                         "SITECOUNT";
CONST_CHARS Tag_SiteName =                          "SITENAME";
CONST_CHARS Tag_SiteID =                            "SITEID";
CONST_CHARS Tag_ReachName =                         "REACHNAME";
CONST_CHARS Tag_Count =                             "COUNT";

//// Output data aggregation type //////
CONST_CHARS Tag_Unknown =                           "UNKNOWN";
CONST_CHARS Tag_Sum =                               "SUM";
CONST_CHARS Tag_Average =                           "AVE";
CONST_CHARS Tag_Minimum =                           "MIN";
CONST_CHARS Tag_Maximum =                           "MAX";
CONST_CHARS Tag_SpecificCells =                     "SPECIFIC";
CONST_CHARS Tag_TimeSeries =                        "TS";

CONST_CHARS TAG_OUT_OL_IUH =                        "OL_IUH";
//CONST_CHARS TAG_OUT_QOUTLET =                       "QOUTLET"; // currently not used, check if needed?
//CONST_CHARS TAG_OUT_QTOTAL =                        "QTotal";
//CONST_CHARS TAG_OUT_SEDOUTLET =                     "SEDOUTLET";
//CONST_CHARS Tag_DisPOutlet =                        "DissovePOutlet";
//CONST_CHARS Tag_AmmoOutlet =                        "AmmoniumOutlet";
//CONST_CHARS Tag_NitrOutlet =                        "NitrateOutlet";

//CONST_CHARS Tag_SubbasinCount =                     "SUBBASINCOUNT";
CONST_CHARS Tag_SubbasinId =                        "SUBBASINID"; ///< m_inputSubbsnID
//CONST_CHARS Tag_ReservoirCount =                    "RESERVOIRCOUNT";
//CONST_CHARS Tag_ReservoirId =                       "RESERVOIRID";
//CONST_CHARS_LIST Tag_SubbasinSelected[] = {"subbasinSelected", "The subbasion IDs listed in file.out"};
CONST_CHARS_LIST Tag_CellSize[] = {"CELLSIZE", "numble of valid cells, i.e., excluding NODATA"}; ///<
CONST_CHARS_LIST Tag_Mask[] = {"MASK", "MASK raster data indicating valid cells"}; ///<
CONST_CHARS_LIST Tag_TimeStep[] = {"TIMESTEP", "time step of simulation"}; ///< m_dt
CONST_CHARS_LIST Tag_HillSlopeTimeStep[] = {"DT_HS", "Time step of hillslope related processes"}; ///< m_dt
CONST_CHARS_LIST Tag_ChannelTimeStep[] = { "DT_CH", "Time step of channel routing related processes" }; ///< m_chdt
CONST_CHARS_LIST Tag_CellWidth[] = {"CELLWIDTH", "width of the cell"}; ///< m_cellWth

CONST_CHARS_LIST Tag_LayeringMethod[] = {"LayeringMethod", "Routing layering method"}; ///<
CONST_CHARS_LIST Tag_FlowDirectionMethod[] = { "FlowDirMethod", "Flow direction algorithm" }; ///<
CONST_CHARS_LIST Tag_FLOWIN_INDEX[] = { "FLOWIN_INDEX", "Indexes of flow in units" }; ///< m_flowInIdx
CONST_CHARS_LIST Tag_FLOWOUT_INDEX[] = { "FLOWOUT_INDEX", "Indexes of flow out units" }; ///< m_flowOutIdx
CONST_CHARS_LIST Tag_FLOWIN_FRACTION[] = { "FLOWIN_FRACTION", "Flow in fractions from upstream units" }; ///< m_flowInFrac
CONST_CHARS_LIST Tag_FLOWOUT_FRACTION[] = { "FLOWOUT_FRACTION", "Flow out fractions to downstream units" }; ///< m_flowOutFrac
CONST_CHARS_LIST Tag_ROUTING_LAYERS[] = {"ROUTING_LAYERS", "Routing layers according to flow directions, "
                                         "there are no flow relationships within each layer, and the first element in each layer "
                                         "is the number of compute units in current layer"}; ///< m_rteLyrs

// Reach parameters (Replaced Tag_ReachParameter and Tag_RchParam by VAR_REACH_PARAM)
CONST_CHARS_LIST VAR_REACH_PARAM[] = {"ReachParam", "Reach parameters such as stream order, "
                                      "manning's n and downstream subbasin id"}; ///<
// Add Subbasins as AddParameters for modules
CONST_CHARS_LIST VAR_SUBBASIN_PARAM[] = {"SubbasinParam", "Statistics of subbasin related parameters"}; ///<

// Files or database constant strings
CONST_CHARS TextExtension =                         "txt"; ///< plain text format
CONST_CHARS File_Config =                           "config.fig"; ///< modules list
CONST_CHARS File_Input =                            "file.in"; ///< simulation period, timestep, etc.
CONST_CHARS File_Output =                           "file.out"; ///< define output variables
CONST_CHARS Source_HydroClimateDB =                 "HydroClimateDB"; ///< hydro and climate database
CONST_CHARS Source_HydroClimateDB_Optional =        "HydroClimateDB_Optional"; ///< optional hydroclimate
CONST_CHARS Source_ParameterDB =                    "ParameterDB"; ///< model parameters database
CONST_CHARS Source_ParameterDB_Optional =           "ParameterDB_Optional"; ///< optional model parameter
CONST_CHARS Source_Module =                         "Module"; ///< inputs from other modules
CONST_CHARS Source_Module_Optional =                "Module_Optional"; ///< optional inputs

///////// Table Names required in MongoDB /////////
CONST_CHARS DB_TAB_FILE_IN =                           "FILE_IN"; ///< based on file.in
CONST_CHARS DB_TAB_FILE_OUT =                          "FILE_OUT"; ///< based on file.out
CONST_CHARS DB_TAB_PARAMETERS =                        "PARAMETERS"; ///< model parameters table
CONST_CHARS DB_TAB_SITELIST =                          "SITELIST"; ///< meteorology and precipitation sites
CONST_CHARS DB_TAB_SCENARIO =                          "BMPDATABASE"; ///< scenario database name
CONST_CHARS DB_TAB_REACH =                             "REACHES"; ///< parameters of reaches (channels)
CONST_CHARS DB_TAB_SPATIAL =                           "SPATIAL"; ///< spatial data in GridFS format
CONST_CHARS DB_TAB_OUT_SPATIAL =                       "OUTPUT"; ///< output data in GridFS format
CONST_CHARS DB_TAB_SITES =                             "SITES"; ///< hydro and climate sites in HydroClimateDB
CONST_CHARS DB_TAB_DATAVALUES =                        "DATA_VALUES"; ///< data values
CONST_CHARS DB_TAB_MEASUREMENT =                       "MEASUREMENT"; ///< observed hydro data
CONST_CHARS DB_TAB_ANNSTAT =                           "ANNUAL_STATS"; ///< annaul statistics based on DATA_VALUES
// Fields in DB_TAB_REACH, the orders and names should be consistent with db_import_stream_parameters.py and clsReach.cpp!
CONST_CHARS REACH_SUBBASIN =                           "SUBBASINID"; ///< reach ID is consistent with the subbasin ID
CONST_CHARS REACH_NUMCELLS =                           "NUM_CELLS"; ///< cells number of the corresponding subbasin
CONST_CHARS REACH_DOWNSTREAM =                         "DOWNSTREAM"; ///< downstream reach ID
CONST_CHARS REACH_UPDOWN_ORDER =                       "UP_DOWN_ORDER"; ///<
CONST_CHARS REACH_DOWNUP_ORDER =                       "DOWN_UP_ORDER"; ///<
CONST_CHARS REACH_WIDTH =                              "CH_WIDTH"; ///<
CONST_CHARS REACH_LENGTH =                             "CH_LEN"; ///<
CONST_CHARS REACH_DEPTH =                              "CH_DEPTH"; ///<
CONST_CHARS REACH_WDRATIO =                            "CH_WDRATIO"; ///<
CONST_CHARS REACH_AREA =                               "CH_AREA"; ///<
CONST_CHARS REACH_SIDESLP =                            "CH_SSLP"; ///<
CONST_CHARS REACH_SLOPE =                              "CH_SLP"; ///<
CONST_CHARS REACH_SINUOSITY	=                          "CH_SINUOSITY"; ///<
// Hydrological related parameters
CONST_CHARS REACH_MANNING =                         "CH_N"; // Manning's "n" value
CONST_CHARS REACH_BEDK =                            "CH_BED_K"; /// hydraulic conductivity of the channel bed
CONST_CHARS REACH_BNKK =                            "CH_BNK_K"; /// hydraulic conductivity of the channel bank
// Erosion related parameters
CONST_CHARS REACH_BEDBD =                           "CH_BED_BD"; // Bulk density of channel bed sediment
CONST_CHARS REACH_BNKBD =                           "CH_BNK_BD"; // Bulk density of channel bed sediment
CONST_CHARS REACH_BEDCOV =                          "CH_BED_COV"; // Channel bed cover factor, ch_cov2 in SWAT
CONST_CHARS REACH_BNKCOV =                          "CH_BNK_COV"; // Channel bank cover factor, ch_cov1 in SWAT
CONST_CHARS REACH_BEDEROD =                         "CH_BED_EROD"; // Erodibility of channel bed sediment, ch_bed_kd in SWAT
CONST_CHARS REACH_BNKEROD =                         "CH_BNK_EROD"; // Erodibility of channel bank sediment, ch_bnk_kd in SWAT
CONST_CHARS REACH_BEDD50 =                          "CH_BED_D50"; // D50(median) particle size diameter of channel bed sediment
CONST_CHARS REACH_BNKD50 =                          "CH_BNK_D50"; // D50(median) particle size diameter of channel band sediment
// Nutrient cycling related parameters
CONST_CHARS REACH_BC1 =                             "BC1"; ///<
CONST_CHARS REACH_BC2 =                             "BC2"; ///<
CONST_CHARS REACH_BC3 =                             "BC3"; ///<
CONST_CHARS REACH_BC4 =                             "BC4"; ///<
CONST_CHARS REACH_RK1 =                             "RK1"; ///<
CONST_CHARS REACH_RK2 =                             "RK2"; ///<
CONST_CHARS REACH_RK3 =                             "RK3"; ///<
CONST_CHARS REACH_RK4 =                             "RK4"; ///<
CONST_CHARS REACH_RS1 =                             "RS1"; ///<
CONST_CHARS REACH_RS2 =                             "RS2"; ///<
CONST_CHARS REACH_RS3 =                             "RS3"; ///<
CONST_CHARS REACH_RS4 =                             "RS4"; ///<
CONST_CHARS REACH_RS5 =                             "RS5"; ///<
CONST_CHARS REACH_DISOX =                           "DISOX"; ///<
CONST_CHARS REACH_BOD =                             "BOD"; ///<
CONST_CHARS REACH_ALGAE =                           "ALGAE"; ///<
CONST_CHARS REACH_ORGN =                            "ORGN"; ///< ch_onco in SWAT
CONST_CHARS REACH_NH4 =                             "NH4"; ///<
CONST_CHARS REACH_NO2 =                             "NO2"; ///<
CONST_CHARS REACH_NO3 =                             "NO3"; ///<
CONST_CHARS REACH_ORGP =                            "ORGP"; ///< ch_opco in SWAT
CONST_CHARS REACH_SOLP =                            "SOLP"; ///<
// Groundwater nutrient related parameters
CONST_CHARS REACH_GWNO3 =                           "GWNO3"; ///<
CONST_CHARS REACH_GWSOLP =                          "GWSOLP"; ///<
// Derived parameters according to the input parameters of Reach, which may also be provided in database.
CONST_CHARS REACH_BEDTC =                           "CH_BED_TC"; ///< Critical shear stress of channel bed
CONST_CHARS REACH_BNKTC =                           "CH_BNK_TC"; ///< Critical shear stress of channel bank
CONST_CHARS REACH_BNKSAND =                         "CH_BNK_SAND"; ///< Fraction of sand in channel bank sediment
CONST_CHARS REACH_BNKSILT =                         "CH_BNK_SILT"; ///< Fraction of silt in channel bank sediment
CONST_CHARS REACH_BNKCLAY =                         "CH_BNK_CLAY"; ///< Fraction of clay in channel bank sediment
CONST_CHARS REACH_BNKGRAVEL =                       "CH_BNK_GRAVEL"; ///< Fraction of gravel in channel bank sediment
CONST_CHARS REACH_BEDSAND =                         "CH_BED_SAND"; ///< Fraction of sand in channel bed sediment
CONST_CHARS REACH_BEDSILT =                         "CH_BED_SILT"; ///< Fraction of silt in channel bed sediment
CONST_CHARS REACH_BEDCLAY =                         "CH_BED_CLAY"; ///< Fraction of clay in channel bed sediment
CONST_CHARS REACH_BEDGRAVEL =                       "CH_BED_GRAVEL"; ///< Fraction of gravel in channel bed sediment
// Grouping related
CONST_CHARS REACH_GROUP =                              "GROUP"; ///<
CONST_CHARS REACH_KMETIS =                             "KMETIS"; ///<
CONST_CHARS REACH_PMETIS =                             "PMETIS"; ///<
// Coordinates
CONST_CHARS REACH_COORX =                              "CH_COORX"; ///< X coordinates (not cols!)
CONST_CHARS REACH_COORY =                              "CH_COORY"; ///< Y coordinates (not rows!)

// these four are defined in DB_TAB_SITELIST in Source_ParameterDB
CONST_CHARS SITELIST_TABLE_M =                      "SITELISTM"; ///<
CONST_CHARS SITELIST_TABLE_P =                      "SITELISTP"; ///<
CONST_CHARS SITELIST_TABLE_PET =                    "SITELISTPET"; ///<

// define string constants used in the code, also used in the mongoDB.SiteList table's header
CONST_CHARS Tag_Mode =                              "MODE"; ///<
CONST_CHARS Tag_Mode_Storm =                        "STORM"; ///<
CONST_CHARS Tag_Mode_Daily =                        "DAILY"; ///<`

CONST_CHARS Type_Scenario =                         "SCENARIO"; ///<
CONST_CHARS Type_Reach =                            "REACH"; ///<
CONST_CHARS Type_Subbasin =                         "SUBBASIN"; ///<
CONST_CHARS Type_Raster1D =                         "RASTER1D"; ///<
CONST_CHARS Type_Raster1DInt =                      "RASTER1DINT"; ///<
CONST_CHARS Type_Raster2D =                         "RASTER2D"; ///<
CONST_CHARS Type_Raster2DInt =                      "RASTER2DINT"; ///<
CONST_CHARS Type_Array1DDateValue =                 "ARRAY1DDATEVALUE"; ///<
CONST_CHARS Type_Array2D =                          "ARRAY2D"; ///<
CONST_CHARS Type_Array2DInt =                       "ARRAY2DINT"; ///<
CONST_CHARS Type_Array1D =                          "ARRAY1D"; ///<
CONST_CHARS Type_Array1DInt =                       "ARRAY1DINT"; ///<
CONST_CHARS Type_Single =                           "SINGLE"; ///<
CONST_CHARS Type_SingleInt =                        "SINGLEINT"; ///<
CONST_CHARS Type_RasterPositionData =               "RASTERPOSITION"; ///<

CONST_CHARS TFType_Whole =                          "TFWhole"; ///<
CONST_CHARS TFType_Single =                         "TFSINGLE"; ///<
CONST_CHARS TFType_Array1D =                        "TFARRAY1D"; ///<

//////////////////////////////////////////////////////////////////////////
/// Define models' ID and description in SEIMS  //////////////////////////
/// By Liangjun Zhu, Apr. 26, 2016  //////////////////////////////////////
/// Updated by Tong Wu and Liangjun Zhu, Jul. 2021 ///////////////////////
//////////////////////////////////////////////////////////////////////////

// Hydro-Meteorological data related modules
CONST_CHARS_LIST MCLS_CLIMATE[] = {"HydroClimate", "HydroClimate data modules"}; ///<
CONST_CHARS_LIST M_TSD_RD[] = {"TSD_RD", "Read time series data from HydroClimate database."}; ///<
CONST_CHARS_LIST M_ITP[] = {"ITP", "Interpolation of P, T, etc."}; ///<

// Soil properties related modules
CONST_CHARS_LIST MCLS_SOIL[] = {"Soil property", "Soil properties related modules"}; ///<
CONST_CHARS_LIST M_STP_FP[] = {"STP_FP", "Finn Plauborg Method to compute soil temperature."}; ///<
CONST_CHARS_LIST M_SOL_WB[] = {"SOL_WB", "Soil water balance calculation" }; ///<

// Canopy interception related modules
CONST_CHARS_LIST MCLS_INTERC[] = {"Interception", "Canopy interception module"}; ///<
CONST_CHARS_LIST M_PI_SVSC[] = {"PI_SVSC", "Precipitation interception by seasonal variation of storage capacity method"}; ///<
CONST_CHARS_LIST M_PI_MCS[] = {"PI_MCS", "Precipitation interception based on Maximum Canopy Storage"}; ///<

// Snow redistribution related modules
CONST_CHARS_LIST MCLS_SNO_RD[] = {"Snow redistribution", "Snow redistribution calculation"};
CONST_CHARS_LIST M_SRD_MB[] = {"SRD_MB", "Original WetSpa algorithm"};

// Snow sublimation related modules
CONST_CHARS_LIST MCLS_SNO_SB[] = {"Snow sublimation", "Calculate the amount of snow sublimation."}; ///<
CONST_CHARS_LIST M_SSM_PE[] = {"SSM_PE", "A simple method that used in the old WetSpa to calculate snow sublimation."}; ///<

// Snow melt related modules
CONST_CHARS_LIST MCLS_SNOW[] = {"Snow accumulation and melt", "Snow accumulation and melt."}; ///<
CONST_CHARS_LIST M_SNO_WB[] = {"SNO_WB", "Calculate snow water balance"}; ///<
CONST_CHARS_LIST M_SNO_DD[] = {"SNO_DD", "Degree-Day method (Martinec et al., 1983) for snow melt modeling"}; ///<
CONST_CHARS_LIST M_SNO_SP[] = {"SNO_SP", "Snowpack Daily method from SWAT"}; ///<

// Potential Evapotranspiration related modules
CONST_CHARS_LIST MCLS_PET[] = {"Potential Evapotranspiration", "Calculate the potential evapotranspiration"}; ///<
CONST_CHARS_LIST M_PET_H[] = {"PET_H", "Hargreaves method for potential evapotranspiration."}; ///<
CONST_CHARS_LIST M_PET_PT[] = {"PET_PT", "PriestleyTaylor method for potential evapotranspiration."}; ///<
CONST_CHARS_LIST M_PET_PM[] = {"PET_PM", "Penman Monteith method for potential evapotranspiration."}; ///<

// Actual Evapotranspiration from plant and soil related modules
CONST_CHARS_LIST MCLS_AET[] = {"Actual Evapotranspiration", "Calculates potential plant transpiration "
                               "and potential and actual soil evaporation."}; ///<
CONST_CHARS_LIST M_AET_PTH[] = {"AET_PTH", "Potential plant transpiration for Priestley-Taylor and Hargreaves ET methods"}; ///<
CONST_CHARS_LIST M_SET_LM[] = {"SET_LM", "Evapotranspiration from soil related linearly with soil moisture (WetSpa)"}; ///<

// Depression related modules
CONST_CHARS_LIST MCLS_DEP[] = {"Depression", "Fill depression"}; ///<
CONST_CHARS_LIST M_DEP_FS[] = {"DEP_FS", "A simple fill and spill method method to calculate depression storage."}; ///<
CONST_CHARS_LIST M_DEP_LINSLEY[] = {"DEP_LINSLEY", "Linsley method to calculate depression storage"}; ///<

// Surface runoff and infiltration related modules
CONST_CHARS_LIST MCLS_SUR_RUNOFF[] = {"Surface runoff", "Infiltration and surface runoff of excess precipitation."}; ///<
CONST_CHARS_LIST M_SUR_MR[] = {"SUR_MR", "Modified rational method to calculate infiltration and excess precipitation."}; ///<
CONST_CHARS_LIST M_SUR_CN[] = {"SUR_CN", "SCS curve number method to calculate infiltration and excess precipitation."}; ///<
CONST_CHARS_LIST M_SUR_SGA[] = {"SUR_SGA", "Green-Ampt method for infiltration and excess precipitation in storm mode."}; ///<
CONST_CHARS_LIST M_SUR_GR4J[] = {"SUR_GR4J", "GR4J's method for infiltration and excess precipitation."}; ///<

// Interflow (subsurface flow) routing related modules
// TODO, uniform the prefix as SSR_. By lj
CONST_CHARS_LIST MCLS_INTERFLOW[] = {"Interflow (subsurface flow) routing", "Interflow routing."}; ///<
CONST_CHARS_LIST M_IKW_IF[] = {"IKW_IF", "interflow routing using the method of WetSpa model."}; ///< TODO rewrite the desc
CONST_CHARS_LIST M_IUH_IF[] = {"IUH_IF", "IUH overland method to calculate interflow routing."}; ///<
CONST_CHARS_LIST M_SSR_DA[] = {"SSR_DA", "Darcy's law and the kinematic approximation."}; ///<

// Percolation related modules
CONST_CHARS_LIST MCLS_PERCO[] = {"Percolation", "Water percolated out of the root zone."}; ///<
CONST_CHARS_LIST M_PER_PI[] = {"PER_PI", "Percolation based on Darcy's law and Brooks-Corey equation"}; ///<
CONST_CHARS_LIST M_PER_STR[] = {"PET_STR", "Percolation based on storage routing method"}; ///<
CONST_CHARS_LIST M_PERCO_DARCY[] = {"PERCO_DARCY", "Original WetSpa method which relates percolation with "
                                    "soil moisture and pore size distribution index."}; ///<

// Hillslope hydrology related modules
CONST_CHARS_LIST MCLS_HS_HYDRO[] = {"Hillslope water balance", "Water balance calculation in hillslope."}; ///<
CONST_CHARS_LIST M_HS_WB[] = {"HS_WB", "Hillsloope water balance."}; ///<

// Paddy related modules
CONST_CHARS_LIST MCLS_PADDY[] = {"Paddy", "Paddy simulations"}; ///<
CONST_CHARS_LIST M_IMP_SWAT[] = {"IMP_SWAT", "SWAT method, simulates depressional areas that do not drain to "
                                 "the stream network (pothole) and impounded areas such as rice paddies"}; ///<
// Groundwater related modules
CONST_CHARS_LIST MCLS_GW[] = {"Groundwater", "Groundwater routing and baseflow."}; ///<
CONST_CHARS_LIST M_GW_RSVR[] = {"GW_RSVR", "Groundwater routing based on reservoir method."}; ///< TODO, maybe should be removed!
CONST_CHARS_LIST M_GWA_RE[] = {"GWA_RE", "Groundwater routing based on reservoir method."}; ///<

// Erosion related modules
CONST_CHARS_LIST MCLS_OL_EROSION[] = {"Overland erosion", "Calculate the amount sediment yield of overland erosion."}; ///<
CONST_CHARS_LIST MCLS_CH_EROSION[] = {"Channel erosion", "Calculate the amount channel erosion."}; ///<
CONST_CHARS_LIST M_SplashEro_Park[] = {"SplashEro_Park", "Park equation to calculate sediment yield of each unit"}; ///<
CONST_CHARS_LIST M_KINWAVSED_OL[] = {"KinWavSed_OL", "Energy function(Govers) method for sediment yield routing."}; ///<
CONST_CHARS_LIST M_KINWAVSED_CH[] = {"KinWavSed_CH", "Srinivasan & Galvao function for sediment yield routing."}; ///<
CONST_CHARS_LIST M_SERO_MUSLE[] = {"SERO_MUSLE", "MUSLE method for sediment yield."}; ///<
CONST_CHARS_LIST M_IUH_SED_OL[] = {"IUH_SED_OL", "Overland routing of sediment using IUH."}; ///<

// Management related modules
CONST_CHARS_LIST MCLS_MGT[] = {"Mangement practices", "BMP related modules"}; ///<
CONST_CHARS_LIST M_PLTMGT_SWAT[] = {"PLTMGT_SWAT", "Plant mangement operation modeling method in SWAT"}; ///<
CONST_CHARS_LIST M_NPSMGT[] = {"NPSMGT", "Non-point source pollution management"}; ///<

// Ecology (e.g., plant growth) related modules
CONST_CHARS_LIST MCLS_PG[] = {"Plant growth", "Calculate the amount of plant growth."}; ///<
CONST_CHARS_LIST M_PG_EPIC[] = {"PG_EPIC", "Plant growth based on a simplified version of EPIC used in SWAT."}; ///<
CONST_CHARS_LIST M_PG_ORYZA[] = {"PG_ORYZA", "Rice crop growth module of ORYZA2000 model."}; ///<

// Overland routing related modules
CONST_CHARS_LIST MCLS_OL_ROUTING[] = {"Overland routing", "Overland routing module"}; ///<
CONST_CHARS_LIST M_IKW_OL[] = {"IKW_OL", "Overland routing using 4-point implicit finite difference method."}; ///<
CONST_CHARS_LIST M_IUH_OL[] = {"IUH_OL", "IUH overland method to calculate overland flow routing."}; ///<

// Channel routing related modules
CONST_CHARS_LIST MCLS_CH_ROUTING[] = {"Channel routing", "Channel routing modules"}; ///<
CONST_CHARS_LIST M_CH_DW[] = {"CH_DW", "Channel routing using diffusive wave equation."}; ///<
CONST_CHARS_LIST M_CH_MSK[] = {"CH_MSK", "Channel routing using Muskingum-Cunge method of storm model."}; ///<
CONST_CHARS_LIST M_IKW_CH[] = {"IKW_CH", "Channel routing using 4-point implicit finite difference method for kinematic wave."}; ///<
CONST_CHARS_LIST M_MUSK_CH[] = {"MUSK_CH", "Channel routing using Muskingum-Cunge method of longterm model."}; ///<
CONST_CHARS_LIST M_NUTR_CH[] = { "NUTR_CH", "Channel routing of nutrients" }; ///<
CONST_CHARS_LIST M_SEDR_SBAGNOLD[] = {"SEDR_SBAGNOLD", "Sediment channel routing using "
                                      "variable channel dimension method as used in SWAT."}; ///<

// Nutrient related modules, e.g., carbon, nitrogen, and phosphorus mineralization and immobilization.
CONST_CHARS_LIST MCLS_NUTRCYC[] = {"Nutrient cycling", "Carbon, nitrogen, and phosphorus cycling"}; ///<
CONST_CHARS_LIST M_NUTR_TF[] = {"NUTR_TF", "Daily nitrogen and phosphorus mineralization and immobilization "
                                "considering fresh organic material (plant residue) and active and stable humus material."}; ///<
CONST_CHARS_LIST M_NUTRSED[] = {"NUTRSED", "Nutrient removed and loss in surface runoff, lateral flow, tile flow,"
                                " and percolation out of the profile."}; ///<
CONST_CHARS_LIST M_NUTRMV[] = {"NutrMV", "Simulates the loss of nitrate and phosphorus via surface runoff"}; ///<
CONST_CHARS_LIST M_NUTRGW[] = {"NutrGW", "Simulates the tutrient loading contributed by groundwater flow"}; ///<
CONST_CHARS_LIST M_NUTRCH_QUAL2E[] = {"NutrCH_QUAL2E", "In-stream nutrient transformations"}; ///<

// Atmospheric Deposition
CONST_CHARS_LIST MCLS_ATMDEP[] = {"AtmosphericDeposition", "AtmosphericDeposition"}; ///<
CONST_CHARS_LIST M_ATMDEP[] = {"ATMDEP", "AtmosphericDeposition"}; ///<


CONST_CHARS_LIST MCLS_CONCEPTUAL_MODEL[] = {"Conceptual Model", "Conceptual Model Description."}; ///<
CONST_CHARS_LIST CM_GR4J[] = {"GR4J", "GR4J model"}; ///<


///////////////////////////////////////////////////////////////////////////////////////////
/// Define units' names and descriptions common used in SEIMS, in case of inconsistency ///
/// By LiangJun Zhu, HuiRan Gao, Tong Wu                                                ///
/// Last updated: Jul., 2021                                                            ///
///////////////////////////////////////////////////////////////////////////////////////////

CONST_CHARS_LIST VAR_A_BNK[] = {"a_bnk", "bank flow recession constant"};
CONST_CHARS_LIST VAR_ACC[] = {"acc", "Flow accumulation, equals to the number of accumulated cells"}; /// m_flowAccm
CONST_CHARS_LIST VAR_ACC_INFIL[] = {"AccumuInfil", "accumulative infiltration"};
CONST_CHARS_LIST VAR_ADDRNH4[] = {"addrnh4", "ammonium added by rainfall"};
CONST_CHARS_LIST VAR_ADDRNO3[] = {"addrno3", "nitrate added by rainfall"};
CONST_CHARS_LIST VAR_AET_PLT[] = {"AET_PLT", "actual amount of plant transpiration, ep_day in SWAT"}; /// m_actPltET
CONST_CHARS_LIST VAR_AFERT_AMAXN[] = {"afert_AmaxN", "Maximum amount of mineral N allowed to be applied in any one year"}; /// m_autoFertMaxAnnApldMinN
CONST_CHARS_LIST VAR_AFERT_FRTEFF[] = {"afert_frteff", "fertilizer application efficiency calculated as the amount of N applied divided by the amount of N removed at harvest"}; /// m_autoFertEff
CONST_CHARS_LIST VAR_AFERT_FRTSURF[] = {"afert_frtsurf", "Fraction of fertilizer applied to top 10mm of soil"}; /// m_autoFertSurfFr
CONST_CHARS_LIST VAR_AFERT_ID[] = {"afert_id", "fertilizer ID from fertilizer database"}; /// m_fertID
CONST_CHARS_LIST VAR_AFERT_MAXN[] = {"afert_maxN", "Maximum amount of mineral N allowed in any one application"}; /// m_autoFertMaxApldN
CONST_CHARS_LIST VAR_AFERT_NSTRS[] = {"afert_nstrs", "Nitrogen stress factor of cover/plant that triggers fertilization"}; /// m_autoNStrsTrig
CONST_CHARS_LIST VAR_AFERT_NSTRSID[] = {"afert_nstrsID", "Code for approach used to determine amount of nitrogen to Cell"}; /// m_NStrsMeth
CONST_CHARS_LIST VAR_AFERT_NYLDT[] = {"afert_nyldTarg", "modifier for auto fertilization target nitrogen content"}; /// m_autoFertNtrgtMod
CONST_CHARS_LIST VAR_AI0[] = {"ai0", "ratio of chlorophyll-a to algal biomass"};
CONST_CHARS_LIST VAR_AI1[] = {"ai1", "fraction of algal biomass that is nitrogen"};
CONST_CHARS_LIST VAR_AI2[] = {"ai2", "fraction of algal biomass that is phosphorus"};
CONST_CHARS_LIST VAR_AI3[] = {"ai3", "the rate of oxygen production per unit of algal photosynthesis"};
CONST_CHARS_LIST VAR_AI4[] = {"ai4", " the rate of oxygen uptake per unit of algae respiration"};
CONST_CHARS_LIST VAR_AI5[] = {"ai5", "the rate of oxygen uptake per unit of NH3 nitrogen oxidation"};
CONST_CHARS_LIST VAR_AI6[] = {"ai6", "the rate of oxygen uptake per unit of NO2 nitrogen oxidation"};
CONST_CHARS_LIST VAR_AIRR_EFF[] = {"airr_eff", "auto irrigation efficiency, 0 ~ 100"}; /// m_autoIrrEff
CONST_CHARS_LIST VAR_AIRR_LOCATION[] = {"airr_location", "location of irrigation source"}; /// m_autoIrrLocNo
CONST_CHARS_LIST VAR_AIRR_SOURCE[] = {"airr_source", "irrigation source"}; /// m_autoIrrSrc
CONST_CHARS_LIST VAR_AIRRSURF_RATIO[] = {"airrsurf_ratio", "surface runoff ratio (0-1)"}; /// m_autoIrrWtr2SurfqR
CONST_CHARS_LIST VAR_AIRRWTR_DEPTH[] = {"airrwtr_depth", "amount of irrigation water applied each time"}; /// m_autoIrrWtrD
CONST_CHARS_LIST VAR_ALAIMIN[] = {"alai_min", "minimum LAI during winter dormant period"}; /// m_minLaiDorm
CONST_CHARS_LIST VAR_ALBDAY[] = {"ALBDAY", "Albedo of the current day"}; /// m_alb
CONST_CHARS_LIST VAR_CH_ALGAE[] = {"ch_algae", "algal biomass in reach"};
CONST_CHARS_LIST VAR_CH_ALGAEConc[] = {"ch_algaeConc", ""};
CONST_CHARS_LIST VAR_CH_ONCO[] = {"ch_onco", "Channel organic nitrogen concentration in basin"};
CONST_CHARS_LIST VAR_CH_OPCO[] = {"ch_opco", "Channel organic phosphorus concentration in basin"};
CONST_CHARS_LIST VAR_AMMO_CH[] = {"ammoToCh", "amount of ammonium transported with lateral flow"};
CONST_CHARS_LIST VAR_CH_NH4[] = {"ch_nh4", "ammonia nitrogen in reach"};
CONST_CHARS_LIST VAR_CH_NH4Conc[] = {"ch_nh4Conc", ""};
CONST_CHARS_LIST VAR_ANION_EXCL[] = {"anion_excl", "fraction of porosity from which anions are excluded"}; /// m_anionExclFr
CONST_CHARS_LIST VAR_AWTR_STRS_ID[] = {"awtr_strsID", "Water stress identifier, 1 plant water demand, 2 soil water content"}; /// m_wtrStrsID
CONST_CHARS_LIST VAR_AWTR_STRS_TRIG[] = {"awtr_strsTrig", "Water stress threshold that triggers irrigation"}; /// m_autoWtrStrsTrig
CONST_CHARS_LIST VAR_B_BNK[] = {"b_bnk", "bank storage loss coefficient"};
CONST_CHARS_LIST VAR_BACT_SWF[] = {"bact_swf", "fraction of manure containing active colony forming units (cfu)"};
CONST_CHARS_LIST VAR_Base_ex[] = {"Base_ex", "baseflow recession exponent"};
CONST_CHARS_LIST VAR_BIO_E[] = {"BIO_E", "the potential or unstressed growth rate (including roots) per unit of intercepted photsynthetically active radiation"}; /// m_biomEnrgRatio
CONST_CHARS_LIST VAR_BIOEHI[] = {"BIOEHI", "Biomass-energy ratio corresponding to the 2nd point on the radiation use efficiency curve"}; /// m_biomEnrgRatio2ndPt
CONST_CHARS_LIST VAR_BIOINIT[] = {"BIO_INIT", "initial dry weight biomass (kg/ha)"}; /// m_initBiom
CONST_CHARS_LIST VAR_BIOLEAF[] = {"BIO_LEAF", "fraction of biomass that drops during dormancy (for tree only)"}; /// m_biomDropFr
CONST_CHARS_LIST VAR_BIOMASS[] = {"BIOMASS", "land cover/crop biomass (dry weight)"}; /// m_biomass
CONST_CHARS_LIST VAR_BIOTARG[] = {"biotarg", "Biomass (dry weight) target (kg/ha), defined in plant management operation"}; /// m_biomTrgt
CONST_CHARS_LIST VAR_BKST[] = {"BKST", "bank storage"};
CONST_CHARS_LIST VAR_BLAI[] = {"BLAI", "maximum leaf area index"}; /// m_maxLai
CONST_CHARS_LIST VAR_BMX_TREES[] = {"BMX_TREES", "Maximum biomass for a forest (metric tons/ha)"}; /// m_maxBiomTree
CONST_CHARS_LIST VAR_BN1[] = {"BN1", "the normal fraction of nitrogen in the plant biomass at emergence"}; /// m_biomNFr1
CONST_CHARS_LIST VAR_BN2[] = {"BN2", "the normal fraction of nitrogen in the plant biomass at 50% maturity"}; /// m_biomNFr2
CONST_CHARS_LIST VAR_BN3[] = {"BN3", "the normal fraction of nitrogen in the plant biomass at maturity"}; /// m_biomNFr3
CONST_CHARS_LIST VAR_BNK0[] = {"Bnk0", "initial bank storage per meter of reach length"};
CONST_CHARS_LIST VAR_BP1[] = {"BP1", "the normal fraction of phosphorus in the plant biomass at emergence"}; /// m_biomPFr1
CONST_CHARS_LIST VAR_BP2[] = {"BP2", "the normal fraction of phosphorus in the plant biomass at 50% maturity"}; /// m_biomPFr2
CONST_CHARS_LIST VAR_BP3[] = {"BP3", "the normal fraction of phosphorus in the plant biomass at maturity"}; /// m_biomPFr3
CONST_CHARS_LIST VAR_C_RAIN[] = {"c_rain", "Rainfall impact factor"};
CONST_CHARS_LIST VAR_C_SNOW[] = {"c_snow", "temperature impact factor"};
CONST_CHARS_LIST VAR_C_SNOW12[] = {"c_snow12", "Melt factor on Decemeber 21, Minimum melt rate for snow during year"};
CONST_CHARS_LIST VAR_C_SNOW6[] = {"c_snow6", "Melt factor on June 21, Maximum melt rate for snow during year"};
CONST_CHARS_LIST VAR_C_WABA[] = {"C_WABA", "Channel water balance in a text format for each reach and at each time step"};
CONST_CHARS_LIST VAR_CDN[] = {"cdn", "rate coefficient for denitrification"}; /// m_denitCoef
CONST_CHARS_LIST VAR_CELL_LAT[] = {"celllat", "latitude of each valid cells"}; /// m_cellLat
CONST_CHARS_LIST VAR_CH_DEP[] = {"DEP", "distribution of channel sediment deposition"};
CONST_CHARS_LIST VAR_CH_DET[] = {"DET", "distribution of channel flow detachment"};
CONST_CHARS_LIST VAR_CH_DETCO[] = {"ChDetCo", "Calibration coefficient of channel flow detachment"};
CONST_CHARS_LIST VAR_CH_FLOWCAP[] = {"CAP", "distribution of channel flow capacity"};
//CONST_CHARS_LIST VAR_CH_MANNING_FACTOR[] = {"CH_ManningFactor", "Manning scaling factor for channel routing"};
CONST_CHARS_LIST VAR_CH_SEDRATE[] = {"QSN", "distribution of channel sediment rate"};
CONST_CHARS_LIST VAR_CH_TCCO[] = {"ChTcCo", "Calibration coefficient of transport capacity calculation"};
CONST_CHARS_LIST VAR_CH_V[] = {"CHANV", "flow velocity"};
CONST_CHARS_LIST VAR_CH_VOL[] = {"CHANVOL", "water volume"};
CONST_CHARS_LIST VAR_CH_CHLORA[] = {"CH_chlora", "chlorophyll-a in reach"};
CONST_CHARS_LIST VAR_CH_CHLORAConc[] = {"CH_chloraConc", ""};
CONST_CHARS_LIST VAR_CHL_A[] = {"chl_a", "chlorophyll-a concentration in water yield"}; /// m_surfRfChlA
CONST_CHARS_LIST VAR_CHS0[] = {"Chs0", "initial channel storage per meter of reach length"}; /// m_initChStorage
CONST_CHARS_LIST VAR_CHS0_PERC[] = {"chs0_perc", "initial percentage of channel volume"}; ///
CONST_CHARS_LIST VAR_CHSB[] = {"CHSB", "Channel sediment balance for each reach and at each time step"};
CONST_CHARS_LIST VAR_CHST[] = {"CHST", "channel storage"}; /// m_chStorage
CONST_CHARS DESC_PRECHST = "channel storage at previous timestep";
CONST_CHARS_LIST VAR_CHT[] = {"CHT", "canopy height for the day (m)"}; /// m_canHgt
CONST_CHARS_LIST VAR_CHTMX[] = {"CHTMX", "maximum canopy height (m)"}; /// m_maxCanHgt
CONST_CHARS_LIST VAR_CHWTRWIDTH[] = {"chwtrwidth", "Channel water width"}; /// m_chWtrWth
CONST_CHARS_LIST VAR_CHBTMWIDTH[] = {"chbtmwidth", "the bottom width of channel"};
CONST_CHARS_LIST VAR_CHCROSSAREA[] = {"chCrossArea", "channel cross-sectional area"};
CONST_CHARS_LIST VAR_CHWIDTH[] = {"CH_WIDTH", "Channel width"};
CONST_CHARS_LIST VAR_CHWTRDEPTH[] = {"CHWTRDEPTH", "channel water depth"}; // m_chWtrDepth
CONST_CHARS DESC_PRECHWTDEPTH = "channel water depth of previous timestep";
CONST_CHARS_LIST VAR_CLAY[] = {"CLAY", "Percent of clay content"}; // m_soilClay
CONST_CHARS_LIST VAR_CMN[] = {"cmn", "Rate coefficient for mineralization of the humus active organic nutrients"}; // m_minrlCoef
CONST_CHARS_LIST VAR_CN2[] = {"CN2", "Curve Number value under moisture condition II"}; // m_cn2
CONST_CHARS_LIST VAR_CO2[] = {"Co2", "CO2 Concentration"}; // m_co2Conc
CONST_CHARS_LIST VAR_CO2HI[] = {"CO2HI", "elevated CO2 atmospheric concentration corresponding the 2nd point on the radiation use efficiency curve"}; /// m_co2Conc2ndPt
CONST_CHARS_LIST VAR_SUR_COD[] = {"sur_cod", "carbonaceous oxygen demand of surface runoff"}; // m_surfRfCod
CONST_CHARS DESC_COD_CH = "carbonaceous oxygen demand loading to reach";
CONST_CHARS_LIST VAR_COD_N[] = {"cod_n", "Conversion factor"};
CONST_CHARS_LIST VAR_COD_K[] = {"cod_k", "Reaction coefficient"};
CONST_CHARS_LIST VAR_COND_RATE[] = {"Cond_rate", "Rate of decline in stomatal conductance per unit increase in vapor pressure deficit"};
CONST_CHARS_LIST VAR_CONDUCT[] = {"Conductivity", "saturation hydraulic conductivity"}; //
CONST_CHARS_LIST VAR_CONV_WT[] = {"conv_wt", "factor which converts kg/kg soil to kg/ha"}; // m_cvtWt
CONST_CHARS_LIST VAR_CROP_LOOKUP[] = {"CropLookup", "Crop lookup table"}; // m_cropLookup
CONST_CHARS_LIST VAR_CSWAT[] = {"cswat", "carbon modeling method"}; /// m_cbnModel
CONST_CHARS_LIST VAR_PCP[] = {"D_P", "Precipitation of each time step on current cell"}; /// m_pcp
CONST_CHARS_LIST VAR_DAYLEN[] = {"daylength", "day length"}; /// m_dayLen
CONST_CHARS_LIST VAR_DAYLEN_MIN[] = {"daylenmin", "minimum day length"}; /// m_dayLenMin
CONST_CHARS_LIST VAR_DEEPST[] = {"deepst", "depth of water in deep aquifer"};
CONST_CHARS_LIST VAR_DEET[] = {"DEET", "evaporation from depression storage"}; /// m_deprStoET
CONST_CHARS_LIST VAR_DEM[] = {"DEM", "Digital Elevation Model in meters"}; /// m_dem
CONST_CHARS_LIST VAR_DEPREIN[] = {"Depre_in", "initial depression storage coefficient"};
CONST_CHARS_LIST VAR_DEPRESSION[] = {"Depression", "Depression storage capacity"};
CONST_CHARS_LIST VAR_DETSPLASH[] = {"DETSplash", "distribution of splash detachment"};
CONST_CHARS_LIST VAR_DETACH_SAND[] = {"det_sand", "sand fraction of detached sediment"}; /// m_detSand
CONST_CHARS_LIST VAR_DETACH_SILT[] = {"det_silt", "silt fraction of detached sediment"}; /// m_detSilt
CONST_CHARS_LIST VAR_DETACH_CLAY[] = {"det_clay", "clay fraction of detached sediment"}; /// m_detClay
CONST_CHARS_LIST VAR_DETACH_SAG[] = {"det_smagg", "small aggeregate fraction of detached sediment"}; /// m_detSmAgg
CONST_CHARS_LIST VAR_DETACH_LAG[] = {"det_lgagg", "large aggregate fraction of detached sediment"}; /// m_detLgAgg
CONST_CHARS_LIST VAR_SANDYLD[] = {"sand_yld", "sand yield amount"}; /// m_eroSand
CONST_CHARS_LIST VAR_SILTYLD[] = {"silt_yld", "silt yield amount"}; /// m_eroSilt
CONST_CHARS_LIST VAR_CLAYYLD[] = {"clay_yld", "clay yield amount"}; /// m_eroClay
CONST_CHARS_LIST VAR_SAGYLD[] = {"sag_yld", "small aggeregate yield amout"}; /// m_eroSmAgg
CONST_CHARS_LIST VAR_LAGYLD[] = {"lag_yld", "large aggregate yield amout"}; /// m_eroLgAgg
CONST_CHARS_LIST VAR_DF_COEF[] = {"df_coef", "Deep percolation coefficient"};
CONST_CHARS_LIST VAR_CH_SOLP[] = {"CH_SOLP", "dissolved phosphorus in reach"};
CONST_CHARS_LIST VAR_CH_SOLPConc[] = {"CH_SOLPConc", ""};
CONST_CHARS_LIST VAR_DLAI[] = {"DLAI", "the fraction of growing season(PHU) when senescence becomes dominant"}; /// m_dormPHUFr
CONST_CHARS_LIST VAR_DORMHR[] = {"dormhr", "time threshold used to define dormant period for plant"}; /// m_dormHr
CONST_CHARS_LIST VAR_DORMI[] = {"dormi", "dormancy status code, 0 for land cover growing and 1 for dormant"}; /// m_dormFlag
CONST_CHARS_LIST VAR_DPST[] = {"DPST", "depression storage"}; /// m_deprSto
CONST_CHARS_LIST VAR_DRYDEP_NH4[] = {"drydep_nh4", "atmospheric dry deposition of ammonia"}; /// m_dryDepNH4
CONST_CHARS_LIST VAR_DRYDEP_NO3[] = {"drydep_no3", "atmospheric dry deposition of nitrates"}; /// m_dryDepNO3
CONST_CHARS_LIST VAR_EP_CH[] = {"Ep_ch", "reach evaporation adjustment factor"};
CONST_CHARS_LIST VAR_EPCO[] = {"epco", "plant water uptake compensation factor"}; /// m_epco
CONST_CHARS_LIST VAR_ESCO[] = {"esco", "soil evaporation compensation factor"}; /// m_esco
CONST_CHARS_LIST VAR_EVLAI[] = {"evlai", "leaf area index at which no evaporation occurs from the water surface"};
CONST_CHARS_LIST VAR_POT_TILE[] = {"pot_tile", "Average daily outflow to main channel from tile flow if drainage tiles are installed in the pothole"};
CONST_CHARS_LIST VAR_POT_NO3DECAY[] = {"pot_no3l", "Nitrate decay rate in impounded water body"};
CONST_CHARS_LIST VAR_POT_SOLPDECAY[] = {"pot_solpl", "Soluble phosphorus decay rate in impounded water body"};
CONST_CHARS_LIST VAR_EXCP[] = {"EXCP", "excess precipitation"}; /// m_exsPcp
CONST_CHARS_LIST VAR_EXT_COEF[] = {"EXT_COEF", "light extinction coefficient"}; /// m_lightExtCoef
CONST_CHARS_LIST VAR_FERTILIZER_LOOKUP[] = {"FertilizerLookup", "Fertilizer lookup table"}; /// m_fertLookup
CONST_CHARS_LIST VAR_FIELDCAP[] = {"FieldCap", "Soil field capacity"};
CONST_CHARS_LIST VAR_FLAT[] = {"flat", "lateral flow in soil layer"};
CONST_CHARS_LIST VAR_FLOWDIR[] = {"FLOW_DIR", "Flow direction by the rule of TauDEM"};
CONST_CHARS_LIST VAR_FLOWWIDTH[] = {"FlowWidth", "Flow width of overland plane"};
CONST_CHARS_LIST VAR_FR_PHU_ACC[] = {"frPHUacc", "fraction of plant heat units (PHU) accumulated"}; /// m_phuAccum
CONST_CHARS_LIST VAR_FR_PLANT_N[] = {"frPlantN", "fraction of plant biomass that is nitrogen, pltfr_n in SWAT"}; /// m_frPltN
CONST_CHARS_LIST VAR_FR_PLANT_P[] = {"frPlantP", "fraction of plant biomass that is phosphorous, pltfr_p in SWAT"}; /// m_frPltP
CONST_CHARS_LIST VAR_FR_ROOT[] = {"frRoot", "fraction of total plant biomass that is in roots, rwt in SWAT"}; /// m_frRoot
CONST_CHARS_LIST VAR_FR_STRSWTR[] = {"frStrsWtr", "fraction of potential plant growth achieved where the reduction is caused by water stress, strsw in SWAT"}; /// m_frStrsWtr
CONST_CHARS_LIST VAR_FRGMAX[] = {"frgmax", "fraction of maximum stomatal conductance corresponding to the second point on the stomatal conductance curve"};
CONST_CHARS_LIST VAR_FRGRW1[] = {"FRGRW1", "fraction of total potential heat units corresponding to the 1st point on the optimal leaf area development curve"}; /// m_frGrow1stPt
CONST_CHARS_LIST VAR_FRGRW2[] = {"FRGRW2", "fraction of total potential heat units corresponding to the 2nd point on the optimal leaf area development curve"}; /// m_frGrow2ndPt
CONST_CHARS_LIST VAR_GRRE[] = {"GRRE", ""};
CONST_CHARS_LIST VAR_GRZ_DAYS[] = {"grz_days", "number of days cell has been grazed"}; /// m_nGrazDays
CONST_CHARS_LIST VAR_GRZ_FLAG[] = {"grz_flag", "grazing flag for cell"}; /// m_grazFlag
CONST_CHARS_LIST VAR_GSI[] = {"gsi", "maximum stomatal conductance at high solar radiation and low vpd"};
CONST_CHARS_LIST VAR_GW_KG[] = {"kg", "baseflow recession coefficient"};
CONST_CHARS_LIST VAR_GW_Q[] = {"GW", "groundwater contribution to stream flow"};
CONST_CHARS_LIST VAR_GW0[] = {"GW0", "initial ground water storage"};
CONST_CHARS_LIST VAR_GWMAX[] = {"GWMAX", "maximum ground water storage"};
CONST_CHARS_LIST VAR_GWSOLP_CONC[] = {"gwsolp_conc", "soluble P concentration in groundwater"}; /// m_gwSolPConc
CONST_CHARS_LIST VAR_GWSOLP[] = {"gwsolp", "soluble P amount in groundwater"}; /// m_gwSolP
CONST_CHARS_LIST VAR_GWNEW[] = {"GWNEW", "The volume of water from the bank storage to the adjacent unsaturated zone and groundwater storage"};
CONST_CHARS_LIST VAR_GWNO3_CONC[] = {"gwno3_conc", "nitrate N concentration in groundwater"}; /// m_gwNO3Conc
CONST_CHARS_LIST VAR_GWNO3[] = {"gwno3", "nitrate N amount in groundwater"}; /// m_gwNO3
CONST_CHARS_LIST VAR_GWWB[] = {"GWWB", "groundwater water balance"};
CONST_CHARS_LIST VAR_GWRQ[] = {"GWRQ", "groundwater recharge to channel or perennial base flow"};
CONST_CHARS_LIST VAR_HCH[] = {"HCH", "Water depth in the downslope boundary of cells"};
CONST_CHARS_LIST VAR_HITARG[] = {"hi_targ", "Harvest index target "}; /// m_HvstIdxTrgt
CONST_CHARS_LIST VAR_HMNTL[] = {"hmntl", "amount of nitrogen moving from active organic to nitrate pool in soil profile on current day in cell"};
CONST_CHARS_LIST VAR_HMPTL[] = {"hmptl", "amount of phosphorus moving from the organic to labile pool in soil profile on current day in cell"};
CONST_CHARS_LIST VAR_HVSTI[] = {"hvsti", "harvest index: crop yield/aboveground biomass"}; /// m_hvstIdx
CONST_CHARS_LIST VAR_HVSTI_ADJ[] = {"hvsti_adj", "optimal harvest index for current time during growing season"}; /// m_hvstIdxAdj
CONST_CHARS_LIST VAR_HVSTI_TARG[] = {"hi_targ", "harvest index target"};
CONST_CHARS_LIST VAR_ID_OUTLET[] = {"ID_OUTLET", "index of outlet in the compressed array"};
CONST_CHARS_LIST VAR_IDC[] = {"IDC", "crop/landcover category"}; /// m_landCoverCls
CONST_CHARS_LIST VAR_IGRO[] = {"IGRO", "Land cover status code"}; /// m_igro
CONST_CHARS_LIST VAR_IGROPT[] = {"igropt", "option for calculating the local specific growth rate of algae"};
CONST_CHARS_LIST VAR_IMPOUND_TRIG[] = {"impound_trig", "release/impound action code"}; /// m_impndTrig
CONST_CHARS_LIST VAR_POT_VOLMAXMM[] = {"pot_volmaxmm", "maximum volume of water stored in the depression/impounded area"}; /// m_potVolMax
CONST_CHARS_LIST VAR_POT_VOLLOWMM[] = {"pot_vollowmm", "lowest volume of water stored in the depression/impounded area"}; /// m_potVolLow
CONST_CHARS_LIST VAR_INET[] = {"INET", "evaporation from the interception storage"}; /// m_IntcpET
CONST_CHARS_LIST VAR_INFIL[] = {"INFIL", "Infiltration"}; /// m_infil
CONST_CHARS_LIST VAR_INFILCAPSURPLUS[] = {"INFILCAPSURPLUS", "surplus of infiltration capacity"};
CONST_CHARS_LIST VAR_INIT_IS[] = {"Init_IS", "Initial interception storage"}; /// m_initIntcpSto
CONST_CHARS_LIST VAR_INLO[] = {"INLO", "Interception loss"}; /// m_intcpLoss
CONST_CHARS_LIST VAR_CANSTOR[] = {"canstor", "amount of water held in canopy storage"}; /// m_canSto
CONST_CHARS_LIST VAR_INTERC_MAX[] = {"Interc_max", "Maximum Interception Storage Capacity"}; /// m_maxIntcpStoCap
CONST_CHARS_LIST VAR_INTERC_MIN[] = {"Interc_min", "Minimum Interception Storage Capacity"}; /// m_minIntcpStoCap
CONST_CHARS_LIST VAR_IRR_FLAG[] = {"irr_flag", "irrigation flag, 1 or 0"}; /// m_irrFlag
CONST_CHARS_LIST VAR_IRR_SURFQ[] = {"irr_surfq", "amount of water from irrigation to become surface runoff"}; /// m_irrWtr2SurfqAmt
CONST_CHARS_LIST VAR_IRR_WTR[] = {"irr_water", "amount of water applied to cell on current day"}; /// m_irrWtrAmt
CONST_CHARS_LIST VAR_ISEP_OPT[] = {"isep_opt", "initial septic operational condition"};
CONST_CHARS_LIST VAR_JULIAN_DAY[] = {"JDay", "Julian day (int)"};
CONST_CHARS_LIST VAR_K_BLOW[] = {"K_blow", "fraction coefficient of precipitation as snow"};
CONST_CHARS_LIST VAR_K_L[] = {"k_l", "half saturation coefficient for light"};
CONST_CHARS_LIST VAR_K_N[] = {"k_n", "half-saturation constant for nitrogen"};
CONST_CHARS_LIST VAR_K_P[] = {"k_p", "half saturation constant for phosphorus"};
CONST_CHARS_LIST VAR_K_PET[] = {"K_pet", "Correction factor for PET"}; /// m_petFactor
CONST_CHARS_LIST VAR_K_RUN[] = {"K_run", "Runoff exponent"}; /// m_rfExp
CONST_CHARS_LIST VAR_K_SOIL10[] = {"k_soil10", "Ratio between soil temperature at 10 cm and the mean"};
CONST_CHARS_LIST VAR_K_SUBLI[] = {"K_subli", "Fraction of PET for sublimation"};
CONST_CHARS_LIST VAR_KG[] = {"Kg", "Baseflow recession coefficient"};
CONST_CHARS_LIST VAR_KI[] = {"Ki", "Interflow scale factor"};
CONST_CHARS_LIST VAR_LAG_SNOW[] = {"lag_snow", "Snow temperature lag factor"};
CONST_CHARS_LIST VAR_LAIDAY[] = {"LAIDAY", "Leaf area index of current day"}; /// m_lai
CONST_CHARS_LIST VAR_LAIINIT[] = {"LAI_INIT", "initial leaf area index of transplants"}; /// m_initLai
CONST_CHARS_LIST VAR_LAIMAXFR[] = {"laimaxfr", "DO NOT KNOW MEANING"}; /// m_laiMaxFr
CONST_CHARS_LIST VAR_LAIMX1[] = {"LAIMX1", "fraction of max LAI corresponding to the 1st point on the optimal leaf area development curve"}; /// m_frMaxLai1stPt
CONST_CHARS_LIST VAR_LAIMX2[] = {"LAIMX2", "fraction of max LAI corresponding to the 2nd point on the optimal leaf area development curve"}; /// m_frMaxLai2ndPt
CONST_CHARS_LIST VAR_LAIPRE[] = {"LAIPRE", "leaf area index for the previous day"};
CONST_CHARS_LIST VAR_LAIYRMAX[] = {"laiyrmax", "maximum LAI for the year"}; /// m_maxLaiYr
CONST_CHARS_LIST VAR_LAMBDA0[] = {"lambda0", "non-algal portion of the light extinction coefficient"};
CONST_CHARS_LIST VAR_LAMBDA1[] = {"lambda1", "linear algal self-shading coefficient"};
CONST_CHARS_LIST VAR_LAMBDA2[] = {"lambda2", "nonlinear algal self-shading coefficient"};
CONST_CHARS_LIST VAR_LANDCOVER[] = {"landcover", "landcover code"}; /// m_landCover
CONST_CHARS_LIST VAR_LANDUSE[] = {"landuse", "landuse code"}; /// m_landUse
CONST_CHARS_LIST VAR_LANDUSE_LOOKUP[] = {"LanduseLookup", "lookup table of landuse"}; /// m_landuseLookup
CONST_CHARS_LIST VAR_LAST_SOILRD[] = {"lastSoilRD", "storing last soil root depth for use in harvestkillop/killop"}; /// m_stoSoilRootD
CONST_CHARS_LIST VAR_LATNO3[] = {"latno3", "amount of nitrate transported with lateral flow"}; /// m_latNO3
CONST_CHARS_LIST VAR_LATNO3_TOCH[] = {"latno3ToCh", "amount of nitrate transported with lateral flow to channel"}; /// m_latNO3ToCh
CONST_CHARS_LIST VAR_LDRAIN[] = {"ldrain", "soil layer where drainage tile is located"}; /// m_drainLyr
CONST_CHARS_LIST VAR_KV_PADDY[] = {"kv_paddy", "volatilization rate constant in impounded water body"};
CONST_CHARS_LIST VAR_KN_PADDY[] = {"kn_paddy", "nitrification rate constant in impounded water body"};
CONST_CHARS_LIST VAR_POT_K[] = {"pot_k", "hydraulic conductivity of soil surface of pothole"};
CONST_CHARS_LIST VAR_MANNING[] = {"Manning", "Manning's roughness"};
CONST_CHARS_LIST VAR_MAT_YRS[] = {"MAT_YRS", "the number of years for the tree species to reach full development"}; /// m_matYrs
CONST_CHARS DESC_MAXCOND = "Maximum stomatal conductance";
CONST_CHARS DESC_METEOLAT = "Latitude of MeteoClimate station";
CONST_CHARS_LIST VAR_MINPGW_TOCH[] = {"minpgwToCh", "soluble P in groundwater to channel"}; // m_gwSolPToCh
CONST_CHARS_LIST VAR_MOIST_IN[] = {"Moist_in", "Initial soil moisture"}; // m_initSoilWtrStoRatio
CONST_CHARS_LIST VAR_MSF[] = {"ManningScaleFactor", "flow velocity scaling factor for calibration"};
CONST_CHARS_LIST VAR_MSK_CO1[] = {"MSK_co1", "Calibration coefficient used to control impact of the storage time constant for normal flow"}; /// m_mskCoef1
//CONST_CHARS_LIST VAR_MSK_CO2[] = {"MSK_co2", "Calibration coefficient used to control impact of the storage time constant fro low flow"};
CONST_CHARS_LIST VAR_MSK_X[] = {"MSK_X", "Weighting factor controlling relative importance of inflow rate and outflow rate in determining water storage in reach segment"}; /// m_mskX
CONST_CHARS_LIST VAR_MUMAX[] = {"mumax", "maximum specific algal growth rate at 20 deg C"};
CONST_CHARS_LIST VAR_NACTFR[] = {"nactfr", "The fraction of organic nitrogen in the nitrogen active pool."}; /// m_orgNFrActN
CONST_CHARS_LIST VAR_NEPR[] = {"NEPR", "Net Precipitation"}; /// m_netPcp
CONST_CHARS_LIST VAR_NFIXCO[] = {"nfixco", "Nitrogen fixation coefficient"}; /// m_NFixCoef
CONST_CHARS_LIST VAR_NFIXMX[] = {"nfixmx", "Maximum daily-N fixation (kg/ha)"}; /// m_NFixMax
CONST_CHARS_LIST VAR_CH_NO3[] = {"CH_NO3", "nitrate in reach"};
CONST_CHARS DESC_NITRITE_CH = "amount of nitrite transported with lateral flow";
CONST_CHARS_LIST VAR_CH_NO3Conc[] = {"CH_NO3Conc", ""};
CONST_CHARS_LIST VAR_NO2_TOCH[] = {"nitriteToCh", ""}; /// m_no2ToCh
CONST_CHARS_LIST VAR_CH_NO2[] = {"CH_NO2", "nitrite in reach"};
CONST_CHARS_LIST VAR_CH_NO2Conc[] = {"CH_NO2Conc", ""};
CONST_CHARS_LIST VAR_DISTSTREAM[] = {"dist2stream", "distance to the stream"}; /// m_distToRch
CONST_CHARS_LIST VAR_NO3GW[] = {"no3gw", "nitrate loading to reach in groundwater"};
CONST_CHARS_LIST VAR_NO3GW_TOCH[] = {"no3gwToCh", "nitrate in groundwater to channel"}; /// m_gwNO3ToCh
CONST_CHARS DESC_NONE = "NO DESCRIPTION";
CONST_CHARS_LIST VAR_NPERCO[] = {"nperco", "nitrate percolation coefficient"};
CONST_CHARS_LIST VAR_NUPDIS[] = {"n_updis", "Nitrogen uptake distribution parameter"}; /// m_upTkDistN
CONST_CHARS_LIST VAR_OL_DET[] = {"DETOverland", "distribution of overland flow detachment"};
CONST_CHARS_LIST VAR_OL_IUH[] = {"Ol_iuh", "IUH of each grid cell"}; /// m_iuhCell
CONST_CHARS_LIST VAR_OL_SED_CCOE[] = {"ccoe", "calibration coefficient of overland flow detachment erosion"};
CONST_CHARS_LIST VAR_OL_SED_ECO1[] = {"eco1", "calibration coefficient 1 of transport capacity calculation"};
CONST_CHARS_LIST VAR_OL_SED_ECO2[] = {"eco2", "calibration coefficient 2 of transport capacity calculation"};
CONST_CHARS_LIST VAR_OLAI[] = {"olai", "DO NOT KNOW MEANING"}; /// m_oLai
CONST_CHARS_LIST VAR_OMEGA[] = {"Omega", "calibration coefficient of splash erosion"};
CONST_CHARS_LIST VAR_CH_ORGN[] = {"CH_ORGN", "organic nitrogen in reach"};
CONST_CHARS_LIST VAR_CH_ORGNConc[] = {"CH_ORGNConc", ""};
CONST_CHARS_LIST VAR_CH_ORGP[] = {"CH_ORGP", "organic phosphorus in reach"};
CONST_CHARS_LIST VAR_CH_ORGPConc[] = {"CH_ORGPConc", ""};
CONST_CHARS_LIST VAR_CH_TN[] = {"CH_TN", "total N amount in reach"};
CONST_CHARS_LIST VAR_CH_TNConc[] = {"CH_TNConc", "total N concentration in reach"};
CONST_CHARS_LIST VAR_CH_TP[] = {"CH_TP", "total P amount in reach"};
CONST_CHARS_LIST VAR_CH_TPConc[] = {"CH_TPConc", "total P concentration in reach"};
CONST_CHARS_LIST VAR_CHSTR_NO3[] = {"CHSTR_NO3", "NO3-N stored in channel"};
CONST_CHARS_LIST VAR_CHSTR_NH4[] = {"CHSTR_NH4", "NH4-N stored in channel"};
CONST_CHARS_LIST VAR_CHSTR_TN[] = {"CHSTR_TN", "total nitrogen stored in channel"};
CONST_CHARS_LIST VAR_CHSTR_TP[] = {"CHSTR_TP", "total phosphrous stored in channel"};
CONST_CHARS_LIST VAR_OUTLETID[] = {"OUTLET_ID", "subbasin ID which outlet located"}; /// m_outletID
CONST_CHARS_LIST VAR_P_MAX[] = {"P_max", "Maximum precipitation corresponding to potential runoff coefficient"}; /// m_maxPcpRf
CONST_CHARS_LIST VAR_P_N[] = {"p_n", "algal preference factor for ammonia"};
CONST_CHARS_LIST VAR_P_RF[] = {"p_rf", "Peak rate adjustment factor"}; /// m_peakRateAdj
CONST_CHARS_LIST VAR_PERCO_N_GW[] = {"perco_n_gw", "amount of nitrate percolating past bottom of soil profile"}; /// m_percoNGw
CONST_CHARS_LIST VAR_PERCO_P_GW[] = {"perco_p_gw", "amount of soluble P percolating past bottom of soil profile"}; /// m_percoPGw
CONST_CHARS_LIST VAR_PERCO[] = {"Perco", "the amount of water percolated from the soil water reservoir, i.e., groundwater recharge"}; /// m_soilPerco
CONST_CHARS_LIST VAR_PERDE[] = {"perde", ""};
CONST_CHARS_LIST VAR_PET[] = {"PET", "Potential Evapotranspiration of day"}; /// m_pet
CONST_CHARS_LIST VAR_PET_HCOEF[] = {"HCoef_pet", "Coefficient related to radiation used in Hargreaves method"};
CONST_CHARS_LIST VAR_PHOSKD[] = {"phoskd", "Phosphorus soil partitioning coefficient"};
CONST_CHARS_LIST VAR_PHUBASE[] = {"PHUBASE", "base zero total heat units (used when no land cover is growing)"}; /// m_phuBase
CONST_CHARS_LIST VAR_PHUPLT[] = {"PHU_PLT", "total number of heat unites (hours) needed to bring plant to maturity"}; /// m_phuPlt
CONST_CHARS_LIST VAR_PHUTOT[] = {"PHU0", "annual average total potential heat units (used when no crop is growing)"}; /// m_phuAnn
CONST_CHARS_LIST VAR_PI_B[] = {"Pi_b", "Interception Storage Capacity Exponent"}; /// m_intcpStoCapExp
CONST_CHARS_LIST VAR_PCP2CANFR_PR[] = {"pcp2canfr_pr", "fraction of precipitation falling down to canal"}; /// m_pcp2CanalFr
CONST_CHARS_LIST VAR_EMBNKFR_PR[] = {"embnkfr_pr", ""}; /// m_embnkFr
CONST_CHARS_LIST VAR_PL_RSDCO[] = {"rsdco_pl", "Plant residue decomposition coefficient"}; /// m_pltRsdDecCoef
CONST_CHARS_LIST VAR_PLANT_N[] = {"plant_N", "amount of nitrogen in plant biomass (kg/ha), plantn in SWAT"}; /// m_pltN
CONST_CHARS_LIST VAR_PLANT_P[] = {"plant_P", "amount of phosphorus in plant biomass (kg/ha), plantp in SWAT"}; /// m_pltP
CONST_CHARS_LIST VAR_PLTET_TOT[] = {"plt_et_tot", "actual ET simulated during life of plant"}; /// m_totActPltET
CONST_CHARS_LIST VAR_PLTPET_TOT[] = {"plt_pet_tot", "potential ET simulated during life of plant"}; /// m_totPltPET
CONST_CHARS_LIST VAR_POREIDX[] = {"Poreindex", "pore size distribution index"}; /// m_poreIdx
CONST_CHARS_LIST VAR_POROST[] = {"Porosity", "soil porosity"}; /// m_soilPor
CONST_CHARS_LIST VAR_POT_NO3[] = {"pot_no3", "amount of nitrate in pothole water body"}; /// m_potNo3
CONST_CHARS_LIST VAR_POT_NH4[] = {"pot_nh4", "amount of ammonian in pothole water body"};
CONST_CHARS_LIST VAR_POT_ORGN[] = {"pot_orgn", "amount of organic N in pothole water body"};
CONST_CHARS_LIST VAR_POT_SOLP[] = {"pot_solp", "soluble P amount in pothole water body"}; /// m_potSolP
CONST_CHARS_LIST VAR_POT_ORGP[] = {"pot_orgp", "amount of organic P in pothole water body"};
CONST_CHARS_LIST VAR_POT_AMINP[] = {"pot_aminp", "amount of active mineral pool P in pothole water body"};
CONST_CHARS_LIST VAR_POT_SMINP[] = {"pot_sminp", "amount of stable mineral pool P in pothole water body"};
CONST_CHARS_LIST VAR_POT_SED[] = {"pot_sed", "amount of sediment in pothole water body"};
CONST_CHARS_LIST VAR_POT_VOL[] = {"pot_vol", "current volume of water stored in the depression/impounded area"}; /// m_potVol
CONST_CHARS_LIST VAR_POT_SA[] = {"pot_sa", "surface area of impounded area"}; /// m_potArea
CONST_CHARS_LIST VAR_POT_FLOWIN[] = {"pot_flowin", "water entering pothole on day"};
CONST_CHARS_LIST VAR_POT_FLOWOUT[] = {"pot_flowout", "discharge from pothole expressed as depth"};
CONST_CHARS_LIST VAR_POT_SEDIN[] = {"pot_sedin", "sediment entering pothole on day"};
CONST_CHARS_LIST VAR_POT_SEDOUT[] = {"pot_sedout", "sediment leaving pothole on day"};
CONST_CHARS_LIST VAR_PPERCO[] = {"pperco", "phosphorus percolation coefficient"};
CONST_CHARS_LIST VAR_PPT[] = {"PPT", "maximum amount of transpiration (plant et)"}; /// m_maxPltET
CONST_CHARS_LIST VAR_PSP[] = {"psp", "Phosphorus availability index"}; /// m_phpSorpIdxBsn
CONST_CHARS_LIST VAR_PTTN2CH[] = {"ptTNToCh", "total nitrogen loaded from point sources"};
CONST_CHARS_LIST VAR_PTTP2CH[] = {"ptTPToCh", "total phosphrus loaded from point sources"};
CONST_CHARS_LIST VAR_PTCOD2CH[] = {"ptCODToCh", "total COD loaded from point sources"};
CONST_CHARS_LIST VAR_PUPDIS[] = {"p_updis", "Phosphorus uptake distribution parameter"}; /// m_upTkDistP
CONST_CHARS_LIST VAR_QCH[] = {"QCH", "Flux in the downslope boundary of cells"};
CONST_CHARS_LIST VAR_OLFLOW[] = {"OL_Flow", "overland flow in each cell calculated during overland routing"}; /// m_surfRf
CONST_CHARS_LIST VAR_QG[] = {"QG", "Groundwater discharge at each reach outlet"}; /// m_qgRchOut
CONST_CHARS_LIST VAR_QI[] = {"QI", "Interflow at each reach outlet"}; /// m_qiRchOut
CONST_CHARS_LIST VAR_QOVERLAND[] = {"QOverland", "Water discharge in the downslope boundary of cells"};
CONST_CHARS_LIST VAR_QRECH[] = {"QRECH", "Discharge at each reach outlet of each time step"}; /// m_qRchOut
CONST_CHARS_LIST VAR_QS[] = {"QS", "Overland discharge at each reach outlet"}; /// m_qsRchOut
CONST_CHARS_LIST VAR_QSOIL[] = {"QSoil", "discharge added to channel flow from interflow"};
CONST_CHARS_LIST VAR_QSUBBASIN[] = {"QSUBBASIN", "discharge at each subbasin outlet"};
CONST_CHARS_LIST VAR_QTILE[] = {"qtile", "drainage tile flow in soil profile"};
CONST_CHARS_LIST VAR_QTOTAL[] = {"QTotal", "discharge at the watershed outlet"};
CONST_CHARS_LIST VAR_RadianSlope[] = {"RadianSlope", "radian slope"};
CONST_CHARS_LIST VAR_RCA[] = {"rca", "concentration of ammonia in the rain"}; /// m_rainNH4Conc
CONST_CHARS_LIST VAR_CH_COD[] = {"CH_COD", "carbonaceous oxygen demand in reach"};
CONST_CHARS_LIST VAR_CH_CODConc[] = {"CH_CODConc", ""};
CONST_CHARS_LIST VAR_CH_DOX[] = {"ch_dox", "dissolved oxygen in reach"};
CONST_CHARS_LIST VAR_CH_DOXConc[] = {"ch_doxConc", "" };
CONST_CHARS_LIST VAR_RCH_BANKERO[] = {"rch_bank_ero", "reach bank erosion"}; /// m_rchBankEro
CONST_CHARS_LIST VAR_RCH_DEG[] = {"rch_deg", "reach degradation"}; /// m_rchDeg
CONST_CHARS_LIST VAR_RCH_DEP[] = {"rch_dep", "reach deposition"}; /// m_rchDep
CONST_CHARS_LIST VAR_RCH_DEPNEW[] = {"rch_depnew", "Channel new deposition"}; /// m_dltRchDep
CONST_CHARS_LIST VAR_RCH_DEPSAND[] = {"rch_depsand", "Sand deposition in channel"}; /// m_rchDepSand
CONST_CHARS_LIST VAR_RCH_DEPSILT[] = {"rch_depsilt", "Silt deposition in channel"}; /// m_rchDepSilt
CONST_CHARS_LIST VAR_RCH_DEPCLAY[] = {"rch_depclay", "Clay deposition in channel" }; /// m_rchDepClay
CONST_CHARS_LIST VAR_RCH_DEPSAG[] = {"rch_depsag", "Small aggregate deposition in channel"}; /// m_rchDepSag
CONST_CHARS_LIST VAR_RCH_DEPLAG[] = {"rch_deplag", "Large aggregate deposition in channel"}; /// m_rchDepLag
CONST_CHARS_LIST VAR_RCH_DEPGRAVEL[] = {"rch_depgravel", "Gravel deposition in channel"}; /// m_rchDepGravel
CONST_CHARS_LIST VAR_FLDPLN_DEP[] = {"floodplain_dep", "Floodplain Deposition"}; /// m_fldplnDep
CONST_CHARS_LIST VAR_FLDPLN_DEPNEW[] = {"floodplain_depnew", "New deposits on floodplain"}; /// m_dltFldplnDep
CONST_CHARS_LIST VAR_FLDPLN_DEPSILT[] = {"floodplain_depsilt", "Deposition silt on floodplain"}; /// m_fldplnDepSilt
CONST_CHARS_LIST VAR_FLDPLN_DEPCLAY[] = {"floodplain_depclay", "Deposition clay on floodplain"}; /// m_fldplnDepClay
CONST_CHARS_LIST VAR_RCN[] = {"rcn", "concentration of nitrate in the rain"}; /// m_rainNO3Conc
CONST_CHARS_LIST VAR_Reinfiltration[] = {"Reinfiltration", "Reinfiltration" };
CONST_CHARS_LIST VAR_RETURNFLOW[] = {"ReturnFlow", "water depth of return flow"};
CONST_CHARS_LIST VAR_REVAP[] = {"Revap", "revaporization from groundwater to the last soil layer"};
CONST_CHARS_LIST VAR_RG[] = {"RG", "groundwater runoff"};
CONST_CHARS DESC_RM = "Relative humidity";
CONST_CHARS_LIST VAR_RHOQ[] = {"rhoq", "algal respiration rate at 20 deg C"};
CONST_CHARS_LIST VAR_RMN2TL[] = {"rmn2tl", "amount of nitrogen moving from the fresh organic (residue) to the nitrate(80%) and active organic(20%) pools in soil profile on current day in cell"};
CONST_CHARS_LIST VAR_RMP1TL[] = {"rmp1tl", "amount of phosphorus moving from the labile mineral pool to the active mineral pool in the soil profile on the current day in cell"};
CONST_CHARS_LIST VAR_RMPTL[] = {"rmptl", "amount of phosphorus moving from the fresh organic (residue) to the labile(80%) and organic(20%) pools in soil profile on current day in cell"};
CONST_CHARS_LIST VAR_RNUM1[] = {"rnum1", "fraction of overland flow"};
CONST_CHARS_LIST VAR_ROCK[] = {"rock", "Percent of rock content"}; /// m_soilRock
CONST_CHARS_LIST VAR_ROCTL[] = {"roctl", "amount of phosphorus moving from the active mineral pool to the stable mineral pool in the soil profile on the current day in cell"};
CONST_CHARS_LIST VAR_ROOTDEPTH[] = {"rootdepth", "root depth of plants (mm)"}; /// m_pltRootD
CONST_CHARS_LIST VAR_RTE_WTRIN[] = {"rtwtr_in", "water flow in reach on day before channel routing"}; /// m_rteWtrIn
CONST_CHARS_LIST VAR_RTE_WTROUT[] = {"rtwtr", "water leaving reach on day after channel routing"}; /// m_rteWtrOut
CONST_CHARS_LIST VAR_RUNOFF_CO[] = {"Runoff_co", "Potential runoff coefficient"}; /// m_potRfCoef
CONST_CHARS_LIST VAR_RWNTL[] = {"rwntl", "amount of nitrogen moving from active organic to stable organic pool in soil profile on current day in cell"};
CONST_CHARS_LIST VAR_S_FROZEN[] = {"s_frozen", "Frozen moisture relative to porosity with no infiltration"}; /// m_soilFrozenWtrRatio
CONST_CHARS_LIST VAR_SAND[] = {"sand", "Percent of sand content"}; /// m_soilSand
CONST_CHARS_LIST VAR_SBGS[] = {"SBGS", "Groundwater storage of the subbasin"}; /// m_gwSto
CONST_CHARS_LIST VAR_SBIF[] = {"SBIF", "Subsurface volume (m3) to streams from each subbasin"}; /// m_ifluQ2Rch
CONST_CHARS_LIST VAR_SBOF[] = {"SBOF", "overland flow to streams from each subbasin"}; /// m_olQ2Rch
CONST_CHARS_LIST VAR_SBPET[] = {"SBPET", "the potential evapotranspiration rate of the subbasin"}; /// m_petSubbsn
CONST_CHARS_LIST VAR_SBQG[] = {"SBQG", "groundwater flow out of the subbasin"}; /// m_gndQ2Rch
CONST_CHARS_LIST VAR_SCENARIO[] = {"SCENARIO", "BMPs scenario information"};
CONST_CHARS_LIST VAR_SDNCO[] = {"sdnco", "denitrification threshold: fraction of field capacity"}; ///
CONST_CHARS_LIST VAR_SED_DEP[] = {"SEDDEP", "distribution of sediment deposition"};
CONST_CHARS_LIST VAR_SED_FLOW[] = {"sed_flow", "sediment in flow"};
CONST_CHARS_LIST VAR_SED_FLUX[] = {"sed_flux", "outgoing sediment flux"};
CONST_CHARS_LIST VAR_SED_RECH[] = {"SEDRECH", "Sediment output at reach outlet"}; /// m_sedRchOut
CONST_CHARS_LIST VAR_SED_RECHConc[] = {"SEDRECHConc", ""}; /// m_sedConcRchOut
CONST_CHARS_LIST VAR_SAND_RECH[] = {"SandRchOut", "Sand output at reach outlet"}; /// m_sandRchOut
CONST_CHARS_LIST VAR_SILT_RECH[] = {"SiltRchOut", "Silt output at reach outlet"}; /// m_siltRchOut
CONST_CHARS_LIST VAR_CLAY_RECH[] = {"ClayRchOut", "Clay output at reach outlet"}; /// m_clayRchOut
CONST_CHARS_LIST VAR_SAG_RECH[] = {"SagRchOut", "Small aggregate output at reach outlet"}; /// m_sagRchOut
CONST_CHARS_LIST VAR_LAG_RECH[] = {"LagRchOut", "Large aggregate output at reach outlet"}; /// m_lagRchOut
CONST_CHARS_LIST VAR_GRAVEL_RECH[] = {"GravelRchOut", "Gravel output at reach outlet"}; /// m_gravelRchOut
CONST_CHARS_LIST VAR_SED_TO_CH[] = {"SEDTOCH", "Sediment flowing to channel by hillslope routing"}; /// m_sedtoCh
CONST_CHARS_LIST VAR_SAND_TO_CH[] = {"SandToCh", "Sand flowing to channel by hillslope routing"}; /// m_sandtoCh
CONST_CHARS_LIST VAR_SILT_TO_CH[] = {"SiltToCh", "Silt flowing to channel by hillslope routing"}; /// m_silttoCh
CONST_CHARS_LIST VAR_CLAY_TO_CH[] = {"ClayToCh", "Clay flowing to channel by hillslope routing"}; /// m_claytoCh
CONST_CHARS_LIST VAR_SAG_TO_CH[] = {"SagToCh", "Small aggregate flowing to channel by hillslope routing"}; /// m_sagtoCh
CONST_CHARS_LIST VAR_LAG_TO_CH[] = {"LagToCh", "Large aggregate flowing to channel by hillslope routing"}; /// m_lagtoCh
CONST_CHARS_LIST VAR_GRAVEL_TO_CH[] = {"GravelToCh", "Gravel flowing to channel by hillslope routing"}; /// m_graveltoCh
CONST_CHARS_LIST VAR_SEDSTO_CH[] = {"SedStorageCH", "Channel sediment storage (kg)"}; /// m_sedSto
CONST_CHARS_LIST VAR_SANDSTO_CH[] = {"SandStorageCH", "Channel sand storage (kg)"}; /// m_sandSto
CONST_CHARS_LIST VAR_SILTSTO_CH[] = {"SiltStorageCH", "Channel silt storage (kg)"}; /// m_siltSto
CONST_CHARS_LIST VAR_CLAYSTO_CH[] = {"ClayStorageCH", "Channel clay storage (kg)"}; /// m_claySto
CONST_CHARS_LIST VAR_SAGSTO_CH[] = {"SagStorageCH", "Channel small aggregate storage (kg)"}; /// m_sagSto
CONST_CHARS_LIST VAR_LAGSTO_CH[] = {"LagStorageCH", "Channel large aggregate storage (kg)"}; /// m_lagSto
CONST_CHARS_LIST VAR_GRAVELSTO_CH[] = {"GravelStorageCH", "Channel gravel storage (kg)"}; /// m_gravelSto
CONST_CHARS_LIST VAR_SEDYLD[] = {"SED_OL", "sediment yield that transported to channel at each cell"}; /// m_olWtrEroSed
CONST_CHARS_LIST VAR_SEDMINPA[] = {"sedminpa", " amount of active mineral phosphorus absorbed to sediment in surface runoff"}; /// m_surfRfSedAbsorbMinP
CONST_CHARS_LIST VAR_SEDMINPA_TOCH[] = {"sedminpaToCh", "amount of active mineral phosphorus absorbed to sediment in surface runoff moved to channel"}; /// m_surfRfSedAbsorbMinPToCh
CONST_CHARS_LIST VAR_SEDMINPS[] = {"sedminps", "amount of stable mineral phosphorus sorbed to sediment in surface runoff"}; /// m_surfRfSedSorbMinP
CONST_CHARS_LIST VAR_SEDMINPS_TOCH[] = {"sedminpsToCh", "amount of stable mineral phosphorus sorbed to sediment in surface runoff moved to channel"}; /// m_surfRfSedSorbMinPToCh
CONST_CHARS_LIST VAR_SEDORGN[] = {"sedorgn", "amount of organic nitrogen in surface runoff"}; /// m_surfRfSedOrgN
CONST_CHARS_LIST VAR_SEDORGN_TOCH[] = {"sedorgnToCh", "amount of organic nitrogen in surface runoff moved to channel"}; /// m_surfRfSedOrgNToCh
CONST_CHARS_LIST VAR_SEDORGP[] = {"sedorgp", "amount of organic phosphorus in surface runoff"}; /// m_surfRfSedOrgP
CONST_CHARS_LIST VAR_SEDORGP_TOCH[] = {"sedorgpToCh", "amount of organic phosphorus in surface runoff moved to channel"}; /// m_surfRfSedOrgPToCh
CONST_CHARS_LIST VAR_SEEPAGE[] = {"SEEPAGE", "seepage"};
CONST_CHARS_LIST VAR_SHALLST[] = {"shallst", "depth of water in shallow aquifer"};
CONST_CHARS_LIST VAR_SILT[] = {"silt", "Percent of silt content"}; /// m_soilSilt
CONST_CHARS_LIST VAR_SLOPE[] = {"slope", "Slope gradient (drop/distance, i.e., tan, or percent)"}; /// m_slope
CONST_CHARS_LIST VAR_SLPLEN[] = {"slope_length", "Slope length"};
CONST_CHARS_LIST VAR_SNAC[] = {"SNAC", "snow accumulation"}; /// m_snowAccum
CONST_CHARS_LIST VAR_SNME[] = {"SNME", "snow melt"}; /// m_snowMelt
CONST_CHARS_LIST VAR_SNO3UP[] = {"sno3up", "amount of nitrate moving upward in the soil profile in watershed"};
CONST_CHARS_LIST VAR_SNOCOVMX[] = {"SNOCOVMX", "Minimum snow water content that corresponds to 100% snow cover"};
CONST_CHARS_LIST VAR_SNO50COV[] = {"SNO50COV", "Fraction of SNOCOVMX that corresponds to 50% snow cover"};
CONST_CHARS_LIST VAR_SNRD[] = {"SNRD", "snow blowing in or out the cell"};
CONST_CHARS_LIST VAR_SNSB[] = {"SNSB", "snow sublimation (water equivalent)"}; /// m_snowSublim
CONST_CHARS_LIST VAR_SNWB[] = {"SNWB", "snow water balance for selected subbasins"};
CONST_CHARS_LIST VAR_SOER[] = {"SOER", "soil loss caused by water erosion"}; /// m_eroSed
CONST_CHARS_LIST VAR_SOET[] = {"SOET", "evaporation from the soil water storage"}; /// m_soilET
CONST_CHARS_LIST VAR_SOIL_T10[] = {"soil_t10", "Factor of soil temperature relative to short grass (degree)"}; /// m_soilTempRelFactor10
CONST_CHARS_LIST VAR_SOILDEPTH[] = {"soilDepth", "depth to bottom of each soil layer"}; /// m_soilDepth
CONST_CHARS_LIST VAR_SOILLAYERS[] = {"soillayers", "Soil layers number"}; /// m_nSoilLyrs
CONST_CHARS_LIST VAR_SOILTHICK[] = {"soilthick", "soil thickness of each soil layer"}; /// m_soilThk
CONST_CHARS_LIST VAR_SOL_ACTP[] = {"sol_actp", "amount of phosphorus stored in the active mineral phosphorus pool"}; /// m_soilActvMinP
CONST_CHARS_LIST VAR_SOL_ALB[] = {"sol_alb", "albedo when soil is moist"}; /// m_soilAlb
CONST_CHARS_LIST VAR_SOL_AORGN[] = {"sol_aorgn", "amount of nitrogen stored in the active organic (humic) nitrogen pool"}; /// m_soilActvOrgN
CONST_CHARS_LIST VAR_SOL_AWC[] = {"sol_awc", "amount of water available to plants in soil layer at field capacity (AWC=FC-WP)"}; /// m_soilFC
CONST_CHARS_LIST VAR_SOL_BD[] = {"density", "bulk density of the soil"}; /// m_soilBD
CONST_CHARS_LIST VAR_SOL_CBN[] = {"sol_cbn", "soil carbon content"}; /// m_soilCbn
CONST_CHARS_LIST VAR_SOL_COV[] = {"sol_cov", "amount of residue on soil surface"}; /// m_rsdCovSoil
CONST_CHARS_LIST VAR_SOL_CRK[] = {"sol_crk", "crack volume potential of soil"}; /// m_soilCrk
CONST_CHARS_LIST VAR_SOL_FORGN[] = {"sol_fon", "amount of nitrogen stored in the fresh organic (residue) pool"}; /// m_soilFrshOrgN
CONST_CHARS_LIST VAR_SOL_FORGP[] = {"sol_fop", "amount of phosphorus stored in the fresh organic (residue) pool"}; /// m_soilFrshOrgP
CONST_CHARS_LIST VAR_SOL_MC[] = {"sol_mc", "manure carbon in soil"}; /// m_soilManC
CONST_CHARS_LIST VAR_SOL_MN[] = {"sol_mn", "manure nitrogen in soil"}; /// m_soilManN
CONST_CHARS_LIST VAR_SOL_MP[] = {"sol_mp", "manure phosphorus in soil"}; /// m_soilManP
CONST_CHARS_LIST VAR_SOL_N[] = {"sol_N", "soil organic nitrogen, include nitrogen in manure"}; /// m_soilN

/// define rice related parameters, used by PG_ORYZA module, by Fang Shen
CONST_CHARS_LIST VAR_CROPSTA[] = {"cropsta", "rice status code"};
CONST_CHARS_LIST VAR_TBD[] = {"tbd", "Base temperature for development"};
CONST_CHARS_LIST VAR_TOD[] = {"tod", "Optimum temperature for development"};
CONST_CHARS_LIST VAR_TMD[] = {"tmd", "Maximum temperature for development"};
CONST_CHARS_LIST VAR_DVRJ[] = {"dvrj", "Development rate during juvenile phase"};
CONST_CHARS_LIST VAR_DVRI[] = {"dvri", "Development rate during photoperiod-sensitive phase"};
CONST_CHARS_LIST VAR_DVRP[] = {"dvrp", "Development rate during panicle development phase"};
CONST_CHARS_LIST VAR_DVRR[] = {"dvrr", "Development rate in reproductive phase (post anthesis)"};
CONST_CHARS_LIST VAR_MOPP[] = {"mopp", "Maximum optimum photoperiod"};
CONST_CHARS_LIST VAR_PPSE[] = {"ppse", "Photoperiod sensitivity"};
CONST_CHARS_LIST VAR_SHCKD[] = {"shckd", "Relation between seedling age and delay in phenological development"};
CONST_CHARS_LIST VAR_KNF[] = {"knf", "extinction coefficient of N profile in the canopy as a function of development stage"};
CONST_CHARS_LIST VAR_RGRLMX[] = {"rgrlMX", "Maximum relative growth rate of leaf area"};
CONST_CHARS_LIST VAR_RGRLMN[] = {"rgrgMN", "Minimum relative growth rate of leaf area"};
CONST_CHARS_LIST VAR_NH[] = {"nh", "Number of hills"};
CONST_CHARS_LIST VAR_NPLH[] = {"nplh", "Number of plants per hill"};
CONST_CHARS_LIST VAR_NPLSB[] = {"nplsb", "Number of plants in seedbed"};
CONST_CHARS_LIST VAR_LAPE[] = {"lape", "Leaf area per plant at emergence"};
CONST_CHARS_LIST VAR_ZRTTR[] = {"zrttr", "Root length/depth at transplanting"};
CONST_CHARS_LIST VAR_TMPSB[] = {"tmpsb", "Temperature increase caused by greenhouse use (over seedbed)"};
CONST_CHARS_LIST VAR_AFSH[] = {"aFsh", "function parameters of fraction shoot dry matter partitioned to the leaves according to DVS"};
CONST_CHARS_LIST VAR_BFSH[] = {"bFsh", "function parameters of fraction shoot dry matter partitioned to the leaves according to DVS"};
CONST_CHARS_LIST VAR_AFLV[] = {"aFlv", "function parameters of fraction total dry matter partitioned to the shoot according to DVS"};
CONST_CHARS_LIST VAR_BFLV[] = {"bFlv", "function parameters of fraction total dry matter partitioned to the shoot according to DVS"};
CONST_CHARS_LIST VAR_AFSO[] = {"aFso", "function parameters of fraction shoot dry matter partitioned to the panicles according to DVS"};
CONST_CHARS_LIST VAR_BFSO[] = {"bFso", "function parameters of fraction shoot dry matter partitioned to the panicles according to DVS"};
CONST_CHARS_LIST VAR_ADRLV[] = {"aDrlv", "function parameters of leaf death coefficient according to DVS"};
CONST_CHARS_LIST VAR_BDRLV[] = {"bDrlv", "function parameters of leaf death coefficient according to DVS"};
CONST_CHARS_LIST VAR_TCLSTR[] = {"tclstr", "Time coefficient for loss of stem reserves"};
CONST_CHARS_LIST VAR_Q10[] = {"q10", "Factor accounting for increase in maintenance respiration with a 10 oC rise in temperature"};
CONST_CHARS_LIST VAR_TREF[] = {"tref", "Reference temperature"};
CONST_CHARS_LIST VAR_MAINLV[] = {"mainLV", "Maintenance respiration coefficient:Leaves"};
CONST_CHARS_LIST VAR_MAINST[] = {"mainST", "Maintenance respiration coefficient:Stems"};
CONST_CHARS_LIST VAR_MAINSO[] = {"mainSO", "Maintenance respiration coefficient:Storage organs (panicles)"};
CONST_CHARS_LIST VAR_MAINRT[] = {"mainRT", "Maintenance respiration coefficient:Roots"};
CONST_CHARS_LIST VAR_CRGLV[] = {"crgLV", "Carbohydrate requirement for dry matter production:Leaves"};
CONST_CHARS_LIST VAR_CRGST[] = {"crgST", "Carbohydrate requirement for dry matter production:Stems"};
CONST_CHARS_LIST VAR_CRGSTR[] = {"crgSTR", "Carbohydrate requirement for dry matter production:Stem reserves"};
CONST_CHARS_LIST VAR_CRGSO[] = {"crgSO", "Carbohydrate requirement for dry matter production:Storage organs (panicles)"};
CONST_CHARS_LIST VAR_CRGRT[] = {"crgRT", "Carbohydrate requirement for dry matter production:Roots"};
CONST_CHARS_LIST VAR_FSTR[] = {"fstr", "Fraction of carbohydrates allocated to stems that is stored as reserves"};
CONST_CHARS_LIST VAR_LRSTR[] = {"lrstr", "Fraction of allocated stem reserves that is available for growth"};
CONST_CHARS_LIST VAR_ASLA[] = {"aSLA", "SLA function parameters"};
CONST_CHARS_LIST VAR_BSLA[] = {"bSLA", "SLA function parameters"};
CONST_CHARS_LIST VAR_CSLA[] = {"cSLA", "SLA function parameters"};
CONST_CHARS_LIST VAR_DSLA[] = {"dSLA", "SLA function parameters"};
CONST_CHARS_LIST VAR_SLAMX[] = {"slaMX", "maximum value of SLA"};
CONST_CHARS_LIST VAR_FCRT[] = {"fcRT", "Carbon balance parameters, Mass fraction carbon:Roots"};
CONST_CHARS_LIST VAR_FCST[] = {"fcST", "Carbon balance parameters, Mass fraction carbon:Stems"};
CONST_CHARS_LIST VAR_FCLV[] = {"fcLV", "Carbon balance parameters, Mass fraction carbon:Leaves"};
CONST_CHARS_LIST VAR_FCSTR[] = {"fcSTR", "Carbon balance parameters, Mass fraction carbon:Stem reserves"};
CONST_CHARS_LIST VAR_FCSO[] = {"fcSO", "Carbon balance parameters, Mass fraction carbon:Storage organs (panicles)"};
CONST_CHARS_LIST VAR_WGRMX[] = {"wgrMX", "Maximum individual grain weight"};
CONST_CHARS_LIST VAR_GZRT[] = {"gzrt", "Growth rate of roots"};
CONST_CHARS_LIST VAR_ZRTMCD[] = {"zrtMCD", "Maximum depth of roots if drought"};
CONST_CHARS_LIST VAR_FRPAR[] = {"frpar", "Fraction of total shortwave irradiation that is photo-synthetically active (PAR)"};
CONST_CHARS_LIST VAR_SPGF[] = {"spgf", "Spikelet growth factor"};
CONST_CHARS_LIST VAR_NMAXL[] = {"nmaxl", "function parameters of maximum leaf N fraction"};
CONST_CHARS_LIST VAR_NMINL[] = {"nminl", "function parameters of minimum leaf N fraction"};
CONST_CHARS_LIST VAR_RFNLV[] = {"rfnlv", "Residual N fraction of leaves (kg N kg-1 leaves)"};
CONST_CHARS_LIST VAR_RFNST[] = {"rfnst", "Residual N fraction of stems (kg N kg-1 stems)"};
CONST_CHARS_LIST VAR_RFNRT[] = {"rfnrt", "Fraction N translocation from roots as (additonal) fraction of total N translocation from stems and leaves"};
CONST_CHARS_LIST VAR_TCNTRF[] = {"tcntrf", "Time coefficient for N translocation to grains"};
CONST_CHARS_LIST VAR_NMAXSO[] = {"nmaxso", "Maximum N concentration in storage organs"};
CONST_CHARS_LIST VAR_ANMINSO[] = {"anminso", "function parameters of minimum N concentration in storage organs"};
CONST_CHARS_LIST VAR_BNMINSO[] = {"bnminso", "function parameters of minimum N concentration in storage organs"};
CONST_CHARS_LIST VAR_SHCKL[] = {"shckl", "Relation between seedling age and delay in leaf area development"};
CONST_CHARS_LIST VAR_SBDUR[] = {"sbdur", "Duration of seedbed"};
CONST_CHARS_LIST VAR_LLLS[] = {"llls", "Lower limit leaf rolling (kPa)"};
CONST_CHARS_LIST VAR_ULLS[] = {"ulls", "Upper limit leaf rolling (kPa)"};
CONST_CHARS_LIST VAR_LLLE[] = {"llle", "Lower limit leaf expansion (kPa)"};
CONST_CHARS_LIST VAR_ULLE[] = {"ulle", "Upper limit leaf expansion (kPa)"};
CONST_CHARS_LIST VAR_LLDL[] = {"lldl", "Lower limit death of leaves (kPa)"};
CONST_CHARS_LIST VAR_ULDL[] = {"uldl", "Upper limit death of leaves (kPa)"};
CONST_CHARS_LIST VAR_TS[] = {"ts", "Temperature sum"};
CONST_CHARS_LIST VAR_WLVG[] = {"wlvg", "Dry weight of green leaves  kg / ha"};
CONST_CHARS_LIST VAR_WLVD[] = {"wlvd", "Dry weight of dead leaves"};
CONST_CHARS_LIST VAR_WSTS[] = {"wsts", "dry weight of stems reserves"};
CONST_CHARS_LIST VAR_WSTR[] = {"wstr", "dry weight of structural stems"};
CONST_CHARS_LIST VAR_WSO[] = {"wso", "dry weight of storage organs"};
CONST_CHARS_LIST VAR_WRT[] = {"wrt", "Dry weight of roots"};
CONST_CHARS_LIST VAR_WRR[] = {"wrr", "Dry weight of rough rice (final yield)"};
CONST_CHARS_LIST VAR_NGR[] = {"ngr", "Number of grains"};
CONST_CHARS_LIST VAR_NSP[] = {"nsp", "Number of spikelets"};
CONST_CHARS_LIST VAR_TNASS[] = {"tnass", "Total net CO2 assimilation  kg CO2 ha-1"};
CONST_CHARS_LIST VAR_WST[] = {"wst", "dry weight of stems"};
CONST_CHARS_LIST VAR_WLV[] = {"wlv", "Dry weight of leaves"};
CONST_CHARS_LIST VAR_WAGT[] = {"wagt", "Total aboveground dry matter"};
CONST_CHARS_LIST VAR_ZRT[] = {"zrt", "root length or root depth"};
CONST_CHARS_LIST VAR_DVS[] = {"dvs", "Development stage of the crop"};
CONST_CHARS_LIST VAR_ANCRF[] = {"ancrf", "Amount of N in crop till flowering"};

/// pond, figure out if pond and pothole can be share these names. By liangjun.
CONST_CHARS_LIST VAR_POND[] = {"pond", "pond id"};
CONST_CHARS_LIST VAR_POND_VOL[] = {"pond_vol", "pond volumn"};
CONST_CHARS_LIST VAR_POND_SA[] = {"pondSurfaceArea", "pond surface area"};
CONST_CHARS_LIST VAR_IRRDEPTH[] = {"irrDepth", ""};
CONST_CHARS_LIST VAR_POND_SOLPDECAY[] = {"pond_solpl", ""};

/// CENTURY model for C/N cycling
CONST_CHARS_LIST VAR_SOL_BMC[] = {"sol_BMC", "NEED to figure out"};
CONST_CHARS_LIST VAR_SOL_BMN[] = {"sol_BMN", "NEED to figure out"};
CONST_CHARS_LIST VAR_SOL_HSC[] = {"sol_HSC", "mass of C present in slow humus"};
CONST_CHARS_LIST VAR_SOL_HSN[] = {"sol_HSN", "mass of N present in slow humus"};
CONST_CHARS_LIST VAR_SOL_HPC[] = {"sol_HPC", "mass of C present in passive humus"};
CONST_CHARS_LIST VAR_SOL_HPN[] = {"sol_HPN", "mass of N present in passive humus"};
CONST_CHARS_LIST VAR_SOL_LM[] = {"sol_LM", "mass of metabolic litter"};
CONST_CHARS_LIST VAR_SOL_LMC[] = {"sol_LMC", "metabolic litter C pool"};
CONST_CHARS_LIST VAR_SOL_LMN[] = {"sol_LMN", "metabolic litter N pool"};
CONST_CHARS_LIST VAR_SOL_LS[] = {"sol_LS", "structural litter SOM pool"};
CONST_CHARS_LIST VAR_SOL_LSL[] = {"sol_LSL", "lignin weight in structural litter"};
CONST_CHARS_LIST VAR_SOL_LSC[] = {"sol_LSC", "structural litter C pool"};
CONST_CHARS_LIST VAR_SOL_LSN[] = {"sol_LSN", "structural litter N pool"};
CONST_CHARS_LIST VAR_SOL_RNMN[] = {"sol_RNMN", "NEED to figure out"};
CONST_CHARS_LIST VAR_SOL_LSLC[] = {"sol_LSLC", "lignin amount in structural litter pool"};
CONST_CHARS_LIST VAR_SOL_LSLNC[] = {"sol_LSLNC", "non-lignin part of the structural litter C"};
CONST_CHARS_LIST VAR_SOL_RSPC[] = {"sol_RSPC", "NEED to figure out"};
CONST_CHARS_LIST VAR_SOL_WOC[] = {"sol_WOC", "NEED to figure out"};
CONST_CHARS_LIST VAR_SOL_WON[] = {"sol_WON", "NEED to figure out"};
CONST_CHARS_LIST VAR_SOL_HP[] = {"sol_HP", "mass of OM in passive humus"};
CONST_CHARS_LIST VAR_SOL_HS[] = {"sol_HS", "mass of OM in slow humus"};
CONST_CHARS_LIST VAR_SOL_BM[] = {"sol_BM", "NEED to figure out"};

CONST_CHARS_LIST VAR_SOL_LATERAL_C[] = {"sol_latc", "lateral flow Carbon loss in each soil layer"}; /// m_soilIfluCbn
CONST_CHARS_LIST VAR_SOL_PERCO_C[] = {"sol_percoc", "percolation Carbon loss in each soil layer"}; /// m_soilPercoCbn
CONST_CHARS_LIST VAR_LATERAL_C[] = {"latc", "lateral flow Carbon loss in soil profile"}; /// m_soilIfluCbnPrfl
CONST_CHARS_LIST VAR_PERCO_C[] = {"percoc", "percolation Carbon loss in soil profile"}; /// m_soilPercoCbnPrfl
CONST_CHARS_LIST VAR_SEDLOSS_C[] = {"sedc", "amount of Carbon lost with sediment"}; /// m_sedLossCbn

CONST_CHARS_LIST VAR_SOL_NH4[] = {"sol_nh4", "amount of nitrogen stored in the ammonium pool in soil layer"}; /// m_soilNH4
CONST_CHARS_LIST VAR_SOL_NO3[] = {"sol_no3", "amount of nitrogen stored in the nitrate pool in soil layer"}; /// m_soilNO3
CONST_CHARS_LIST VAR_SOL_OM[] = {"om", "percent of organic matter in soil"}; /// m_soilOM
CONST_CHARS_LIST VAR_SOL_SORGN[] = {"sol_orgn", "amount of nitrogen stored in the stable organic N pool"}; /// m_soilStabOrgN
CONST_CHARS_LIST VAR_SOL_HORGP[] = {"sol_orgp", "amount of phosphorus stored in the humic organic P pool in soil layer"}; /// m_soilHumOrgP
CONST_CHARS_LIST VAR_SOL_PERCO[] = {"sol_perco", "percolation from soil layer"};
CONST_CHARS_LIST VAR_RSDCOV_COEF[] = {"rsd_covco", "residue cover factor for computing fraction of cover"};
CONST_CHARS_LIST VAR_SOL_RSD[] = {"sol_rsd", "amount of organic matter in the soil classified as residue"}; /// m_soilRsd
CONST_CHARS_LIST VAR_SOL_RSDIN[] = {"rsdin", "amount of organic matter in the soil classified as residue"}; /// m_rsdInitSoil
CONST_CHARS_LIST VAR_SOL_SOLP[] = {"sol_solp", "amount of phosphorus stored in solution"}; /// m_soilSolP
CONST_CHARS_LIST VAR_SOL_STAP[] = {"sol_stap", "amount of phosphorus in the soil layer stored in the stable mineral phosphorus pool"}; /// m_soilStabMinP
CONST_CHARS_LIST VAR_SOL_SUMAWC[] = {"sol_sumAWC", "amount of water held in soil profile at field capacity"}; /// m_soilSumFC
CONST_CHARS_LIST VAR_SOL_SUMSAT[] = {"sol_sumul", "amount of water held in soil profile at saturation"}; /// m_soilSumSat
CONST_CHARS_LIST VAR_SOL_TA0[] = {"soil_ta0", "Coefficient a0 for Finn Plauborg Method"};
CONST_CHARS_LIST VAR_SOL_TA1[] = {"soil_ta1", "Coefficient a1 for Finn Plauborg Method"};
CONST_CHARS_LIST VAR_SOL_TA2[] = {"soil_ta2", "Coefficient a2 for Finn Plauborg Method"};
CONST_CHARS_LIST VAR_SOL_TA3[] = {"soil_ta3", "Coefficient a3 for Finn Plauborg Method"};
CONST_CHARS_LIST VAR_SOL_TB1[] = {"soil_tb1", "Coefficient b1 for Finn Plauborg Method"};
CONST_CHARS_LIST VAR_SOL_TB2[] = {"soil_tb2", "Coefficient b2 for Finn Plauborg Method"};
CONST_CHARS_LIST VAR_SOL_TD1[] = {"soil_td1", "Coefficient d1 for Finn Plauborg Method"};
CONST_CHARS_LIST VAR_SOL_TD2[] = {"soil_td2", "Coefficient d2 for Finn Plauborg Method"};
CONST_CHARS_LIST VAR_SOL_TMP[] = {"sol_tmp", "daily average temperature of soil layer"};
CONST_CHARS_LIST VAR_SOL_UL[] = {"sol_ul", "amount of water held in the soil layer at saturation (sat - wp water)"}; /// m_soilSat
CONST_CHARS DESC_SOL_WFC = "Water content of soil profile at field capacity";
CONST_CHARS_LIST VAR_SOL_WPMM[] = {"sol_wpmm", "water content of soil at -1.5 MPa (wilting point)"}; /// m_soilWP
CONST_CHARS_LIST VAR_SOL_ZMX[] = {"SOL_ZMX", "Maximum rooting depth of soil profile (mm)"}; /// m_soilMaxRootD
CONST_CHARS_LIST VAR_SOL_ST[] = {"solst", "amount of water stored in the soil layer on current day(mm H2O)"}; /// m_soilWtrSto
CONST_CHARS_LIST VAR_SOL_SW[] = {"solsw", "amount of water stored in soil profile on current day (mm H2O)"}; /// m_soilWtrStoPrfl
CONST_CHARS_LIST VAR_SW_CAP[] = {"sw_cap", "amount of water capacity in soil layers such as sol_awc sol_ul and wiltingpoint"};
CONST_CHARS_LIST VAR_SOTE[] = {"SOTE", "soil Temperature"}; /// m_soilTemp
CONST_CHARS_LIST VAR_SOWB[] = {"SOWB", "soil water balance"}; /// m_soilWtrBal
CONST_CHARS_LIST VAR_SOXY[] = {"soxy", "saturation concentration of dissolved oxygen"};
CONST_CHARS_LIST VAR_SOXYConc[] = {"soxyConc", ""};
CONST_CHARS_LIST VAR_SPCON[] = {"spcon", "Coefficient in sediment transport equation"}; /// m_sedTransEqCoef
CONST_CHARS_LIST VAR_SPEXP[] = {"spexp", "Exponent in sediment transport equation"}; /// m_sedTransEqExp
CONST_CHARS DESC_SR = "Solar radiation";
CONST_CHARS_LIST VAR_SR_MAX[] = {"srMax", "Max solar radiation"};
CONST_CHARS_LIST VAR_SRA[] = {"sra", "solar radiation for the day"};
CONST_CHARS_LIST VAR_SSRU[] = {"SSRU", "Subsurface runoff"}; /// m_subSurfRf
CONST_CHARS_LIST VAR_SSRUVOL[] = {"SSRUVOL", "Subsurface runoff volume (m3)."}; /// m_subSurfRfVol
CONST_CHARS_LIST VAR_STCAPSURPLUS[] = {"STCAPSURPLUS", "surplus of storage capacity"};
CONST_CHARS_LIST VAR_STREAM_LINK[] = {"STREAM_LINK", "Stream link (id of reaches)"}; /// m_rchID
CONST_CHARS_LIST VAR_SUB_SEDTOCH[] = {"", "sediment to streams from each subbasin"}; /// TODO, for storm mode
CONST_CHARS_LIST VAR_SUBBSN[] = {"subbasin", "The subbasion grid"}; /// m_subbsnID
CONST_CHARS_LIST VAR_SUBBSNID_NUM[] = {"SUBBASINID_NUM", "number of subbasins"}; /// m_nSubbsns
CONST_CHARS_LIST VAR_SUR_NO3[] = {"sur_no3", "amount of nitrate transported with surface runoff"}; /// m_surfRfNO3
CONST_CHARS_LIST VAR_SUR_NO3_TOCH[] = {"sur_no3_ToCh", "amount of nitrate transported with surface runoff to channel"}; /// m_surfRfNO3ToCh
CONST_CHARS_LIST VAR_SUR_NH4[] = {"sur_nh4", "amount of ammonian transported with surface runoff"}; /// m_surfRfNH4
CONST_CHARS_LIST VAR_SUR_NH4_TOCH[] = {"SUR_NH4_TOCH", "amount of ammonian transported with surface runoff to channel"}; /// m_surfRfNH4ToCh
CONST_CHARS_LIST VAR_SUR_SOLP[] = {"sur_solp", "amount of solution phosphorus in surface runoff"}; /// m_surfRfSolP
CONST_CHARS_LIST VAR_SUR_SOLP_TOCH[] = {"sur_solp_ToCh", "amount of soluble phosphorus from surface runoff to channel"}; /// m_surfRfSolPToCh
CONST_CHARS_LIST VAR_SUR_COD_TOCH[] = {"sur_cod_ToCH", "amount of COD to reach in surface runoff"}; /// m_surfRfCodToCh
CONST_CHARS_LIST VAR_SURU[] = {"SURU", "surface runoff"}; /// m_surfRf
CONST_CHARS_LIST VAR_SUR_SDEP[] = { "SUR_SDEP", "initail water depth of surface and channel" }; // m_surSdep
CONST_CHARS_LIST VAR_SUR_WRT_DEPTH[] = { "SUR_WRT_DEPTH", "surface runoff depth(spatially for output of casc2d module)" }; /// m_surWtrDepth
CONST_CHARS_LIST VAR_CH_WRT_DEPTH[] = { "CH_WRT_DEPTH", "channel water depth(spatially for output of casc2d module)" }; /// m_chWtrDepth
CONST_CHARS_LIST VAR_SWE[] = {"SWE", "average snow accumulation of the watershed"};
CONST_CHARS_LIST VAR_SWE0[] = {"swe0", "Initial snow water equivalent"};
CONST_CHARS_LIST VAR_T_BASE[] = {"T_BASE", "base or minimum temperature for plant growth"}; /// m_pgTempBase
CONST_CHARS_LIST VAR_T_OPT[] = {"T_OPT", "optimal temperature for plant growth"}; /// m_pgOptTemp
CONST_CHARS_LIST VAR_T_RG[] = {"T_RG", "groundwater runoff"};
CONST_CHARS_LIST VAR_T_SNOW[] = {"T_snow", "Snowfall temperature"}; /// m_snowTemp
CONST_CHARS_LIST VAR_T_SOIL[] = {"t_soil", "soil freezing temperature threshold"}; /// m_soilFrozenTemp
CONST_CHARS_LIST VAR_T0[] = {"T0", "the snowmelt threshold temperature"};
CONST_CHARS_LIST VAR_TFACT[] = {"tfact", "fraction of solar radiation computed in the temperature heat balance that is photo synthetically active"};
CONST_CHARS_LIST VAR_TILLAGE_LOOKUP[] = {"TillageLookup", "Tillage lookup table"}; /// m_tillageLookup
CONST_CHARS_LIST VAR_TILLAGE_DAYS[] = {"tillage_days", "days from tillage"}; /// m_tillDays
CONST_CHARS_LIST VAR_TILLAGE_DEPTH[] = {"tillage_depth", "tillage depth"}; /// m_tillDepth
CONST_CHARS_LIST VAR_TILLAGE_FACTOR[] = {"tillage_factor", "influence factor of tillage operation"}; /// m_tillFactor
CONST_CHARS_LIST VAR_TILLAGE_SWITCH[] = {"tillage_switch", "switch of whether to tillage"}; /// m_tillSwitch
CONST_CHARS_LIST VAR_TMAX[] = {"TMAX", "max temperature"}; /// m_maxTemp
CONST_CHARS_LIST VAR_TMEAN[] = {"TMEAN", "mean temperature"}; /// m_meanTemp
CONST_CHARS_LIST VAR_TMEAN_ANN[] = {"TMEAN0", "annual mean temperature"}; /// m_annMeanTemp
CONST_CHARS_LIST VAR_TMEAN1[] = {"TMEAN1", "Mean air temperature of the (d-1)th day"}; /// m_meanTempPre1
CONST_CHARS_LIST VAR_TMEAN2[] = {"TMEAN2", "Mean air temperature of the (d-2)th day"}; /// m_meanTempPre2
CONST_CHARS_LIST VAR_TMIN[] = {"TMIN", "min temperature"}; /// m_minTemp
CONST_CHARS_LIST VAR_TREEYRS[] = {"CURYR_INIT", "initial age of tress (yrs)"}; /// m_curYrMat
CONST_CHARS DESC_TSD_CLIMATE = "Climate data of all the stations";
CONST_CHARS_LIST VAR_TSD_DT[] = {"DATATYPE", "Time series data type, e.g., climate data"};
CONST_CHARS DESC_UPSOLDEP = "depth of the upper soil layer";
CONST_CHARS_LIST VAR_ICFAC[] = {"icfac", "C-factor calculation using Cmin (0, default) or new method from RUSLE (1)"}; /// m_iCfac
CONST_CHARS_LIST VAR_USLE_C[] = {"USLE_C", "the average annual cover management factor for the land cover"}; /// m_usleC
CONST_CHARS_LIST VAR_USLE_K[] = {"USLE_K", "The soil erodibility factor used in USLE"}; /// m_usleK
CONST_CHARS_LIST VAR_USLE_L[] = {"USLE_L", "USLE slope length factor"}; /// m_usleL
CONST_CHARS_LIST VAR_USLE_S[] = {"USLE_S", "USLE slope factor"}; /// m_usleS
CONST_CHARS_LIST VAR_USLE_P[] = {"USLE_P", "the erosion control practice factor"}; /// m_usleP
CONST_CHARS_LIST VAR_VCD[] = {"vcd", "compute changes in channel dimensions"}; /// m_vcd
CONST_CHARS_LIST VAR_VDIV[] = {"Vdiv", "diversion loss of the river reach"};
CONST_CHARS_LIST VAR_VP_ACT[] = {"avp", "actual vapor pressure"};
CONST_CHARS_LIST VAR_VP_SAT[] = {"svp", "Saturated vapor pressure"};
CONST_CHARS_LIST VAR_VPD[] = {"VPD", "vapor pressure deficit"}; /// m_vpd
CONST_CHARS_LIST VAR_VPDFR[] = {"vpdfr", "vapor pressure deficit(kPa) corresponding to the second point on the stomatal conductance curve"};
CONST_CHARS_LIST VAR_VSEEP0[] = {"Vseep0", "the initial volume of transmission loss to the deep aquifer over the time interval"};
CONST_CHARS_LIST VAR_WATTEMP[] = {"wattemp", "temperature of water in reach"}; /// m_chTemp
CONST_CHARS_LIST VAR_WAVP[] = {"WAVP", "rate of decline in rue per unit increase in vapor pressure deficit"}; /// m_wavp
CONST_CHARS_LIST VAR_WDNTL[] = {"wdntl", "amount of nitrogen lost from nitrate pool by denitrification in soil profile on current day in cell"};
CONST_CHARS_LIST VAR_WILTPOINT[] = {"WiltingPoint", "Plant wilting point moisture"};
CONST_CHARS_LIST VAR_WS[] = {"WS", "Wind speed (measured at 10 meters above surface)"};
CONST_CHARS_LIST VAR_WSHD_DNIT[] = {"wshd_dnit", "nitrogen lost from nitrate pool due to denitrification in watershed"};
CONST_CHARS_LIST VAR_WSHD_HMN[] = {"wshd_hmn", "nitrogen moving from active organic to nitrate pool in watershed"};
CONST_CHARS_LIST VAR_WSHD_HMP[] = {"wshd_hmp", "phosphorus moving from organic to labile pool in watershed"};
CONST_CHARS_LIST VAR_WSHD_NITN[] = {"wshd_nitn", "nitrogen moving from the NH3 to the NO3 pool by nitrification in the watershed"};
CONST_CHARS_LIST VAR_WSHD_PAL[] = {"wshd_pal", "phosphorus moving from labile mineral to active mineral pool in watershed"};
CONST_CHARS_LIST VAR_WSHD_PAS[] = {"wshd_pas", "phosphorus moving from active mineral to stable mineral pool in watershed"};
CONST_CHARS_LIST VAR_WSHD_PLCH[] = {"wshd_plch", "phosphorus leached into second soil layer"}; /// m_wshdLchP
CONST_CHARS_LIST VAR_WSHD_RMN[] = {"wshd_rmn", "nitrogen moving from fresh organic (residue) to nitrate and active organic pools in watershed"};
CONST_CHARS_LIST VAR_WSHD_RMP[] = {"wshd_rmp", "phosphorus moving from fresh organic (residue) to labile and organic pools in watershed"};
CONST_CHARS_LIST VAR_WSHD_RNO3[] = {"wshd_rno3", "NO3 added to soil by rainfall in watershed"};
CONST_CHARS_LIST VAR_WSHD_RWN[] = {"wshd_rwn", "nitrogen moving from active organic to stable organic pool in watershed"};
CONST_CHARS_LIST VAR_WSHD_VOLN[] = {"wshd_voln", "average annual amount if nitrogen lost by ammonia volatilization in watershed"};
CONST_CHARS_LIST VAR_WSYF[] = {"wsyf", "Lower limit of harvest index ((kg/ha)/(kg/ha))"}; /// m_wtrStrsHvst
CONST_CHARS_LIST VAR_AL_OUTLET[] = {"algae_outlet", "algae concentration at the watershed outlet"};
CONST_CHARS_LIST VAR_ON_OUTLET[] = {"organicn_outlet", "organicn concentration at the watershed outlet"};
CONST_CHARS_LIST VAR_AN_OUTLET[] = {"ammonian_outlet", "ammonian concentration at the watershed outlet"};
CONST_CHARS_LIST VAR_NIN_OUTLET[] = {"nitriten_outlet", "nitriten concentration at the watershed outlet"};
CONST_CHARS_LIST VAR_NAN_OUTLET[] = {"nitraten_outlet", "nitraten concentration at the watershed outlet"};
CONST_CHARS_LIST VAR_OP_OUTLET[] = {"organicp_outlet", "organicp concentration at the watershed outlet"};
CONST_CHARS_LIST VAR_DP_OUTLET[] = {"disolvp_outlet", "disolvp concentration at the watershed outlet"};
CONST_CHARS_LIST VAR_COD_OUTLET[] = {"cod_outlet", "cod concentration at the watershed outlet"};
CONST_CHARS_LIST VAR_CHL_OUTLET[] = {"chlora_outlet", "chlora concentration at the watershed outlet"};

CONST_CHARS_LIST VAR_A_DAYS[] = {"a_days", "days since P Application"}; /// m_phpApldDays
CONST_CHARS_LIST VAR_B_DAYS[] = {"b_days", "days since P deficit"}; /// m_phpDefDays


//////////////////////////////////////////////////////////////////////////
/// Define units common used in SEIMS, in case of inconsistency //////////
/// By LiangJun Zhu, HuiRan Gao ///
/// Apr. , 2016  //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CONST_CHARS UNIT_AREA_HA = "ha";
CONST_CHARS UNIT_AREA_KM2 = "km2";                         /// Square kilometer of area
CONST_CHARS UNIT_AREA_RATIO = "m2/m2";
CONST_CHARS UNIT_CONDRATE_MSPA = "m/s/kPa";                /// Rate of decline in stomatal conductance per unit increase in vapor pressure deficit
CONST_CHARS UNIT_CONT_KGHA = "kg/ha";                      /// For convenient, keep consistent with SWAT, need Conversion later.
CONST_CHARS UNIT_CONT_KGKM2 = "kg/km2";                    /// Kilograms per Square kilometers of nutrient content
CONST_CHARS UNIT_CONT_RATIO = "(kg/ha)/(kg/ha)";
CONST_CHARS UNIT_CONT_TONHA = "tons/ha";
CONST_CHARS UNIT_DENSITY = "Mg/m3";                        /// density, equal to g/cm3, Mg/m3, ton/m3
CONST_CHARS UNIT_SEDCONC = "g/L";                         /// i.e., kg/m3
CONST_CHARS UNIT_CONCENTRATION = "mg/L";                        /// concentration, or mg/kg
CONST_CHARS UNIT_DEPTH_MM = "mm";                          /// Depth related unit, mm
CONST_CHARS UNIT_FLOW_CMS = "m3/s";                        /// Cubic meters per second of flow discharge
CONST_CHARS UNIT_GAS_CON = "uL/L";   /// e.g., uL CO2/L air, IS this same with ppmv? LJ
CONST_CHARS UNIT_GAS_PPMV = "ppmv";                        /// Concentration of gas, e.g., CO2
CONST_CHARS UNIT_HEAT_UNIT = "hr";
CONST_CHARS UNIT_KG = "kg";                                /// mass Kg
CONST_CHARS UNIT_TONS = "t";                               /// metric tons
CONST_CHARS UNIT_KG_S = "kg/s";
CONST_CHARS UNIT_KGM3 = "kg/m3";
CONST_CHARS UNIT_LAP_RATE = "/100m";                       /// Lapse rate
CONST_CHARS UNIT_LEN_M = "m";                              /// Meter of length
CONST_CHARS UNIT_LONLAT_DEG = "degree";                    /// Degree of longitude and latitude
CONST_CHARS UNIT_MELT_FACTOR = "mm/deg C/day";                 /// Melt factor
CONST_CHARS UNIT_NON_DIM = "";                             /// Non dimension
CONST_CHARS UNIT_NUTR_RATIO = "mg/mg";         /// mg H2O/mg Nutrient
CONST_CHARS UNIT_PER_DAY = "1/day";               /// rate per day
CONST_CHARS UNIT_PERCENT = "%";                            /// Percent
CONST_CHARS UNIT_PRESSURE = "kPa";                         /// Vapor pressure
CONST_CHARS UNIT_RAD_USE_EFFI = "(kg/ha)/(MJ/m2)";
CONST_CHARS UNIT_SPEED_MS = "m/s";                         /// Speed related
CONST_CHARS UNIT_SR = "MJ/m2/d";                           /// Solar Radiation
CONST_CHARS UNIT_STRG_M3M = "m3/m";                       /// storage per meter of reach length
CONST_CHARS UNIT_TEMP_DEG = "deg C";                       /// Celsius degree of air temperature
CONST_CHARS UNIT_TEMP_FACTOR = "mm/deg C";                 /// temperature factor
CONST_CHARS UNIT_YEAR = "yr";
CONST_CHARS UNIT_DAY = "day";                    /// Time step (day)
CONST_CHARS UNIT_HOUR = "hr";                     /// Time step (h)
CONST_CHARS UNIT_SECOND = "sec";                      /// Time step (sec)
CONST_CHARS UNIT_VOL_FRA_M3M3 = "m3/m3";
CONST_CHARS UNIT_VOL_M3 = "m3";                           /// volume
CONST_CHARS UNIT_AREA_M2 = "m2";               /// Area
CONST_CHARS UNIT_WAT_RATIO = "mm/mm";         /// mm H2O/mm Soil
CONST_CHARS UNIT_WTRDLT_MMD = "mm/d";                      /// Millimeter per day of water changes
CONST_CHARS UNIT_WTRDLT_MMH = "mm/h";                      /// Millimeter per hour of water changes

/// Units used in rice growth module (PG_ORYZA), by Fang Shen
CONST_CHARS UNIT_DVR = "deg C/d";
CONST_CHARS UNIT_PER_HOUR = "1/h";
CONST_CHARS UNIT_SOW_HILL = "hill/m2";
CONST_CHARS UNIT_SOW_PLANT = "pl/hill";
CONST_CHARS UNIT_SOW_SEEDBED = "pl/m2";
CONST_CHARS UNIT_LAPE = "m2/pl";
CONST_CHARS UNIT_MAIN = "kg/kg/d";
CONST_CHARS UNIT_CRG = "kg/kg";
CONST_CHARS UNIT_SLA = "ha/kg";
CONST_CHARS UNIT_ROOT_RATIO = "m/d";
CONST_CHARS UNIT_GRAIN_WEIGHT = "kg/grain";
CONST_CHARS UNIT_PHENOLOGY = "deg C d";
CONST_CHARS UNIT_NUMBERHA = "no/ha";


//////////////////////////////////////////////////////////////////////////
/// Define MongoDB related constant strings used in SEIMS and preprocess//
/// By LiangJun Zhu, May. 4, 2016  ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CONST_CHARS MONG_GRIDFS_FN =                        "filename";
CONST_CHARS MONG_GRIDFS_WEIGHT_CELLS =              "CELLSNUM";
CONST_CHARS MONG_GRIDFS_WEIGHT_SITES =              "NUM_SITES";
CONST_CHARS MONG_GRIDFS_ID =                        "ID";
CONST_CHARS MONG_GRIDFS_SUBBSN =                    "SUBBASIN";
CONST_CHARS MONG_HYDRO_SITE_TYPE =                  "TYPE";
CONST_CHARS MONG_HYDRO_SITE_LAT =                   "LAT";
CONST_CHARS MONG_HYDRO_SITE_ELEV =                  "ELEVATION";
CONST_CHARS MONG_HYDRO_DATA_SITEID =                "STATIONID";
CONST_CHARS MONG_HYDRO_DATA_UTC =                   "UTCDATETIME";
CONST_CHARS MONG_HYDRO_DATA_LOCALT =                "LOCALDATETIME";
CONST_CHARS MONG_HYDRO_DATA_VALUE =                 "VALUE";
CONST_CHARS MONG_SITELIST_SUBBSN =                  "SUBBASINID";
CONST_CHARS MONG_SITELIST_DB =                      "DB";


//////////////////////////////////////////////////////////////////////////
/// Define Raster/ related constant strings used in SEIMS and preprocess//
/// By LiangJun Zhu, May. 5, 2016  ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//#define HEADER_RS_NODATA                       "NODATA_VALUE"
//#define HEADER_RS_XLL                          "XLLCENTER"
//#define HEADER_RS_YLL                          "YLLCENTER"
//#define HEADER_RS_NROWS                        "NROWS"
//#define HEADER_RS_NCOLS                        "NCOLS"
//#define HEADER_RS_CELLSIZE                     "CELLSIZE"
//#define HEADER_RS_LAYERS                       "LAYERS"
//#define HEADER_RS_SRS                          "SRS"

#define OUTPUT_ICELL 1000;
#endif
