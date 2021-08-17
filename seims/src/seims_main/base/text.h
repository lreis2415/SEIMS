/*!
 * \file text.h
 * \brief Predefined string constants used in the code
 *        BE CAUTION, constant value must be aligned by SPACE, not TAB!
 * \author Junzhi Liu, LiangJun Zhu, Huiran Gao
 * \version 1.3
 * \date Jun.2010, Apr.2016, Apr.2018, Mar. 2021
 *
 * Changelog:
 *   - 1. 2021-03-13 - lj - Instead of using a macro to store a constant, use a const variable.
 *
 */
#ifndef SEIMS_TEXT_H
#define SEIMS_TEXT_H

#define CONST_CHARS static const char*
#define CONST_CHARS_LIST static const char* const

CONST_CHARS MODEL_NAME =                      "SEIMS";
CONST_CHARS MODEL_FULLNAME =                  "Spatially Explicit Integrated Modeling System";
CONST_CHARS MODEL_VERSION =                   "2021";
CONST_CHARS SEIMS_EMAIL =                     "zlj@lreis.ac.cn";
CONST_CHARS SEIMS_SITE =                      "https://github.com/lreis2415/SEIMS";

//! Constant input variables
CONST_CHARS CONS_IN_ELEV =                          "Elevation";
CONST_CHARS CONS_IN_LAT =                          "Latitude";
CONST_CHARS CONS_IN_XPR =                           "xpr";
CONST_CHARS CONS_IN_YPR =                           "ypr";

//! Climate data type
CONST_CHARS DataType_Precipitation =                "P";             //1, Suffix of precipitation data
CONST_CHARS DataType_MeanTemperature =              "TMEAN";         //2       /// m_meanTemp
CONST_CHARS DataType_MinimumTemperature =           "TMIN";          //3       /// m_minTemp
CONST_CHARS DataType_MaximumTemperature =           "TMAX";          //4       /// m_maxTemp
CONST_CHARS DataType_PotentialEvapotranspiration =  "PET";           //5
CONST_CHARS DataType_SolarRadiation =               "SR";            //6       /// m_sr
CONST_CHARS DataType_WindSpeed =                    "WS";            //7       /// m_ws
CONST_CHARS DataType_RelativeAirMoisture =          "RM";            //8       /// m_rhd
CONST_CHARS DataType_Meteorology =                  "M";             // Suffix of meteorology data
CONST_CHARS DataType_Prefix_TS =                    "T";             // m_stationData, Prefix of time series data
CONST_CHARS DataType_Prefix_DIS =                   "D";             // Prefix of distributed data

/// Tags of climate related data.
///                                                                           // VariableNameInModules
CONST_CHARS Tag_DEM =                               "DEM";                     // m_dem
CONST_CHARS Tag_Elevation_Meteorology =             "StationElevation_M";      // m_hStations
CONST_CHARS Tag_Elevation_PET =                     "StationElevation_PET";    // m_hStations
CONST_CHARS Tag_Elevation_Precipitation =           "StationElevation_P";      // m_hStations
CONST_CHARS Tag_Elevation_Temperature =             "StationElevation_T";      // m_hStations
CONST_CHARS Tag_LapseRate =                         "LapseRate";               // m_lapseRate
CONST_CHARS Tag_Latitude_Meteorology =              "Latitude_M";
CONST_CHARS Tag_StationElevation =                  "StationElevation";        // m_hStations
CONST_CHARS_LIST Tag_VerticalInterpolation[] = {"VERTICALINTERPOLATION", "Execute vertical interpolation (1) or not (0), defined in config.fig"};   // m_itpVertical
CONST_CHARS_LIST Tag_Weight[] = {"WEIGHT", "Weight used for interpolation"};                  // m_itpWeights
CONST_CHARS Tag_DataType =                          "DATATYPE";                // m_dataType
///////  define parameter calibration related string constants  ///////
CONST_CHARS PARAM_CHANGE_VC =                       "VC";  // replace by a value
CONST_CHARS PARAM_CHANGE_RC =                       "RC";  // multiply a ratio, which is diff from SWAT: * (1+ratio)
CONST_CHARS PARAM_CHANGE_AC =                       "AC";  // add a value
CONST_CHARS PARAM_CHANGE_NC =                       "NC";  // no change
CONST_CHARS PARAM_FLD_NAME =                        "NAME";
CONST_CHARS PARAM_FLD_DESC =                        "DESCRIPTION";
CONST_CHARS PARAM_FLD_UNIT =                        "UNIT";
CONST_CHARS PARAM_FLD_MIDS =                        "MODULE";
CONST_CHARS PARAM_FLD_VALUE =                       "VALUE";
CONST_CHARS PARAM_FLD_IMPACT =                      "IMPACT";
CONST_CHARS PARAM_FLD_CHANGE =                      "CHANGE";
CONST_CHARS PARAM_FLD_MAX =                         "MAX";
CONST_CHARS PARAM_FLD_MIN =                         "MIN";
CONST_CHARS PARAM_FLD_USE =                         "USE";
CONST_CHARS PARAM_USE_Y =                           "Y";
CONST_CHARS PARAM_USE_N =                           "N";
CONST_CHARS PARAM_CALI_VALUES =                     "CALI_VALUES";  /// for calibration

//!
CONST_CHARS PARAM_CHANGEFUNCTION_LIN =              "LIN";  // linear change function, ax+x0, a,x0
CONST_CHARS PARAM_CHANGEFUNCTION_LOG =              "LOG";  // logistic growth change function, a(e^(x-x0)/(e^(x-x0)+1), a,x0
CONST_CHARS PARAM_CHANGEFUNCTION_EXP =              "EXP";  // exponential change function, 1-e^a(x-x0), a,x0


////////////  Input and Output Tags   ///////////////
// Fields in Model Configuration Collections //
//// Tags in file.in
CONST_CHARS Tag_ConfTag =                           "TAG";
CONST_CHARS Tag_ConfValue =                         "VALUE";
CONST_CHARS Tag_MODCLS =                            "MODULE_CLASS";
//// Tags in file.out
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

/// Available values of Tag_outputSubbsn
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
CONST_CHARS Tag_Average =                           "AVERAGE";
CONST_CHARS Tag_Average2 =                          "AVE";
CONST_CHARS Tag_Average3 =                          "MEAN";
CONST_CHARS Tag_Minimum =                           "MIN";
CONST_CHARS Tag_Maximum =                           "MAX";
CONST_CHARS Tag_SpecificCells =                     "SPECIFIC";

CONST_CHARS TAG_OUT_QOUTLET =                       "QOUTLET";
CONST_CHARS TAG_OUT_QTOTAL =                        "QTotal";
CONST_CHARS TAG_OUT_SEDOUTLET =                     "SEDOUTLET";
CONST_CHARS TAG_OUT_OL_IUH =                        "OL_IUH";
CONST_CHARS Tag_DisPOutlet =                        "DissovePOutlet";
CONST_CHARS Tag_AmmoOutlet =                        "AmmoniumOutlet";
CONST_CHARS Tag_NitrOutlet =                        "NitrateOutlet";

CONST_CHARS Tag_SubbasinCount =                     "SUBBASINCOUNT";
CONST_CHARS Tag_SubbasinId =                        "SUBBASINID"; /// m_inputSubbsnID
CONST_CHARS Tag_ReservoirCount =                    "RESERVOIRCOUNT";
CONST_CHARS Tag_ReservoirId =                       "RESERVOIRID";
CONST_CHARS_LIST Tag_SubbasinSelected[] = {"subbasinSelected", "The subbasion IDs listed in file.out"};
CONST_CHARS_LIST Tag_CellSize[] = {"CELLSIZE", "numble of valid cells, i.e., excluding NODATA"};
CONST_CHARS_LIST Tag_Mask[] = {"MASK", "Array containing the row and column numbers for valid cells"};
CONST_CHARS_LIST Tag_TimeStep[] = {"TIMESTEP", "time step"}; /// m_dt
CONST_CHARS_LIST Tag_HillSlopeTimeStep[] = {"DT_HS", "Time step of the simulation"};
CONST_CHARS Tag_ChannelTimeStep =                   "DT_CH";
CONST_CHARS_LIST Tag_CellWidth[] = {"CELLWIDTH", "width of the cell"}; /// m_cellWth, the size of a single CELL
CONST_CHARS_LIST Tag_LayeringMethod[] = {"LayeringMethod", "Routing layering method"};

// D8 Flow model
CONST_CHARS_LIST Tag_FLOWIN_INDEX_D8[] = {"FLOWIN_INDEX_D8", "The index of flow in (D8) cell in the compressed array, and the first element in each sub-array is the number of flow in cells in this sub-array"}; /// m_flowInIdxD8
CONST_CHARS_LIST Tag_FLOWOUT_INDEX_D8[] = {"FLOWOUT_INDEX_D8", "The index of flow out (D8) cell of each cells in the compressed array"}; /// m_flowOutIdxD8
CONST_CHARS_LIST Tag_ROUTING_LAYERS[] = {"ROUTING_LAYERS", "Routing layers according to the flow direction, there are no flow relationships within each layer, and the first element in each layer is the number of cells in the layer"}; /// m_rteLyrs

// TODO: Dinf, MFD, MFD-md integrated into SEIMS.
//! D-infinity Flow model after Tarboton et al.(1991)
CONST_CHARS Tag_FLOWIN_INDEX_DINF =                 "FLOWIN_INDEX_DINF";
CONST_CHARS_LIST Tag_FLOWIN_PERCENTAGE_DINF[] = {"FLOWIN_PERCENTAGE_DINF", "Percentage of flow in"};
CONST_CHARS_LIST Tag_FLOWOUT_INDEX_DINF[] = {"FLOWOUT_INDEX_DINF", "The index of flow in cell in the compressed array"};
CONST_CHARS Tag_FLOWOUT_PERCENTAGE_DINF =           "FLOWOUT_PERCENTAGE_DINF";
CONST_CHARS Tag_ROUTING_LAYERS_DINF =               "ROUTING_LAYERS_DINF";
//! Multi-Flow model after Quinn et al.(1991)
CONST_CHARS Tag_FLOWIN_INDEX_MFD =                  "FLOWIN_INDEX_MFD";
CONST_CHARS Tag_FLOWIN_PERCENTAGE_MFD =             "FLOWIN_PERCENTAGE_MFD";
CONST_CHARS Tag_FLOWOUT_INDEX_MFD =                 "FLOWOUT_INDEX_MFD";
CONST_CHARS Tag_FLOWOUT_PERCENTAGE_MFD =            "FLOWOUT_PERCENTAGE_MFD";
CONST_CHARS Tag_ROUTING_LAYERS_MFD =                "ROUTING_LAYERS_MFD";
//! MFD-md flow model after Qin et al.(2007) (md means maximum downslope), TODO.
CONST_CHARS Tag_FLOWIN_INDEX_MFD_MD =               "FLOWIN_INDEX_MFD_MD";
CONST_CHARS Tag_FLOWIN_PERCENTAGE_MFD_MD =          "FLOWIN_PERCENTAGE_MFD_MD";
CONST_CHARS Tag_FLOWOUT_INDEX_MFD_MD =              "FLOWOUT_INDEX_MFD_MD";
CONST_CHARS Tag_FLOWOUT_PERCENTAGE_MFD_MD =         "FLOWOUT_PERCENTAGE_MFD_MD";
CONST_CHARS Tag_ROUTING_LAYERS_MFD_MD =             "ROUTING_LAYERS_MFD_MD";

/// Reach parameters (Replaced Tag_ReachParameter and Tag_RchParam by VAR_REACH_PARAM)
CONST_CHARS_LIST VAR_REACH_PARAM[] = {"ReachParam", "Reach parameters such as stream order, manning's n and downstream subbasin id"};
/// Add Subbasins as AddParameters for modules
CONST_CHARS_LIST VAR_SUBBASIN_PARAM[] = {"SubbasinParam", "Statistics of subbasin related parameters"};
/// Files or database constant strings
//#define ASCIIExtension                       ".asc"  /// defined in clsRasterData.h
//#define GTiffExtension                       ".tif"
CONST_CHARS TextExtension =                         "txt";

CONST_CHARS File_Config =                           "config.fig";
CONST_CHARS File_Input =                            "file.in";
CONST_CHARS File_Output =                           "file.out";
CONST_CHARS Source_HydroClimateDB =                 "HydroClimateDB";
CONST_CHARS Source_HydroClimateDB_Optional =        "HydroClimateDB_Optional";
CONST_CHARS Source_ParameterDB =                    "ParameterDB";
CONST_CHARS Source_ParameterDB_Optional =           "ParameterDB_Optional";
CONST_CHARS Source_Module =                         "Module";
CONST_CHARS Source_Module_Optional =                "Module_Optional";

///////// Table Names required in MongoDB /////////
const char DB_TAB_PARAMETERS[] =                      "PARAMETERS";
const char DB_TAB_SITELIST[] =                        "SITELIST";
const char DB_TAB_SCENARIO[] =                        "BMPDATABASE";
const char DB_TAB_REACH[] =                           "REACHES";
const char DB_TAB_SPATIAL[] =                         "SPATIAL"; // i.e., SPATIAL.files
const char DB_TAB_SITES[] =                           "SITES";
const char DB_TAB_DATAVALUES[] =                      "DATA_VALUES"; // hydroClimate data values
const char DB_TAB_MEASUREMENT[] =                     "MEASUREMENT";
const char DB_TAB_ANNSTAT[] =                         "ANNUAL_STATS";
const char DB_TAB_OUT_SPATIAL[] =                     "OUTPUT"; // i.e., OUTPUT.files
const char DB_TAB_FILE_IN[] =                         "FILE_IN";
const char DB_TAB_FILE_OUT[] =                        "FILE_OUT";
/// Fields in DB_TAB_REACH ///
const char REACH_SUBBASIN[] =                         "SUBBASINID";
const char REACH_NUMCELLS[] =                         "NUM_CELLS";
const char REACH_GROUP[] =                            "GROUP";
const char REACH_KMETIS[] =                           "KMETIS";
const char REACH_PMETIS[] =                           "PMETIS";
const char REACH_DOWNSTREAM[] =                       "DOWNSTREAM";
const char REACH_UPDOWN_ORDER[] =                     "UP_DOWN_ORDER";
const char REACH_DOWNUP_ORDER[] =                     "DOWN_UP_ORDER";
const char REACH_WIDTH[] =                            "CH_WIDTH";
const char REACH_LENGTH[] =                           "CH_LEN";
const char REACH_DEPTH[] =                            "CH_DEPTH";
const char REACH_WDRATIO[] =                          "CH_WDRATIO";
const char REACH_AREA[] =                             "CH_AREA";
const char REACH_SIDESLP[] =                          "CH_SSLP";
const char REACH_SLOPE[] =                            "CH_SLP";
// Hydrological related parameters
CONST_CHARS REACH_MANNING =                         "CH_N"; // Manning's "n" value
CONST_CHARS REACH_BEDK =                            "CH_BED_K"; /// hydraulic conductivity of the channel bed
CONST_CHARS REACH_BNKK =                            "CH_BNK_K"; /// hydraulic conductivity of the channel bank
// Erosion related parameters
CONST_CHARS REACH_BEDBD =                          "CH_BED_BD"; // Bulk density of channel bed sediment
CONST_CHARS REACH_BNKBD =                           "CH_BNK_BD"; // Bulk density of channel bed sediment
CONST_CHARS REACH_BEDCOV =                          "CH_BED_COV"; // Channel bed cover factor, ch_cov2 in SWAT
CONST_CHARS REACH_BNKCOV =                          "CH_BNK_COV"; // Channel bank cover factor, ch_cov1 in SWAT
CONST_CHARS REACH_BEDEROD =                         "CH_BED_EROD"; // Erodibility of channel bed sediment, ch_bed_kd in SWAT
CONST_CHARS REACH_BNKEROD =                         "CH_BNK_EROD"; // Erodibility of channel bank sediment, ch_bnk_kd in SWAT
CONST_CHARS REACH_BEDD50 =                          "CH_BED_D50"; // D50(median) particle size diameter of channel bed sediment
CONST_CHARS REACH_BNKD50 =                          "CH_BNK_D50"; // D50(median) particle size diameter of channel band sediment
// Nutrient cycling related parameters
CONST_CHARS REACH_BC1 =                             "BC1";
CONST_CHARS REACH_BC2 =                             "BC2";
CONST_CHARS REACH_BC3 =                             "BC3";
CONST_CHARS REACH_BC4 =                             "BC4";
CONST_CHARS REACH_RS1 =                             "RS1";
CONST_CHARS REACH_RS2 =                             "RS2";
CONST_CHARS REACH_RS3 =                             "RS3";
CONST_CHARS REACH_RS4 =                             "RS4";
CONST_CHARS REACH_RS5 =                             "RS5";
CONST_CHARS REACH_RK1 =                             "RK1";
CONST_CHARS REACH_RK2 =                             "RK2";
CONST_CHARS REACH_RK3 =                             "RK3";
CONST_CHARS REACH_RK4 =                             "RK4";
CONST_CHARS REACH_DISOX =                           "DISOX";
CONST_CHARS REACH_BOD =                             "BOD";
CONST_CHARS REACH_ALGAE =                           "ALGAE";
CONST_CHARS REACH_ORGN =                            "ORGN"; // ch_onco in SWAT
CONST_CHARS REACH_NH4 =                             "NH4";
CONST_CHARS REACH_NO2 =                             "NO2";
CONST_CHARS REACH_NO3 =                             "NO3";
CONST_CHARS REACH_ORGP =                            "ORGP"; // ch_opco in SWAT
CONST_CHARS REACH_SOLP =                            "SOLP";
// Groundwater nutrient related parameters
CONST_CHARS REACH_GWNO3 =                           "GWNO3";
CONST_CHARS REACH_GWSOLP =                          "GWSOLP";
/// Derived parameters according to the input parameters of Reach, which may also be provided in database.
CONST_CHARS REACH_BEDTC =                           "CH_BED_TC"; // Critical shear stress of channel bed
CONST_CHARS REACH_BNKTC =                           "CH_BNK_TC"; // Critical shear stress of channel bank
CONST_CHARS REACH_BNKSAND =                         "CH_BNK_SAND"; // Fraction of sand in channel bank sediment
CONST_CHARS REACH_BNKSILT =                         "CH_BNK_SILT"; // Fraction of silt in channel bank sediment
CONST_CHARS REACH_BNKCLAY =                         "CH_BNK_CLAY"; // Fraction of clay in channel bank sediment
CONST_CHARS REACH_BNKGRAVEL =                       "CH_BNK_GRAVEL"; // Fraction of gravel in channel bank sediment
CONST_CHARS REACH_BEDSAND =                         "CH_BED_SAND"; // Fraction of sand in channel bed sediment
CONST_CHARS REACH_BEDSILT =                         "CH_BED_SILT"; // Fraction of silt in channel bed sediment
CONST_CHARS REACH_BEDCLAY =                         "CH_BED_CLAY"; // Fraction of clay in channel bed sediment
CONST_CHARS REACH_BEDGRAVEL =                       "CH_BED_GRAVEL"; // Fraction of gravel in channel bed sediment

/// these four are defined in DB_TAB_SITELIST in Source_ParameterDB
CONST_CHARS SITELIST_TABLE_M =                      "SITELISTM";
CONST_CHARS SITELIST_TABLE_P =                      "SITELISTP";
CONST_CHARS SITELIST_TABLE_PET =                    "SITELISTPET";
CONST_CHARS SITELIST_TABLE_STORM =                  "STORMSITELIST";

//! define string constants used in the code, also used in the mongoDB.SiteList table's header
CONST_CHARS Tag_Mode =                              "MODE";
CONST_CHARS Tag_Mode_Storm =                        "STORM";
CONST_CHARS Tag_Mode_Daily =                        "DAILY";

CONST_CHARS Type_Scenario =                         "SCENARIO";
CONST_CHARS Type_Reach =                            "REACH";
CONST_CHARS Type_Subbasin =                         "SUBBASIN";
CONST_CHARS Type_Raster1D =                         "RASTER1D";
CONST_CHARS Type_Raster2D =                         "RASTER2D";
CONST_CHARS Type_Array1DDateValue =                 "ARRAY1DDATEVALUE";
CONST_CHARS Type_Array2D =                          "ARRAY2D";
CONST_CHARS Type_Array1D =                          "ARRAY1D";
CONST_CHARS Type_Single =                           "SINGLE";

CONST_CHARS TFType_Whole =                          "TFWhole";
CONST_CHARS TFType_Single =                         "TFSINGLE";
CONST_CHARS TFType_Array1D =                        "TFARRAY1D";

//////////////////////////////////////////////////////////////////////////
/// Define models' ID and description in SEIMS  //////////////////////////
/// By LiangJun Zhu, Apr. 26, 2016  //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// Hydro-Meteorological data
CONST_CHARS_LIST MCLS_CLIMATE[] = {"HydroClimate", "HydroClimate data modules"};
CONST_CHARS_LIST M_TSD_RD[] = {"TSD_RD", "Read time series data from HydroClimate database."};
CONST_CHARS_LIST M_ITP[] = {"ITP", "Interpolation of P, T, etc."};

