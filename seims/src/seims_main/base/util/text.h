/*!
 * \brief Predefined string constants used in the code
 *        BE CAUTION, constant value must be aligned by SPACE, not TAB!
 * \author Junzhi Liu, LiangJun Zhu, Huiran Gao
 * \version 1.1
 * \date Jun. 2010, Apr.2016
 */
#ifndef SEIMS_TEXT_H
#define SEIMS_TEXT_H

#define MODEL_NAME                             "SEIMS"
#define MODEL_VERSION                          "2017"
#define SEIMS_EMAIL                            "zlj@lreis.ac.cn"
#define SEIMS_SITE                             "https://github.com/lreis2415/SEIMS"

//! Constant input variables
#define CONS_IN_ELEV                           "Elevation"
#define CONS_IN_LAT                            "Latitude"
#define CONS_IN_XPR                            "xpr"
#define CONS_IN_YPR                            "ypr"

//! Climate data type
#define DataType_Precipitation                 "P"             //1, Suffix of precipitation data
#define DataType_MeanTemperature               "TMEAN"         //2
#define DataType_MinimumTemperature            "TMIN"          //3
#define DataType_MaximumTemperature            "TMAX"          //4
#define DataType_PotentialEvapotranspiration   "PET"           //5
#define DataType_SolarRadiation                "SR"            //6
#define DataType_WindSpeed                     "WS"            //7
#define DataType_RelativeAirMoisture           "RM"            //8
#define DataType_Meteorology                   "M"             // Suffix of meteorology data
#define DataType_Prefix_TS                     "T"             // Prefix of time series data
#define DataType_Prefix_DIS                    "D"             // Prefix of distributed data

/// Tags of climate related data
#define Tag_DEM                                "DEM"
#define Tag_Elevation_Meteorology              "StationElevation_M"
#define Tag_Elevation_PET                      "StationElevation_PET"
#define Tag_Elevation_Precipitation            "StationElevation_P"
#define Tag_Elevation_Temperature              "StationElevation_T"
#define Tag_LapseRate                          "LapseRate"
#define Tag_Latitude_Meteorology               "Latitude_M"
#define Tag_ProjectsPath                       "ProjectsPath"
#define Tag_StationElevation                   "StationElevation"
#define Tag_VerticalInterpolation              "VERTICALINTERPOLATION"
#define Tag_Weight                             "WEIGHT"
#define Tag_DataType                           "DATATYPE"   /// For TSD_RD module
///////  define parameter calibration related string constants  ///////
#define PARAM_CHANGE_VC                        "VC"  // replace by a value
#define PARAM_CHANGE_RC                        "RC"  // multiply a ratio, which is diff from SWAT: * (1+ratio)
#define PARAM_CHANGE_AC                        "AC"  // add a value
#define PARAM_CHANGE_NC                        "NC"  // no change
#define PARAM_FLD_NAME                         "NAME"
#define PARAM_FLD_DESC                         "DESCRIPTION"
#define PARAM_FLD_UNIT                         "UNIT"
#define PARAM_FLD_MIDS                         "MODULE"
#define PARAM_FLD_VALUE                        "VALUE"
#define PARAM_FLD_IMPACT                       "IMPACT"
#define PARAM_FLD_CHANGE                       "CHANGE"
#define PARAM_FLD_MAX                          "MAX"
#define PARAM_FLD_MIN                          "MIN"
#define PARAM_FLD_USE                          "USE"
#define PARAM_USE_Y                            "Y"
#define PARAM_USE_N                            "N"
#define PARAM_CALI_VALUES                      "CALI_VALUES"  /// for calibration

////////////  Input and Output Tags   ///////////////
// Fields in Model Configuration Collections //
//// Tags in file.in
#define Tag_ConfTag                            "TAG"
#define Tag_ConfValue                          "VALUE"
#define Tag_MODCLS                             "MODULE_CLASS"
//// Tags in file.out
#define Tag_OutputUSE                          "USE"
#define Tag_OutputID                           "OUTPUTID"
#define Tag_OutputDESC                         "DESCRIPTION"
#define Tag_OutputUNIT                         "UNIT"
#define Tag_StartTime                          "STARTTIME"
#define Tag_EndTime                            "ENDTIME"
#define Tag_FileName                           "FILENAME"
#define Tag_AggType                            "TYPE"
#define Tag_OutputSubbsn                       "SUBBASIN"
#define Tag_Interval                           "INTERVAL"
#define Tag_IntervalUnit                       "INTERVAL_UNIT"

/// Available values of Tag_outputSubbsn
#define Tag_AllSubbsn                          "ALL"
#define Tag_Outlet                             "OUTLET"

#define Tag_SiteCount                          "SITECOUNT"
#define Tag_SiteName                           "SITENAME"
#define Tag_SiteID                             "SITEID"
#define Tag_ReachName                          "REACHNAME"
#define Tag_Count                              "COUNT"

//// Output data aggregation type //////
#define Tag_Unknown                            "UNKNOWN"
#define Tag_Sum                                "SUM"
#define Tag_Average                            "AVERAGE"
#define Tag_Average2                           "AVE"
#define Tag_Average3                           "MEAN"
#define Tag_Minimum                            "MIN"
#define Tag_Maximum                            "MAX"
#define Tag_SpecificCells                      "SPECIFIC"

#define TAG_OUT_QOUTLET                        "QOUTLET"
#define TAG_OUT_QTOTAL                         "QTotal"
#define TAG_OUT_SEDOUTLET                      "SEDOUTLET"
#define TAG_OUT_OL_IUH                         "OL_IUH"
#define Tag_DisPOutlet                         "DissovePOutlet"
#define Tag_AmmoOutlet                         "AmmoniumOutlet"
#define Tag_NitrOutlet                         "NitrateOutlet"

#define Tag_SubbasinCount                      "SUBBASINCOUNT"
#define Tag_SubbasinId                         "SUBBASINID"
#define Tag_ReservoirCount                     "RESERVOIRCOUNT"
#define Tag_ReservoirId                        "RESERVOIRID"
#define Tag_SubbasinSelected                   "subbasinSelected"
#define Tag_CellSize                           "CELLSIZE"
#define Tag_Mask                               "MASK"
#define Tag_TimeStep                           "TIMESTEP"
#define Tag_HillSlopeTimeStep                  "DT_HS"
#define Tag_ChannelTimeStep                    "DT_CH"
#define Tag_CellWidth                          "CELLWIDTH" // this is the size of a single CELL
#define Tag_LayeringMethod                     "LayeringMethod"

// D8 Flow model
#define Tag_FLOWIN_INDEX_D8                    "FLOWIN_INDEX_D8"
#define Tag_FLOWOUT_INDEX_D8                   "FLOWOUT_INDEX_D8"
#define Tag_ROUTING_LAYERS                     "ROUTING_LAYERS"

// TODO: Dinf, MFD, MFD-md integrated into SEIMS.
//! D-infinity Flow model after Tarboton et al.(1991)
#define Tag_FLOWIN_INDEX_DINF                  "FLOWIN_INDEX_DINF"
#define Tag_FLOWIN_PERCENTAGE_DINF             "FLOWIN_PERCENTAGE_DINF"
#define Tag_FLOWOUT_INDEX_DINF                 "FLOWOUT_INDEX_DINF"
#define Tag_FLOWOUT_PERCENTAGE_DINF            "FLOWOUT_PERCENTAGE_DINF"
#define Tag_ROUTING_LAYERS_DINF                "ROUTING_LAYERS_DINF"
//! Multi-Flow model after Quinn et al.(1991)
#define Tag_FLOWIN_INDEX_MFD                   "FLOWIN_INDEX_MFD"
#define Tag_FLOWIN_PERCENTAGE_MFD              "FLOWIN_PERCENTAGE_MFD"
#define Tag_FLOWOUT_INDEX_MFD                  "FLOWOUT_INDEX_MFD"
#define Tag_FLOWOUT_PERCENTAGE_MFD             "FLOWOUT_PERCENTAGE_MFD"
#define Tag_ROUTING_LAYERS_MFD                 "ROUTING_LAYERS_MFD"
//! MFD-md flow model after Qin et al.(2007) (md means maximum downslope), TODO.
#define Tag_FLOWIN_INDEX_MFD_MD                "FLOWIN_INDEX_MFD_MD"
#define Tag_FLOWIN_PERCENTAGE_MFD_MD           "FLOWIN_PERCENTAGE_MFD_MD"
#define Tag_FLOWOUT_INDEX_MFD_MD               "FLOWOUT_INDEX_MFD_MD"
#define Tag_FLOWOUT_PERCENTAGE_MFD_MD          "FLOWOUT_PERCENTAGE_MFD_MD"
#define Tag_ROUTING_LAYERS_MFD_MD              "ROUTING_LAYERS_MFD_MD"

//#define Tag_ReachParameter                     "ReachParameter"
//#define Tag_RchParam                           "RchParam"
/// Replaced Tag_ReachParameter and Tag_RchParam by VAR_REACH_PARAM
#define VAR_REACH_PARAM                        "ReachParam"
#define DESC_REACH_PARAM                       "Reach parameters such as stream order, manning's n and downstream subbasin id"
/// Add Subbasins as AddParameters for modules
#define VAR_SUBBASIN_PARAM                     "SubbasinParam"
#define DESC_SUBBASIN_PARAM                    "Statistics of subbasin related parameters"
/// Files or database constant strings
//#define ASCIIExtension                       ".asc"  /// defined in clsRasterData.h
//#define GTiffExtension                       ".tif"
#define TextExtension                          "txt"