// #define MCLS_CLIMATE                           "HydroClimate"
// #define MCLSDESC_CLIMATE                       "HydroClimate data modules"
// #define MID_TSD_RD                             "TSD_RD"
// #define MDESC_TSD_RD                           "Read time series data from HydroClimate database."
// #define MID_ITP                                "ITP"
// #define MDESC_ITP                              "Interpolation of P, T, etc."

/// Soil temperature
CONST_CHARS_LIST MCLS_SOLT[] = {"Soil Temperature", "Calculate the soil temperature."};
CONST_CHARS_LIST M_STP_FP[] = {"STP_FP", "Finn Plauborg Method to compute soil temperature."};

// #define MCLS_SOLT                              "Soil Temperature"
// #define MCLSDESC_SOLT                          "Calculate the soil temperature."
// #define MID_STP_FP                             "STP_FP"
// #define MDESC_STP_FP                           "Finn Plauborg Method to compute soil temperature."

/// Interception
CONST_CHARS_LIST MCLS_INTERC[] = {"Interception", "Precipation interception module"};
CONST_CHARS_LIST M_PI_SVSC[] = {"PI_SVSC", "Precipitation interception by seasonal variation of storage capacity method"};
CONST_CHARS_LIST M_PI_MCS[] = {"PI_MCS", "Precipitation interception based on Maximum Canopy Storage"};

// #define MCLS_INTERC                            "Interception"
// #define MCLSDESC_INTERC                        "Precipation interception module"
// #define MID_PI_SVSC                            "PI_SVSC"
// #define MDESC_PI_SVSC                          "Precipitation interception by seasonal variation of storage capacity method"
// #define MID_PI_MCS                             "PI_MCS"
// #define MDESC_PI_MCS                           "Precipitation interception based on Maximum Canopy Storage"

/// Snow redistribution
CONST_CHARS_LIST MCLS_SNO_RD[] = {"Snow redistribution", "Snow redistribution calculation"};
CONST_CHARS_LIST M_SRD_MB[] = {"SRD_MB", "Original WetSpa algorithm"};

// #define MCLS_SNO_RD                            "Snow redistribution"
// #define MCLSDESC_SNO_RD                        "Snow redistribution calculation"
// #define MID_SRD_MB                             "SRD_MB"
// #define MDESC_SRD_MB                           "Original WetSpa algorithm"

/// Snow sublimation
CONST_CHARS_LIST MCLS_SNO_SB[] = {"Snow sublimation", "Calculate the amount of snow sublimation ."};
CONST_CHARS_LIST M_SSM_PE[] = {"SSM_PE", "A simple method that used in the old WetSpa to calculate snow sublimation."};

// #define MCLS_SNO_SB                            "Snow sublimation"
// #define MCLSDESC_SNO_SB                        "Calculate the amount of snow sublimation ."
// #define MID_SSM_PE                             "SSM_PE"
// #define MDESC_SSM_PE                           "A simple method that used in the old WetSpa to calculate snow sublimation."

/// Snow melt
CONST_CHARS_LIST MCLS_SNOW[] = {"Snow accumulation and melt", "Calculate the amount of snow  accumulation andmelt."};
CONST_CHARS_LIST M_SNO_WB[] = {"SNO_WB", "Calculate snow water balance"};
CONST_CHARS_LIST M_SNO_DD[] = {"SNO_DD", "Degree-Day method (Martinec et al., 1983) for snow melt modeling"};
CONST_CHARS_LIST M_SNO_SP[] = {"SNO_SP", "Snowpack Daily method from SWAT"};

// #define MCLS_SNOW                              "Snow accumulation and melt"
// #define MCLSDESC_SNOW                          "Calculate the amount of snow  accumulation andmelt."
// #define MID_SNO_WB                             "SNO_WB"
// #define MDESC_SNO_WB                           "Calculate snow water balance"
// #define MID_SNO_DD                             "SNO_DD"
// #define MDESC_SNO_DD                           "Degree-Day method (Martinec et al., 1983) for snow melt modeling"
// #define MID_SNO_SP                             "SNO_SP"
// #define MDESC_SNO_SP                           "Snowpack Daily method from SWAT"

/// Potential Evapotranspiration Modules
CONST_CHARS_LIST MCLS_PET[] = {"Potential Evapotranspiration", "Calculate the potential evapotranspiration"};
CONST_CHARS_LIST M_PET_H[] = {"PET_H", "Hargreaves method for calculating the potential evapotranspiration."};
CONST_CHARS_LIST M_PET_PT[] = {"PET_PT", "PriestleyTaylor method for calculating the potential evapotranspiration."};
CONST_CHARS_LIST M_PET_PM[] = {"PET_PM", "Penman Monteith method for calculating the potential evapotranspiration."};

// #define MCLS_PET                               "Potential Evapotranspiration"
// #define MCLSDESC_PET                           "Calculate the potential evapotranspiration"
// #define MID_PET_H                              "PET_H"
// #define MDESC_PET_H                            "Hargreaves method for calculating the potential evapotranspiration."
// #define MID_PET_PT                             "PET_PT"
// #define MDESC_PET_PT                           "PriestleyTaylor method for calculating the potential evapotranspiration."
// #define MID_PET_PM                             "PET_PM"
// #define MDESC_PET_PM                           "Penman Monteith method for calculating the potential evapotranspiration."

/// Actual Evapotranspiration
CONST_CHARS_LIST MCLS_AET[] = {"Actual Evapotranspiration", "Calculates potential plant transpiration and potential and actual soil evaporation. "};
CONST_CHARS_LIST M_AET_PTH[] = {"AET_PTH", "Potential plant transpiration for Priestley-Taylor and Hargreaves ET methods "};
CONST_CHARS_LIST M_SET_LM[] = {"SET_LM", "Evapotranspiration from soil related linearly with soil moisture (WetSpa)"};

// #define MCLS_AET                               "Actual Evapotranspiration"
// #define MCLSDESC_AET                           "Calculates potential plant transpiration and potential and actual soil evaporation. "
// #define MID_AET_PTH                            "AET_PTH"
// #define MDESC_AET_PTH                          "Potential plant transpiration for Priestley-Taylor and Hargreaves ET methods "
// #define MID_SET_LM                             "SET_LM"
// #define MDESC_SET_LM                           "Evapotranspiration from soil related linearly with soil moisture (WetSpa)"

/// Depression
CONST_CHARS_LIST MCLS_DEP[] = {"Depression", "Calculate depression storage."};
CONST_CHARS_LIST M_DEP_FS[] = {"DEP_FS", "A simple fill and spill method method to calculate depression storage."};
CONST_CHARS_LIST M_DEP_LINSLEY[] = {"DEP_LINSLEY", "Linsley method to calculate depression storage"};

// #define MCLS_DEP                               "Depression"
// #define MCLSDESC_DEP                           "Calculate depression storage."
// #define MID_DEP_FS                             "DEP_FS"
// #define MDESC_DEP_FS                           "A simple fill and spill method method to calculate depression storage."
// #define MID_DEP_LINSLEY                        "DEP_LINSLEY"
// #define MDESC_DEP_LINSLEY                      "Linsley method to calculate depression storage"

/// Surface runoff
CONST_CHARS_LIST MCLS_SUR_RUNOFF[] = {"Surface runoff", "Infiltration and surface runoff of excess precipitation."};
CONST_CHARS_LIST M_SUR_MR[] = {"SUR_MR", "Modified rational method to calculate infiltration and excess precipitation."};
CONST_CHARS_LIST M_SUR_CN[] = {"SUR_CN", "SCS curve number method to calculate infiltration and excess precipitation."};
CONST_CHARS_LIST M_SUR_SGA[] = {"SUR_SGA", "Green Ampt Method to calculate infiltration and excess precipitation."};

// #define MCLS_SUR_RUNOFF                        "Surface runoff"
// #define MCLSDESC_SUR_RUNOFF                    "Infiltration and surface runoff of excess precipitation."
// #define MID_SUR_MR                             "SUR_MR"
// #define MDESC_SUR_MR                           "Modified rational method to calculate infiltration and excess precipitation."
// #define MID_SUR_CN                             "SUR_CN"
// #define MDESC_SUR_CN                           "SCS curve number method to calculate infiltration and excess precipitation."
// #define MID_SUR_SGA                            "SUR_SGA"
// #define MDESC_SUR_SGA                          "Modified rational method to calculate infiltration and excess precipitation."

/// Interflow
CONST_CHARS_LIST MCLS_INTERFLOW[] = {"Interflow routing", "Interflow routing."};
CONST_CHARS_LIST M_IKW_IF[] = {"IKW_IF", "interflow routing using the method of WetSpa model."};
CONST_CHARS_LIST M_IUH_IF[] = {"IUH_IF", "IUH overland method to calculate interflow routing."};

// #define MCLS_INTERFLOW                         "Interflow routing"
// #define MCLSDESC_INTERFLOW                     "Interflow routing."
// #define MID_IKW_IF                             "IKW_IF"
// #define MDESC_IKW_IF                           "interflow routing using the method of WetSpa model."
// #define MID_IUH_IF                             "IUH_IF"
// #define MDESC_IUH_IF                           "IUH overland method to calculate interflow routing."

/// Percolation
CONST_CHARS_LIST MCLS_PERCO[] = {"Percolation", "Calculate the amount of water percolated out of the root zone within the time step."};
CONST_CHARS_LIST M_PER_PI[] = {"PER_PI", "Percolation calculated by Darcy's law and Brooks-Corey equation"};
CONST_CHARS_LIST M_PER_STR[] = {"PET_STR", "Percolation calculated by storage routing method"};
CONST_CHARS_LIST M_PERCO_DARCY[] = {"PERCO_DARCY", "The method relating percolation with soil moisture and pore size distribution index used in the original WetSpa will be the default method to estimate percolation out of the root zone."};

// #define MCLS_PERCO                             "Percolation"
// #define MCLSDESC_PERCO                         "Calculate the amount of water percolated out of the root zone within the time step."
// #define MID_PER_PI                             "PER_PI"
// #define MDESC_PER_PI                           "Percolation calculated by Darcy's law and Brooks-Corey equation"
// #define MID_PER_STR                            "PET_STR"
// #define MDESC_PER_STR                          "Percolation calculated by storage routing method"
// #define MID_PERCO_DARCY                        "PERCO_DARCY"
// #define MDESC_PERCO_DARCY                      "The method relating percolation with soil moisture and pore size distribution index used in the original WetSpa will be the default method to estimate percolation out of the root zone."

/// Subsurface
CONST_CHARS_LIST MCLS_SUBSURFACE[] = {"Subsurface", "Subsurface Runoff"};
CONST_CHARS_LIST M_SSR_DA[] = {"SSR_DA", "Darcy's law and the kinematic approximation; Water is routed cell-to-cell according to D8 flow direction"};

// #define MCLS_SUBSURFACE                        "Subsurface"
// #define MCLSDESC_SUBSURFACE                    "Subsurface Runoff"
// #define MID_SSR_DA                             "SSR_DA"
// #define MDESC_SSR_DA                           "Darcy's law and the kinematic approximation; Water is routed cell-to-cell according to D8 flow direction"

/// Soil water balance
CONST_CHARS_LIST MCLS_WTRBALANCE[] = {"Water banlance", "Water balance calculation"};
CONST_CHARS_LIST M_SOL_WB[] = {"SOL_WB", "Soil water balance calculation"};

// #define MCLS_WTRBALANCE                        "Water banlance"
// #define MCLSDESC_WTRBALANCE                    "Water balance calculation"
// #define MID_SOL_WB                             "SOL_WB"
// #define MDESC_SOL_WB                           "Soil water balance calculation"

/// Hillslope hydrology
CONST_CHARS_LIST MCLS_HS_HYDRO[] = {"Hillslope water balance", "Water balance calculation in hillslope."};
CONST_CHARS_LIST M_HS_WB[] = {"HS_WB", "Hillsloope water balance."};

// #define MCLS_HS_HYDRO                          "Hillslope water balance"
// #define MCLSDESC_HS_HYDRO                      "Water balance calculation in hillslope."
// #define MID_HS_WB                              "HS_WB"
// #define MDESC_HS_WB                            "Hillsloope water balance."

/// Paddy
CONST_CHARS_LIST MCLS_PADDY[] = {"Paddy", "Paddy simulations"};
CONST_CHARS_LIST M_IMP_SWAT[] = {"IMP_SWAT", "SWAT method, simulates depressional areas that do not drain to the stream network (pothole) and impounded areas such as rice paddies"};

// #define MCLS_PADDY                             "Paddy"
// #define MCLSDESC_PADDY                         "Paddy simulations"
// #define MID_IMP_SWAT                           "IMP_SWAT"
// #define MDESC_IMP_SWAT                         "SWAT method, simulates depressional areas that do not drain to the stream network (pothole) and impounded areas such as rice paddies"

/// Groundwater
CONST_CHARS_LIST MCLS_GW[] = {"Groundwater", "Calculate groundwater balance and baseflow."};
CONST_CHARS_LIST M_GW_RSVR[] = {"GW_RSVR", "Calculate groundwater using reservoir method for storm model"};
CONST_CHARS_LIST M_GWA_RE[] = {"GWA_RE", "Reservoir Method to calculate groundwater balance and baseflow for longterm model"};

// #define MCLS_GW                                "Groundwater"
// #define MCLSDESC_GW                            "Calculate groundwater balance and baseflow."
// #define MID_GW_RSVR                            "GW_RSVR"
// #define MDESC_GW_RSVR                          "Calculate groundwater using reservoir method for storm model"
// #define MID_GWA_RE                             "GWA_RE"
// #define MDESC_GWA_RE                           "Reservoir Method to calculate groundwater balance and baseflow for longterm model"

/// Erosion related modules
CONST_CHARS_LIST MCLS_OL_EROSION[] = {"Overland erosion", "Calculate the amount sediment yield of overland erosion."};
CONST_CHARS_LIST MCLS_CH_EROSION[] = {"Channel erosion", "Calculate the amount channel erosion."};
CONST_CHARS_LIST M_SplashEro_Park[] = {"SplashEro_Park", "use Park equation to calculate sediment yield of each cell"};
CONST_CHARS_LIST M_KINWAVSED_OL[] = {"KinWavSed_OL", "Use energy function(Govers) method to calculate sediment yield routing of each hillslope cell"};
CONST_CHARS_LIST M_KINWAVSED_CH[] = {"KinWavSed_CH", "Srinivasan & Galvao function to calculate sediment yield routing of each channel cell"};
CONST_CHARS_LIST M_SERO_MUSLE[] = {"SERO_MUSLE", "use MUSLE method to calculate sediment yield of each cell"};
CONST_CHARS_LIST M_IUH_SED_OL[] = {"IUH_SED_OL", "Overland routing of sediment using IUH"};
CONST_CHARS_LIST M_NUTR_CH[] = {"NUTR_CH", "Channel routing of nutrients"};

// #define MCLS_OL_EROSION                        "Overland erosion"
// #define MCLS_CH_EROSION                        "Channel erosion"
// #define MCLSDESC_OL_EROSION                    "Calculate the amount sediment yield of overland erosion."
// #define MCLSDESC_CH_EROSION                    "Calculate the amount channel erosion."
// #define MID_SplashEro_Park                     "SplashEro_Park"
// #define MDESC_SplashEro_Park                   "use Park equation to calculate sediment yield of each cell"
// #define MID_KINWAVSED_OL                       "KinWavSed_OL"
// #define MID_KINWAVSED_CH                       "KinWavSed_CH"
// #define MDESC_KINWAVSED_OL                     "Use energy function(Govers) method to calculate sediment yield routing of each hillslope cell"
// #define MDESC_KINWAVSED_CH                     "Srinivasan & Galvao function to calculate sediment yield routing of each channel cell"
// #define MID_SERO_MUSLE                         "SERO_MUSLE"
// #define MDESC_SERO_MUSLE                       "use MUSLE method to calculate sediment yield of each cell"
// #define MID_IUH_SED_OL                         "IUH_SED_OL"
// #define MDESC_IUH_SED_OL                       "Overland routing of sediment using IUH"

// #define MID_NUTR_CH                            "NUTR_CH"
// #define MDESC_NUTR_CH                          "Channel routing of nutrients"

/// Management
CONST_CHARS_LIST MCLS_MGT[] = {"Mangement practices", "BMP related modules"};
CONST_CHARS_LIST M_PLTMGT_SWAT[] = {"PLTMGT_SWAT", "Plant mangement operation modeling method in SWAT"};
CONST_CHARS_LIST M_NPSMGT[] = {"NPSMGT", "Non-point source pollution management"};

// #define MCLS_MGT                               "Mangement practices"
// #define MCLSDESC_MGT                           "BMP related modules"
// #define MID_PLTMGT_SWAT                        "PLTMGT_SWAT"
// #define MDESC_PLTMGT_SWAT                      "Plant mangement operation modeling method in SWAT"
// #define MID_NPSMGT                             "NPSMGT"
// #define MDESC_NPSMGT                           "Non-point source pollution management"

/// Ecology
CONST_CHARS_LIST MCLS_PG[] = {"Plant growth", "Calculate the amount of plant growth."};
CONST_CHARS_LIST M_PG_EPIC[] = {"PG_EPIC", "Calculate plant growth using a simplified version of the EPIC plant growth model as in SWAT"};
CONST_CHARS_LIST M_PG_ORYZA[] = {"PG_ORYZA", "Rice crop growth module of ORYZA2000 model"};

// #define MCLS_PG                                "Plant growth"
// #define MCLSDESC_PG                            "Calculate the amount of plant growth."
// #define MID_PG_EPIC                            "PG_EPIC"
// #define MDESC_PG_EPIC                          "Calculate plant growth using a simplified version of the EPIC plant growth model as in SWAT"
// #define MID_PG_ORYZA                           "PG_ORYZA"
// #define MDESC_PG_ORYZA                         "Rice crop growth module of ORYZA2000 model"

/// Overland routing related modules
CONST_CHARS_LIST MCLS_OL_ROUTING[] = {"Overland routing", "Overland routing module"};
CONST_CHARS_LIST M_IKW_OL[] = {"IKW_OL", "Overland routing using 4-point implicit finite difference method."};
CONST_CHARS_LIST M_IUH_OL[] = {"IUH_OL", "IUH overland method to calculate overland flow routing."};

// #define MCLS_OL_ROUTING                        "Overland routing"
// #define MCLSDESC_OL_ROUTING                    "Overland routing module"
// #define MID_IKW_OL                             "IKW_OL"
// #define MDESC_IKW_OL                           "Overland routing using 4-point implicit finite difference method."
// #define M_IUH_OL                             "IUH_OL"
// #define MDESC_IUH_OL                           "IUH overland method to calculate overland flow routing."

/// Channel routing related modules
CONST_CHARS_LIST MCLS_CH_ROUTING[] = {"Channel routing", "Channel routing modules"};
CONST_CHARS_LIST M_CH_DW[] = {"CH_DW", "Channel routing using diffusive wave equation."};
CONST_CHARS_LIST M_CH_MSK[] = {"CH_MSK", "Channel routing using Muskingum-Cunge method of storm model."};
CONST_CHARS_LIST M_IKW_CH[] = {"IKW_CH", "Channel routing using 4-point implicit finite difference method for kinematic wave."};
CONST_CHARS_LIST M_MUSK_CH[] = {"MUSK_CH", "Channel routing using Muskingum-Cunge method of longterm model."};

// #define MCLS_CH_ROUTING                        "Channel routing"
// #define MCLSDESC_CH_ROUTING                    "Channel routing modules"
// #define MID_CH_DW                              "CH_DW"
// #define MDESC_CH_DW                            "Channel routing using diffusive wave equation."
// #define MID_CH_MSK                             "CH_MSK"
// #define MDESC_CH_MSK                           "Channel routing using Muskingum-Cunge method of storm model."
// #define MID_IKW_CH                             "IKW_CH"
// #define MDESC_IKW_CH                           "Channel routing using 4-point implicit finite difference method for kinematic wave."
// #define MID_MUSK_CH                            "MUSK_CH"
// #define MDESC_MUSK_CH                          "Channel routing using Muskingum-Cunge method of longterm model."

/// Sediment routing related modules
CONST_CHARS_LIST MCLS_SED_ROUTING[] = {"Sediment routing", "Sediment channel routing modules."};
CONST_CHARS_LIST M_SEDR_SBAGNOLD[] = {"SEDR_SBAGNOLD", "Sediment channel routing using variable channel dimension method as used in SWAT."};

// #define MCLS_SED_ROUTING                       "Sediment routing"
// #define MCLSDESC_SED_ROUTING                   "Sediment channel routing modules."
// #define MID_SEDR_SBAGNOLD                      "SEDR_SBAGNOLD"
// #define MDESC_SEDR_SBAGNOLD                    "Sediment channel routing using variable channel dimension method as used in SWAT."