#define File_Config                            "config.fig"
#define File_Input                             "file.in"
#define File_Output                            "file.out"
#define Source_HydroClimateDB                  "HydroClimateDB"
#define Source_ParameterDB                     "ParameterDB"
#define Source_Module                          "Module"
#define Source_Module_Optional                 "Module_Optional"

///////// Table Names required in MongoDB /////////
#define DB_TAB_PARAMETERS                      "PARAMETERS"
#define DB_TAB_SITELIST                        "SITELIST"
#define DB_TAB_SCENARIO                        "BMPDATABASE"
#define DB_TAB_REACH                           "REACHES"
#define DB_TAB_SPATIAL                         "SPATIAL"  /// i.e., spatial.files
#define DB_TAB_SITES                           "SITES"
#define DB_TAB_DATAVALUES                      "DATA_VALUES" // hydroClimate data values
#define DB_TAB_MEASUREMENT                     "MEASUREMENT"
#define DB_TAB_ANNSTAT                         "ANNUAL_STATS"
#define DB_TAB_OUT_SPATIAL                     "OUTPUT"
#define DB_TAB_FILE_IN                         "FILE_IN"
#define DB_TAB_FILE_OUT                        "FILE_OUT"
/// Fields in DB_TAB_REACH ///
#define REACH_SUBBASIN                         "SUBBASINID"
#define REACH_NUMCELLS                         "NUM_CELLS"
#define REACH_GROUP                            "GROUP"
#define REACH_GROUPDIVIDED                     "GROUP_DIVIDE"
#define REACH_KMETIS                           "KMETIS"
#define REACH_PMETIS                           "PMETIS"
#define REACH_DOWNSTREAM                       "DOWNSTREAM"
#define REACH_UPDOWN_ORDER                     "UP_DOWN_ORDER"
#define REACH_DOWNUP_ORDER                     "DOWN_UP_ORDER"
#define REACH_WIDTH                            "CH_WIDTH"
#define REACH_SIDESLP                          "CH_SSLP"
#define REACH_LENGTH                           "CH_LEN"
#define REACH_DEPTH                            "CH_DEPTH"
#define REACH_V0                               "CH_V0"
#define REACH_AREA                             "CH_AREA"
#define REACH_MANNING                          "CH_N"
#define REACH_SLOPE                            "CH_SLP"
#define REACH_KBANK                            "CH_K_BANK"
#define REACH_KBED                             "CH_K_BED"
#define REACH_COVER                            "CH_COVER"
#define REACH_EROD                             "CH_EROD"
#define REACH_BC1                              "BC1"
#define REACH_BC2                              "BC2"
#define REACH_BC3                              "BC3"
#define REACH_BC4                              "BC4"
#define REACH_RS1                              "RS1"
#define REACH_RS2                              "RS2"
#define REACH_RS3                              "RS3"
#define REACH_RS4                              "RS4"
#define REACH_RS5                              "RS5"
#define REACH_RK1                              "RK1"
#define REACH_RK2                              "RK2"
#define REACH_RK3                              "RK3"
#define REACH_RK4                              "RK4"
#define REACH_DISOX                            "DISOX"
#define REACH_BOD                              "BOD"
#define REACH_ALGAE                            "ALGAE"
#define REACH_ORGN                             "ORGN"
#define REACH_NH4                              "NH4"
#define REACH_NO2                              "NO2"
#define REACH_NO3                              "NO3"
#define REACH_ORGP                             "ORGP"
#define REACH_SOLP                             "SOLP"
#define REACH_GWNO3                            "GWNO3"
#define REACH_GWSOLP                           "GWSOLP"
/// these four are defined in DB_TAB_SITELIST in Source_ParameterDB
#define SITELIST_TABLE_M                       "SITELISTM"
#define SITELIST_TABLE_P                       "SITELISTP"
#define SITELIST_TABLE_PET                     "SITELISTPET"
#define SITELIST_TABLE_STORM                   "STORMSITELIST"

//! define string constants used in the code, also used in the mongoDB.SiteList table's header
#define Tag_Mode                               "MODE"
#define Tag_Mode_Storm                         "STORM"
#define Tag_Mode_Daily                         "DAILY"

#define Type_Scenario                          "SCENARIO"
#define Type_Reach                             "REACH"
#define Type_Subbasin                          "SUBBASIN"
#define Type_Raster1D                          "RASTER1D"
#define Type_Raster2D                          "RASTER2D"
#define Type_Array1DDateValue                  "ARRAY1DDATEVALUE"
#define Type_Array3D                           "ARRAY3D"
#define Type_Array2D                           "ARRAY2D"
#define Type_Array1D                           "ARRAY1D"
#define Type_Single                            "SINGLE"

//////////////////////////////////////////////////////////////////////////
/// Define models' ID and description in SEIMS  //////////////////////////
/// By LiangJun Zhu, Apr. 26, 2016  //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/// Hydro-Meteorological data
#define MCLS_CLIMATE                           "HydroClimate"
#define MCLSDESC_CLIMATE                       "HydroClimate data modules"
#define MID_TSD_RD                             "TSD_RD"
#define MDESC_TSD_RD                           "Read time series data from HydroClimate database."
#define MID_ITP                                "ITP"
#define MDESC_ITP                              "Interpolation of P, T, etc."

/// Soil temperature
#define MCLS_SOLT                              "Soil Temperature"
#define MCLSDESC_SOLT                          "Calculate the soil temperature."
#define MID_STP_FP                             "STP_FP"
#define MDESC_STP_FP                           "Finn Plauborg Method to compute soil temperature."
/// Interception
#define MCLS_INTERC                            "Interception"
#define MCLSDESC_INTERC                        "Precipation interception module"
#define MID_PI_SVSC                            "PI_SVSC"
#define MDESC_PI_SVSC                          "Precipitation interception by seasonal variation of storage capacity method"
#define MID_PI_MSC                             "PI_MSC"
#define MDESC_PI_MCS                           "Precipitation interception based on Maximum Canopy Storage"
/// Snow redistribution
#define MCLS_SNO_RD                            "Snow redistribution"
#define MCLSDESC_SNO_RD                        "Snow redistribution calculation"
#define MID_SRD_MB                             "SRD_MB"
#define MDESC_SRD_MB                           "Original WetSpa algorithm"
/// Snow sublimation
#define MCLS_SNO_SB                            "Snow sublimation"
#define MCLSDESC_SNO_SB                        "Calculate the amount of snow sublimation ."
#define MID_SSM_PE                             "SSM_PE"
#define MDESC_SSM_PE                           "A simple method that used in the old WetSpa to calculate snow sublimation."
/// Snow melt
#define MCLS_SNOW                              "Snow accumulation and melt"
#define MCLSDESC_SNOW                          "Calculate the amount of snow  accumulation andmelt."
#define MID_SNO_WB                             "SNO_WB"
#define MDESC_SNO_WB                           "Calculate snow water balance"
#define MID_SNO_DD                             "SNO_DD"
#define MDESC_SNO_DD                           "Degree-Day method (Martinec et al., 1983) for snow melt modeling"
#define MID_SNO_SP                             "SNO_SP"
#define MDESC_SNO_SP                           "Snowpack Daily method from SWAT"