/// Nutrient
/// carbon, nitrogen, and phosphorus mineralization and immobilization etc
CONST_CHARS_LIST MCLS_NUTRCYC[] = {"nutrient cycling", "Carbon, nitrogen, and phosphorus cycling"};
CONST_CHARS_LIST M_NUTR_TF[] = {"NUTR_TF", "Daily nitrogen and phosphorus mineralization and immobilization considering fresh organic material (plant residue) and active and stable humus material."};

// #define MCLS_NUTRCYC                           "nutrient cycling"
// #define MCLSDESC_NUTRCYC                       "Carbon, nitrogen, and phosphorus cycling"
// #define MID_NUTR_TF                            "NUTR_TF"
// #define MDESC_NUTR_TF                          "Daily nitrogen and phosphorus mineralization and immobilization considering fresh organic material (plant residue) and active and stable humus material."

/// Nutrient removed and loss in surface runoff
CONST_CHARS_LIST MCLS_NUTRSED[] = {"Nutrient removed and loss in surface runoff, lateral flow, tile flow, and percolation out of the profile.", "Nutrient removed and loss in surface runoff, lateral flow, tile flow, and percolation out of the profile."};
CONST_CHARS_LIST M_NUTRSED[] = {"NUTRSED", "Nutrient removed and loss in surface runoff, lateral flow, tile flow, and percolation out of the profile."};

// #define MCLS_NUTRSED                           "Nutrient removed and loss in surface runoff, lateral flow, tile flow, and percolation out of the profile."
// #define MCLSDESC_NUTRSED                       "Nutrient removed and loss in surface runoff, lateral flow, tile flow, and percolation out of the profile."
// #define MID_NUTRSED                            "NUTRSED"
// #define MDESC_NUTRSED                          "Nutrient removed and loss in surface runoff, lateral flow, tile flow, and percolation out of the profile."

/// Atmospheric Deposition
CONST_CHARS_LIST MCLS_ATMDEP[] = {"AtmosphericDeposition", "AtmosphericDeposition"};
CONST_CHARS_LIST M_ATMDEP[] = {"ATMDEP", "AtmosphericDeposition"};

// #define MCLS_ATMDEP                            "AtmosphericDeposition"
// #define MCLSDESC_ATMDEP                        "AtmosphericDeposition"
// #define MID_ATMDEP                             "ATMDEP"
// #define MDESC_ATMDEP                           "AtmosphericDeposition"

/// Nutrient remove
CONST_CHARS_LIST MCLS_NutRemv[] = {"Nutrient remove", "Simulates the loss of nitrate and phosphorus via surface runoff"};
CONST_CHARS_LIST M_NUTRMV[] = {"NutrMV", "Simulates the loss of nitrate and phosphorus via surface runoff"};

// #define MCLS_NutRemv                           "Nutrient remove"
// #define MCLSDESC_NutRemv                       "Simulates the loss of nitrate and phosphorus via surface runoff"
// #define MID_NUTRMV                             "NutrMV"
// #define MDESC_NUTRMV                           "Simulates the loss of nitrate and phosphorus via surface runoff"

/// Nutrient routing
CONST_CHARS_LIST M_SSR_NUTR[] = {"SSR_NUTR", "Nutrient routing through soil flow"};
CONST_CHARS_LIST M_IUH_NUTR_OL[] = {"IUH_NUTR_OL", "Overland nutrient routing"};

// #define MID_SSR_NUTR                           "SSR_NUTR"
// #define MDESC_SSR_NUTR                         "Nutrient routing through soil flow"
// #define MID_IUH_NUTR_OL                        "IUH_NUTR_OL"
// #define MDESC_IUH_NUTR_OL                      "Overland nutrient routing"

/// Nutrient loading contributed by groundwater flow
CONST_CHARS_LIST MCLS_NUTRGW[] = {"Nutrient in groundwater", "Simulates the tutrient loading contributed by groundwater flow"};
CONST_CHARS_LIST M_NUTRGW[] = {"NutrGW", "Simulates the tutrient loading contributed by groundwater flow"};

// #define MCLS_NUTRGW                            "Nutrient in groundwater"
// #define MCLSDESC_NUTRGW                        "Simulates the tutrient loading contributed by groundwater flow"
// #define MID_NUTRGW                             "NutrGW"
// #define MDESC_NUTRGW                           "Simulates the tutrient loading contributed by groundwater flow"

/// In-stream nutrient transformations
CONST_CHARS_LIST MCLS_NutCHRout[] = {"Nutrient in reach", "In-stream nutrient transformations"};
CONST_CHARS_LIST M_NUTRCH_QUAL2E[] = {"NutrCH_QUAL2E", "In-stream nutrient transformations"};

// #define MCLS_NutCHRout                         "Nutrient in reach"
// #define MCLSDESC_NutCHRout                     "In-stream nutrient transformations"

// #define MID_NUTRCH_QUAL2E                      "NutrCH_QUAL2E"
// #define MDESC_NUTRCH_QUAL2E                    "In-stream nutrient transformations"

///////////////////////////////////////////////////////////////////////////////////////////
/// Define units' names and descriptions common used in SEIMS, in case of inconsistency ///
/// By LiangJun Zhu, HuiRan Gao ///
/// Apr. , 2016  //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

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
CONST_CHARS_LIST VAR_CH_ALGAEConc[] = {"ch_algaeConc" }; 
CONST_CHARS_LIST VAR_CH_ONCO[] = {"ch_onco", "Channel organic nitrogen concentration in basin"}; 
CONST_CHARS_LIST VAR_CH_OPCO[] = {"ch_opco", "Channel organic phosphorus concentration in basin"}; 
CONST_CHARS_LIST VAR_AMMO_CH[] = {"ammoToCh", "amount of ammonium transported with lateral flow"}; 
CONST_CHARS_LIST VAR_CH_NH4[] = {"ch_nh4", "ammonia nitrogen in reach"}; 
CONST_CHARS_LIST VAR_CH_NH4Conc[] = {"ch_nh4Conc" }; 
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
CONST_CHARS_LIST VAR_CH_CHLORAConc[] = {"CH_chloraConc" }; 
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
CONST_CHARS_LIST VAR_CHWTRDEPTH[] = {"CHWTRDEPTH", "channel water depth"}; /// m_chWtrDepth
CONST_CHARS DESC_PRECHWTDEPTH = "channel water depth of previous timestep";
CONST_CHARS_LIST VAR_CLAY[] = {"CLAY", "Percent of clay content"}; /// m_soilClay
CONST_CHARS_LIST VAR_CMN[] = {"cmn", "Rate coefficient for mineralization of the humus active organic nutrients"}; /// m_minrlCoef
CONST_CHARS_LIST VAR_CN2[] = {"CN2", "Curve Number value under moisture condition II"}; /// m_cn2
CONST_CHARS_LIST VAR_CO2[] = {"Co2", "CO2 Concentration"}; /// m_co2Conc
CONST_CHARS_LIST VAR_CO2HI[] = {"CO2HI", "elevated CO2 atmospheric concentration corresponding the 2nd point on the radiation use efficiency curve"}; /// m_co2Conc2ndPt
CONST_CHARS_LIST VAR_SUR_COD[] = {"sur_cod", "carbonaceous oxygen demand of surface runoff"}; /// m_surfRfCod
CONST_CHARS DESC_COD_CH = "carbonaceous oxygen demand loading to reach";
CONST_CHARS_LIST VAR_COD_N[] = {"cod_n", "Conversion factor"}; 
CONST_CHARS_LIST VAR_COD_K[] = {"cod_k", "Reaction coefficient"}; 
CONST_CHARS_LIST VAR_COND_RATE[] = {"Cond_rate", "Rate of decline in stomatal conductance per unit increase in vapor pressure deficit"}; 
CONST_CHARS_LIST VAR_CONDUCT[] = {"Conductivity", "saturation hydraulic conductivity"}; 
CONST_CHARS_LIST VAR_CONV_WT[] = {"conv_wt", "factor which converts kg/kg soil to kg/ha"}; /// m_cvtWt
CONST_CHARS_LIST VAR_CROP_LOOKUP[] = {"CropLookup", "Crop lookup table"}; /// m_cropLookup
CONST_CHARS_LIST VAR_CSWAT[] = {"cswat", "carbon modeling method"}; /// m_cbnModel
CONST_CHARS_LIST VAR_PCP[] = {"D_P", "Precipitation of each time step on current cell"}; /// m_pcp
CONST_CHARS_LIST VAR_DAYLEN[] = {"daylength", "day length"}; /// m_dayLen
CONST_CHARS_LIST VAR_DAYLEN_MIN[] = {"daylenmin", "minimum day length"}; /// m_dayLenMin
CONST_CHARS_LIST VAR_DEEPST[] = {"deepst", "depth of water in deep aquifer"}; 
CONST_CHARS_LIST VAR_DEET[] = {"DEET", "evaporation from depression storage"}; /// m_deprStoET
CONST_CHARS_LIST VAR_DEM[] = {"DEM", "Digital Elevation Model"}; /// m_dem
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
CONST_CHARS_LIST VAR_CH_SOLPConc[] = {"CH_SOLPConc" }; 
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
CONST_CHARS_LIST VAR_POT_TILEMM[] = {"pot_tilemm", "Average daily outflow to main channel from tile flow if drainage tiles are installed in the pothole"}; 
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
CONST_CHARS_LIST VAR_GRRE[] = {"GRRE" }; 
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
CONST_CHARS_LIST VAR_MINPGW_TOCH[] = {"minpgwToCh", "soluble P in groundwater to channel"}; /// m_gwSolPToCh
CONST_CHARS_LIST VAR_MOIST_IN[] = {"Moist_in", "Initial soil moisture"}; /// m_initSoilWtrStoRatio
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
CONST_CHARS_LIST VAR_CH_NO3Conc[] = {"CH_NO3Conc" }; 
CONST_CHARS_LIST VAR_NO2_TOCH[] = {"nitriteToCh" }; /// m_no2ToCh
CONST_CHARS_LIST VAR_CH_NO2[] = {"CH_NO2", "nitrite in reach"}; 
CONST_CHARS_LIST VAR_CH_NO2Conc[] = {"CH_NO2Conc" }; 
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
CONST_CHARS_LIST VAR_CH_ORGNConc[] = {"CH_ORGNConc" }; 
CONST_CHARS_LIST VAR_CH_ORGP[] = {"CH_ORGP", "organic phosphorus in reach"}; 
CONST_CHARS_LIST VAR_CH_ORGPConc[] = {"CH_ORGPConc" }; 
CONST_CHARS_LIST VAR_CH_TN[] = {"CH_TN", " total N amount in reach"}; 
CONST_CHARS_LIST VAR_CH_TNConc[] = {"CH_TNConc", "total N concentration in reach"}; 
CONST_CHARS_LIST VAR_CH_TP[] = {"CH_TP", " total P amount in reach"}; 
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
CONST_CHARS_LIST VAR_PERDE[] = {"perde" }; 
CONST_CHARS_LIST VAR_PET[] = {"PET", "Potential Evapotranspiration of day"}; /// m_pet
CONST_CHARS_LIST VAR_PET_HCOEF[] = {"HCoef_pet", "Coefficient related to radiation used in Hargreaves method"}; 
CONST_CHARS_LIST VAR_PHOSKD[] = {"phoskd", "Phosphorus soil partitioning coefficient"}; 
CONST_CHARS_LIST VAR_PHUBASE[] = {"PHUBASE", "base zero total heat units (used when no land cover is growing)"}; /// m_phuBase
CONST_CHARS_LIST VAR_PHUPLT[] = {"PHU_PLT", "total number of heat unites (hours) needed to bring plant to maturity"}; /// m_phuPlt
CONST_CHARS_LIST VAR_PHUTOT[] = {"PHU0", "annual average total potential heat units (used when no crop is growing)"}; /// m_phuAnn
CONST_CHARS_LIST VAR_PI_B[] = {"Pi_b", "Interception Storage Capacity Exponent"}; /// m_intcpStoCapExp
CONST_CHARS_LIST VAR_PCP2CANFR_PR[] = {"pcp2canfr_pr" }; /// m_pcp2CanalFr
CONST_CHARS_LIST VAR_EMBNKFR_PR[] = {"embnkfr_pr" }; /// m_embnkFr
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
CONST_CHARS_LIST VAR_CH_CODConc[] = {"CH_CODConc" }; 
CONST_CHARS_LIST VAR_CH_DOX[] = {"ch_dox", "dissolved oxygen in reach"}; 
CONST_CHARS_LIST VAR_CH_DOXConc[] = {"ch_doxConc" }; 
CONST_CHARS_LIST VAR_RCH_BANKERO[] = {"rch_bank_ero", "reach bank erosion"}; /// m_rchBankEro
CONST_CHARS_LIST VAR_RCH_DEG[] = {"rch_deg", "reach degradation"}; /// m_rchDeg
CONST_CHARS_LIST VAR_RCH_DEP[] = {"rch_dep", "reach deposition"}; /// m_rchDep
CONST_CHARS_LIST VAR_RCH_DEPNEW[] = {"rch_depnew", "Channel new deposition"}; /// m_dltRchDep
CONST_CHARS_LIST VAR_RCH_DEPSAND[] = {"rch_depsand", "Sand deposition in channel"}; /// m_rchDepSand
CONST_CHARS_LIST VAR_RCH_DEPSILT[] = {"rch_depsilt", "Silt deposition in channel"}; /// m_rchDepSilt
CONST_CHARS_LIST VAR_RCH_DEPCLAY[] = {"rch_depclay" }; /// m_rchDepClay
CONST_CHARS_LIST VAR_RCH_DEPSAG[] = {"rch_depsag", "Small aggregate deposition in channel"}; /// m_rchDepSag
CONST_CHARS_LIST VAR_RCH_DEPLAG[] = {"rch_deplag", "Large aggregate deposition in channel"}; /// m_rchDepLag
CONST_CHARS_LIST VAR_RCH_DEPGRAVEL[] = {"rch_depgravel", "Gravel deposition in channel"}; /// m_rchDepGravel
CONST_CHARS_LIST VAR_FLDPLN_DEP[] = {"floodplain_dep", "Floodplain Deposition"}; /// m_fldplnDep
CONST_CHARS_LIST VAR_FLDPLN_DEPNEW[] = {"floodplain_depnew", "New deposits on floodplain"}; /// m_dltFldplnDep
CONST_CHARS_LIST VAR_FLDPLN_DEPSILT[] = {"floodplain_depsilt", "Deposition silt on floodplain"}; /// m_fldplnDepSilt
CONST_CHARS_LIST VAR_FLDPLN_DEPCLAY[] = {"floodplain_depclay", "Deposition clay on floodplain"}; /// m_fldplnDepClay
CONST_CHARS_LIST VAR_RCN[] = {"rcn", "concentration of nitrate in the rain"}; /// m_rainNO3Conc
CONST_CHARS_LIST VAR_Reinfiltration[] = {"Reinfiltration" }; 
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
CONST_CHARS_LIST VAR_SED_RECHConc[] = {"SEDRECHConc" }; /// m_sedConcRchOut
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
CONST_CHARS_LIST VAR_IRRDEPTH[] = {"irrDepth" }; 
CONST_CHARS_LIST VAR_POND_SOLPDECAY[] = {"pond_solpl" }; 

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
CONST_CHARS_LIST VAR_SOXYConc[] = {"soxyConc" }; 
CONST_CHARS_LIST VAR_SPCON[] = {"spcon", "Coefficient in sediment transport equation"}; /// m_sedTransEqCoef
CONST_CHARS_LIST VAR_SPEXP[] = {"spexp", "Exponent in sediment transport equation"}; /// m_sedTransEqExp
CONST_CHARS DESC_SR = "Solar radiation";
CONST_CHARS_LIST VAR_SR_MAX[] = {"srMax", "Max solar radiation"}; 
CONST_CHARS_LIST VAR_SRA[] = {"sra", " solar radiation for the day"}; 
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
/// Define unit names common used in SEIMS, in case of inconsistency /////
/// By LiangJun Zhu, HuiRan Gao ///
///Apr. , 2016  //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/* #define VAR_A_BNK "a_bnk"                           /// bank flow recession constant

CONST_CHARS_LIST VAR_ACC[] = { "acc", "Flow accumulation, equals to the number of accumulated cells" }; /// m_flowAccm