/// Potential Evapotranspiration Modules
#define MCLS_PET                               "Potential Evapotranspiration"
#define MCLSDESC_PET                           "Calculate the potential evapotranspiration"
#define MID_PET_H                              "PET_H"
#define MDESC_PET_H                            "Hargreaves method for calculating the potential evapotranspiration."
#define MID_PET_PT                             "PET_PT"
#define MDESC_PET_PT                           "PriestleyTaylor method for calculating the potential evapotranspiration."
#define MID_PET_PM                             "PET_PM"
#define MDESC_PET_PM                           "Penman Monteith method for calculating the potential evapotranspiration."
/// Actual Evapotranspiration
#define MCLS_AET                               "Actual Evapotranspiration"
#define MCLSDESC_AET                           "Calculates potential plant transpiration and potential and actual soil evaporation. "
#define MID_AET_PTH                            "AET_PTH"
#define MDESC_AET_PTH                          "Potential plant transpiration for Priestley-Taylor and Hargreaves ET methods "
/// Depression
#define MCLS_DEP                               "Depression"
#define MCLSDESC_DEP                           "Calculate depression storage."
#define MID_DEP_FS                             "DEP_FS"
#define MDESC_DEP_FS                           "A simple fill and spill method method to calculate depression storage."
#define MID_DEP_LINSLEY                        "DEP_LINSLEY"
#define MDESC_DEP_LINSLEY                      "Linsley method to calculate depression storage"
/// Surface runoff
#define MCLS_SUR_RUNOFF                        "Surface runoff"
#define MCLSDESC_SUR_RUNOFF                    "Infiltration and surface runoff of excess precipitation."
#define MID_SUR_MR                             "SUR_MR"
#define MDESC_SUR_MR                           "Modified rational method to calculate infiltration and excess precipitation."
#define MID_SUR_CN                             "SUR_CN"
#define MDESC_SUR_CN                           "SCS curve number method to calculate infiltration and excess precipitation."
#define MID_SUR_SGA                            "SUR_SGA"
#define MDESC_SUR_SGA                          "Modified rational method to calculate infiltration and excess precipitation."
/// Interflow
#define MCLS_INTERFLOW                         "Interflow routing"
#define MCLSDESC_INTERFLOW                     "Interflow routing."
#define MID_IKW_IF                             "IKW_IF"
#define MDESC_IKW_IF                           "interflow routing using the method of WetSpa model."
#define MID_IUH_IF                             "IUH_IF"
#define MDESC_IUH_IF                           "IUH overland method to calculate interflow routing."
/// Percolation
#define MCLS_PERCO                             "Percolation"
#define MCLSDESC_PERCO                         "Calculate the amount of water percolated out of the root zone within the time step."
#define MID_PER_PI                             "PER_PI"
#define MDESC_PER_PI                           "Percolation calculated by Darcy's law and Brooks-Corey equation"
#define MID_PER_STR                            "PET_STR"
#define MDESC_PER_STR                          "Percolation calculated by storage routing method"
#define MID_PERCO_DARCY                        "PERCO_DARCY"
#define MDESC_PERCO_DARCY                      "The method relating percolation with soil moisture and pore size distribution index used in the original WetSpa will be the default method to estimate percolation out of the root zone."
/// Subsurface
#define MCLS_SUBSURFACE                        "Subsurface"
#define MCLSDESC_SUBSURFACE                    "Subsurface Runoff"
#define MID_SSR_DA                             "SSR_DA"
#define MDESC_SSR_DA                           "Darcy's law and the kinematic approximation; Water is routed cell-to-cell according to D8 flow direction"
/// Soil water balance
#define MCLS_WTRBALANCE                        "Water banlance"
#define MCLSDESC_WTRBALANCE                    "Water balance calculation"
#define MID_SOL_WB                             "SOL_WB"
#define MDESC_SOL_WB                           "Soil water balance calculation"
/// Hillslope hydrology
#define MCLS_HS_HYDRO                          "Hillslope water balance"
#define MCLSDESC_HS_HYDRO                      "Water balance calculation in hillslope."
#define MID_HS_WB                              "HS_WB"
#define MDESC_HS_WB                            "Hillsloope water balance."
/// Paddy
#define MCLS_PADDY                             "Paddy"
#define MCLSDESC_PADDY                         "Paddy simulations"
#define MID_IMP_SWAT                           "IMP_SWAT"
#define MDESC_IMP_SWAT                         "SWAT method, simulates depressional areas that do not drain to the stream network (pothole) and impounded areas such as rice paddies"
/// Groundwater
#define MCLS_GW                                "Groundwater"
#define MCLSDESC_GW                            "Calculate groundwater balance and baseflow."
#define MID_GW_RSVR                            "GW_RSVR"
#define MDESC_GW_RSVR                          "Calculate groundwater using reservoir method for storm model"
#define MID_GWA_RE                             "GWA_RE"
#define MDESC_GWA_RE                           "Reservoir Method to calculate groundwater balance and baseflow for longterm model"
/// Erosion related modules
#define MCLS_OL_EROSION                        "Overland erosion"
#define MCLS_CH_EROSION                        "Channel erosion"
#define MCLSDESC_OL_EROSION                    "Calculate the amount sediment yield of overland erosion."
#define MCLSDESC_CH_EROSION                    "Calculate the amount channel erosion."
#define MID_SplashEro_Park                     "SplashEro_Park"
#define MDESC_SplashEro_Park                   "use Park equation to calculate sediment yield of each cell"
#define MID_KINWAVSED_OL                       "KinWavSed_OL"
#define MID_KINWAVSED_CH                       "KinWavSed_CH"
#define MDESC_KINWAVSED_OL                     "Use energy function(Govers) method to calculate sediment yield routing of each hillslope cell"
#define MDESC_KINWAVSED_CH                     "Srinivasan & Galvao function to calculate sediment yield routing of each channel cell"
#define MID_MUSLE_AS                           "MUSLE_AS"
#define MDESC_MUSLE_AS                         "use MUSLE method to calculate sediment yield of each cell"
#define MID_IUH_SED_OL                         "IUH_SED_OL"
#define MDESC_IUH_SED_OL                       "Overland routing of sediment using IUH"

#define MID_NUTR_CH                            "NUTR_CH"
#define MDESC_NUTR_CH                          "Channel routing of nutrients"
/// Management
#define MCLS_MGT                               "Mangement practices"
#define MCLSDESC_MGT                           "BMP related modules"
#define MID_PLTMGT_SWAT                        "PLTMGT_SWAT"
#define MDESC_PLTMGT_SWAT                      "Plant mangement operation modeling method in SWAT"
#define MID_NPSMGT                             "NPSMGT"
#define MDESC_NPSMGT                           "Non-point source pollution management"
/// Ecology
#define MCLS_PG                                "Plant growth"
#define MCLSDESC_PG                            "Calculate the amount of plant growth."
#define MID_BIO_EPIC                           "BIO_EPIC"
#define MDESC_BIO_EPIC                         "Calculate plant growth using a simplified version of the EPIC plant growth model as in SWAT"
/// Overland routing related modules
#define MCLS_OL_ROUTING                        "Overland routing"
#define MCLSDESC_OL_ROUTING                    "Overland routing module"
#define MID_IKW_OL                             "IKW_OL"
#define MDESC_IKW_OL                           "Overland routing using 4-point implicit finite difference method."
#define MID_IUH_OL                             "IUH_OL"
#define MDESC_IUH_OL                           "IUH overland method to calculate overland flow routing."
/// Channel routing related modules
#define MCLS_CH_ROUTING                        "Channel routing"
#define MCLSDESC_CH_ROUTING                    "Channel routing modules"
#define MID_CH_DW                              "CH_DW"
#define MDESC_CH_DW                            "Channel routing using diffusive wave equation."
#define MID_CH_MSK                             "CH_MSK"
#define MDESC_CH_MSK                           "Channel routing using Muskingum-Cunge method of storm model."
#define MID_IKW_CH                             "IKW_CH"
#define MDESC_IKW_CH                           "Channel routing using 4-point implicit finite difference method for kinematic wave."
#define MID_MUSK_CH                            "MUSK_CH"
#define MDESC_MUSK_CH                          "Channel routing using Muskingum-Cunge method of longterm model."
/// Sediment routing related modules
#define MCLS_SED_ROUTING                       "Sediment routing"
#define MCLSDESC_SED_ROUTING                   "Sediment channel routing modules."
#define MID_SEDR_SBAGNOLD                      "SEDR_SBAGNOLD"
#define MDESC_SEDR_SBAGNOLD                    "Sediment channel routing using variable channel dimension method as used in SWAT."
/// Nutrient
/// carbon, nitrogen, and phosphorus mineralization and immobilization etc
#define MCLS_NUTRCYC                           "nutrient cycling"
#define MCLSDESC_NUTRCYC                       "Carbon, nitrogen, and phosphorus cycling"
#define MID_NUTR_TF                            "NUTR_TF"
#define MDESC_NUTR_TF                          "Daily nitrogen and phosphorus mineralization and immobilization considering fresh organic material (plant residue) and active and stable humus material."
/// Nutrient removed and loss in surface runoff
#define MCLS_NUTRSED                           "Nutrient removed and loss in surface runoff, lateral flow, tile flow, and percolation out of the profile."
#define MCLSDESC_NUTRSED                       "Nutrient removed and loss in surface runoff, lateral flow, tile flow, and percolation out of the profile."
#define MID_NUTRSED                            "NUTRSED"
#define MDESC_NUTRSED                          "Nutrient removed and loss in surface runoff, lateral flow, tile flow, and percolation out of the profile."
/// Atmospheric Deposition
#define MCLS_ATMDEP                            "AtmosphericDeposition"
#define MCLSDESC_ATMDEP                        "AtmosphericDeposition"
#define MID_ATMDEP                             "ATMDEP"
#define MDESC_ATMDEP                           "AtmosphericDeposition"
/// Nutrient remove
#define MCLS_NutRemv                           "Nutrient remove"
#define MCLSDESC_NutRemv                       "Simulates the loss of nitrate and phosphorus via surface runoff"
#define MID_NUTRMV                             "NutrMV"
#define MDESC_NUTRMV                           "Simulates the loss of nitrate and phosphorus via surface runoff"
/// Nutrient routing
#define MID_SSR_NUTR                           "SSR_NUTR"
#define MDESC_SSR_NUTR                         "Nutrient routing through soil flow"
#define MID_IUH_NUTR_OL                        "IUH_NUTR_OL"
#define MDESC_IUH_NUTR_OL                      "Overland nutrient routing"
/// Nutrient loading contributed by groundwater flow
#define MCLS_NUTRGW                            "Nutrient in groundwater"
#define MCLSDESC_NUTRGW                        "Simulates the tutrient loading contributed by groundwater flow"
#define MID_NUTRGW                             "NutrGW"
#define MDESC_NUTRGW                           "Simulates the tutrient loading contributed by groundwater flow"
/// In-stream nutrient transformations
#define MCLS_NutCHRout                         "Nutrient in reach"
#define MCLSDESC_NutCHRout                     "In-stream nutrient transformations"

#define MID_NUTRCH_QUAL2E                      "NutrCH_QUAL2E"
#define MDESC_NUTRCH_QUAL2E                    "In-stream nutrient transformations"