//#define VAR_ACC "acc" /// m_flowAccm, flow accumulation (number of accumulated cells)
#define VAR_ACC_INFIL "AccumuInfil"
#define VAR_ADDRNH4 "addrnh4"                       /// ammonium added by rainfall (kg/ha)
#define VAR_ADDRNO3 "addrno3"                       /// nitrate added by rainfall (kg/ha)
#define VAR_AET_PLT "AET_PLT" /// m_actPltET, actual amount of transpiration (mm H2O), ep_day in SWAT
#define VAR_AFERT_AMAXN "afert_AmaxN" /// m_autoFertMaxAnnApldMinN, Maximum amount of mineral N allowed to be applied in any one year (kg N/ha), auto_nyr
#define VAR_AFERT_FRTEFF "afert_frteff" /// m_autoFertEff, fertilizer application efficiency calculated as the amount of N applied divided by the amount of N removed at harvest
#define VAR_AFERT_FRTSURF "afert_frtsurf" /// m_autoFertSurfFr, Fraction of fertilizer applied to top 10mm of soil, the default is 0.2
#define VAR_AFERT_ID "afert_id" /// m_fertID, fertilizer ID from fertilizer database
#define VAR_AFERT_MAXN "afert_maxN" /// m_autoFertMaxApldN,  Maximum amount of mineral N allowed in any one application (kg N/ha)
#define VAR_AFERT_NSTRS "afert_nstrs" /// m_autoNStrsTrig, Nitrogen stress factor of cover/plant that triggers fertilization, usually set 0.90 to 0.95
#define VAR_AFERT_NSTRSID "afert_nstrsID" /// m_NStrsMeth, Code for approach used to determine amount of nitrogen to Cell
#define VAR_AFERT_NYLDT "afert_nyldTarg" /// m_autoFertNtrgtMod, modifier for auto fertilization target nitrogen content
#define VAR_AI0 "ai0"
#define VAR_AI1 "ai1"
#define VAR_AI2 "ai2"
#define VAR_AI3 "ai3"
#define VAR_AI4 "ai4"
#define VAR_AI5 "ai5"
#define VAR_AI6 "ai6"
#define VAR_AIRR_EFF "airr_eff" /// m_autoIrrEff, auto irrigation efficiency, 0 ~ 100
#define VAR_AIRR_LOCATION "airr_location" /// m_autoIrrLocNo, irrigation source location code
#define VAR_AIRR_SOURCE "airr_source" /// m_autoIrrSrc, auto irrigation source
#define VAR_AIRRSURF_RATIO "airrsurf_ratio" /// m_autoIrrWtr2SurfqR, surface runoff ratio (0-1) (0.1 is 10% surface runoff), IRR_ASQ
#define VAR_AIRRWTR_DEPTH "airrwtr_depth" /// m_autoIrrWtrD, amount of irrigation water applied each time auto irrigation is triggered (mm), 0 ~ 100, IRR_MX
#define VAR_ALAIMIN "alai_min" /// m_minLaiDorm, minimum LAI during winter dormant period
#define VAR_ALBDAY "ALBDAY" /// m_alb, albedo in the current day
#define VAR_CH_ALGAE "ch_algae"
#define VAR_CH_ALGAEConc "ch_algaeConc"
#define VAR_CH_ONCO "ch_onco"
#define VAR_CH_OPCO "ch_opco"
#define VAR_AMMO_CH "ammoToCh"
#define VAR_CH_NH4 "ch_nh4"
#define VAR_CH_NH4Conc "ch_nh4Conc"
#define VAR_ANION_EXCL "anion_excl" /// m_anionExclFr, fraction of porosity from which anions are excluded
#define VAR_AWTR_STRS_ID "awtr_strsID" /// m_wtrStrsID, Water stress identifier, 1 plant water demand, 2 soil water content
#define VAR_AWTR_STRS_TRIG "awtr_strsTrig" /// m_autoWtrStrsTrig, Water stress threshold that triggers irrigation, if m_wtrStresID is 1, the value usually 0.90 ~ 0.95
#define VAR_B_BNK "b_bnk"                           /// bank storage loss coefficient
#define VAR_BACT_SWF "bact_swf"
#define VAR_Base_ex "Base_ex"                       /// base flow recession exponent
#define VAR_BIO_E "BIO_E" /// m_biomEnrgRatio, Radiation-use efficicency or biomass-energy ratio
#define VAR_BIOEHI "BIOEHI" /// m_biomEnrgRatio2ndPt, Biomass-energy ratio corresponding to the 2nd point on the radiation use efficiency curve
#define VAR_BIOINIT "BIO_INIT" /// m_initBiom, initial dry weight biomass
#define VAR_BIOLEAF "BIO_LEAF" /// m_biomDropFr, fraction of biomass that drops during dormancy (for tree only), bio_leaf
#define VAR_BIOMASS "BIOMASS" /// m_biomass, land cover/crop biomass (dry weight), bio_ms in SWAT
#define VAR_BIOTARG "biotarg" /// m_biomTrgt, Biomass target
#define VAR_BKST "BKST"                             /// bank storage
#define VAR_BLAI "BLAI" /// m_maxLai, maximum (potential) leaf area index (BLAI in cropLookup db)
#define VAR_BMX_TREES "BMX_TREES" /// m_maxBiomTree, Maximum biomass for a forest (metric tons/ha), BMX_TREES in SWAT
#define VAR_BN1 "BN1" /// m_biomNFr1, nitrogen uptake parameter #1: normal fraction of N in crop biomass at emergence
#define VAR_BN2 "BN2" /// m_biomNFr2, nitrogen uptake parameter #2: normal fraction of N in crop biomass at 50% maturity
#define VAR_BN3 "BN3" /// m_biomNFr3, nitrogen uptake parameter #3: normal fraction of N in crop biomass at maturity
#define VAR_BNK0 "Bnk0" /// initial bank storage per meter of reach length
#define VAR_BP1 "BP1" /// m_biomPFr1, phosphorus uptake parameter #1: normal fraction of P in crop biomass at emergence
#define VAR_BP2 "BP2" /// m_biomPFr2, phosphorus uptake parameter #2: normal fraction of P in crop biomass at 50% maturity
#define VAR_BP3 "BP3" /// m_biomPFr3, phosphorus uptake parameter #3: normal fraction of P in crop biomass at maturity
#define VAR_C_RAIN "c_rain"
#define VAR_C_SNOW "c_snow"
#define VAR_C_SNOW12 "c_snow12"
#define VAR_C_SNOW6 "c_snow6"
#define VAR_C_WABA "C_WABA"                         /// Channel water balance in a text format for each reach and at each time step
#define VAR_CDN "cdn" /// m_denitCoef, rate coefficient for denitrification
#define VAR_CELL_LAT "celllat" /// m_cellLat, latitude of each valid cells
#define VAR_CH_DEP "DEP"
#define VAR_CH_DET "DET"
#define VAR_CH_DETCO "ChDetCo"
#define VAR_CH_FLOWCAP "CAP"
//#define VAR_CH_MANNING_FACTOR "CH_ManningFactor"
#define VAR_CH_SEDRATE "QSN"
#define VAR_CH_TCCO "ChTcCo"
#define VAR_CH_V "CHANV"
#define VAR_CH_VOL "CHANVOL"
#define VAR_CH_CHLORA "CH_chlora"
#define VAR_CH_CHLORAConc "CH_chloraConc"
#define VAR_CHL_A "chl_a" /// m_surfRfChlA, chlorophyll-a concentration in water yield
#define VAR_CHS0 "Chs0" /// m_initChStorage, initial channel storage per meter of reach length (m^3/m)
#define VAR_CHS0_PERC "chs0_perc" /// ,
#define VAR_CHSB "CHSB"
#define VAR_CHST "CHST" /// m_chStorage, channel storage
#define VAR_CHT "CHT" /// m_canHgt, canopy height for the day(m)
#define VAR_CHTMX "CHTMX" /// m_maxCanHgt, maximum canopy height (m)
#define VAR_CHWTRWIDTH "chwtrwidth" /// m_chWtrWth, channel water width
#define VAR_CHBTMWIDTH "chbtmwidth" // m_chBtmWth, channel bottom width, m
#define VAR_CHCROSSAREA "chCrossArea" // m_chCrossArea, channel cross-sectional area, m^2
#define VAR_CHWIDTH "CH_WIDTH" // m_chWth, channel width at bankfull
#define VAR_CHWTRDEPTH "CHWTRDEPTH" /// m_chWtrDepth, channel water depth
#define VAR_CLAY "CLAY" /// m_soilClay, Percent of clay content
#define VAR_CMN "cmn" /// m_minrlCoef, Rate coefficient for mineralization of the humus active organic nutrients
#define VAR_CN2 "CN2" /// m_cn2, Curve Number value under moisture condition II
#define VAR_CO2 "Co2" /// m_co2Conc, CO2 Concentration
#define VAR_CO2HI "CO2HI" /// m_co2Conc2ndPt, elevated CO2 atmospheric concentration corresponding the 2nd point on the radiation use efficiency curve
#define VAR_SUR_COD "sur_cod" /// m_surfRfCod, carbonaceous oxygen demand of surface runoff
#define VAR_COD_N "cod_n"
#define VAR_COD_K "cod_k"
#define VAR_COND_RATE "Cond_rate"                     /// Rate of decline in automata's conductance per unit increase in vapor pressure deficit
#define VAR_CONDUCT "Conductivity"
#define VAR_CONV_WT "conv_wt" /// m_cvtWt, factor which converts kg/kg soil to kg/ha
#define VAR_CROP_LOOKUP "CropLookup" /// m_cropLookup, Crop lookup table
#define VAR_CSWAT "cswat" /// m_cbnModel, carbon modeling method
#define VAR_PCP "D_P" /// m_pcp, Distribution of precipitation
#define VAR_DAYLEN "daylength" /// m_dayLen, day length
#define VAR_DAYLEN_MIN "daylenmin" /// m_dayLenMin, minimum day length
#define VAR_DEEPST "deepst"
#define VAR_DEET "DEET" /// m_deprStoET, evaporation from the depression storage
#define VAR_DEM "DEM" /// m_dem, Digital Elevation Model
#define VAR_DEPREIN "Depre_in"                        /// initial depression storage coefficient
#define VAR_DEPRESSION "Depression"                   /// Depression storage capacity
#define VAR_DETSPLASH "DETSplash"
#define VAR_DETACH_SAND "det_sand" /// m_detSand
#define VAR_DETACH_SILT "det_silt" /// m_detSilt
#define VAR_DETACH_CLAY "det_clay" /// m_detClay
#define VAR_DETACH_SAG "det_smagg" /// m_detSmAgg
#define VAR_DETACH_LAG "det_lgagg" /// m_detLgAgg
#define VAR_SANDYLD "sand_yld" /// m_eroSand
#define VAR_SILTYLD "silt_yld" /// m_eroSilt
#define VAR_CLAYYLD "clay_yld" /// m_eroClay
#define VAR_SAGYLD "sag_yld" /// m_eroSmAgg
#define VAR_LAGYLD "lag_yld" /// m_eroLgAgg
#define VAR_DF_COEF "df_coef" /// Deep percolation coefficient
#define VAR_CH_SOLP "CH_SOLP"
#define VAR_CH_SOLPConc "CH_SOLPConc"
#define VAR_DLAI "DLAI" /// m_dormPHUFr, fraction of growing season(PHU) when senescence becomes dominant
#define VAR_DORMHR "dormhr" /// m_dormHr, time threshold (hour) used to define dormant period for plant
#define VAR_DORMI "dormi" /// m_dormFlag, dormancy status code, 0 for land cover growing and 1 for dormant
#define VAR_DPST "DPST" /// m_deprSto, depression storage
#define VAR_DRYDEP_NH4 "drydep_nh4" /// m_dryDepNH4, atmospheric dry deposition of ammonia (kg/ha)
#define VAR_DRYDEP_NO3 "drydep_no3" /// m_dryDepNO3, atmospheric dry deposition of nitrates (kg/ha)
#define VAR_EP_CH "Ep_ch"                           /// reach evaporation adjustment factor
#define VAR_EPCO "epco" /// m_epco                  /// plant water uptake compensation factor
#define VAR_ESCO "esco" /// m_esco
#define VAR_EVLAI "evlai"
#define VAR_POT_TILEMM "pot_tilemm"
#define VAR_POT_NO3DECAY "pot_no3l"
#define VAR_POT_SOLPDECAY "pot_solpl"
#define VAR_EXCP "EXCP" /// m_exsPcp, excess precipitation, which could be depressed or generated as runoff
#define VAR_EXT_COEF "EXT_COEF" /// m_lightExtCoef, light extinction coefficient
#define VAR_FERTILIZER_LOOKUP "FertilizerLookup" /// m_fertLookup, Fertilizer lookup table
#define VAR_FIELDCAP "FieldCap"                     /// Soil field capacity"
#define VAR_FLAT "flat"
#define VAR_FLOWDIR "FLOW_DIR"
#define VAR_FLOWWIDTH "FlowWidth"
#define VAR_FR_PHU_ACC "frPHUacc" /// m_phuAccum, fraction of plant heat units (PHU) accumulated
#define VAR_FR_PLANT_N "frPlantN" /// m_frPltN, fraction of plant biomass that is nitrogen, pltfr_n in SWAT
#define VAR_FR_PLANT_P "frPlantP" /// m_frPltP, fraction of plant biomass that is phosphorous, pltfr_p in SWAT
#define VAR_FR_ROOT "frRoot" /// m_frRoot, fraction of total plant biomass that is in roots, rwt in SWAT
#define VAR_FR_STRSWTR "frStrsWtr" /// m_frStrsWtr, fraction of potential plant growth achieved where the reduction is caused by water stress, strsw in SWAT
#define VAR_FRGMAX "frgmax"
#define VAR_FRGRW1 "FRGRW1" /// m_frGrow1stPt, fraction of the growing season corresponding to the 1st point on optimal leaf area development curve
#define VAR_FRGRW2 "FRGRW2" /// m_frGrow2ndPt, fraction of the growing season corresponding to the 2nd point on optimal leaf area development curve
#define VAR_GRRE "GRRE"
#define VAR_GRZ_DAYS "grz_days" /// m_nGrazDays, number of days cell has been grazed
#define VAR_GRZ_FLAG "grz_flag" /// m_grazFlag, grazing flag for cell, 0 for ungrazed, 1 for grazed
#define VAR_GSI "gsi"
#define VAR_GW_KG "kg"
#define VAR_GW_Q "GW"
#define VAR_GW0 "GW0"                               /// initial ground water storage
#define VAR_GWMAX "GWMAX"                           /// maximum ground water storage
#define VAR_GWSOLP_CONC "gwsolp_conc" /// m_gwSolPConc
#define VAR_GWSOLP "gwsolp" /// m_gwSolP
#define VAR_GWNEW "GWNEW"                           /// The volume of water from the bank storage to the adjacent unsaturated zone and groundwater storage
#define VAR_GWNO3_CONC "gwno3_conc" /// m_gwNO3Conc
#define VAR_GWNO3 "gwno3" /// m_gwNO3
#define VAR_GWWB "GWWB"        // groundwater water balance
#define VAR_GWRQ "GWRQ"
#define VAR_HCH "HCH"
#define VAR_HITARG "hi_targ" /// m_HvstIdxTrgt, Harvest index target, defined in plant operation and used in harvest/kill operation
#define VAR_HMNTL "hmntl"                           /// amount of nitrogen moving from active organic to nitrate pool in soil profile on current day in cell(kg N/ha)
#define VAR_HMPTL "hmptl"                           /// amount of phosphorus moving from the organic to labile pool in soil profile on current day in cell(kg P/ha)
#define VAR_HVSTI "hvsti" /// m_hvstIdx, harvest index: crop yield/aboveground biomass, (kg/ha)/(kg/ha)
#define VAR_HVSTI_ADJ "hvsti_adj" /// m_hvstIdxAdj, optimal harvest index for current time during growing season
#define VAR_HVSTI_TARG "hi_targ"
#define VAR_ID_OUTLET "ID_OUTLET"
#define VAR_IDC "IDC" /// m_landCoverCls, Crop/landcover category, 1 to 7, defined in "BMPText.h"
#define VAR_IGRO "IGRO" /// m_igro, plant growth code, 0 or 1
#define VAR_IGROPT "igropt"
#define VAR_IMPOUND_TRIG "impound_trig" /// m_impndTrig, release/impound action code, 0 for impound, 1 for release
#define VAR_POT_VOLMAXMM "pot_volmaxmm" /// m_potVolMax, maximum volume of water stored in the depression/impounded area, mm
#define VAR_POT_VOLLOWMM "pot_vollowmm" /// m_potVolLow, low depth ...., mm
#define VAR_INET "INET" /// m_IntcpET, evaporation from the interception storage
#define VAR_INFIL "INFIL" /// m_infil, Infiltration
#define VAR_INFILCAPSURPLUS "INFILCAPSURPLUS"
#define VAR_INIT_IS "Init_IS" /// m_initIntcpSto, Calibration parameter, the initial interception storage for all cells, mm
#define VAR_INLO "INLO" /// m_intcpLoss, Interception loss of precipitation, mm
#define VAR_CANSTOR "canstor" /// m_canSto, current interception storage, mm
#define VAR_INTERC_MAX "Interc_max" /// m_maxIntcpStoCap, Maximum Interception Storage Capacity
#define VAR_INTERC_MIN "Interc_min" /// m_minIntcpStoCap, Minimum Interception Storage Capacity
#define VAR_IRR_FLAG "irr_flag" /// m_irrFlag, irrigation flag
#define VAR_IRR_SURFQ "irr_surfq" /// m_irrWtr2SurfqAmt, amount of water from irrigation to become surface runoff
#define VAR_IRR_WTR "irr_water" /// m_irrWtrAmt, amount of water (mm) applied to cell on current day
#define VAR_ISEP_OPT "isep_opt"
#define VAR_JULIAN_DAY "JDay"                       /// Julian day (int)
#define VAR_K_BLOW "K_blow"
#define VAR_K_L "k_l"
#define VAR_K_N "k_n"
#define VAR_K_P "k_p"
#define VAR_K_PET "K_pet" /// m_petFactor, Correction factor for PET
#define VAR_K_RUN "K_run" /// m_rfExp, runoff exponent
#define VAR_K_SOIL10 "k_soil10"
#define VAR_K_SUBLI "K_subli"
#define VAR_KG "Kg"                                 /// Baseflow recession coefficient
#define VAR_KI "Ki"
#define VAR_LAG_SNOW "lag_snow"
#define VAR_LAIDAY "LAIDAY" /// m_lai
#define VAR_LAIINIT "LAI_INIT" /// m_initLai, initial LAI at the beginning of the simulation
#define VAR_LAIMAXFR "laimaxfr" /// m_laiMaxFr, TODO, what's meaning
#define VAR_LAIMX1 "LAIMX1" /// m_frMaxLai1stPt, fraction of maximum leaf area index corresponding to the 1st point on optimal leaf area development curve
#define VAR_LAIMX2 "LAIMX2" /// m_frMaxLai2ndPt, fraction of maximum leaf area index corresponding to the 2nd point on optimal leaf area development curve
#define VAR_LAIPRE "LAIPRE"
#define VAR_LAIYRMAX "laiyrmax" /// m_maxLaiYr, maximum leaf area index for the current year (m_yearIdx), lai_yrmx in SWAT
#define VAR_LAMBDA0 "lambda0"
#define VAR_LAMBDA1 "lambda1"
#define VAR_LAMBDA2 "lambda2"
#define VAR_LANDCOVER "landcover" /// m_landCover, raster data of land cover code, idplt in SWAT
#define VAR_LANDUSE "landuse" /// m_landUse, raster data of landuse
#define VAR_LANDUSE_LOOKUP "LanduseLookup" /// m_landuseLookup, lookup table of landuse
#define VAR_LAST_SOILRD "lastSoilRD" /// m_stoSoilRootD, storing last soil root depth for use in harvestkillop/killop
#define VAR_LATNO3 "latno3" /// m_latNO3, amount of nitrate transported with lateral flow, kg/ha
#define VAR_LATNO3_TOCH "latno3ToCh" /// m_latNO3ToCh, amount of nitrate transported with lateral flow to channel, kg
#define VAR_LDRAIN "ldrain" /// m_drainLyr, soil layer where drainage tile is located
#define VAR_KV_PADDY "kv_paddy"
#define VAR_KN_PADDY "kn_paddy"
#define VAR_POT_K "pot_k"
#define VAR_MANNING "Manning"
#define VAR_MAT_YRS "MAT_YRS" /// m_matYrs, the number of years for the tree species to reach full development (years), MAT_YRS in SWAT
#define VAR_MINPGW_TOCH "minpgwToCh" /// m_gwSolPToCh
#define VAR_MOIST_IN "Moist_in" /// m_initSoilWtrStoRatio, initial soil water storage fraction related to field capacity (FC-WP)
#define VAR_MSF "ManningScaleFactor"                /// flow velocity scaling factor for calibration
#define VAR_MSK_CO1 "MSK_co1" /// m_mskCoef1, Calibration coefficient used to control impact of the storage time constant for normal flow
//#define VAR_MSK_CO2 "MSK_co2" /// m_mskCoef2, Calibration coefficient used to control impact of the storage time constant fro low flow
#define VAR_MSK_X "MSK_X" /// m_mskX, Weighting factor controlling relative importance of inflow rate and outflow rate in determining water storage in reach segment
#define VAR_MUMAX "mumax"
#define VAR_NACTFR "nactfr" /// m_orgNFrActN, The fraction of organic nitrogen in the nitrogen active pool
#define VAR_NEPR "NEPR" /// m_netPcp, Net precipitation (after slope correction, of course), mm
#define VAR_NFIXCO "nfixco" /// m_NFixCoef, Nitrogen fixation coefficient, FIXCO in SWAT
#define VAR_NFIXMX "nfixmx" /// m_NFixMax, Maximum daily-n fixation (kg/ha), NFIXMX in SWAT
#define VAR_CH_NO3 "CH_NO3"
#define VAR_CH_NO3Conc "CH_NO3Conc"
#define VAR_NO2_TOCH "nitriteToCh" /// m_no2ToCh
#define VAR_CH_NO2 "CH_NO2"
#define VAR_CH_NO2Conc "CH_NO2Conc"
#define VAR_DISTSTREAM "dist2stream" /// m_distToRch, distance to the downstream reach
#define VAR_NO3GW "no3gw"
#define VAR_NO3GW_TOCH "no3gwToCh" /// m_gwNO3ToCh
#define VAR_NPERCO "nperco"
#define VAR_NUPDIS "n_updis" /// m_upTkDistN, Nitrogen uptake distribution parameter
#define VAR_OL_DET "DETOverland"
#define VAR_OL_IUH "Ol_iuh" /// m_iuhCell, IUH of each grid cell
#define VAR_OL_SED_CCOE "ccoe"
#define VAR_OL_SED_ECO1 "eco1"
#define VAR_OL_SED_ECO2 "eco2"
#define VAR_OLAI "olai" /// m_oLai, TODO, what's meaning
#define VAR_OMEGA "Omega"
#define VAR_CH_ORGN "CH_ORGN"
#define VAR_CH_ORGNConc "CH_ORGNConc"
#define VAR_CH_ORGP "CH_ORGP"
#define VAR_CH_ORGPConc "CH_ORGPConc"
#define VAR_CH_TN "CH_TN"
#define VAR_CH_TNConc "CH_TNConc"
#define VAR_CH_TP "CH_TP"
#define VAR_CH_TPConc "CH_TPConc"
#define VAR_CHSTR_NO3 "CHSTR_NO3"
#define VAR_CHSTR_NH4 "CHSTR_NH4"
#define VAR_CHSTR_TN "CHSTR_TN"
#define VAR_CHSTR_TP "CHSTR_TP"
#define VAR_OUTLETID "OUTLET_ID" /// m_outletID
#define VAR_P_MAX "P_max" /// m_maxPcpRf, maximum precipitation corresponding to potential runoff coefficient
#define VAR_P_N "p_n"
#define VAR_P_RF "p_rf" /// m_peakRateAdj, the peak rate adjustment factor
#define VAR_PERCO_N_GW "perco_n_gw" /// m_percoNGw, amount of nitrate percolating past bottom of soil profile
#define VAR_PERCO_P_GW "perco_p_gw" /// m_percoPGw, amount of soluble P percolating past bottom of soil profile
#define VAR_PERCO "Perco" /// m_soilPerco, the amount of water percolated from the soil water reservoir
#define VAR_PERDE "perde"
#define VAR_PET "PET" /// m_pet, Potential Evapotranspiration of day
#define VAR_PET_HCOEF "HCoef_pet"                   /// Coefficient related to radiation used in Hargreaves method
#define VAR_PHOSKD "phoskd"
#define VAR_PHUBASE "PHUBASE" /// m_phuBase, base zero total heat units (used when no land cover is growing)
#define VAR_PHUPLT "PHU_PLT" /// m_phuPlt, total number of heat units to bring plant to maturity
#define VAR_PHUTOT "PHU0" /// m_phuAnn, annual average total potential heat units (used when no crop is growing)
#define VAR_PI_B "Pi_b" /// m_intcpStoCapExp, Calibration parameter of interception storage, the sine-shaped curve controller exponent b, default is 1.35
#define VAR_PCP2CANFR_PR "pcp2canfr_pr" /// m_pcp2CanalFr, The fraction of precipitation fall on the embankment that drain into ditches or canals directly
#define VAR_EMBNKFR_PR "embnkfr_pr" /// m_embnkFr, The embankment area ratio of paddy rice cells
#define VAR_PL_RSDCO "rsdco_pl" /// m_pltRsdDecCoef, Plant residue decomposition coefficient
#define VAR_PLANT_N "plant_N" /// m_pltN, amount of nitrogen in plant biomass (kg/ha), plantn in SWAT
#define VAR_PLANT_P "plant_P" /// m_pltP, amount of phosphorus in plant biomass (kg/ha), plantp in SWAT
#define VAR_PLTET_TOT "plt_et_tot" /// m_totActPltET, actual ET simulated during life of plant, plt_et in SWAT
#define VAR_PLTPET_TOT "plt_pet_tot" /// m_totPltPET, potential ET simulated during life of plant, plt_pet in SWAT
#define VAR_POREIDX "Poreindex" /// m_poreIdx, pore size distribution index
#define VAR_POROST "Porosity" /// m_soilPor, soil porosity
#define VAR_POT_NO3 "pot_no3" /// m_potNo3, amount of nitrate in pothole water body
#define VAR_POT_NH4 "pot_nh4"
#define VAR_POT_ORGN "pot_orgn"
#define VAR_POT_SOLP "pot_solp" /// m_potSolP, soluble P amount in pothole water body
#define VAR_POT_ORGP "pot_orgp"
#define VAR_POT_AMINP "pot_aminp"
#define VAR_POT_SMINP "pot_sminp"
#define VAR_POT_SED "pot_sed"
#define VAR_POT_VOL "pot_vol" /// m_potVol, volume of water stored in the depression/impounded area, mm
#define VAR_POT_SA "pot_sa" /// m_potArea, surface area of impounded water body
#define VAR_POT_FLOWIN "pot_flowin"
#define VAR_POT_FLOWOUT "pot_flowout"
#define VAR_POT_SEDIN "pot_sedin"
#define VAR_POT_SEDOUT "pot_sedout"
#define VAR_PPERCO "pperco"
#define VAR_PPT "PPT" /// m_maxPltET, maximum amount of transpiration (plant et) that can occur on current day, ep_max in SWAT
#define VAR_PSP "psp" /// m_phpSorpIdxBsn, Phosphorus sorption index
#define VAR_PTTN2CH "ptTNToCh"
#define VAR_PTTP2CH "ptTPToCh"
#define VAR_PTCOD2CH "ptCODToCh"
#define VAR_PUPDIS "p_updis" /// m_upTkDistP, Phosphorus uptake distribution parameter
#define VAR_QCH "QCH"
#define VAR_OLFLOW "OL_Flow" /// m_surfRf, overland flow in each cell calculated during overland routing
#define VAR_QG "QG" /// m_qgRchOut, Groundwater discharge at each reach outlet and at each time step
#define VAR_QI "QI" /// m_qiRchOut, Interflow at each reach outlet and at each time step
#define VAR_QOVERLAND "QOverland"
#define VAR_QRECH "QRECH" /// m_qRchOut, Discharge at reach outlet of each time step
#define VAR_QS "QS" /// m_qsRchOut, Overland discharge at each reach outlet and at each time step
#define VAR_QSOIL "QSoil"
#define VAR_QSUBBASIN "QSUBBASIN"
#define VAR_QTILE "qtile"
#define VAR_QTOTAL "QTotal"
#define VAR_RadianSlope "RadianSlope"
#define VAR_RCA "rca" /// m_rainNH4Conc, concentration of ammonia in the rain (mg N/m3)  L -> 0.001 * m3
#define VAR_CH_COD "CH_COD"
#define VAR_CH_CODConc "CH_CODConc"
#define VAR_CH_DOX "ch_dox"
#define VAR_CH_DOXConc "ch_doxConc"
#define VAR_RCH_BANKERO "rch_bank_ero" /// m_rchBankEro, reach bank erosion
#define VAR_RCH_DEG "rch_deg" /// m_rchDeg, reach bed degradation
#define VAR_RCH_DEP "rch_dep" /// m_rchDep, reach deposition
#define VAR_RCH_DEPNEW "rch_depnew" /// m_dltRchDep, Channel new deposition
#define VAR_RCH_DEPSAND "rch_depsand" /// m_rchDepSand, Sand deposition in channel
#define VAR_RCH_DEPSILT "rch_depsilt" /// m_rchDepSilt, Silt deposition in channel
#define VAR_RCH_DEPCLAY "rch_depclay" /// m_rchDepClay, Clay deposition in channel
#define VAR_RCH_DEPSAG "rch_depsag" /// m_rchDepSag, Small aggregate deposition in channel
#define VAR_RCH_DEPLAG "rch_deplag" /// m_rchDepLag, Large aggregate deposition in channel
#define VAR_RCH_DEPGRAVEL "rch_depgravel" /// m_rchDepGravel, Gravel deposition in channel
#define VAR_FLDPLN_DEP "floodplain_dep" /// m_fldplnDep, flood plain deposition
#define VAR_FLDPLN_DEPNEW "floodplain_depnew" /// m_dltFldplnDep, New deposits on floodplain
#define VAR_FLDPLN_DEPSILT "floodplain_depsilt" /// m_fldplnDepSilt, Deposition silt on floodplain
#define VAR_FLDPLN_DEPCLAY "floodplain_depclay" /// m_fldplnDepClay, Deposition clay on floodplain
#define VAR_RCN "rcn" /// m_rainNO3Conc, concentration of nitrate in the rain (mg N/m3)  L -> 0.001 * m3
#define VAR_Reinfiltration "Reinfiltration"
#define VAR_RETURNFLOW "ReturnFlow"
#define VAR_REVAP "Revap"
#define VAR_RG "RG"
#define VAR_RHOQ "rhoq"
#define VAR_RMN2TL "rmn2tl"                         /// amount of nitrogen moving from the fresh organic (residue) to the nitrate(80%) and active organic(20%) pools in soil profile on current day in cell(kg N/ha)
#define VAR_RMP1TL "rmp1tl"                         /// amount of phosphorus moving from the labile mineral pool to the active mineral pool in the soil profile on the current day in cell
#define VAR_RMPTL "rmptl"                           /// amount of phosphorus moving from the fresh organic (residue) to the labile(80%) and organic(20%) pools in soil profile on current day in cell(kg P/ha)
#define VAR_RNUM1 "rnum1"
#define VAR_ROCK "rock" /// m_soilRock, Percent of rock content
#define VAR_ROCTL "roctl"                           /// amount of phosphorus moving from the active mineral pool to the stable mineral pool in the soil profile on the current day in cell
#define VAR_ROOTDEPTH "rootdepth" /// m_pltRootD, root depth of plants (mm)
#define VAR_RTE_WTRIN "rtwtr_in" /// m_rteWtrIn, water flow in reach on day before channel routing, m^3
#define VAR_RTE_WTROUT "rtwtr" /// m_rteWtrOut, water leaving reach on day after channel routing, m^3
#define VAR_RUNOFF_CO "Runoff_co" /// m_potRfCoef, potential runoff coefficient
#define VAR_RWNTL "rwntl"                           /// amount of nitrogen moving from active organic to stable organic pool in soil profile on current day in cell(kg N/ha)
#define VAR_S_FROZEN "s_frozen" /// m_soilFrozenWtrRatio, frozen soil moisture relative to saturation above which no infiltration occur
#define VAR_SAND "sand" /// m_soilSand, Percent of sand content
#define VAR_SBGS "SBGS" /// m_gwSto, Groundwater storage of the subbasin
#define VAR_SBIF "SBIF" /// m_ifluQ2Rch, interflow to streams from each subbasin
#define VAR_SBOF "SBOF" /// m_olQ2Rch, overland flow to streams from each subbasin
#define VAR_SBPET "SBPET" /// m_petSubbsn, Average potential evapotranspiration rate of the subbasin
#define VAR_SBQG "SBQG" /// m_gndQ2Rch, groundwater flow out of the subbasin
#define VAR_SCENARIO "SCENARIO"
#define VAR_SDNCO "sdnco" /// , denitrification threshold: fraction of field capacity
#define VAR_SED_DEP "SEDDEP"
#define VAR_SED_FLOW "sed_flow"
#define VAR_SED_FLUX "sed_flux"
#define VAR_SED_RECH "SEDRECH" /// m_sedRchOut, Sediment output at reach outlet
#define VAR_SED_RECHConc "SEDRECHConc" /// m_sedConcRchOut, Sediment at each reach outlet at each time step
#define VAR_SAND_RECH "SandRchOut" /// m_sandRchOut, Sand output at reach outlet
#define VAR_SILT_RECH "SiltRchOut" /// m_siltRchOut, Silt output at reach outlet
#define VAR_CLAY_RECH "ClayRchOut" /// m_clayRchOut, Clay output at reach outlet
#define VAR_SAG_RECH "SagRchOut" /// m_sagRchOut, Small aggregate output at reach outlet
#define VAR_LAG_RECH "LagRchOut" /// m_lagRchOut, Large aggregate output at reach outlet
#define VAR_GRAVEL_RECH "GravelRchOut" /// m_gravelRchOut, Gravel output at reach outlet
#define VAR_SED_TO_CH "SEDTOCH" /// m_sedtoCh, Sediment flowing to channel by hillslope routing
#define VAR_SAND_TO_CH "SandToCh" /// m_sandtoCh, Sand flowing to channel by hillslope routing
#define VAR_SILT_TO_CH "SiltToCh" /// m_silttoCh, Silt flowing to channel by hillslope routing
#define VAR_CLAY_TO_CH "ClayToCh" /// m_claytoCh, Clay flowing to channel by hillslope routing
#define VAR_SAG_TO_CH "SagToCh" /// m_sagtoCh, Small aggregate flowing to channel by hillslope routing
#define VAR_LAG_TO_CH "LagToCh" /// m_lagtoCh, Large aggregate flowing to channel by hillslope routing
#define VAR_GRAVEL_TO_CH "GravelToCh" /// m_graveltoCh, Gravel flowing to channel by hillslope routing
#define VAR_SEDSTO_CH "SedStorageCH" /// m_sedSto, Channel sediment storage (kg)
#define VAR_SANDSTO_CH "SandStorageCH" /// m_sandSto, Channel sand storage (kg)
#define VAR_SILTSTO_CH "SiltStorageCH" /// m_siltSto, Channel silt storage (kg)
#define VAR_CLAYSTO_CH "ClayStorageCH" /// m_claySto, Channel clay storage (kg)
#define VAR_SAGSTO_CH "SagStorageCH" /// m_sagSto, Channel small aggregate storage (kg)
#define VAR_LAGSTO_CH "LagStorageCH" /// m_lagSto, Channel large aggregate storage (kg)
#define VAR_GRAVELSTO_CH "GravelStorageCH" /// m_gravelSto, Channel gravel storage (kg)
#define VAR_SEDYLD "SED_OL" /// m_olWtrEroSed, distribution of soil loss caused by water erosion
#define VAR_SEDMINPA "sedminpa" /// m_surfRfSedAbsorbMinP, amount of active mineral phosphorus adsorbed to sediment in surface runoff
#define VAR_SEDMINPA_TOCH "sedminpaToCh" /// m_surfRfSedAbsorbMinPToCh
#define VAR_SEDMINPS "sedminps" /// m_surfRfSedSorbMinP, amount of stable mineral phosphorus adsorbed to sediment in surface runoff
#define VAR_SEDMINPS_TOCH "sedminpsToCh" /// m_surfRfSedSorbMinPToCh
#define VAR_SEDORGN "sedorgn" /// m_surfRfSedOrgN, amount of organic nitrogen in surface runoff
#define VAR_SEDORGN_TOCH "sedorgnToCh" /// m_surfRfSedOrgNToCh, amount of organic nitrogen in surface runoff moved to channel
#define VAR_SEDORGP "sedorgp" /// m_surfRfSedOrgP, amount of organic phosphorus in surface runoff
#define VAR_SEDORGP_TOCH "sedorgpToCh" /// m_surfRfSedOrgPToCh, amount of organic phosphorus in surface runoff moved to channel
#define VAR_SEEPAGE "SEEPAGE" /// seepage
#define VAR_SHALLST "shallst"
#define VAR_SILT "silt" /// m_soilSilt, Percent of silt content
#define VAR_SLOPE "slope" /// m_slope, Slope gradient (drop/distance, i.e., tan, or percent)
#define VAR_SLPLEN "slope_length" // m_slpLen, Slope length (unit: m)
#define VAR_SNAC "SNAC" /// m_snowAccum
#define VAR_SNME "SNME" /// m_snowMelt, snow melt amount
#define VAR_SNO3UP "sno3up"
#define VAR_SNOCOVMX "SNOCOVMX"
#define VAR_SNO50COV "SNO50COV"
#define VAR_SNRD "SNRD"
#define VAR_SNSB "SNSB" /// m_snowSublim, snow sublimation (water equivalent)
#define VAR_SNWB "SNWB"
#define VAR_SOER "SOER" /// m_eroSed, soil loss caused by water erosion
#define VAR_SOET "SOET" /// m_soilET, evaporation from the soil water storage, es_day in SWAT
#define VAR_SOIL_T10 "soil_t10" /// m_soilTempRelFactor10, factor of soil temperature relative to short grass (degree)
#define VAR_SOILDEPTH "soilDepth" /// m_soilDepth, depth to bottom of each soil layer
#define VAR_SOILLAYERS "soillayers" /// m_nSoilLyrs, soil layer numbers
#define VAR_SOILTHICK "soilthick" /// m_soilThk, soil thickness of each soil layer
#define VAR_SOL_ACTP "sol_actp" /// m_soilActvMinP, amount of phosphorus stored in the active mineral phosphorus pool(kg P/ha)
#define VAR_SOL_ALB "sol_alb" /// m_soilAlb, albedo when soil is moist
#define VAR_SOL_AORGN "sol_aorgn" /// m_soilActvOrgN, amount of nitrogen stored in the active organic (humic) nitrogen pool(kg N/ha)
#define VAR_SOL_AWC "sol_awc" /// m_soilFC, amount of water available to plants in soil layer at field capacity (FC-WP)
#define VAR_SOL_BD "density" /// m_soilBD, bulk density of the soil (mg/m3)
#define VAR_SOL_CBN "sol_cbn" /// m_soilCbn, soil carbon content
#define VAR_SOL_COV "sol_cov" /// m_rsdCovSoil, amount of residue on soil surface (kg/ha)
#define VAR_SOL_CRK "sol_crk" /// m_soilCrk, crack volume potential of soil
#define VAR_SOL_FORGN "sol_fon" /// m_soilFrshOrgN, amount of nitrogen stored in the fresh organic (residue) pool(kg N/ha)
#define VAR_SOL_FORGP "sol_fop" /// m_soilFrshOrgP, amount of phosphorus stored in the fresh organic (residue) pool(kg P/ha)
#define VAR_SOL_MC "sol_mc" /// m_soilManC, manure organic carbon in soil, kg/ha
#define VAR_SOL_MN "sol_mn" /// m_soilManN, manure organic nitrogen in soil, kg/ha
#define VAR_SOL_MP "sol_mp" /// m_soilManP, manure organic phosphorus in soil, kg/ha
#define VAR_SOL_N "sol_N" /// m_soilN, soil organic nitrogen, include nitrogen in manure

/// define rice related parameters, used by PG_ORYZA module, by Fang Shen
#define VAR_CROPSTA "cropsta"
#define VAR_TBD "tbd"
#define VAR_TOD "tod"
#define VAR_TMD "tmd"
#define VAR_DVRJ "dvrj"
#define VAR_DVRI "dvri"
#define VAR_DVRP "dvrp"
#define VAR_DVRR "dvrr"
#define VAR_MOPP "mopp"
#define VAR_PPSE "ppse"
#define VAR_SHCKD "shckd"
#define VAR_KNF "knf"
#define VAR_RGRLMX "rgrlMX"
#define VAR_RGRLMN "rgrgMN"
#define VAR_NH "nh"
#define VAR_NPLH "nplh"
#define VAR_NPLSB "nplsb"
#define VAR_LAPE "lape"
#define VAR_ZRTTR "zrttr"
#define VAR_TMPSB "tmpsb"
#define VAR_AFSH "aFsh"
#define VAR_BFSH "bFsh"
#define VAR_AFLV "aFlv"
#define VAR_BFLV "bFlv"
#define VAR_AFSO "aFso"
#define VAR_BFSO "bFso"
#define VAR_ADRLV "aDrlv"
#define VAR_BDRLV "bDrlv"
#define VAR_TCLSTR "tclstr"
#define VAR_Q10 "q10"
#define VAR_TREF "tref"
#define VAR_MAINLV "mainLV"
#define VAR_MAINST "mainST"
#define VAR_MAINSO "mainSO"
#define VAR_MAINRT "mainRT"
#define VAR_CRGLV "crgLV"
#define VAR_CRGST "crgST"
#define VAR_CRGSTR "crgSTR"
#define VAR_CRGSO "crgSO"
#define VAR_CRGRT "crgRT"
#define VAR_FSTR "fstr"
#define VAR_LRSTR "lrstr"
#define VAR_ASLA "aSLA"
#define VAR_BSLA "bSLA"
#define VAR_CSLA "cSLA"
#define VAR_DSLA "dSLA"
#define VAR_SLAMX "slaMX"
#define VAR_FCRT "fcRT"
#define VAR_FCST "fcST"
#define VAR_FCLV "fcLV"
#define VAR_FCSTR "fcSTR"
#define VAR_FCSO "fcSO"
#define VAR_WGRMX "wgrMX"
#define VAR_GZRT "gzrt"
#define VAR_ZRTMCD "zrtMCD"
#define VAR_FRPAR "frpar"
#define VAR_SPGF "spgf"
#define VAR_NMAXL "nmaxl"
#define VAR_NMINL "nminl"
#define VAR_RFNLV "rfnlv"
#define VAR_RFNST "rfnst"
#define VAR_RFNRT "rfnrt"
#define VAR_TCNTRF "tcntrf"
#define VAR_NMAXSO "nmaxso"
#define VAR_ANMINSO "anminso"
#define VAR_BNMINSO "bnminso"
#define VAR_SHCKL "shckl"
#define VAR_SBDUR "sbdur"
#define VAR_LLLS "llls"
#define VAR_ULLS "ulls"
#define VAR_LLLE "llle"
#define VAR_ULLE "ulle"
#define VAR_LLDL "lldl"
#define VAR_ULDL "uldl"
#define VAR_TS "ts"
#define VAR_WLVG "wlvg"
#define VAR_WLVD "wlvd"
#define VAR_WSTS "wsts"
#define VAR_WSTR "wstr"
#define VAR_WSO "wso"
#define VAR_WRT "wrt"
#define VAR_WRR "wrr"
#define VAR_NGR "ngr"
#define VAR_NSP "nsp"
#define VAR_TNASS "tnass"
#define VAR_WST "wst"
#define VAR_WLV "wlv"
#define VAR_WAGT "wagt"
#define VAR_ZRT "zrt"
#define VAR_DVS "dvs"
#define VAR_ANCRF "ancrf"

/// pond, figure out if pond and pothole can be share these names. By liangjun.
#define VAR_POND "pond"
#define VAR_POND_VOL "pond_vol"
#define VAR_POND_SA "pondSurfaceArea"
#define VAR_IRRDEPTH "irrDepth"
#define VAR_POND_SOLPDECAY "pond_solpl"
#define DESC_POND "pond id"
#define DESC_POND_VOL "pond volumn"
#define DESC_POND_SA "pond surface area"

/// CENTURY model for C/N cycling
#define    VAR_SOL_BMC    "sol_BMC"
#define    VAR_SOL_BMN    "sol_BMN"
#define    VAR_SOL_HSC    "sol_HSC"
#define    VAR_SOL_HSN    "sol_HSN"
#define    VAR_SOL_HPC    "sol_HPC"
#define    VAR_SOL_HPN    "sol_HPN"
#define    VAR_SOL_LM    "sol_LM"
#define    VAR_SOL_LMC    "sol_LMC"
#define    VAR_SOL_LMN    "sol_LMN"
#define    VAR_SOL_LS    "sol_LS"
#define    VAR_SOL_LSL    "sol_LSL"
#define    VAR_SOL_LSC    "sol_LSC"
#define    VAR_SOL_LSN    "sol_LSN"
#define    VAR_SOL_RNMN    "sol_RNMN"
#define    VAR_SOL_LSLC    "sol_LSLC"
#define    VAR_SOL_LSLNC    "sol_LSLNC"
#define    VAR_SOL_RSPC    "sol_RSPC"
#define    VAR_SOL_WOC    "sol_WOC"
#define    VAR_SOL_WON    "sol_WON"
#define    VAR_SOL_HP    "sol_HP"
#define    VAR_SOL_HS    "sol_HS"
#define    VAR_SOL_BM    "sol_BM"