//////////////////////////////////////////////////////////////////////////
/// Define unit names common used in SEIMS, in case of inconsistency /////
/// By LiangJun Zhu, HuiRan Gao ///
///Apr. , 2016  //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#define VAR_A_BNK "a_bnk"                           /// bank flow recession constant
#define VAR_ACC "acc"
#define VAR_ACC_INFIL "AccumuInfil"
#define VAR_ADDRNH4 "addrnh4"                       /// ammonium added by rainfall (kg/ha)
#define VAR_ADDRNO3 "addrno3"                       /// nitrate added by rainfall (kg/ha)
#define VAR_AET_PLT "AET_PLT"
#define VAR_AFERT_AMAXN "afert_AmaxN"
#define VAR_AFERT_FRTEFF "afert_frteff"
#define VAR_AFERT_FRTSURF "afert_frtsurf"
#define VAR_AFERT_ID "afert_id"
#define VAR_AFERT_MAXN "afert_maxN"
#define VAR_AFERT_NSTRS "afert_nstrs"
#define VAR_AFERT_NSTRSID "afert_nstrsID"
#define VAR_AFERT_NYLDT "afert_nyldTarg"
#define VAR_AI0 "ai0"
#define VAR_AI1 "ai1"
#define VAR_AI2 "ai2"
#define VAR_AI3 "ai3"
#define VAR_AI4 "ai4"
#define VAR_AI5 "ai5"
#define VAR_AI6 "ai6"
#define VAR_AIRR_EFF "airr_eff"
#define VAR_AIRR_LOCATION "airr_location"
#define VAR_AIRR_SOURCE "airr_source"
#define VAR_AIRRSURF_RATIO "airrsurf_ratio"
#define VAR_AIRRWTR_DEPTH "airrwtr_depth"
#define VAR_ALAIMIN "alai_min"
#define VAR_ALBDAY "ALBDAY"
#define VAR_CH_ALGAE "ch_algae"
#define VAR_CH_ALGAEConc "ch_algaeConc"
#define VAR_CH_ONCO "ch_onco"
#define VAR_CH_OPCO "ch_opco"
#define VAR_AMMO_CH "ammoToCh"
#define VAR_CH_NH4 "ch_nh4"
#define VAR_CH_NH4Conc "ch_nh4Conc"
#define VAR_ANION_EXCL "anion_excl"
#define VAR_AWTR_STRS_ID "awtr_strsID"
#define VAR_AWTR_STRS_TRIG "awtr_strsTrig"
#define VAR_B_BNK "b_bnk"                           /// bank storage loss coefficient
#define VAR_BACT_SWF "bact_swf"
#define VAR_Base_ex "Base_ex"                       /// base flow recession exponent
#define VAR_BIO_E "BIO_E"
#define VAR_BIOEHI "BIOEHI"
#define VAR_BIOINIT "BIO_INIT"
#define VAR_BIOLEAF "BIO_LEAF"
#define VAR_BIOMASS "BIOMASS"
#define VAR_BIOTARG "biotarg"
#define VAR_BKST "BKST"                             /// bank storage
#define VAR_BLAI "BLAI"
#define VAR_BMX_TREES "BMX_TREES"
#define VAR_BN1 "BN1"
#define VAR_BN2 "BN2"
#define VAR_BN3 "BN3"
#define VAR_BNK0 "Bnk0"                             /// initial bank storage per meter of reach length
#define VAR_BP1 "BP1"
#define VAR_BP2 "BP2"
#define VAR_BP3 "BP3"
#define VAR_C_RAIN "c_rain"
#define VAR_C_SNOW "c_snow"
#define VAR_C_SNOW12 "c_snow12"
#define VAR_C_SNOW6 "c_snow6"
#define VAR_C_WABA "C_WABA"                         /// Channel water balance in a text format for each reach and at each time step
#define VAR_CDN "cdn"                               /// rate coefficient for denitrification
#define VAR_CELL_LAT "celllat"            /// latitude of each valid cells
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
#define VAR_CHL_A "chl_a"
#define VAR_CHS0 "Chs0"                               /// initial channel storage per meter of reach length
#define VAR_CHS0_PERC "chs0_perc"
#define VAR_CHSB "CHSB"
#define VAR_CHST "CHST"                               /// channel storage
#define VAR_PRECHST "preCHST"                        /// channel storage at previous timestep
#define VAR_CHT "CHT" /// canopy height for the day(m)
#define VAR_CHTMX "CHTMX" /// maximum canopy height (m)
#define VAR_CHWTWIDTH "chwtwidth"
#define VAR_CHBTMWIDTH "chbtmwidth"
#define VAR_CHWIDTH "CHWIDTH"
#define VAR_CHWTDEPTH "CHWTDEPTH"                     /// channel water depth
#define VAR_PRECHWTDEPTH "prechwtdepth"
#define VAR_CLAY "CLAY"
#define VAR_CMN "cmn"                                 /// Rate coefficient for mineralization of the humus active organic nutrients
#define VAR_CN2 "CN2"
#define VAR_CO2 "Co2"                                 /// CO2 Concentration
#define VAR_CO2HI "CO2HI"
#define VAR_SUR_COD "sur_cod"
#define VAR_COD_N "cod_n"
#define VAR_COD_K "cod_k"
#define VAR_COND_MAX "Cond_max"                       /// "Maximum automata's conductance"
#define VAR_COND_RATE "Cond_rate"                     /// Rate of decline in automata's conductance per unit increase in vapor pressure deficit
#define VAR_CONDUCT "Conductivity"
#define VAR_CONV_WT "conv_wt"
#define VAR_CROP_LOOKUP "CropLookup"
#define VAR_CSWAT "cswat"
#define VAR_PCP "D_P" /// Distribution of precipitation
#define VAR_DAYLEN "daylength"
#define VAR_DAYLEN_MIN "daylenmin"  /// minimum day length
#define VAR_DEEPST "deepst"
#define VAR_DEET "DEET"                               /// evaporation from the depression storage
#define VAR_DEM "DEM"                                 /// Digital Elevation Model
#define VAR_DEPRATIO "depRatio"
#define VAR_DEPREIN "Depre_in"                        /// initial depression storage coefficient
#define VAR_DEPRESSION "Depression"                   /// Depression storage capacity
#define VAR_DETSPLASH "DETSplash"
#define VAR_DETACH_SAND "det_sand"
#define VAR_DETACH_SILT "det_silt"
#define VAR_DETACH_CLAY "det_clay"
#define VAR_DETACH_SAG "det_smagg"
#define VAR_DETACH_LAG "det_lgagg"
#define VAR_SANDYLD "sand_yld"
#define VAR_SILTYLD "silt_yld"
#define VAR_CLAYYLD "clay_yld"
#define VAR_SAGYLD "sag_yld"
#define VAR_LAGYLD "lag_yld"
#define VAR_DF_COEF "df_coef"                         /// Deep percolation coefficient
#define VAR_CH_SOLP "CH_SOLP"
#define VAR_CH_SOLPConc "CH_SOLPConc"
#define VAR_DLAI "DLAI"
#define VAR_DORMHR "dormhr"
#define VAR_DORMI "dormi"
#define VAR_DPST "DPST"                               /// depression storage
#define VAR_DRYDEP_NH4 "drydep_nh4"                 /// atmospheric dry deposition of ammonia (kg/ha)
#define VAR_DRYDEP_NO3 "drydep_no3"                 /// atmospheric dry deposition of nitrates (kg/ha)
#define VAR_EP_CH "Ep_ch"                           /// reach evaporation adjustment factor
#define VAR_EPCO "epco"                              /// plant water uptake compensation factor
#define VAR_ESCO "esco"
#define VAR_EVLAI "evlai"
#define VAR_POT_TILEMM "pot_tilemm"
#define VAR_POT_NO3DECAY "pot_no3l"
#define VAR_POT_SOLPDECAY "pot_solpl"
#define VAR_EXCP "EXCP"                             /// excess precipitation
#define VAR_EXT_COEF "EXT_COEF"
#define VAR_FERTILIZER_LOOKUP "FertilizerLookup"
#define VAR_FIELDCAP "FieldCap"                     /// Soil field capacity"
#define VAR_FLAT "flat"
#define VAR_FLOWDIR "FLOW_DIR"
#define VAR_FLOWWIDTH "FlowWidth"
#define VAR_FR_PHU_ACC "frPHUacc"
#define VAR_FR_PLANT_N "frPlantN"
#define VAR_FR_PLANT_P "frPlantP"
#define VAR_FR_ROOT "frRoot"
#define VAR_FR_STRSWTR "frStrsWtr"
#define VAR_FRGMAX "frgmax"
#define VAR_FRGRW1 "FRGRW1"
#define VAR_FRGRW2 "FRGRW2"
#define VAR_GRRE "GRRE"
#define VAR_GRZ_DAYS "grz_days"
#define VAR_GRZ_FLAG "grz_flag"
#define VAR_GSI "gsi"
#define VAR_GW_KG "kg"
#define VAR_GW_Q "GW"
#define VAR_GW0 "GW0"                               /// initial ground water storage
#define VAR_GWMAX "GWMAX"                           /// maximum ground water storage
#define VAR_GWSOLP_CONC "gwsolp_conc"
#define VAR_GWSOLP "gwsolp"
#define VAR_GWNEW "GWNEW"                           /// The volume of water from the bank storage to the adjacent unsaturated zone and groundwater storage
#define VAR_GWNO3_CONC "gwno3_conc"
#define VAR_GWNO3 "gwno3"
#define VAR_GWWB "GWWB"        // groundwater water balance
#define VAR_GWRQ "GWRQ"
#define VAR_HCH "HCH"
#define VAR_HITARG "hi_targ"
#define VAR_HMNTL "hmntl"                           /// amount of nitrogen moving from active organic to nitrate pool in soil profile on current day in cell(kg N/ha)
#define VAR_HMPTL "hmptl"                           /// amount of phosphorus moving from the organic to labile pool in soil profile on current day in cell(kg P/ha)
#define VAR_HVSTI "hvsti"   /// harvest index
#define VAR_HVSTI_ADJ "hvsti_adj"
#define VAR_HVSTI_TARG "hi_targ"
#define VAR_ID_OUTLET "ID_OUTLET"
#define VAR_IDC "IDC"             /// crop/landcover category, 1 to 7
#define VAR_IGRO "IGRO"
#define VAR_IGROPT "igropt"
#define VAR_IMPOUND_TRIG "impound_trig"
#define VAR_POT_VOLMAXMM "pot_volmaxmm"
#define VAR_POT_VOLLOWMM "pot_vollowmm"
#define VAR_INET "INET"                             /// evaporation from the interception storage
#define VAR_INFIL "INFIL"                           /// Infiltration
#define VAR_INFILCAPSURPLUS "INFILCAPSURPLUS"
#define VAR_INIT_IS "Init_IS"
#define VAR_INLO "INLO"
#define VAR_CANSTOR "canstor"
#define VAR_INTERC_MAX "Interc_max"                 /// Maximum Interception Storage Capacity
#define VAR_INTERC_MIN "Interc_min"                 /// Minimum Interception Storage Capacity
#define VAR_IRR_FLAG "irr_flag"
#define VAR_IRR_SURFQ "irr_surfq"
#define VAR_IRR_WTR "irr_water"
#define VAR_ISEP_OPT "isep_opt"
#define VAR_JULIAN_DAY "JDay"                       /// Julian day (int)
//#define VAR_K_BANK "K_bank"                         /// hydraulic conductivity of the channel bank
#define VAR_K_BLOW "K_blow"
//#define VAR_K_CHB "K_chb"                           /// hydraulic conductivity of the channel bed
#define VAR_K_L "k_l"
#define VAR_K_N "k_n"
#define VAR_K_P "k_p"
#define VAR_K_PET "K_pet"                           /// Correction factor for PET
#define VAR_K_RUN "K_run"
#define VAR_K_SOIL10 "k_soil10"
#define VAR_K_SUBLI "K_subli"
#define VAR_KG "Kg"                                 /// Baseflow recession coefficient
#define VAR_KI "Ki"
#define VAR_LAG_SNOW "lag_snow"
#define VAR_LAIDAY "LAIDAY"
#define VAR_LAIINIT "LAI_INIT" /// initial LAI at the beginning of the simulation
#define VAR_LAIMAXFR "laimaxfr"
#define VAR_LAIMX1 "LAIMX1"
#define VAR_LAIMX2 "LAIMX2"
#define VAR_LAIPRE "LAIPRE"
#define VAR_LAIYRMAX "laiyrmax"
#define VAR_LAMBDA0 "lambda0"
#define VAR_LAMBDA1 "lambda1"
#define VAR_LAMBDA2 "lambda2"
#define VAR_LANDCOVER "landcover"
#define VAR_LANDUSE "landuse"
#define VAR_LANDUSE_LOOKUP "LanduseLookup"
#define VAR_LAP_RATE "LapseRate"                    /// Lapse rate
#define VAR_LAST_SOILRD "lastSoilRD"
#define VAR_LATNO3 "latno3"
#define VAR_LATNO3_TOCH "latno3ToCh"
#define VAR_LCC "landcover"                             /// land cover code, idplt in SWAT
#define VAR_LDRAIN "ldrain"
#define VAR_KV_PADDY "kv_paddy"
#define VAR_KN_PADDY "kn_paddy"
#define VAR_POT_K "pot_k"
#define VAR_MANNING "Manning"
#define VAR_MAT_YRS "MAT_YRS"
//#define VAR_MGT_FIELD "mgt_fields" // remove by lj, 08/16/17
#define VAR_MINPGW_TOCH "minpgwToCh"
#define VAR_MOIST_IN "Moist_in"
#define VAR_MSF "ManningScaleFactor"                /// flow velocity scaling factor for calibration
#define VAR_MSK_CO1 "MSK_co1"                       /// Weighting factor of bankful flow
#define VAR_MSK_X "MSK_X"                           /// muskingum weighing factor
#define VAR_MUMAX "mumax"
#define VAR_NACTFR "nactfr"                         /// nitrogen active pool fraction. The fraction of organic nitrogen in the active pool.
#define VAR_NEPR "NEPR"
#define VAR_NFIXCO "nfixco"                     /// Nitrogen fixation coefficient, FIXCO in SWAT
#define VAR_NFIXMX "nfixmx"                    /// Maximum daily-n fixation (kg/ha), NFIXMX in SWAT
#define VAR_CH_NO3 "CH_NO3"
#define VAR_CH_NO3Conc "CH_NO3Conc"
#define VAR_NO2_TOCH "nitriteToCh"
#define VAR_CH_NO2 "CH_NO2"
#define VAR_CH_NO2Conc "CH_NO2Conc"
#define VAR_DISTSTREAM "dist2stream"
#define VAR_NO3GW "no3gw"
#define VAR_NO3GW_TOCH "no3gwToCh"
#define VAR_NPERCO "nperco"
#define VAR_NUPDIS "n_updis"
#define VAR_OL_DET "DETOverland"
#define VAR_OL_IUH "Ol_iuh"                         /// IUH of each grid cell
#define VAR_OL_SED_CCOE "ccoe"
#define VAR_OL_SED_ECO1 "eco1"
#define VAR_OL_SED_ECO2 "eco2"
#define VAR_OLAI "olai"
#define VAR_OMEGA "Omega"
#define VAR_OMP_THREADNUM "ThreadNum"               /// Thread numbers for OMP
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
#define VAR_OUTLETID "OUTLET_ID"
#define VAR_P_MAX "P_max"
#define VAR_P_N "p_n"
#define VAR_P_RF "p_rf"
#define VAR_PERCO_N_GW "perco_n_gw"
#define VAR_PERCO_P_GW "perco_p_gw"
#define VAR_PERCO "Perco"                     /// the amount of water percolated from the soil water reservoir
#define VAR_PERDE "perde"
#define VAR_PET "PET"                           /// Potential Evapotranspiration of day
#define VAR_PET_HCOEF "HCoef_pet"                   /// Coefficient related to radiation used in Hargreaves method
#define VAR_PHOSKD "phoskd"
#define VAR_PHUBASE "PHUBASE"
#define VAR_PHUPLT "PHU_PLT"
#define VAR_PHUTOT "PHU0"
#define VAR_PI_B "Pi_b"
#define VAR_PCP2CANFR_PR "pcp2canfr_pr"
#define VAR_EMBNKFR_PR "embnkfr_pr"
#define VAR_PL_RSDCO "rsdco_pl"                     /// Plant residue decomposition coefficient
#define VAR_PLANT_N "plant_N"
#define VAR_PLANT_P "plant_P"
#define VAR_PLTET_TOT "plt_et_tot"
#define VAR_PLTPET_TOT "plt_pet_tot"
#define VAR_POREIDX "Poreindex"                      /// pore size distribution index
#define VAR_POROST "Porosity"                       /// soil porosity
#define VAR_POT_NO3 "pot_no3"
#define VAR_POT_NH4 "pot_nh4"
#define VAR_POT_ORGN "pot_orgn"
#define VAR_POT_SOLP "pot_solp"
#define VAR_POT_ORGP "pot_orgp"
#define VAR_POT_AMINP "pot_aminp"
#define VAR_POT_SMINP "pot_sminp"
#define VAR_POT_SED "pot_sed"
#define VAR_POT_VOL "pot_vol"
#define VAR_POT_SA "pot_sa"
#define VAR_POT_FLOWIN "pot_flowin"
#define VAR_POT_FLOWOUT "pot_flowout"
#define VAR_POT_SEDIN "pot_sedin"
#define VAR_POT_SEDOUT "pot_sedout"
#define VAR_PPERCO "pperco"
#define VAR_PPT "PPT"
#define VAR_PSP "psp"                               /// Phosphorus availability index
#define VAR_SSP "ssp"
#define VAR_PTTN2CH "ptTNToCh"
#define VAR_PTTP2CH "ptTPToCh"
#define VAR_PTCOD2CH "ptCODToCh"
#define VAR_PUPDIS "p_updis"
#define VAR_QCH "QCH"
#define VAR_OLFLOW "OL_Flow"                        /// overland flow in each cell calculated during overland routing
#define VAR_QG "QG"                                 /// Groundwater discharge at each reach outlet and at each time step
#define VAR_QI "QI"                                 /// Interflow at each reach outlet and at each time step
#define VAR_QOUTLET "QOUTLET"                       /// discharge at the watershed outlet
#define VAR_QOVERLAND "QOverland"
#define VAR_QRECH "QRECH"                           /// Discharge at reach outlet of each time step
#define VAR_QS "QS"                                 /// Overland discharge at each reach outlet and at each time step
#define VAR_QSOIL "QSoil"
#define VAR_QSOUTLET "QSOUTLET"                     /// discharge at the watershed outlet
#define VAR_QSUBBASIN "QSUBBASIN"
#define VAR_QTILE "qtile"
#define VAR_QTOTAL "QTotal"
#define VAR_QUPREACH "QUPREACH"                     /// upreach
#define VAR_RadianSlope "RadianSlope"
#define VAR_RCA "rca"                               /// concentration of ammonia in the rain (mg N/m3)  L -> 0.001 * m3
#define VAR_CH_COD "CH_COD"
#define VAR_CH_CODConc "CH_CODConc"
#define VAR_CH_DOX "ch_dox"
#define VAR_CH_DOXConc "ch_doxConc"
#define VAR_RCH_BANKERO "rch_bank_ero"
#define VAR_RCH_DEG "rch_deg"
#define VAR_RCH_DEP "rch_dep"
#define VAR_FLPLAIN_DEP "flplain_dep"
#define VAR_RCN "rcn"                               /// concentration of nitrate in the rain (mg N/m3)  L -> 0.001 * m3
#define VAR_Reinfiltration "Reinfiltration"
#define VAR_RETURNFLOW "ReturnFlow"
#define VAR_REVAP "Revap"
#define VAR_RG "RG"
#define VAR_RHOQ "rhoq"
#define VAR_RMN2TL "rmn2tl"                         /// amount of nitrogen moving from the fresh organic (residue) to the nitrate(80%) and active organic(20%) pools in soil profile on current day in cell(kg N/ha)
#define VAR_RMP1TL "rmp1tl"                         /// amount of phosphorus moving from the labile mineral pool to the active mineral pool in the soil profile on the current day in cell
#define VAR_RMPTL "rmptl"                           /// amount of phosphorus moving from the fresh organic (residue) to the labile(80%) and organic(20%) pools in soil profile on current day in cell(kg P/ha)
#define VAR_RNUM1 "rnum1"
#define VAR_ROCK "rock"
#define VAR_ROCTL "roctl"                           /// amount of phosphorus moving from the active mineral pool to the stable mineral pool in the soil profile on the current day in cell
#define VAR_ROOTDEPTH "rootdepth"      /// Maximum root depth of plant/land cover (mm)
#define VAR_RUNOFF_CO "Runoff_co"
#define VAR_RWNTL "rwntl"                           /// amount of nitrogen moving from active organic to stable organic pool in soil profile on current day in cell(kg N/ha)
#define VAR_S_FROZEN "s_frozen"
#define VAR_SAND "sand"
#define VAR_SBGS "SBGS"                             /// Groundwater storage of the subbasin
#define VAR_SBIF "SBIF"                             /// interflow to streams from each subbasin
#define VAR_SBOF "SBOF"                             /// overland flow to streams from each subbasin
#define VAR_SBPET "SBPET"                           /// the potential evapotranspiration rate of the subbasin
#define VAR_SBQG "SBQG"                             /// groundwater flow out of the subbasin
#define VAR_SCENARIO "SCENARIO"
#define VAR_SDNCO "sdnco"
#define VAR_SED_DEP "SEDDEP"
#define VAR_SED_FLOW "sed_flow"
#define VAR_SED_FLUX "sed_flux"
#define VAR_SED_OUTLET "SEDOUTLET"
#define VAR_SED_RECH "SEDRECH"
#define VAR_SED_RECHConc "SEDRECHConc"
#define VAR_SED_TO_CH "SEDTOCH"
#define VAR_SEDYLD "SED_OL"
#define VAR_SED_CHI0 "sed_chi"
#define VAR_SEDMINPA "sedminpa"                     /// amount of active mineral phosphorus adsorbed to sediment in surface runoff
#define VAR_SEDMINPA_TOCH "sedminpaToCh"
#define VAR_SEDMINPS "sedminps"                     /// amount of stable mineral phosphorus adsorbed to sediment in surface runoff
#define VAR_SEDMINPS_TOCH "sedminpsToCh"
#define VAR_SEDORGN "sedorgn"
#define VAR_SEDORGN_TOCH "sedorgnToCh"
#define VAR_SEDORGP "sedorgp"                       /// amount of organic phosphorus in surface runoff
#define VAR_SEDORGP_TOCH "sedorgpToCh"
#define VAR_SEEPAGE "SEEPAGE"                               /// seepage
#define VAR_SHALLST "shallst"
#define VAR_SILT "silt"
#define VAR_SLOPE "slope"
#define VAR_SNAC "SNAC"
#define VAR_SNME "SNME"
#define VAR_SNO3UP "sno3up"
#define VAR_SNOCOVMX "SNOCOVMX"
#define VAR_SNO50COV "SNO50COV"
#define VAR_SNRD "SNRD"
#define VAR_SNSB "SNSB"
#define VAR_SNWB "SNWB"
#define VAR_SOER "SOER"                             /// soil loss caused by water erosion
#define VAR_SOET "SOET"                             /// evaporation from the soil water storage, es_day in SWAT
#define VAR_SOIL_T10 "soil_t10"
#define VAR_SOILDEPTH "soilDepth"                       /// depth to bottom of soil layer
#define VAR_SOILLAYERS "soillayers"
#define VAR_SOILTHICK "soilthick"
#define VAR_SOL_ACTP "sol_actp"                     /// amount of phosphorus stored in the active mineral phosphorus pool(kg P/ha)
#define VAR_SOL_ALB "sol_alb"               /// albedo when soil is moist
#define VAR_SOL_AORGN "sol_aorgn"                   /// amount of nitrogen stored in the active organic (humic) nitrogen pool(kg N/ha)
#define VAR_SOL_AWC "sol_awc"            /// amount of water available to plants in soil layer at field capacity (FC-WP)
#define VAR_SOL_BD "density"                         /// bulk density of the soil (mg/m3)
#define VAR_SOL_CBN "sol_cbn"
#define VAR_SOL_COV "sol_cov"                    /// amount of residue on soil surface (kg/ha)
#define VAR_SOL_CRK "sol_crk"                    /// crack volume potential of soil
#define VAR_SOL_FORGN "sol_fon"                       /// amount of nitrogen stored in the fresh organic (residue) pool(kg N/ha)
#define VAR_SOL_FORGP "sol_fop"                       /// amount of phosphorus stored in the fresh organic (residue) pool(kg P/ha)
#define VAR_SOL_MC "sol_mc"
#define VAR_SOL_MN "sol_mn"
#define VAR_SOL_MP "sol_mp"
#define VAR_SOL_N "sol_N"

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

#define VAR_SOL_LATERAL_C "sol_latc"
#define VAR_SOL_PERCO_C "sol_percoc"
#define VAR_LATERAL_C "latc"
#define VAR_PERCO_C "percoc"
#define VAR_SEDLOSS_C "sedc"

#define VAR_SOL_NH4 "sol_nh4"                       /// amount of nitrogen stored in the ammonium pool in soil layer
#define VAR_SOL_NO3 "sol_no3"                       /// amount of nitrogen stored in the nitrate pool in soil layer(kg N/ha)
#define VAR_SOL_OM "om"
#define VAR_SOL_SORGN "sol_orgn"                     /// amount of nitrogen stored in the stable organic N pool(kg N/ha)
#define VAR_SOL_HORGP "sol_orgp"                     /// amount of phosphorus stored in the humic organic P pool in soil layer(kg P/ha)
#define VAR_SOL_PERCO "sol_perco"
#define VAR_SOL_RSD "sol_rsd"                       /// amount of organic matter in the soil classified as residue(kg/ha)
#define VAR_SOL_RSDIN "rsdin"                       /// amount of organic matter in the soil classified as residue(kg/ha)
#define VAR_SOL_SOLP "sol_solp"                     /// amount of phosphorus stored in solution(kg P/ha)
#define VAR_SOL_STAP "sol_stap"                     /// amount of phosphorus in the soil layer stored in the stable mineral phosphorus pool(kg P/ha)
#define VAR_SOL_SUMAWC "sol_sumAWC"  /// mm H2O sol_sumfc in SWAT
#define VAR_SOL_SUMSAT "sol_sumul"  /// mm H2O  sol_sumul in SWAT
#define VAR_SOL_TA0 "soil_ta0"
#define VAR_SOL_TA1 "soil_ta1"
#define VAR_SOL_TA2 "soil_ta2"
#define VAR_SOL_TA3 "soil_ta3"
#define VAR_SOL_TB1 "soil_tb1"
#define VAR_SOL_TB2 "soil_tb2"
#define VAR_SOL_TD1 "soil_td1"
#define VAR_SOL_TD2 "soil_td2"
#define VAR_SOL_TMP "sol_tmp"                       /// daily average temperature of soil layer(deg C)
#define VAR_SOL_UL "sol_ul"                       /// mm H2O
#define VAR_SOL_WPMM "sol_wpmm"                     /// water content of soil at -1.5 MPa (wilting point)
#define VAR_SOL_ZMX "SOL_ZMX"          /// Maximum rooting depth of soil profile (mm)
//#define VAR_SOMO "SOMO"                     /// soil moisture, deprecated and replaced by VAR_SOL_ST
//#define VAR_SOMO_TOT  "somo_total"          /// Total soil water content in soil profile, deprecated and replaced by VAR_SOL_SW
#define VAR_SOL_ST "solst"                     /// amount of water stored in the soil layer on current day(mm H2O)
#define VAR_SOL_SW  "solsw"          /// amount of water stored in soil profile on current day (mm H2O)
#define VAR_SOTE "SOTE"                             /// Soil Temperature
#define VAR_SOWB "SOWB"
#define VAR_SOXY "soxy"
#define VAR_SOXYConc "soxyConc"
#define VAR_SPCON "spcon"
#define VAR_SPEXP "spexp"
#define VAR_SR_MAX "srMax"                          /// Max solar radiation
#define VAR_SRA "sra"
#define VAR_SSRU "SSRU"                             /// The subsurface runoff
#define VAR_SSRUVOL "SSRUVOL"
#define VAR_STCAPSURPLUS "STCAPSURPLUS"
#define VAR_STREAM_LINK "STREAM_LINK"
#define VAR_SUB_SEDTOCH ""
#define VAR_SUBBSN "subbasin"                       /// The subbasin grid
#define VAR_SUBBSNID_NUM "SUBBASINID_NUM"                /// number of subbasins
#define VAR_SUR_NO3 "sur_no3"
#define VAR_SUR_NO3_TOCH "sur_no3_ToCh"
#define VAR_SUR_NH4 "sur_nh4"
#define VAR_SUR_NH4_TOCH "SUR_NH4_TOCH"
#define VAR_SUR_SOLP "sur_solp"
#define VAR_SUR_SOLP_TOCH "sur_solp_ToCh"
#define VAR_SUR_COD_TOCH "sur_cod_ToCH"
#define VAR_SURU "SURU"                             /// surface runoff
#define VAR_SWE "SWE"
#define VAR_SWE0 "swe0"
#define VAR_T_BASE "T_BASE"
#define VAR_T_OPT "T_OPT"
#define VAR_T_RG "T_RG"   /// groundwater runoff
#define VAR_T_SNOW "T_snow"                /// Snowfall temperature
#define VAR_T_SOIL "t_soil"                         /// threshold soil freezing temperature
#define VAR_T0 "T0"
#define VAR_TFACT "tfact"
#define VAR_TILLAGE_LOOKUP "TillageLookup"
#define VAR_TILLAGE_DAYS "tillage_days"
#define VAR_TILLAGE_DEPTH "tillage_depth"
#define VAR_TILLAGE_FACTOR "tillage_factor"
#define VAR_TILLAGE_SWITCH "tillage_switch"
#define VAR_TMAX "TMAX"
#define VAR_TMEAN "TMEAN"
#define VAR_TMEAN_ANN "TMEAN0"  /// annual mean temperature
#define VAR_TMEAN1 "TMEAN1"
#define VAR_TMEAN2 "TMEAN2"
#define VAR_TMIN "TMIN"
#define VAR_TREEYRS "CURYR_INIT"
#define VAR_TSD_DT "DATATYPE"                      /// Time series data type
#define VAR_USLE_C "USLE_C"
#define VAR_USLE_K "USLE_K"
#define VAR_USLE_LS "USLE_LS"
#define VAR_USLE_P "USLE_P"
#define VAR_VCD "vcd"
#define VAR_VCRIT "vcrit"
#define VAR_VDIV "Vdiv"                             /// diversion loss of the river reach
#define VAR_VP_ACT "avp"                            /// actual vapor pressure
#define VAR_VP_SAT "svp"                            /// Saturated vapor pressure
#define VAR_VPD "VPD"                               /// vapor pressure deficit
#define VAR_VPDFR "vpdfr"
#define VAR_VSEEP0 "Vseep0"                         ///  the initial volume of transmission loss to the deep aquifer over the time interval
#define VAR_WATTEMP "wattemp"
#define VAR_WAVP "WAVP"
#define VAR_WDNTL "wdntl"                           /// amount of nitrogen lost from nitrate pool by denitrification in soil profile on current day in cell(kg N/ha)
#define VAR_WILTPOINT "WiltingPoint"
#define VAR_WS "WS"
#define VAR_WSHD_DNIT "wshd_dnit"                   ///  nitrogen lost from nitrate pool due to denitrification in watershed(kg N/ha)
#define VAR_WSHD_HMN "wshd_hmn"                     ///  nitrogen moving from active organic to nitrate pool in watershed(kg N/ha)
#define VAR_WSHD_HMP "wshd_hmp"                     ///  phosphorus moving from organic to labile pool in watershed(kg P/ha)
#define VAR_WSHD_NITN "wshd_nitn"                   ///  nitrogen moving from the NH4 to the NO3 pool by nitrification in the watershed
#define VAR_WSHD_PAL "wshd_pal"                     ///  phosphorus moving from labile mineral to active mineral pool in watershed
#define VAR_WSHD_PAS "wshd_pas"                     ///  phosphorus moving from active mineral to stable mineral pool in watershed
#define VAR_WSHD_PLCH "wshd_plch"
#define VAR_WSHD_RMN "wshd_rmn"                     ///  nitrogen moving from fresh organic (residue) to nitrate and active organic pools in watershed(kg N/ha)
#define VAR_WSHD_RMP "wshd_rmp"                     ///  phosphorus moving from fresh organic (residue) to labile and organic pools in watershed(kg P/ha)
#define VAR_WSHD_RNO3 "wshd_rno3"                   ///  NO3 added to soil by rainfall in watershed (kg/ha)
#define VAR_WSHD_RWN "wshd_rwn"                     /// nitrogen moving from active organic to stable organic pool in watershed(kg N/ha)
#define VAR_WSHD_VOLN "wshd_voln"                   /// nitrogen lost by ammonia volatilization in watershed
#define VAR_WSYF "wsyf"
#define VAR_AL_OUTLET "algae_outlet"
#define VAR_ON_OUTLET "organicn_outlet"
#define VAR_AN_OUTLET "ammonian_outlet"
#define VAR_NIN_OUTLET "nitriten_outlet"
#define VAR_NAN_OUTLET "nitraten_outlet"
#define VAR_OP_OUTLET "organicp_outlet"
#define VAR_DP_OUTLET "disolvp_outlet"
#define VAR_COD_OUTLET "cod_outlet"
#define VAR_CHL_OUTLET "chlora_outlet"