#define VAR_SOL_LATERAL_C "sol_latc" /// m_soilIfluCbn, lateral flow Carbon loss in each soil layer
#define VAR_SOL_PERCO_C "sol_percoc" /// m_soilPercoCbn, percolation Carbon loss in each soil layer
#define VAR_LATERAL_C "latc" /// m_soilIfluCbnPrfl, lateral flow Carbon loss in soil profile
#define VAR_PERCO_C "percoc" /// m_soilPercoCbnPrfl, percolation Carbon loss in soil profile
#define VAR_SEDLOSS_C "sedc" /// m_sedLossCbn, amount of Carbon lost with sediment

#define VAR_SOL_NH4 "sol_nh4" /// m_soilNH4, amount of nitrogen stored in the ammonium pool in soil layer (kg N/ha)
#define VAR_SOL_NO3 "sol_no3" /// m_soilNO3, amount of nitrogen stored in the nitrate pool in soil layer (kg N/ha)
#define VAR_SOL_OM "om" /// m_soilOM, percent of organic matter in soil
#define VAR_SOL_SORGN "sol_orgn" /// m_soilStabOrgN, amount of nitrogen stored in the stable organic N pool(kg N/ha)
#define VAR_SOL_HORGP "sol_orgp" /// m_soilHumOrgP, amount of phosphorus stored in the humic organic P pool in soil layer(kg P/ha)
#define VAR_SOL_PERCO "sol_perco"
#define VAR_RSDCOV_COEF "rsd_covco" // m_rsdCovCoef, residue cover factor for computing fraction of cover
#define VAR_SOL_RSD "sol_rsd" /// m_soilRsd, amount of organic matter in the soil classified as residue(kg/ha)
#define VAR_SOL_RSDIN "rsdin" /// m_rsdInitSoil, initial amount of organic matter in the soil classified as residue(kg/ha)
#define VAR_SOL_SOLP "sol_solp" /// m_soilSolP, amount of phosphorus stored in solution(kg P/ha)
#define VAR_SOL_STAP "sol_stap" /// m_soilStabMinP, amount of phosphorus in the soil layer stored in the stable mineral phosphorus pool(kg P/ha)
#define VAR_SOL_SUMAWC "sol_sumAWC"  /// m_soilSumFC, amount of water held in the soil profile at field capacity (FC-WP) (sol_sumfc in SWAT, mm H2O)
#define VAR_SOL_SUMSAT "sol_sumul"  ///m_soilSumSat , mm H2O  sol_sumul in SWAT
#define VAR_SOL_TA0 "soil_ta0"
#define VAR_SOL_TA1 "soil_ta1"
#define VAR_SOL_TA2 "soil_ta2"
#define VAR_SOL_TA3 "soil_ta3"
#define VAR_SOL_TB1 "soil_tb1"
#define VAR_SOL_TB2 "soil_tb2"
#define VAR_SOL_TD1 "soil_td1"
#define VAR_SOL_TD2 "soil_td2"
#define VAR_SOL_TMP "sol_tmp"                       /// daily average temperature of soil layer(deg C)
#define VAR_SOL_UL "sol_ul" /// m_soilSat, amount of water held in the soil layer at saturation (sat - wp water), mm H2O
#define VAR_SOL_WPMM "sol_wpmm" /// m_soilWP, water content of soil at -1.5 MPa (wilting point)
#define VAR_SOL_ZMX "SOL_ZMX" /// m_soilMaxRootD, Maximum rooting depth of soil profile (mm)
#define VAR_SOL_ST "solst" /// m_soilWtrSto, amount of water stored in the soil layer on current day(mm H2O)
#define VAR_SOL_SW  "solsw" /// m_soilWtrStoPrfl, amount of water stored in soil profile on current day (mm H2O)
#define VAR_SW_CAP "sw_cap"  /// amount of water capacity in soil layers such as sol_awc sol_ul and wiltingpoint
#define VAR_SOTE "SOTE" /// m_soilTemp, Soil Temperature
#define VAR_SOWB "SOWB" /// m_soilWtrBal, soil water balance
#define VAR_SOXY "soxy"
#define VAR_SOXYConc "soxyConc"
#define VAR_SPCON "spcon" /// m_sedTransEqCoef, Coefficient in sediment transport equation
#define VAR_SPEXP "spexp" /// m_sedTransEqExp, Exponent in sediment transport equation
#define VAR_SR_MAX "srMax"                          /// Max solar radiation
#define VAR_SRA "sra"
#define VAR_SSRU "SSRU" /// m_subSurfRf, The subsurface runoff
#define VAR_SSRUVOL "SSRUVOL" /// m_subSurfRfVol
#define VAR_STCAPSURPLUS "STCAPSURPLUS"
#define VAR_STREAM_LINK "STREAM_LINK" /// m_rchID, Stream link (id of reaches)
#define VAR_SUB_SEDTOCH "" /// TODO, for storm mode
#define VAR_SUBBSN "subbasin" /// m_subbsnID, raster data of subbasin ID
#define VAR_SUBBSNID_NUM "SUBBASINID_NUM" /// m_nSubbsns, number of subbasins
#define VAR_SUR_NO3 "sur_no3" /// m_surfRfNO3, amount of nitrate transported with surface runoff, kg/ha
#define VAR_SUR_NO3_TOCH "sur_no3_ToCh" /// m_surfRfNO3ToCh, amount of nitrate transported with surface runoff to channel, kg
#define VAR_SUR_NH4 "sur_nh4" /// m_surfRfNH4, amount of ammonian transported with surface runoff, kg/ha
#define VAR_SUR_NH4_TOCH "SUR_NH4_TOCH" /// m_surfRfNH4ToCh, amount of ammonian transported with surface runoff to channel, kg
#define VAR_SUR_SOLP "sur_solp" /// m_surfRfSolP, amount of soluble phosphorus in surface runoff
#define VAR_SUR_SOLP_TOCH "sur_solp_ToCh" /// m_surfRfSolPToCh, amount of soluble phosphorus in surface runoff to channel, kg
#define VAR_SUR_COD_TOCH "sur_cod_ToCH" /// m_surfRfCodToCh, amount of COD to reach in surface runoff (kg)
#define VAR_SURU "SURU" /// m_surfRf, surface runoff
#define VAR_SWE "SWE"
#define VAR_SWE0 "swe0"
#define VAR_T_BASE "T_BASE" /// m_pgTempBase, minimum temperature for plant growth (degree C)
#define VAR_T_OPT "T_OPT" /// m_pgOptTemp, optional temperature for plant growth
#define VAR_T_RG "T_RG"   /// groundwater runoff
#define VAR_T_SNOW "T_snow" /// m_snowTemp, Snowfall temperature
#define VAR_T_SOIL "t_soil" /// m_soilFrozenTemp, threshold soil freezing temperature
#define VAR_T0 "T0"
#define VAR_TFACT "tfact"
#define VAR_TILLAGE_LOOKUP "TillageLookup" /// m_tillageLookup, Tillage lookup table
#define VAR_TILLAGE_DAYS "tillage_days" /// m_tillDays, days from tillage
#define VAR_TILLAGE_DEPTH "tillage_depth" /// m_tillDepth, tillage depth
#define VAR_TILLAGE_FACTOR "tillage_factor" /// m_tillFactor, influence factor of tillage operation
#define VAR_TILLAGE_SWITCH "tillage_switch" /// m_tillSwitch, switch of whether to tillage
#define VAR_TMAX "TMAX" /// m_maxTemp, maximum air temperature
#define VAR_TMEAN "TMEAN" /// m_meanTemp, mean air temperature
#define VAR_TMEAN_ANN "TMEAN0" /// m_annMeanTemp, annual mean air temperature
#define VAR_TMEAN1 "TMEAN1" /// m_meanTempPre1, Mean air temperature of the (d-1)th day
#define VAR_TMEAN2 "TMEAN2" /// m_meanTempPre2, Mean air temperature of the (d-2)th day
#define VAR_TMIN "TMIN" /// m_minTemp, minimum air temperature
#define VAR_TREEYRS "CURYR_INIT" /// m_curYrMat, initial age of tress (yrs), or current year in rotation to maturity
#define VAR_TSD_DT "DATATYPE"                      /// Time series data type
#define VAR_ICFAC "icfac" /// m_iCfac, C-factor calculation using Cmin (0, default) or new method from RUSLE (1)
#define VAR_USLE_C "USLE_C" /// m_usleC, USLE C factor (land cover)
#define VAR_USLE_K "USLE_K" /// m_usleK
#define VAR_USLE_L "USLE_L" /// m_usleL, USLE slope length factor
#define VAR_USLE_S "USLE_S" /// m_usleS, USLE slope factor
#define VAR_USLE_P "USLE_P" /// m_usleP
#define VAR_VCD "vcd" /// m_vcd, whether change channel dimensions, 0 - do not change (false), 1 - compute channel degredation (true)
#define VAR_VDIV "Vdiv"                             /// diversion loss of the river reach
#define VAR_VP_ACT "avp"                            /// actual vapor pressure
#define VAR_VP_SAT "svp"                            /// Saturated vapor pressure
#define VAR_VPD "VPD" /// m_vpd, vapor pressure deficit
#define VAR_VPDFR "vpdfr"
#define VAR_VSEEP0 "Vseep0"                         ///  the initial volume of transmission loss to the deep aquifer over the time interval
#define VAR_WATTEMP "wattemp" /// m_chTemp
#define VAR_WAVP "WAVP" /// m_wavp, Rate of decline in radiation use efficiency per unit increase in vapor pressure deficit, wavp in SWAT
#define VAR_WDNTL "wdntl" /// amount of nitrogen lost from nitrate pool by denitrification in soil profile on current day in cell(kg N/ha)
#define VAR_WILTPOINT "WiltingPoint"
#define VAR_WS "WS"
#define VAR_WSHD_DNIT "wshd_dnit"                   ///  nitrogen lost from nitrate pool due to denitrification in watershed(kg N/ha)
#define VAR_WSHD_HMN "wshd_hmn"                     ///  nitrogen moving from active organic to nitrate pool in watershed(kg N/ha)
#define VAR_WSHD_HMP "wshd_hmp"                     ///  phosphorus moving from organic to labile pool in watershed(kg P/ha)
#define VAR_WSHD_NITN "wshd_nitn"                   ///  nitrogen moving from the NH4 to the NO3 pool by nitrification in the watershed
#define VAR_WSHD_PAL "wshd_pal"                     ///  phosphorus moving from labile mineral to active mineral pool in watershed
#define VAR_WSHD_PAS "wshd_pas"                     ///  phosphorus moving from active mineral to stable mineral pool in watershed
#define VAR_WSHD_PLCH "wshd_plch" /// m_wshdLchP, phosphorus leached into second soil layer
#define VAR_WSHD_RMN "wshd_rmn"                     ///  nitrogen moving from fresh organic (residue) to nitrate and active organic pools in watershed(kg N/ha)
#define VAR_WSHD_RMP "wshd_rmp"                     ///  phosphorus moving from fresh organic (residue) to labile and organic pools in watershed(kg P/ha)
#define VAR_WSHD_RNO3 "wshd_rno3"                   ///  NO3 added to soil by rainfall in watershed (kg/ha)
#define VAR_WSHD_RWN "wshd_rwn"                     /// nitrogen moving from active organic to stable organic pool in watershed(kg N/ha)
#define VAR_WSHD_VOLN "wshd_voln"                   /// nitrogen lost by ammonia volatilization in watershed
#define VAR_WSYF "wsyf" /// m_wtrStrsHvst, Value of harvest index between 0 and HVSTI, represents the lowest value expected due to water stress
#define VAR_AL_OUTLET "algae_outlet"
#define VAR_ON_OUTLET "organicn_outlet"
#define VAR_AN_OUTLET "ammonian_outlet"
#define VAR_NIN_OUTLET "nitriten_outlet"
#define VAR_NAN_OUTLET "nitraten_outlet"
#define VAR_OP_OUTLET "organicp_outlet"
#define VAR_DP_OUTLET "disolvp_outlet"
#define VAR_COD_OUTLET "cod_outlet"
#define VAR_CHL_OUTLET "chlora_outlet"