#define VAR_A_DAYS "a_days"
#define VAR_B_DAYS "b_days"

//////////////////////////////////////////////////////////////////////////
/// Define units common used in SEIMS, in case of inconsistency //////////
/// By LiangJun Zhu, HuiRan Gao ///
/// Apr. , 2016  //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#define UNIT_AREA_HA "ha"
#define UNIT_AREA_KM2 "km2"                         /// Square kilometer of area
#define UNIT_AREA_RATIO "m2/m2"
#define UNIT_CONDRATE_MSPA "m/s/kPa"                /// Rate of decline in stomatal conductance per unit increase in vapor pressure deficit
#define UNIT_CONT_KGHA "kg/ha"                      /// For convenient, keep consistent with SWAT, need Conversion later.
#define UNIT_CONT_KGKM2 "kg/km2"                    /// Kilograms per Square kilometers of nutrient content
#define UNIT_CONT_RATIO "(kg/ha)/(kg/ha)"
#define UNIT_CONT_TONHA "tons/ha"
#define UNIT_DENSITY "Mg/m3"                        /// density, equal to g/cm3, Mg/m3, ton/m3
#define UNIT_SEDCONC "g/L"                         /// i.e., kg/m3
#define UNIT_CONCENTRATION "mg/L"                        /// concentration, or mg/kg
#define UNIT_DEPTH_MM "mm"                          /// Depth related unit, mm
#define UNIT_FLOW_CMS "m3/s"                        /// Cubic meters per second of flow discharge
#define UNIT_GAS_CON "uL/L"   /// e.g., uL CO2/L air, IS this same with ppmv? LJ
#define UNIT_GAS_PPMV "ppmv"                        /// Concentration of gas, e.g., CO2
#define UNIT_HEAT_UNIT "hr"
#define UNIT_KG "kg"                                /// mass Kg
#define UNIT_TONS "t"                               /// metric tons
#define UNIT_KG_S "kg/s"
#define UNIT_KGM3 "kg/m3"
#define UNIT_LAP_RATE "/100m"                       /// Lapse rate
#define UNIT_LEN_M "m"                              /// Meter of length
#define UNIT_LONLAT_DEG "degree"                    /// Degree of longitude and latitude
#define UNIT_MELT_FACTOR "mm/deg C/day"                 /// Melt factor
#define UNIT_NON_DIM ""                             /// Non dimension
#define UNIT_NUTR_RATIO "mg/mg"         /// mg H2O/mg Nutrient
#define UNIT_PER_DAY "1/day"               /// rate per day
#define UNIT_PERCENT "%"                            /// Percent
#define UNIT_PRESSURE "kPa"                         /// Vapor pressure
#define UNIT_RAD_USE_EFFI "(kg/ha)/(MJ/m2)"
#define UNIT_SPEED_MS "m/s"                         /// Speed related
#define UNIT_SR "MJ/m2/d"                           /// Solar Radiation
#define UNIT_STRG_M3M "m3/m"                       /// storage per meter of reach length
#define UNIT_TEMP_DEG "deg C"                       /// Celsius degree of air temperature
#define UNIT_TEMP_FACTOR "mm/deg C"                 /// temperature factor
#define UNIT_YEAR "yr"
#define UNIT_DAY "day"                    /// Time step (day)
#define UNIT_HOUR "hr"                     /// Time step (h)
#define UNIT_SECOND "sec"                      /// Time step (sec)
#define UNIT_VOL_FRA_M3M3 "m3/m3"
#define UNIT_VOL_M3 "m3"                           /// volume
#define UNIT_WAT_RATIO "mm/mm"         /// mm H2O/mm Soil
#define UNIT_WTRDLT_MMD "mm/d"                      /// Millimeter per day of water changes
#define UNIT_WTRDLT_MMH "mm/h"                      /// Millimeter per hour of water changes




//////////////////////////////////////////////////////////////////////////
/// Define description of units common used in SEIMS            //////////
/// By LiangJun Zhu, HuiRan Gao //////////////////////////
///               Apr. 25, 2016  //////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
#define DESC_A_BNK "bank flow recession constant"
#define DESC_ACC "the number of flow accumulation cells of each cell"
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
#define DESC_CHWTDEPTH "channel water depth"
#define DESC_PRECHWTDEPTH "channel water depth of previous timestep"
#define DESC_CLAY "Percent of clay content"
#define DESC_CMN "Rate coefficient for mineralization of the humus active organic nutrients"
#define DESC_CN2 "CN under moisture condition II"
#define DESC_CO2 "CO2 Concentration"
#define DESC_CO2HI "elevated CO2 atmospheric concentration corresponding the 2nd point on the radiation use efficiency curve"
#define DESC_SUR_COD "carbonaceous oxygen demand of surface runoff"
#define DESC_COD_CH "carbonaceous oxygen demand loading to reach"
#define DESC_COD_N "Conversion factor"
#define DESC_COD_K "Reaction coefficient"
#define DESC_CONDRATE "Rate of decline in stomatal conductance per unit increase in vapor pressure deficit"
#define DESC_CONDUCT "saturation hydraulic conductivity"
#define DESC_CONV_WT "factor which converts kg/kg soil to kg/ha"
#define DESC_CROP_LOOKUP "Crop database"
#define DESC_CSWAT "carbon modeling method"
#define DESC_PCP "Precipitation of each time step on current cell"
#define DESC_DAYLEN "day length"
#define DESC_DAYLEN_MIN  "minimum day length"
#define DESC_DEEPST "depth of water in deep aquifer"
#define DESC_DEET "evaporation from depression storage"
#define DESC_DEM "Digital Elevation Model"
#define DESC_DEPRATIO "Deposition ratio of sediment"
#define DESC_DEPREIN "initial depression storage coefficient"
#define DESC_DEPRESSION "Depression storage capacity"
#define DESC_DETSPLASH "distribution of splash detachment"
#define DESC_DETACH_SAND "sand fraction of detached sediment"
#define DESC_DETACH_SILT "silt fraction of detached sediment"
#define DESC_DETACH_CLAY "clay fraction of detached sediment"
#define DESC_DETACH_SAG "small aggeregate fraction of detached sediment"
#define DESC_DETACH_LAG "large aggregate fraction of detached sediment"
#define DESC_SANDYLD "sand yield amout"
#define DESC_SILTYLD "silt yield amout"
#define DESC_CLAYYLD "clay yield amout"
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
#define DESC_FERTILIZER_LOOKUP "Fertilizer database"
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
#define DESC_HVSTI_ADJ "harvest index adjusted for water stress"
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
//#define DESC_K_BANK "hydraulic conductivity of the channel bank"
#define DESC_K_BLOW "fraction coefficient of precipitation as snow"
//#define DESC_K_CHB "hydraulic conductivity of the channel bed"
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
#define DESC_LAP_RATE "Lapse Rate"
#define DESC_LAST_SOILRD "storing last soil root depth for use in harvestkillop/killop"
#define DESC_LATNO3 "amount of nitrate transported with lateral flow"
#define DESC_LATNO3_CH "amount of nitrate transported with lateral flow to channel"
#define DESC_LCC "land cover code"
#define DESC_LDRAIN "soil layer where drainage tile is located"
#define DESC_KV_PADDY "volatilization rate constant in impounded water body"
#define DESC_POT_K "hydraulic conductivity of soil surface of pothole"
#define DESC_KN_PADDY "nitrification rate constant in impounded water body"
#define DESC_MANNING "Manning's roughness"
#define DESC_MASK "Array containing the row and column numbers for valid cells"
#define DESC_MAT_YRS "the number of years for the tree species to reach full development"
#define DESC_MAXCOND "Maximum stomatal conductance"
#define DESC_MAXTEMP "Maximum Celsius degree of air temperature"
#define DESC_MEANTEMP "Mean Celsius degree of air temperature"
#define DESC_METEOLAT "Latitude of MeteoClimate station"
//#define DESC_MGT_FIELD "Management fields" // remove by lj, 08/16/17
#define DESC_MINPGW_CH "soluble P in groundwater to channel"
#define DESC_MINTEMP "Minimum Celsius degree of air temperature"
#define DESC_MOIST_IN "Initial soil moisture"
#define DESC_MSF "flow velocity scaling factor for calibration"
#define DESC_MSK_CO1 "Weighting factor of bankful flow"
#define DESC_MSK_X "muskingum weighing factor"
#define DESC_MUMAX "maximum specific algal growth rate at 20 deg C"
#define DESC_NACTFR "nitrogen active pool fraction. The fraction of organic nitrogen in the active pool."
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
#define DESC_P_MAX "Maximum P corresponding to runoffCo"
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
#define DESC_SSP "Phosphorus availability index"
#define DESC_PTTN2CH "total nitrogen loaded from point sources"
#define DESC_PTTP2CH "total phosphrus loaded from point sources"
#define DESC_PTCOD2CH "total COD loaded from point sources"
#define DESC_PUPDIS "Phosphorus uptake distribution parameter"
#define DESC_QCH "Flux in the downslope boundary of cells"
#define DESC_OLFLOW "overland flow in each cell calculated during overland routing"
#define DESC_QG "Groundwater discharge at each reach outlet"
#define DESC_QI "Interflow at each reach outlet"
#define DESC_QOUTLET "discharge at the watershed outlet"
#define DESC_QOVERLAND "Water discharge in the downslope boundary of cells"
#define DESC_QRECH "Discharge at each reach outlet of each time step"
#define DESC_QS "Overland discharge at each reach outlet"
#define DESC_QSOIL "discharge added to channel flow from interflow"
#define DESC_QSOUTLET "surface runoff at the watershed outlet"
#define DESC_QSUBBASIN "discharge at each subbasin outlet"
#define DESC_QTILE "drainage tile flow in soil profile"
#define DESC_QTOTAL "discharge at the watershed outlet"
#define DESC_QUPREACH "Upreach"
#define DESC_RadianSlope  "radian slope"
#define DESC_RCA "concentration of ammonia in the rain"
#define DESC_CH_COD "carbonaceous oxygen demand in reach"
#define DESC_CH_DOX "dissolved oxygen in reach"
#define DESC_RCH_BANKERO "bank erosion"
#define DESC_RCH_DEG "reach degradation"
#define DESC_RCH_DEP "reach deposition"
#define DESC_FLPLAIN_DEP "Floodplain Deposition"
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
#define DESC_SED_OUTLET "Sediment concentration at the watershed outlet"
#define DESC_SED_RECH "Sediment at each reach outlet at each time step"
#define DESC_SED_TO_CH "sediment flowing to channel"
#define DESC_SEDYLD "sediment yield that transported to channel at each cell"
#define DESC_SEDMINPA " amount of active mineral phosphorus sorbed to sediment in surface runoff"
#define DESC_SEDMINPA_CH "amount of active mineral phosphorus absorbed to sediment in surface runoff moved to channel"
#define DESC_SEDMINPS "amount of stable mineral phosphorus sorbed to sediment in surface runoff"
#define DESC_SEDMINPS_CH "amount of stable mineral phosphorus absorbed to sediment in surface runoff moved to channel"
#define DESC_SEDORGN "amount of organic nitrogen in surface runoff"
#define DESC_SEDORGN_CH "amount of organic nitrogen in surface runoff moved to channel"
#define DESC_SEDORGP "amount of organic phosphorus in surface runoff"
#define DESC_SEDORGP_CH "amount of organic phosphorus in surface runoff moved to channel"
#define DESC_SED_CHI0 "Initial channel sediment concentration"
#define DESC_SEEPAGE "seepage"
#define DESC_SHALLST "depth of water in shallow aquifer"
#define DESC_SILT "Percent of silt content"
#define DESC_SLOPE "Slope gradient (drop/distance, i.e., tan, or percent)"
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
#define DESC_SOILDEPTH "depth to bottom of soil layer"
#define DESC_SOILLAYERS "Soil layers number"
#define DESC_SOILTHICK "soil thick of each layer"
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
#define DESC_SEDLOSS_C "amount of C lost with sediment"