#define VAR_A_DAYS "a_days" /// m_phpApldDays, days since P Application
#define VAR_B_DAYS "b_days" /// m_phpDefDays, days since P deficit */

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
/// Define description of units common used in SEIMS            //////////
/// By LiangJun Zhu, HuiRan Gao //////////////////////////
///               Apr. 25, 2016  //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
/* CONST_CHARS DESC_A_BNK = "bank flow recession constant"
//#define DESC_ACC "the number of flow accumulation cells of each cell"
#define DESC_ACC_INFIL "accumulative infiltration"
#define DESC_ADDRNH4 "ammonium added by rainfall"
#define DESC_ADDRNO3 "nitrate added by rainfall"
#define DESC_AET_PLT "actual amount of plant transpiration, ep_day in SWAT"
#define DESC_AFERT_AMAXN "Maximum amount of mineral N allowed to be applied in any one year"
#define DESC_AFERT_FRTEFF "fertilizer application efficiency calculated as the amount of N applied divided by the amount of N removed at harvest"
#define DESC_AFERT_FRTSURF "Fraction of fertilizer applied to top 10mm of soil"
#define DESC_AFERT_ID "fertilizer ID from fertilizer database"
#define DESC_AFERT_MAXN "Maximum amount of mineral N allowed in any one application"
#define DESC_AFERT_NSTRS "Nitrogen stress factor of cover/plant that triggers fertilization"
#define DESC_AFERT_NSTRSID "Code for approach used to determine amount of nitrogen to Cell"
#define DESC_AFERT_NYLDT "modifier for auto fertilization target nitrogen content"
#define DESC_AI0 "ratio of chlorophyll-a to algal biomass"
#define DESC_AI1 "fraction of algal biomass that is nitrogen"
#define DESC_AI2 "fraction of algal biomass that is phosphorus"
#define DESC_AI3 "the rate of oxygen production per unit of algal photosynthesis"
#define DESC_AI4 " the rate of oxygen uptake per unit of algae respiration"
#define DESC_AI5 "the rate of oxygen uptake per unit of NH3 nitrogen oxidation"
#define DESC_AI6 "the rate of oxygen uptake per unit of NO2 nitrogen oxidation"
#define DESC_AIRR_EFF "auto irrigation efficiency, 0 ~ 100"
#define DESC_AIRR_LOCATION "location of irrigation source"
#define DESC_AIRR_SOURCE "irrigation source"
#define DESC_AIRRSURF_RATIO "surface runoff ratio (0-1)"
#define DESC_AIRRWTR_DEPTH "amount of irrigation water applied each time"
#define DESC_ALAIMIN "minimum LAI during winter dormant period"
#define DESC_ALBDAY "Albedo of the current day"
#define DESC_CH_ALGAE "algal biomass in reach"
#define DESC_AMMO_CH "amount of ammonium transported with lateral flow"
#define DESC_CH_NH4 "ammonia nitrogen in reach"
#define DESC_CH_ONCO "Channel organic nitrogen concentration in basin"
#define DESC_CH_OPCO "Channel organic phosphorus concentration in basin"
#define DESC_ANION_EXCL "fraction of porosity from which anions are excluded"
#define DESC_AWTR_STRS_ID "Water stress identifier, 1 plant water demand, 2 soil water content"
#define DESC_AWTR_STRS_TRIG "Water stress threshold that triggers irrigation"
#define DESC_B_BNK "bank storage loss coefficient"
#define DESC_BACT_SWF "fraction of manure containing active colony forming units (cfu)"
#define DESC_BASE_EX "baseflow recession exponent"
#define DESC_BIO_E "the potential or unstressed growth rate (including roots) per unit of intercepted photsynthetically active radiation"
#define DESC_BIOEHI "Biomass-energy ratio corresponding to the 2nd point on the radiation use efficiency curve"
#define DESC_BIOINIT "initial dry weight biomass (kg/ha)"
#define DESC_BIOLEAF "fraction of biomass that drops during dormancy (for tree only)"
#define DESC_BIOMASS "land cover/crop biomass (dry weight)"
#define DESC_BIOTARG "Biomass (dry weight) target (kg/ha), defined in plant management operation"
#define DESC_BKST "bank storage"
#define DESC_BLAI "maximum leaf area index"
#define DESC_BMX_TREES "Maximum biomass for a forest (metric tons/ha)"
#define DESC_BN1 "the normal fraction of nitrogen in the plant biomass at emergence"
#define DESC_BN2 "the normal fraction of nitrogen in the plant biomass at 50% maturity"
#define DESC_BN3 "the normal fraction of nitrogen in the plant biomass at maturity"
#define DESC_BNK0 "initial bank storage per meter of reach length"
#define DESC_BP1 "the normal fraction of phosphorus in the plant biomass at emergence"
#define DESC_BP2 "the normal fraction of phosphorus in the plant biomass at 50% maturity"
#define DESC_BP3 "the normal fraction of phosphorus in the plant biomass at maturity"
#define DESC_C_RAIN "Rainfall impact factor"
#define DESC_C_SNOW "temperature impact factor"
#define DESC_C_SNOW12 "Melt factor on Decemeber 21, Minimum melt rate for snow during year"
#define DESC_C_SNOW6 "Melt factor on June 21, Maximum melt rate for snow during year"
#define DESC_C_WABA "Channel water balance in a text format for each reach and at each time step"
#define DESC_CDN "rate coefficient for denitrification"
#define DESC_CELL_LAT "latitude of each valid cells"
#define DESC_CellSize "numble of valid cells, i.e., excluding NODATA"
#define DESC_LayeringMethod "Routing layering method"
#define DESC_CellWidth "width of the cell"
#define DESC_CH_DEP "distribution of channel sediment deposition"
#define DESC_CH_DET "distribution of channel flow detachment"
#define DESC_CH_DETCO "Calibration coefficient of channel flow detachment"
#define DESC_CH_FLOWCAP "distribution of channel flow capacity"
//#define DESC_CH_MANNING_FACTOR "Manning scaling factor for channel routing"
#define DESC_CH_SEDRATE "distribution of channel sediment rate"
#define DESC_CH_TCCO "Calibration coefficient of transport capacity calculation"
#define DESC_CH_TN " total N amount in reach"
#define DESC_CH_TNConc "total N concentration in reach"
#define DESC_CH_TP " total P amount in reach"
#define DESC_CH_TPConc "total P concentration in reach"
#define DESC_CHSTR_NO3 "NO3-N stored in channel"
#define DESC_CHSTR_NH4 "NH4-N stored in channel"
#define DESC_CHSTR_TN "total nitrogen stored in channel"
#define DESC_CHSTR_TP "total phosphrous stored in channel"
#define DESC_CH_V "flow velocity"
#define DESC_CH_VOL "water volume"
#define DESC_CH_CHLORA "chlorophyll-a in reach"
#define DESC_CHL_A "chlorophyll-a concentration in water yield"
#define DESC_CHS0 "initial channel storage per meter of reach length"
#define DESC_CHS0_PERC "initial percentage of channel volume"
#define DESC_CHSB "Channel sediment balance for each reach and at each time step"
#define DESC_CHST "channel storage"
#define DESC_PRECHST "channel storage at previous timestep"
#define DESC_CHT "canopy height for the day (m)"
#define DESC_CHTMX "maximum canopy height (m)"
#define DESC_CHWIDTH "Channel width"
#define DESC_CHWTWIDTH "Channel water width"
#define DESC_CHBTMWIDTH "the bottom width of channel"
#define DESC_CHCROSSAREA "channel cross-sectional area"
#define DESC_CHWTDEPTH "channel water depth"
#define DESC_PRECHWTDEPTH "channel water depth of previous timestep"
#define DESC_CLAY "Percent of clay content"
#define DESC_CMN "Rate coefficient for mineralization of the humus active organic nutrients"
#define DESC_CN2 "Curve Number value under moisture condition II"
#define DESC_CO2 "CO2 Concentration"
#define DESC_CO2HI "elevated CO2 atmospheric concentration corresponding the 2nd point on the radiation use efficiency curve"
#define DESC_SUR_COD "carbonaceous oxygen demand of surface runoff"
#define DESC_COD_CH "carbonaceous oxygen demand loading to reach"
#define DESC_COD_N "Conversion factor"
#define DESC_COD_K "Reaction coefficient"
#define DESC_CONDRATE "Rate of decline in stomatal conductance per unit increase in vapor pressure deficit"
#define DESC_CONDUCT "saturation hydraulic conductivity"
#define DESC_CONV_WT "factor which converts kg/kg soil to kg/ha"
#define DESC_CROP_LOOKUP "Crop lookup table"
#define DESC_CSWAT "carbon modeling method"
#define DESC_PCP "Precipitation of each time step on current cell"
#define DESC_DAYLEN "day length"
#define DESC_DAYLEN_MIN  "minimum day length"
#define DESC_DEEPST "depth of water in deep aquifer"
#define DESC_DEET "evaporation from depression storage"
#define DESC_DEM "Digital Elevation Model"
#define DESC_DEPREIN "initial depression storage coefficient"
#define DESC_DEPRESSION "Depression storage capacity"
#define DESC_DETSPLASH "distribution of splash detachment"
#define DESC_DETACH_SAND "sand fraction of detached sediment"
#define DESC_DETACH_SILT "silt fraction of detached sediment"
#define DESC_DETACH_CLAY "clay fraction of detached sediment"
#define DESC_DETACH_SAG "small aggeregate fraction of detached sediment"
#define DESC_DETACH_LAG "large aggregate fraction of detached sediment"
#define DESC_SANDYLD "sand yield amount"
#define DESC_SILTYLD "silt yield amount"
#define DESC_CLAYYLD "clay yield amount"
#define DESC_SAGYLD "small aggeregate yield amout"
#define DESC_LAGYLD "large aggregate yield amout"
#define DESC_DF_COEF "Deep percolation coefficient"
#define DESC_CH_SOLP "dissolved phosphorus in reach"
#define DESC_DLAI "the fraction of growing season(PHU) when senescence becomes dominant"
#define DESC_DISTSTREAM "distance to the stream"
#define DESC_DORMHR "time threshold used to define dormant period for plant"
#define DESC_DORMI "dormancy status code, 0 for land cover growing and 1 for dormant"
#define DESC_DPST "depression storage"
#define DESC_DRYDEP_NH4 "atmospheric dry deposition of ammonia"
#define DESC_DRYDEP_NO3 "atmospheric dry deposition of nitrates"
#define DESC_DT_HS "Time step of the simulation"
#define DESC_EP_CH "reach evaporation adjustment factor"
#define DESC_EPCO "plant water uptake compensation factor"
#define DESC_ESCO "soil evaporation compensation factor"
#define DESC_EXCP "excess precipitation"
#define DESC_EVLAI "leaf area index at which no evaporation occurs from the water surface"
#define DESC_POT_TILEMM "Average daily outflow to main channel from tile flow if drainage tiles are installed in the pothole"
#define DESC_POT_NO3DECAY "Nitrate decay rate in impounded water body"
#define DESC_POT_SOLPDECAY "Soluble phosphorus decay rate in impounded water body"
#define DESC_POT_VOLMAXMM "maximum volume of water stored in the depression/impounded area"
#define DESC_POT_VOLLOWMM "lowest volume of water stored in the depression/impounded area"
#define DESC_EXT_COEF "light extinction coefficient"
#define DESC_FERTILIZER_LOOKUP "Fertilizer lookup table"
#define DESC_FIELDCAP "Soil field capacity"
#define DESC_FLAT "lateral flow in soil layer"
#define DESC_FLOWDIR "Flow direction by the rule of TauDEM"
#define DESC_FLOWIN_INDEX_D8 "The index of flow in (D8) cell in the compressed array, and the first element in each sub-array is the number of flow in cells in this sub-array"
#define DESC_FLOWOUT_INDEX_D8 "The index of flow out (D8) cell of each cells in the compressed array"
#define DESC_FLOWOUT_INDEX_DINF "The index of flow in cell in the compressed array"
#define DESC_FLOWWIDTH "Flow width of overland plane"
#define DESC_FR_PHU_ACC "fraction of plant heat units (PHU) accumulated"
#define DESC_FR_PLANT_N "fraction of plant biomass that is nitrogen, pltfr_n in SWAT"
#define DESC_FR_PLANT_P "fraction of plant biomass that is phosphorous, pltfr_p in SWAT"
#define DESC_FR_ROOT "fraction of total plant biomass that is in roots, rwt in SWAT"
#define DESC_FR_STRSWTR "fraction of potential plant growth achieved where the reduction is caused by water stress, strsw in SWAT"
#define DESC_FRGMAX "fraction of maximum stomatal conductance corresponding to the second point on the stomatal conductance curve"
#define DESC_FRGRW1 "fraction of total potential heat units corresponding to the 1st point on the optimal leaf area development curve"
#define DESC_FRGRW2 "fraction of total potential heat units corresponding to the 2nd point on the optimal leaf area development curve"
#define DESC_GRRE "water amount of percolation" /// TODO figure out what's meaning?
#define DESC_GRZ_DAYS "number of days cell has been grazed"
#define DESC_GRZ_FLAG "grazing flag for cell"
#define DESC_GSI "maximum stomatal conductance at high solar radiation and low vpd"
#define DESC_GW_KG "baseflow recession coefficient"
#define DESC_GW_Q "groundwater contribution to stream flow"
#define DESC_GW0 "initial ground water storage"
#define DESC_GWMAX "maximum ground water storage"
#define DESC_GWSOLP "soluble P amount in groundwater"
#define DESC_GWSOLP_CONC "soluble P concentration in groundwater"
#define DESC_GWNEW "The volume of water from the bank storage to the adjacent unsaturated zone and groundwater storage"
#define DESC_GWNO3_CONC "nitrate N concentration in groundwater"
#define DESC_GWNO3 "nitrate N amount in groundwater"
#define DESC_GWWB "groundwater water balance"
#define DESC_GWRQ "groundwater recharge to channel or perennial base flow"
#define DESC_HCH "Water depth in the downslope boundary of cells"
#define DESC_HITARG "Harvest index target "
#define DESC_HMNTL "amount of nitrogen moving from active organic to nitrate pool in soil profile on current day in cell"
#define DESC_HMPTL "amount of phosphorus moving from the organic to labile pool in soil profile on current day in cell"
#define DESC_HVSTI "harvest index: crop yield/aboveground biomass"
#define DESC_HVSTI_ADJ "optimal harvest index for current time during growing season"
#define DESC_HVSTI_TARG "harvest index target"
#define DESC_ID_OUTLET  "index of outlet in the compressed array"
#define DESC_IDC "crop/landcover category"
#define DESC_IGRO "Land cover status code"
#define DESC_IGROPT "option for calculating the local specific growth rate of algae"
#define DESC_IMPOUND_TRIG "release/impound action code"
#define DESC_INET "evaporation from the interception storage"
#define DESC_INFIL "Infiltration"
#define DESC_INFILCAPSURPLUS "surplus of infiltration capacity"
#define DESC_INIT_IS "Initial interception storage"
#define DESC_INLO "Interception loss"
#define DESC_CANSTOR "amount of water held in canopy storage"
#define DESC_INTERC_MAX "Maximum Interception Storage Capacity"
#define DESC_INTERC_MIN "Minimum Interception Storage Capacity"
#define DESC_IRR_FLAG "irrigation flag, 1 or 0"
#define DESC_IRR_SURFQ "amount of water from irrigation to become surface runoff"
#define DESC_IRR_WTR "amount of water applied to cell on current day"
#define DESC_ISEP_OPT "initial septic operational condition"
#define DESC_JULIAN_DAY "Julian day (int)"
#define DESC_K_BLOW "fraction coefficient of precipitation as snow"
#define DESC_K_L "half saturation coefficient for light"
#define DESC_K_N "half-saturation constant for nitrogen"
#define DESC_K_P "half saturation constant for phosphorus"
#define DESC_K_RUN "Runoff exponent"
#define DESC_K_SOIL10 "Ratio between soil temperature at 10 cm and the mean"
#define DESC_K_SUBLI "Fraction of PET for sublimation"
#define DESC_KG "Baseflow recession coefficient"
#define DESC_KI "Interflow scale factor"
#define DESC_LAG_SNOW "Snow temperature lag factor"
#define DESC_LAIDAY "Leaf area index of current day"
#define DESC_LAIINIT "initial leaf area index of transplants"
#define DESC_LAIMAXFR "DO NOT KNOW MEANING"
#define DESC_LAIMX1 "fraction of max LAI corresponding to the 1st point on the optimal leaf area development curve"
#define DESC_LAIMX2 "fraction of max LAI corresponding to the 2nd point on the optimal leaf area development curve"
#define DESC_LAIPRE "leaf area index for the previous day"
#define DESC_LAIYRMAX "maximum LAI for the year"
#define DESC_LAMBDA0 "non-algal portion of the light extinction coefficient"
#define DESC_LAMBDA1 "linear algal self-shading coefficient"
#define DESC_LAMBDA2 "nonlinear algal self-shading coefficient"
#define DESC_LANDCOVER "landcover code"
#define DESC_LANDUSE "landuse code"
#define DESC_LANDUSE_LOOKUP "lookup table of landuse"
#define DESC_LAST_SOILRD "storing last soil root depth for use in harvestkillop/killop"
#define DESC_LATNO3 "amount of nitrate transported with lateral flow"
#define DESC_LATNO3_CH "amount of nitrate transported with lateral flow to channel"
#define DESC_LDRAIN "soil layer where drainage tile is located"
#define DESC_KV_PADDY "volatilization rate constant in impounded water body"
#define DESC_POT_K "hydraulic conductivity of soil surface of pothole"
#define DESC_KN_PADDY "nitrification rate constant in impounded water body"
#define DESC_MANNING "Manning's roughness"
#define DESC_MASK "Array containing the row and column numbers for valid cells"
#define DESC_MAT_YRS "the number of years for the tree species to reach full development"
#define DESC_MAXCOND "Maximum stomatal conductance"
#define DESC_METEOLAT "Latitude of MeteoClimate station"
#define DESC_MINPGW_CH "soluble P in groundwater to channel"
#define DESC_MOIST_IN "Initial soil moisture"
#define DESC_MSF "flow velocity scaling factor for calibration"
#define DESC_MSK_CO1 "Calibration coefficient used to control impact of the storage time constant for normal flow"
//#define DESC_MSK_CO2 "Calibration coefficient used to control impact of the storage time constant fro low flow"
#define DESC_MSK_X "Weighting factor controlling relative importance of inflow rate and outflow rate in determining water storage in reach segment"
#define DESC_MUMAX "maximum specific algal growth rate at 20 deg C"
#define DESC_NACTFR "The fraction of organic nitrogen in the nitrogen active pool."
#define DESC_NEPR "Net Precipitation"
#define DESC_NFIXCO "Nitrogen fixation coefficient"
#define DESC_NFIXMX "Maximum daily-N fixation (kg/ha)"
#define DESC_CH_NO3 "nitrate in reach"
#define DESC_NITRITE_CH "amount of nitrite transported with lateral flow"
#define DESC_CH_NO2 "nitrite in reach"
#define DESC_NO3GW "nitrate loading to reach in groundwater"
#define DESC_NO3GW_CH "nitrate in groundwater to channel"
#define DESC_NONE "NO DESCRIPTION"
#define DESC_NPERCO "nitrate percolation coefficient"
#define DESC_NUPDIS "Nitrogen uptake distribution parameter"
#define DESC_OL_DET "distribution of overland flow detachment"
#define DESC_OL_IUH "IUH of each grid cell"
#define DESC_OL_SED_CCOE "calibration coefficient of overland flow detachment erosion"
#define DESC_OL_SED_ECO1 "calibration coefficient 1 of transport capacity calculation"
#define DESC_OL_SED_ECO2 "calibration coefficient 2 of transport capacity calculation"
#define DESC_OLAI "DO NOT KNOW MEANING"
#define DESC_OMEGA "calibration coefficient of splash erosion"
#define DESC_CH_ORGN "organic nitrogen in reach"
#define DESC_CH_ORGP "organic phosphorus in reach"
#define DESC_OUTLETID "subbasin ID which outlet located"
#define DESC_P_MAX "Maximum precipitation corresponding to potential runoff coefficient"
#define DESC_P_N "algal preference factor for ammonia"
#define DESC_P_RF "Peak rate adjustment factor"
#define DESC_PERCO_N "amount of nitrate percolating past bottom of soil profile"
#define DESC_PERCO_P "amount of soluble P percolating past bottom of soil profile"
#define DESC_PERCO "the amount of water percolated from the soil water reservoir, i.e., groundwater recharge"
#define DESC_PET "Potential Evapotranspiration of day"
#define DESC_PET_HCOEF "Coefficient related to radiation used in Hargreaves method"
#define DESC_PET_K "Correction factor for PET"
#define DESC_PHOSKD "Phosphorus soil partitioning coefficient"
#define DESC_PHUBASE "base zero total heat units (used when no land cover is growing)"
#define DESC_PHUPLT "total number of heat unites (hours) needed to bring plant to maturity"
#define DESC_PHUPLT "total number of heat unites (hours) needed to bring plant to maturity"
#define DESC_PHUTOT "annual average total potential heat units (used when no crop is growing)"
#define DESC_PI_B "Interception Storage Capacity Exponent"
#define DESC_PL_RSDCO "Plant residue decomposition coefficient"
#define DESC_PLANT_N "amount of nitrogen in plant biomass (kg/ha), plantn in SWAT"
#define DESC_PLANT_P "amount of phosphorus in plant biomass (kg/ha), plantp in SWAT"
#define DESC_PLTET_TOT "actual ET simulated during life of plant"
#define DESC_PLTPET_TOT "potential ET simulated during life of plant"
#define DESC_POREIDX "pore size distribution index"
#define DESC_POROST "soil porosity"
#define DESC_POT_NO3 "amount of nitrate in pothole water body"
#define DESC_POT_NH4 "amount of ammonian in pothole water body"
#define DESC_POT_ORGN "amount of organic N in pothole water body"
#define DESC_POT_SOLP "soluble P amount in pothole water body"
#define DESC_POT_ORGP "amount of organic P in pothole water body"
#define DESC_POT_AMINP "amount of active mineral pool P in pothole water body"
#define DESC_POT_SMINP "amount of stable mineral pool P in pothole water body"
#define DESC_POT_SED "amount of sediment in pothole water body"
#define DESC_POT_VOL "current volume of water stored in the depression/impounded area"
#define DESC_POT_SA "surface area of impounded area"
#define DESC_POT_FLOWIN "water entering pothole on day"
#define DESC_POT_FLOWOUT "discharge from pothole expressed as depth"
#define DESC_POT_SEDIN "sediment entering pothole on day"
#define DESC_POT_SEDOUT "sediment leaving pothole on day"
#define DESC_PPERCO "phosphorus percolation coefficient"
#define DESC_PPT "maximum amount of transpiration (plant et)"
#define DESC_PSP "Phosphorus availability index"
#define DESC_PTTN2CH "total nitrogen loaded from point sources"
#define DESC_PTTP2CH "total phosphrus loaded from point sources"
#define DESC_PTCOD2CH "total COD loaded from point sources"
#define DESC_PUPDIS "Phosphorus uptake distribution parameter"
#define DESC_QCH "Flux in the downslope boundary of cells"
#define DESC_OLFLOW "overland flow in each cell calculated during overland routing"
#define DESC_QG "Groundwater discharge at each reach outlet"
#define DESC_QI "Interflow at each reach outlet"
#define DESC_QOVERLAND "Water discharge in the downslope boundary of cells"
#define DESC_QRECH "Discharge at each reach outlet of each time step"
#define DESC_QS "Overland discharge at each reach outlet"
#define DESC_QSOIL "discharge added to channel flow from interflow"
#define DESC_QSUBBASIN "discharge at each subbasin outlet"
#define DESC_QTILE "drainage tile flow in soil profile"
#define DESC_QTOTAL "discharge at the watershed outlet"
#define DESC_RadianSlope  "radian slope"
#define DESC_RCA "concentration of ammonia in the rain"
#define DESC_CH_COD "carbonaceous oxygen demand in reach"
#define DESC_CH_DOX "dissolved oxygen in reach"
#define DESC_RCH_BANKERO "reach bank erosion"
#define DESC_RCH_DEG "reach degradation"
#define DESC_RCH_DEP "reach deposition"
#define DESC_RCH_DEPNEW "Channel new deposition"
#define DESC_RCH_DEPSAND "Sand deposition in channel"
#define DESC_RCH_DEPSILT "Silt deposition in channel"
#define DESC_RCH_DEPCLAY "Clay deposition in channel"
#define DESC_RCH_DEPSAG "Small aggregate deposition in channel"
#define DESC_RCH_DEPLAG "Large aggregate deposition in channel"
#define DESC_RCH_DEPGRAVEL "Gravel deposition in channel"
#define DESC_FLDPLN_DEP "Floodplain Deposition"
#define DESC_FLDPLN_DEPNEW "New deposits on floodplain"
#define DESC_FLDPLN_DEPSILT "Deposition silt on floodplain"
#define DESC_FLDPLN_DEPCLAY "Deposition clay on floodplain"
#define DESC_RCN "concentration of nitrate in the rain"
#define DESC_Reinfiltration "TODO: meaning?"
#define DESC_RETURNFLOW "water depth of return flow"
#define DESC_REVAP "revaporization from groundwater to the last soil layer"
#define DESC_RG "groundwater runoff"
#define DESC_RHOQ "algal respiration rate at 20 deg C"
#define DESC_RM "Relative humidity"
#define DESC_RMN2TL "amount of nitrogen moving from the fresh organic (residue) to the nitrate(80%) and active organic(20%) pools in soil profile on current day in cell"
#define DESC_RMP1TL "amount of phosphorus moving from the labile mineral pool to the active mineral pool in the soil profile on the current day in cell"
#define DESC_RMPTL "amount of phosphorus moving from the fresh organic (residue) to the labile(80%) and organic(20%) pools in soil profile on current day in cell"
#define DESC_RNUM1 "fraction of overland flow"
#define DESC_ROCK "Percent of rock content"
#define DESC_ROCTL "amount of phosphorus moving from the active mineral pool to the stable mineral pool in the soil profile on the current day in cell"
#define DESC_ROOTDEPTH "root depth of plants (mm)"
#define DESC_ROUTING_LAYERS "Routing layers according to the flow direction, there are no flow relationships within each layer, and the first element in each layer is the number of cells in the layer"
#define DESC_RTE_WTRIN "water flow in reach on day before channel routing"
#define DESC_RTE_WTROUT "water leaving reach on day after channel routing"
#define DESC_RUNOFF_CO "Potential runoff coefficient"
#define DESC_RWNTL "amount of nitrogen moving from active organic to stable organic pool in soil profile on current day in cell"
#define DESC_S_FROZEN "Frozen moisture relative to porosity with no infiltration"
#define DESC_SAND "Percent of sand content"
#define DESC_SBGS "Groundwater storage of the subbasin"
#define DESC_SBIF "Subsurface volume (m3) to streams from each subbasin"
#define DESC_SBOF "overland flow to streams from each subbasin"
#define DESC_SBPET "the potential evapotranspiration rate of the subbasin"
#define DESC_SBQG "groundwater flow out of the subbasin"
#define DESC_SCENARIO "BMPs scenario information"
#define DESC_SDNCO "denitrification threshold: fraction of field capacity"
#define DESC_SED_DEP "distribution of sediment deposition"
#define DESC_SED_FLOW "sediment in flow"
#define DESC_SED_FLUX "outgoing sediment flux"
#define DESC_SED_RECH "Sediment output at reach outlet"
#define DESC_SED_TO_CH "Sediment flowing to channel by hillslope routing"
#define DESC_SAND_RECH "Sand output at reach outlet"
#define DESC_SILT_RECH "Silt output at reach outlet"
#define DESC_CLAY_RECH "Clay output at reach outlet"
#define DESC_SAG_RECH "Small aggregate output at reach outlet"
#define DESC_LAG_RECH "Large aggregate output at reach outlet"
#define DESC_GRAVEL_RECH "Gravel output at reach outlet"
#define DESC_SAND_TO_CH "Sand flowing to channel by hillslope routing"
#define DESC_SILT_TO_CH "Silt flowing to channel by hillslope routing"
#define DESC_CLAY_TO_CH "Clay flowing to channel by hillslope routing"
#define DESC_SAG_TO_CH "Small aggregate flowing to channel by hillslope routing"
#define DESC_LAG_TO_CH "Large aggregate flowing to channel by hillslope routing"
#define DESC_GRAVEL_TO_CH "Gravel flowing to channel by hillslope routing"
#define DESC_SEDSTO_CH "Channel sediment storage (kg)"
#define DESC_SANDSTO_CH "Channel sand storage (kg)"
#define DESC_SILTSTO_CH "Channel silt storage (kg)"
#define DESC_CLAYSTO_CH "Channel clay storage (kg)"
#define DESC_SAGSTO_CH "Channel small aggregate storage (kg)"
#define DESC_LAGSTO_CH "Channel large aggregate storage (kg)"
#define DESC_GRAVELSTO_CH "Channel gravel storage (kg)"
#define DESC_SEDYLD "sediment yield that transported to channel at each cell"
#define DESC_SEDMINPA " amount of active mineral phosphorus absorbed to sediment in surface runoff"
#define DESC_SEDMINPA_CH "amount of active mineral phosphorus absorbed to sediment in surface runoff moved to channel"
#define DESC_SEDMINPS "amount of stable mineral phosphorus sorbed to sediment in surface runoff"
#define DESC_SEDMINPS_CH "amount of stable mineral phosphorus sorbed to sediment in surface runoff moved to channel"
#define DESC_SEDORGN "amount of organic nitrogen in surface runoff"
#define DESC_SEDORGN_CH "amount of organic nitrogen in surface runoff moved to channel"
#define DESC_SEDORGP "amount of organic phosphorus in surface runoff"
#define DESC_SEDORGP_CH "amount of organic phosphorus in surface runoff moved to channel"
#define DESC_SEEPAGE "seepage"
#define DESC_SHALLST "depth of water in shallow aquifer"
#define DESC_SILT "Percent of silt content"
#define DESC_SLOPE "Slope gradient (drop/distance, i.e., tan, or percent)"
#define DESC_SLPLEN "Slope length"
#define DESC_SNAC "snow accumulation"
#define DESC_SNME "snow melt"
#define DESC_SNO3UP "amount of nitrate moving upward in the soil profile in watershed"
#define DESC_SNOCOVMX "Minimum snow water content that corresponds to 100% snow cover"
#define DESC_SNO50COV "Fraction of SNOCOVMX that corresponds to 50% snow cover"
#define DESC_SNRD "snow blowing in or out the cell"
#define DESC_SNSB "snow sublimation (water equivalent)"
#define DESC_SNWB "snow water balance for selected subbasins"
#define DESC_SOER "soil loss caused by water erosion"
#define DESC_SOET "evaporation from the soil water storage"
#define DESC_SOIL_T10 "Factor of soil temperature relative to short grass (degree)"
#define DESC_SOILDEPTH "depth to bottom of each soil layer"
#define DESC_SOILLAYERS "Soil layers number"
#define DESC_SOILTHICK "soil thickness of each soil layer"
#define DESC_SOL_ACTP "amount of phosphorus stored in the active mineral phosphorus pool"
#define DESC_SOL_ALB "albedo when soil is moist"
#define DESC_SOL_AORGN "amount of nitrogen stored in the active organic (humic) nitrogen pool"
#define DESC_SOL_AWC "amount of water available to plants in soil layer at field capacity (AWC=FC-WP)"
#define DESC_SOL_BD "bulk density of the soil"
#define DESC_SOL_CBN "soil carbon content"
#define DESC_SOL_COV "amount of residue on soil surface"
#define DESC_SOL_CRK "crack volume potential of soil"
#define DESC_SOL_FORGN "amount of nitrogen stored in the fresh organic (residue) pool"
#define DESC_SOL_FORGP "amount of phosphorus stored in the fresh organic (residue) pool"
#define DESC_SOL_MC "manure carbon in soil"
#define DESC_SOL_MN "manure nitrogen in soil"
#define DESC_SOL_MP "manure phosphorus in soil"
#define DESC_SOL_N "soil organic nitrogen, include nitrogen in manure"

/// description of rice related parameters
#define DESC_TBD "Base temperature for development"
#define DESC_TOD "Optimum temperature for development"
#define DESC_TMD "Maximum temperature for development"
#define DESC_DVRJ "Development rate during juvenile phase"
#define DESC_DVRI "Development rate during photoperiod-sensitive phase"
#define DESC_DVRP "Development rate during panicle development phase"
#define DESC_DVRR "Development rate in reproductive phase (post anthesis)"
#define DESC_MOPP "Maximum optimum photoperiod"
#define DESC_PPSE "Photoperiod sensitivity"
#define DESC_SHCKD "Relation between seedling age and delay in phenological development"
#define DESC_KNF "extinction coefficient of N profile in the canopy as a function of development stage"
#define DESC_RGRLMX "Maximum relative growth rate of leaf area"
#define DESC_RGRLMN "Minimum relative growth rate of leaf area"
#define DESC_NH "Number of hills"
#define DESC_NPLH "Number of plants per hill"
#define DESC_NPLSB "Number of plants in seedbed"
#define DESC_LAPE "Leaf area per plant at emergence"
#define DESC_ZRTTR "Root length/depth at transplanting"
#define DESC_TMPSB "Temperature increase caused by greenhouse use (over seedbed)"
#define DESC_AFLV "function parameters of fraction total dry matter partitioned to the shoot according to DVS"
#define DESC_BFLV "function parameters of fraction total dry matter partitioned to the shoot according to DVS"
#define DESC_AFSH "function parameters of fraction shoot dry matter partitioned to the leaves according to DVS"
#define DESC_BFSH "function parameters of fraction shoot dry matter partitioned to the leaves according to DVS"
#define DESC_AFSO "function parameters of fraction shoot dry matter partitioned to the panicles according to DVS"
#define DESC_BFSO "function parameters of fraction shoot dry matter partitioned to the panicles according to DVS"
#define DESC_ADRLV "function parameters of leaf death coefficient according to DVS"
#define DESC_BDRLV "function parameters of leaf death coefficient according to DVS"
#define DESC_TCLSTR "Time coefficient for loss of stem reserves"
#define DESC_Q10 "Factor accounting for increase in maintenance respiration with a 10 oC rise in temperature"
#define DESC_TREF "Reference temperature"
#define DESC_MAINLV "Maintenance respiration coefficient:Leaves"
#define DESC_MAINST "Maintenance respiration coefficient:Stems"
#define DESC_MAINSO "Maintenance respiration coefficient:Storage organs (panicles)"
#define DESC_MAINRT "Maintenance respiration coefficient:Roots"
#define DESC_CRGLV "Carbohydrate requirement for dry matter production:Leaves"
#define DESC_CRGST "Carbohydrate requirement for dry matter production:Stems"
#define DESC_CRGSTR "Carbohydrate requirement for dry matter production:Stem reserves"
#define DESC_CRGSO "Carbohydrate requirement for dry matter production:Storage organs (panicles)"
#define DESC_CRGRT "Carbohydrate requirement for dry matter production:Roots"
#define DESC_FSTR "Fraction of carbohydrates allocated to stems that is stored as reserves"
#define DESC_LRSTR "Fraction of allocated stem reserves that is available for growth"
#define DESC_ASLA "SLA function parameters"
#define DESC_BSLA "SLA function parameters"
#define DESC_CSLA "SLA function parameters"
#define DESC_DSLA "SLA function parameters"
#define DESC_SLAMX "maximum value of SLA"
#define DESC_FCRT "Carbon balance parameters, Mass fraction carbon:Roots"
#define DESC_FCLV "Carbon balance parameters, Mass fraction carbon:Leaves"
#define DESC_FCST "Carbon balance parameters, Mass fraction carbon:Stems"
#define DESC_FCSTR "Carbon balance parameters, Mass fraction carbon:Stem reserves"
#define DESC_FCSO "Carbon balance parameters, Mass fraction carbon:Storage organs (panicles)"
#define DESC_WGRMX "Maximum individual grain weight"
#define DESC_GZRT "Growth rate of roots"
#define DESC_ZRTMCD "Maximum depth of roots if drought"
#define DESC_FRPAR "Fraction of total shortwave irradiation that is photo-synthetically active (PAR)"
#define DESC_SPGF "Spikelet growth factor"
#define DESC_NMAXL "function parameters of maximum leaf N fraction"
#define DESC_NMINL "function parameters of minimum leaf N fraction"
#define DESC_RFNLV "Residual N fraction of leaves (kg N kg-1 leaves)"
#define DESC_RFNST "Residual N fraction of stems (kg N kg-1 stems)"
#define DESC_RFNRT "Fraction N translocation from roots as (additonal) fraction of total N translocation from stems and leaves"
#define DESC_TCNTRF "Time coefficient for N translocation to grains"
#define DESC_NMAXSO "Maximum N concentration in storage organs"
#define DESC_ANMINSO "function parameters of minimum N concentration in storage organs"
#define DESC_BNMINSO "function parameters of minimum N concentration in storage organs"
#define DESC_SHCKL "Relation between seedling age and delay in leaf area development"
#define DESC_SBDUR "Duration of seedbed"
#define DESC_LLLS "Lower limit leaf rolling (kPa)"
#define DESC_ULLS "Upper limit leaf rolling (kPa)"
#define DESC_LLLE "Lower limit leaf expansion (kPa)"
#define DESC_ULLE "Upper limit leaf expansion (kPa)"
#define DESC_LLDL "Lower limit death of leaves (kPa)"
#define DESC_ULDL "Upper limit death of leaves (kPa)"

#define DESC_CROPSTA "rice status code"
#define DESC_TS "Temperature sum"
#define DESC_WLVG "Dry weight of green leaves  kg / ha"
#define DESC_WLVD "Dry weight of dead leaves"
#define DESC_WSTS "dry weight of stems reserves"
#define DESC_WSTR "dry weight of structural stems"
#define DESC_WSO "dry weight of storage organs"
#define DESC_WRT "Dry weight of roots"
#define DESC_WRR "Dry weight of rough rice (final yield)"
#define DESC_NGR "Number of grains"
#define DESC_NSP "Number of spikelets"
#define DESC_TNASS "Total net CO2 assimilation  kg CO2 ha-1"
#define DESC_WST "dry weight of stems"
#define DESC_WLV "Dry weight of leaves"
#define DESC_WAGT "Total aboveground dry matter"
#define DESC_ZRT "root length or root depth"
#define DESC_DVS "Development stage of the crop"
#define DESC_ANCRF "Amount of N in crop till flowering"

/// CENTURY model for C/N cycling
#define    DESC_SOL_BM        "NEED to figure out"
#define    DESC_SOL_BMC    "NEED to figure out"
#define    DESC_SOL_BMN    "NEED to figure out"
#define    DESC_SOL_HSC    "mass of C present in slow humus"
#define    DESC_SOL_HSN    "mass of N present in slow humus"
#define    DESC_SOL_HPC    "mass of C present in passive humus"
#define    DESC_SOL_HPN    "mass of N present in passive humus"
#define    DESC_SOL_LM        "mass of metabolic litter"
#define    DESC_SOL_LMC    "metabolic litter C pool"
#define    DESC_SOL_LMN    "metabolic litter N pool"
#define    DESC_SOL_LS        "structural litter SOM pool"
#define    DESC_SOL_LSL    "lignin weight in structural litter"
#define    DESC_SOL_LSC    "structural litter C pool"
#define    DESC_SOL_LSN    "structural litter N pool"
#define    DESC_SOL_LSLC    "lignin amount in structural litter pool"
#define    DESC_SOL_LSLNC    "non-lignin part of the structural litter C"
#define    DESC_SOL_RNMN    "NEED to figure out"
#define    DESC_SOL_RSPC    "NEED to figure out"
#define    DESC_SOL_WOC    "NEED to figure out"
#define    DESC_SOL_WON    "NEED to figure out"
#define    DESC_SOL_HP    "mass of OM in passive humus"
#define    DESC_SOL_HS    "mass of OM in slow humus"
#define DESC_SOL_LATERAL_C "lateral flow Carbon loss in each soil layer"
#define DESC_SOL_PERCO_C "percolation Carbon loss in each soil layer"
#define DESC_LATERAL_C "lateral flow Carbon loss in soil profile"
#define DESC_PERCO_C "percolation Carbon loss in soil profile"
#define DESC_SEDLOSS_C "amount of Carbon lost with sediment"

#define DESC_SOL_NH4 "amount of nitrogen stored in the ammonium pool in soil layer"
#define DESC_SOL_NO3 "amount of nitrogen stored in the nitrate pool in soil layer"
#define DESC_SOL_OM "percent of organic matter in soil"
#define DESC_SOL_SORGN "amount of nitrogen stored in the stable organic N pool"
#define DESC_SOL_HORGP "amount of phosphorus stored in the humic organic P pool in soil layer"
#define DESC_SOL_PERCO "percolation from soil layer"
#define DESC_RSDCOV_COEF "residue cover factor for computing fraction of cover"
#define DESC_SOL_RSD "amount of organic matter in the soil classified as residue"
#define DESC_SOL_RSDIN "amount of organic matter in the soil classified as residue"
#define DESC_SOL_SOLP "amount of phosphorus stored in solution"
#define DESC_SOL_STAP "amount of phosphorus in the soil layer stored in the stable mineral phosphorus pool"
#define DESC_SOL_SUMAWC "amount of water held in soil profile at field capacity"
#define DESC_SOL_SUMSAT "amount of water held in soil profile at saturation"
#define DESC_SOL_TA0 "Coefficient a0 for Finn Plauborg Method"
#define DESC_SOL_TA1 "Coefficient a1 for Finn Plauborg Method"
#define DESC_SOL_TA2 "Coefficient a2 for Finn Plauborg Method"
#define DESC_SOL_TA3 "Coefficient a3 for Finn Plauborg Method"
#define DESC_SOL_TB1 "Coefficient b1 for Finn Plauborg Method"
#define DESC_SOL_TB2 "Coefficient b2 for Finn Plauborg Method"
#define DESC_SOL_TD1 "Coefficient d1 for Finn Plauborg Method"
#define DESC_SOL_TD2 "Coefficient d2 for Finn Plauborg Method"
#define DESC_SOL_TMP "daily average temperature of soil layer"
#define DESC_SOL_UL "amount of water held in the soil layer at saturation (sat - wp water)"
#define DESC_SOL_WFC "Water content of soil profile at field capacity"
#define DESC_SOL_WPMM "water content of soil at -1.5 MPa (wilting point)"
#define DESC_SOL_ZMX "Maximum rooting depth of soil profile (mm)"
#define DESC_SOL_ST "amount of water stored in the soil layer on current day(mm H2O)"
#define DESC_SOL_SW "amount of water stored in soil profile on current day (mm H2O)"
#define DESC_SW_CAP "amount of water capacity in soil layers such as sol_awc sol_ul and wiltingpoint"
#define DESC_SOTE "soil Temperature"
#define DESC_SOWB "soil water balance"
#define DESC_SOXY "saturation concentration of dissolved oxygen"
#define DESC_SPCON "Coefficient in sediment transport equation"
#define DESC_SPEXP "Exponent in sediment transport equation"
#define DESC_SR "Solar radiation"
#define DESC_SR_MAX "Max solar radiation"
#define DESC_SRA " solar radiation for the day"
#define DESC_SSRU "Subsurface runoff"
#define DESC_SSRUVOL "Subsurface runoff volume (m3)."
#define DESC_STCAPSURPLUS "surplus of storage capacity"
#define DESC_STREAM_LINK "Stream link (id of reaches)"
#define DESC_SUB_SEDTOCH "sediment to streams from each subbasin"
#define DESC_SubbasinSelected "The subbasion IDs listed in file.out"
#define DESC_SUBBSN "The subbasion grid"
#define DESC_SUBBSNID_NUM "number of subbasins"
#define DESC_SUR_NO3 "amount of nitrate transported with surface runoff"
#define DESC_SUR_NO3_ToCH "amount of nitrate transported with surface runoff to channel"
#define DESC_SUR_NH4 "amount of ammonian transported with surface runoff"
#define DESC_SUR_NH4_TOCH "amount of ammonian transported with surface runoff to channel"
#define DESC_SUR_SOLP "amount of solution phosphorus in surface runoff"
#define DESC_SUR_SOLP_ToCH "amount of soluble phosphorus from surface runoff to channel"
#define DESC_SUR_COD_ToCH "amount of COD to reach in surface runoff"
#define DESC_SURU "surface runoff"
#define DESC_SWE "average snow accumulation of the watershed"
#define DESC_SWE0 "Initial snow water equivalent"
#define DESC_T_BASE "base or minimum temperature for plant growth"
#define DESC_T_OPT "optimal temperature for plant growth"
#define DESC_T_RG "groundwater runoff"
#define DESC_T_SNOW "Snowfall temperature"
#define DESC_T_SOIL "soil freezing temperature threshold"
#define DESC_T0 "the snowmelt threshold temperature"
#define DESC_Tag_FLOWIN_PERCENTAGE_DINF "Percentage of flow in"
#define DESC_TFACT "fraction of solar radiation computed in the temperature heat balance that is photo synthetically active"
#define DESC_TILLAGE_LOOKUP "Tillage lookup table"
#define DESC_TILLAGE_DAYS "days from tillage"
#define DESC_TILLAGE_DEPTH "tillage depth"
#define DESC_TILLAGE_SWITCH "switch of whether to tillage"
#define DESC_TILLAGE_FACTOR "influence factor of tillage operation"
#define DESC_TIMESTEP "time step"
#define DESC_TMAX "max temperature"
#define DESC_TMEAN "mean temperature"
#define DESC_TMEAN_ANN "annual mean temperature"
#define DESC_TMEAN1 "Mean air temperature of the (d-1)th day"
#define DESC_TMEAN2 "Mean air temperature of the (d-2)th day"
#define DESC_TMIN "min temperature"
#define DESC_TREEYRS "initial age of tress (yrs)"
#define DESC_TSD_CLIMATE "Climate data of all the stations"
#define DESC_TSD_DT "Time series data type, e.g., climate data"
#define DESC_UPSOLDEP "depth of the upper soil layer"
#define DESC_ICFAC "C-factor calculation using Cmin (0, default) or new method from RUSLE (1)"
#define DESC_USLE_C "the average annual cover management factor for the land cover"
#define DESC_USLE_K "The soil erodibility factor used in USLE"
#define DESC_USLE_L "USLE slope length factor"
#define DESC_USLE_S "USLE slope factor"
#define DESC_USLE_P "the erosion control practice factor"
#define DESC_VCD "compute changes in channel dimensions"
#define DESC_VDIV "diversion loss of the river reach"
#define DESC_VER_ITP "Execute vertical interpolation (1) or not (0), defined in config.fig"
#define DESC_VP_ACT "actual vapor pressure"
#define DESC_VP_SAT "Saturated vapor pressure"
#define DESC_VPD "vapor pressure deficit"
#define DESC_VPDFR "vapor pressure deficit(kPa) corresponding to the second point on the stomatal conductance curve"
#define DESC_VSEEP0  "the initial volume of transmission loss to the deep aquifer over the time interval"
#define DESC_WATTEMP "temperature of water in reach"
#define DESC_WAVP "rate of decline in rue per unit increase in vapor pressure deficit"
#define DESC_WDNTL "amount of nitrogen lost from nitrate pool by denitrification in soil profile on current day in cell"
#define DESC_WEIGHT_ITP "Weight used for interpolation"
#define DESC_WILTPOINT "Plant wilting point moisture"
#define DESC_WS "Wind speed (measured at 10 meters above surface)"
#define DESC_WSHD_DNIT "nitrogen lost from nitrate pool due to denitrification in watershed"
#define DESC_WSHD_HMN "nitrogen moving from active organic to nitrate pool in watershed"
#define DESC_WSHD_HMP "phosphorus moving from organic to labile pool in watershed"
#define DESC_WSHD_NITN "nitrogen moving from the NH3 to the NO3 pool by nitrification in the watershed"
#define DESC_WSHD_PAL "phosphorus moving from labile mineral to active mineral pool in watershed"
#define DESC_WSHD_PAS "phosphorus moving from active mineral to stable mineral pool in watershed"
#define DESC_WSHD_PLCH "phosphorus leached into second soil layer"
#define DESC_WSHD_RMN "nitrogen moving from fresh organic (residue) to nitrate and active organic pools in watershed"
#define DESC_WSHD_RMP "phosphorus moving from fresh organic (residue) to labile and organic pools in watershed"
#define DESC_WSHD_RNO3 "NO3 added to soil by rainfall in watershed"
#define DESC_WSHD_RWN "nitrogen moving from active organic to stable organic pool in watershed"
#define DESC_WSHD_VOLN "average annual amount if nitrogen lost by ammonia volatilization in watershed"
#define DESC_WSYF "Lower limit of harvest index ((kg/ha)/(kg/ha))"
#define DESC_AL_OUTLET "algae concentration at the watershed outlet"
#define DESC_ON_OUTLET "organicn concentration at the watershed outlet"
#define DESC_AN_OUTLET "ammonian concentration at the watershed outlet"
#define DESC_NIN_OUTLET "nitriten concentration at the watershed outlet"
#define DESC_NAN_OUTLET "nitraten concentration at the watershed outlet"
#define DESC_OP_OUTLET "organicp concentration at the watershed outlet"
#define DESC_DP_OUTLET "disolvp concentration at the watershed outlet"
#define DESC_COD_OUTLET "cod concentration at the watershed outlet"
#define DESC_CHL_OUTLET "chlora concentration at the watershed outlet"

#define DESC_A_DAYS "days since P Application"
#define DESC_B_DAYS "days since P deficit" */

//////////////////////////////////////////////////////////////////////////
/// Define MongoDB related constant strings used in SEIMS and preprocess//
/// By LiangJun Zhu, May. 4, 2016  ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CONST_CHARS MONG_GRIDFS_FN =                        "filename";
CONST_CHARS MONG_GRIDFS_WEIGHT_CELLS =              "NUM_CELLS";
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
#define HEADER_RS_NODATA                       "NODATA_VALUE"
#define HEADER_RS_XLL                          "XLLCENTER"
#define HEADER_RS_YLL                          "YLLCENTER"
#define HEADER_RS_NROWS                        "NROWS"
#define HEADER_RS_NCOLS                        "NCOLS"
#define HEADER_RS_CELLSIZE                     "CELLSIZE"
#define HEADER_RS_LAYERS                       "LAYERS"
#define HEADER_RS_SRS                          "SRS"

#endif