#define DESC_SOL_NH4 "amount of nitrogen stored in the ammonium pool in soil layer"
#define DESC_SOL_NO3 "amount of nitrogen stored in the nitrate pool in soil layer"
#define DESC_SOL_OM "percent of organic matter in soil"
#define DESC_SOL_SORGN "amount of nitrogen stored in the stable organic N pool"
#define DESC_SOL_HORGP "amount of phosphorus stored in the humic organic P pool in soil layer"
#define DESC_SOL_PERCO "percolation from soil layer"
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
//#define DESC_SOMO "soil moisture in soil layers"
//#define DESC_SOMO_TOT "amount of water stored in the soil profile"
#define DESC_SOL_ST "amount of water stored in the soil layer on current day(mm H2O)"
#define DESC_SOL_SW "amount of water stored in soil profile on current day (mm H2O)"
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
#define DESC_SUR_NH4_ToCH "amount of ammonian transported with surface runoff to channel"
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
#define DESC_TILLAGE_LOOKUP "Tillage database"
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
#define DESC_USLE_C "the cover management factor"
#define DESC_USLE_K "The soil erodibility factor used in USLE"
#define DESC_USLE_LS "USLE LS factor"
#define DESC_USLE_P "the erosion control practice factor"
#define DESC_VCD "compute changes in channel dimensions"
#define DESC_VCRIT "critical velocity for sediment deposition"
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
#define DESC_B_DAYS "days since P deficit"

//////////////////////////////////////////////////////////////////////////
/// Define MongoDB related constant strings used in SEIMS and preprocess//
/// By LiangJun Zhu, May. 4, 2016  ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//#define MONG_GRIDFS_FN                      "filename"  // alread defined in MongoUtil.h
//#define MONG_GRIDFS_WEIGHT_CELLS            "NUM_CELLS"
//#define MONG_GRIDFS_WEIGHT_SITES            "NUM_SITES"
//#define MONG_GRIDFS_ID                      "ID"
//#define MONG_GRIDFS_SUBBSN                  "SUBBASIN"
#define MONG_HYDRO_SITE_TYPE                   "TYPE"
#define MONG_HYDRO_SITE_LAT                    "LAT"
#define MONG_HYDRO_SITE_ELEV                   "ELEVATION"
#define MONG_HYDRO_DATA_SITEID                 "STATIONID"
#define MONG_HYDRO_DATA_UTC                    "UTCDATETIME"
#define MONG_HYDRO_DATA_LOCALT                 "LOCALDATETIME"
#define MONG_HYDRO_DATA_VALUE                  "VALUE"
#define MONG_SITELIST_SUBBSN                   "SUBBASINID"
#define MONG_SITELIST_DB                       "DB"


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
