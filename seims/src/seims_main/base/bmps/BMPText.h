/*!
 * \brief BMP related const strings
 * \revised Liang-Jun Zhu
 * \date 2016-6-16
 */
#ifndef SEIMS_BMP_TEXT_H
#define SEIMS_BMP_TEXT_H

//database name
//#define BMP_DATABASE_NAME        "BMP.db3"

//base scenario id
#define BASE_SCENARIO_ID           0
//Table names in BMP database
#define TAB_BMP_INDEX              "BMP_INDEX"
#define TAB_BMP_SCENARIO           "BMP_SCENARIOS"
//#define TAB_BMP_REACH            "REACH_BMP"
//#define TAB_BMP_POINT_SRC        "POINTSOURCE"
//#define TAB_BMP_FLOWDIVERSION    "FLOWDIVERSION"
//#define TAB_BMP_RESERVOIR        "RESERVOIR"

/// Table field names
/// TAB_BMP_SCENARIO
#define FLD_SCENARIO_ID            "ID"
#define FLD_SCENARIO_NAME          "NAME"
#define FLD_SCENARIO_BMPID         "BMPID"
#define FLD_SCENARIO_SUB           "SUBSCENARIO"
#define FLD_SCENARIO_DIST          "DISTRIBUTION"
#define FLD_SCENARIO_TABLE         "COLLECTION"
#define FLD_SCENARIO_LOCATION      "LOCATION"
/// Options of FLD_SCENARIO_DIST
#define FLD_SCENARIO_DIST_RASTER   "RASTER"
#define FLD_SCENARIO_DIST_ARRAY    "ARRAY"

/// TAB_BMP_INDEX
#define FLD_BMP_ID                 "ID"
#define FLD_BMP_TYPE               "TYPE"
#define FLD_BMP_PRIORITY           "PRIORITY"

//BMPs
//also the column name in Reach_BMP table for reach BMPs
#define BMP_NAME_POINTSOURCE             "POINT SOURCE"
#define BMP_NAME_FLOWDIVERSION_STREAM    "STREAM FLOW DIVERSION"
#define BMP_NAME_RESERVOIR               "RESERVOIR"
#define BMP_NAME_RIPARIANWETLAND         "RIPARIAN WETLAND"
#define BMP_NAME_RIPARIANBUFFER          "RIPARIAN BUFFER"
#define BMP_NAME_GRASSWATERWAY           "GRASS WATERWAY"
#define BMP_NAME_FILTERSTRIP             "FILTER STRIP"
#define BMP_NAME_POND                    "POND"
#define BMP_NAME_ISOLATEDPOND            "ISOLATED POND"
#define BMP_NAME_TERRACE                 "TERRACE"
#define BMP_NAME_FLOWDIVERSION_OVERLAND  "OVERLAND FLOW DIVERSION"
#define BMP_NAME_CROP                    "PLANT MANAGEMENT"
#define BMP_NAME_RESIDUAL                "RESIDUAL MANAGEMENT"
#define BMP_NAME_TILEDRAIN               "TILE DRAIN MANAGEMENT"
#define BMP_NAME_URBAN                   "URBAN MANAGEMENT"

//BMP Type
#define BMP_TYPE_POINTSOURCE             1
#define BMP_TYPE_FLOWDIVERSION_STREAM    2
#define BMP_TYPE_RESERVOIR               3
#define BMP_TYPE_RIPARIANWETLAND         4
#define BMP_TYPE_RIPARIANBUFFER          5
#define BMP_TYPE_GRASSWATERWAY           6
#define BMP_TYPE_FILTERSTRIP             7
#define BMP_TYPE_POND                    8
#define BMP_TYPE_ISOLATEDPOND            9
#define BMP_TYPE_TERRACE                 10
#define BMP_TYPE_FLOWDIVERSION_OVERLAND  11
#define BMP_TYPE_PLANT_MGT               12
#define BMP_TYPE_RESIDUAL                13
#define BMP_TYPE_TILEDRAIN               14
#define BMP_TYPE_URBAN                   15
#define BMP_TYPE_AREALSOURCE             16
#define BMP_TYPE_AREALSTRUCT             17

//// Common fields
#define BMP_FLD_SUB                      "SUBSCENARIO"
#define BMP_FLD_NAME                     "NAME"
#define BMP_FLD_SEQUENCE                 "SEQUENCE"
#define BMP_FLD_SYEAR                    "SYEAR"
#define BMP_FLD_SMONTH                   "SMONTH"
#define BMP_FLD_SDAY                     "SDAY"
#define BMP_FLD_EYEAR                    "EYEAR"
#define BMP_FLD_EMONTH                   "EMONTH"
#define BMP_FLD_EDAY                     "EDAY"
/// Point source management fields
#define BMP_PTSRC_FLD_CODE               "PTSRC"
#define BMP_PTSRC_FLD_Q                  "Q"
#define BMP_PTSRC_FLD_SED                "SED"
#define BMP_PTSRC_FLD_TN                 "TN"
#define BMP_PTSRC_FLD_NO3                "NO3"
#define BMP_PTSRC_FLD_NH4                "NH4"
#define BMP_PTSRC_FLD_ORGN               "ORGN"
#define BMP_PTSRC_FLD_TP                 "TP"
#define BMP_PTSRC_FLD_SOLP               "SOLP"
#define BMP_PTSRC_FLD_ORGP               "ORGP"
#define BMP_PTSRC_FLD_COD                "COD"
#define BMP_PTSRC_FLD_PTSRCID            "PTSRCID"
#define BMP_PTSRC_FLD_LAT                "LAT"
#define BMP_PTSRC_FLD_LON                "LON"
#define BMP_PTSRC_FLD_LOCALX             "LOCALX"
#define BMP_PTSRC_FLD_LOCALY             "LOCALY"
#define BMP_PTSRC_FLD_SUBBSN             "SUBBASINID"
#define BMP_PTSRC_FLD_SIZE               "SIZE"
#define BMP_PTSRC_FLD_DISTDOWN           "DIST2REACH"
/// Areal source management fields
#define BMP_ARSRC_FLD_CODE               "ARSRC"
#define BMP_ARSRC_FLD_Q                  "Q"
#define BMP_ARSRC_FLD_SED                "SED"
#define BMP_ARSRC_FLD_TN                 "TN"
#define BMP_ARSRC_FLD_NO3                "NO3"
#define BMP_ARSRC_FLD_NH4                "NH4"
#define BMP_ARSRC_FLD_ORGN               "ORGN"
#define BMP_ARSRC_FLD_TP                 "TP"
#define BMP_ARSRC_FLD_SOLP               "SOLP"
#define BMP_ARSRC_FLD_ORGP               "ORGP"
#define BMP_ARSRC_FLD_COD                "COD"
#define BMP_ARSRC_FLD_PTSRCID            "ARSRCID"
#define BMP_ARSRC_FLD_SIZE               "SIZE"
/// Plant management code
#define BMP_PLTOP_Plant                  1
#define BMP_PLTOP_Irrigation             2
#define BMP_PLTOP_Fertilizer             3
#define BMP_PLTOP_Pesticide              4
#define BMP_PLTOP_HarvestKill            5
#define BMP_PLTOP_Tillage                6
#define BMP_PLTOP_Harvest                7
#define BMP_PLTOP_Kill                   8
#define BMP_PLTOP_Grazing                9
#define BMP_PLTOP_AutoIrrigation         10
#define BMP_PLTOP_AutoFertilizer         11
#define BMP_PLTOP_ReleaseImpound         13
#define BMP_PLTOP_ContinuousFertilizer   14
#define BMP_PLTOP_ContinuousPesticide    15
#define BMP_PLTOP_Burning                16
#define BMP_PLTOP_SKIPYEAR               17

/// Plant management fields
#define BMP_PLTOP_FLD_LUCC               "LANDUSE_ID"
#define BMP_PLTOP_FLD_YEAR               "YEAR"
#define BMP_PLTOP_FLD_MONTH              "MONTH"
#define BMP_PLTOP_FLD_DAY                "DAY"
#define BMP_PLTOP_FLD_BASEHU             "BASE_HU"
#define BMP_PLTOP_FLD_HUSC               "HUSC"
#define BMP_PLTOP_FLD_MGTOP              "MGT_OP"
#define BMP_PLTOP_FLD_MGT_PRE            "MGT"

/// Areal structural BMP
#define BMP_ARSTRUCT_FLD_DESC            "DESC"
#define BMP_ARSTRUCT_FLD_REF             "REFERENCE"
#define BMP_ARSTRUCT_FLD_LANDUSE         "LANDUSE"
#define BMP_ARSTRUCT_FLD_PARAMS          "PARAMETERS"

////BMP Type
//#define BMP_TYPE_REACH                   1
//#define BMP_TYPE_AREAL_STRUCTURAL        2
//#define BMP_TYPE_AREAL_NON_STRUCTURAL    3

//Reservoir method column name
//#define RESERVOIR_FLOW_ROUTING_METHOD_COLUMN_NAME        "METHOD"
//#define RESERVOIR_SEDIMENT_ROUTING_METHOD_COLUMN_NAME    "SEDMETHOD"
//#define RESERVOIR_NUTRIENT_ROUTING_METHOD_COLUMN_NAME    "NUTMETHOD"

//Reservoir flow routing method
//#define RESERVOIR_FLOW_ROUTING_NAME_RATING_CURVE         "RAT_RES"
//#define RESERVOIR_FLOW_ROUTING_NAME_DAILY_OUTFLOW        "MDO_RES"
//#define RESERVOIR_FLOW_ROUTING_NAME_MONTHLY_OUTFLOW      "MMO_RES"
//#define RESERVOIR_FLOW_ROUTING_NAME_ANUNAL_RELEASE_RATE  "AAR_RES"
//#define RESERVOIR_FLOW_ROUTING_NAME_TARGET_RELEASE_RATE  "TRR_RES"

//#define RESERVOIR_FLOW_ROUTING_NAME_UNKNOWN             -1
//#define RESERVOIR_FLOW_ROUTING_RATING_CURVE              0
//#define RESERVOIR_FLOW_ROUTING_DAILY_OUTFLOW             1
//#define RESERVOIR_FLOW_ROUTING_MONTHLY_OUTFLOW           2
//#define RESERVOIR_FLOW_ROUTING_ANUNAL_RELEASE_RATE       3
//#define RESERVOIR_FLOW_ROUTING_TARGET_RELEASE_RATE       4

//Reservoir sediment routing method
//#define RESERVOIR_SEDIMENT_ROUTING_NAME_MASS_BALANCE     "SMB_RES"
//#define RESERVOIR_SEDIMENT_ROUTING_MASS_UNKONWN         -1
//#define RESERVOIR_SEDIMENT_ROUTING_MASS_BALANCE          0
//
////Reservoir column index
//#define RESERVOIR_SA_EM_INDEX         4
//#define RESERVOIR_V_EM_INDEX          5
//#define RESERVOIR_SA_PR_INDEX         6
//#define RESERVOIR_V_PR_INDEX          7
//#define RESERVOIR_INI_S_INDEX         8
//#define RESERVOIR_Q_REL_INDEX         9
//#define RESERVOIR_k_res_INDEX         10
//#define RESERVOIR_OFLOWMN01_INDEX     11
//#define RESERVOIR_OFLOWMX01_INDEX     23
//#define RESERVOIR_METHOD_INDEX        35
//#define RESERVOIR_SED_METHOD_INDEX    36    //The method used to do sediment routing
//#define RESERVOIR_INI_SC_INDEX        37    //initial sediment concentration, Mg/m**3, default value is 0.03
//#define RESERVOIR_NSED_INDEX          38    //equilibrium sediment concentration, Mg/m**3,
//#define RESERVOIR_D50_INDEX           39    //median particle size of the inflow sediment (um), default value is 10
//#define RESERVOIR_NUT_METHOD_INDEX    40  //The method used to do nutrient routing

//Crop classification
#define CROP_IDC_WARM_SEASON_ANNUAL_LEGUME    1
#define CROP_IDC_CODE_SEASON_ANNUAL_LEGUME    2
#define CROP_IDC_PERENNIAL_LEGUME             3
#define CROP_IDC_WARM_SEASON_ANNUAL           4
#define CROP_IDC_COLD_SEASON_ANNUAL           5
#define CROP_IDC_PERENNIAL                    6
#define CROP_IDC_TREES                        7

/// Field index in CropLookup table
#define CROP_PARAM_COUNT                      43
/// Index started with 0
/// ICNUM, IDC,BIO_E, HVSTI, BLAI, FRGRW1, LAIMX1, FRGRW2, LAIMX2, DLAI, CHTMX,     1-11
/// RDMX, T_OPT, T_BASE, CNYLD, CPYLD, BN1, BN2, BN3, BP1, BP2,    12-21
/// BP3, WSYF, USLE_C, GSI, VPDFR, FRGMAX, WAVP, CO2HI, BIOEHI, RSDCO_PL,  22-31
/// OV_N, CN2A, CN2B, CN2C, CN2D, FERTFIELD, ALAI_MIN, BIO_LEAF, MAT_YRS, BMX_TREES, 32-41
/// EXT_COEF, BM_DIEOFF  42-43
#define CROP_PARAM_IDX_ICNUM               1
#define CROP_PARAM_IDX_IDC                 2
#define CROP_PARAM_IDX_BIO_E               3
#define CROP_PARAM_IDX_HVSTI               4
#define CROP_PARAM_IDX_BLAI                5
#define CROP_PARAM_IDX_FRGRW1              6
#define CROP_PARAM_IDX_LAIMX1              7
#define CROP_PARAM_IDX_FRGRW2              8
#define CROP_PARAM_IDX_LAIMX2              9
#define CROP_PARAM_IDX_DLAI                10
#define CROP_PARAM_IDX_CHTMX               11
#define CROP_PARAM_IDX_RDMX                12
#define CROP_PARAM_IDX_T_OPT               13
#define CROP_PARAM_IDX_T_BASE              14
#define CROP_PARAM_IDX_CNYLD               15
#define CROP_PARAM_IDX_CPYLD               16
#define CROP_PARAM_IDX_BN1                 17
#define CROP_PARAM_IDX_BN2                 18
#define CROP_PARAM_IDX_BN3                 19
#define CROP_PARAM_IDX_BP1                 20
#define CROP_PARAM_IDX_BP2                 21
#define CROP_PARAM_IDX_BP3                 22
#define CROP_PARAM_IDX_WSYF                23
#define CROP_PARAM_IDX_USLE_C              24
#define CROP_PARAM_IDX_GSI                 25
#define CROP_PARAM_IDX_VPDFR               26
#define CROP_PARAM_IDX_FRGMAX              27
#define CROP_PARAM_IDX_WAVP                28
#define CROP_PARAM_IDX_CO2HI               29
#define CROP_PARAM_IDX_BIOEHI              30
#define CROP_PARAM_IDX_RSDCO_PL            31
#define CROP_PARAM_IDX_OV_N                32
#define CROP_PARAM_IDX_CN2A                33
#define CROP_PARAM_IDX_CN2B                34
#define CROP_PARAM_IDX_CN2C                35
#define CROP_PARAM_IDX_CN2D                36
#define CROP_PARAM_IDX_FERTFIELD           37
#define CROP_PARAM_IDX_ALAI_MIN            38
#define CROP_PARAM_IDX_BIO_LEAF            39

#define CROP_PADDYRICE                     33

/// Field index in TillageLookup table
#define TILLAGE_PARAM_COUNT                7
/// ITNUM, EFTMIX, DEPTIL, RRNS, CNOP_CN2, PRC, DSC
#define TILLAGE_PARAM_ITNUM_IDX            1
#define TILLAGE_PARAM_EFFMIX_IDX           2
#define TILLAGE_PARAM_DEPTIL_IDX           3
#define TILLAGE_PARAM_RRNS_IDX             4
#define TILLAGE_PARAM_CNOP_IDX             5
#define TILLAGE_PARAM_PRC_IDX              6
#define TILLAGE_PARAM_DSC_IDX              7


/// Field index in FertilizerLookup table
#define FERTILIZER_PARAM_COUNT             10
/// IFNUM, FMINN, FMINP, FORGN, FORGP, FNH4N, BACTPDB, BACTLPDB, BACTKDDB, MANURE
#define FERTILIZER_PARAM_IFNUM_IDX         1
#define FERTILIZER_PARAM_FMINN_IDX         2
#define FERTILIZER_PARAM_FMINP_IDX         3
#define FERTILIZER_PARAM_FORGN_IDX         4
#define FERTILIZER_PARAM_FORGP_IDX         5
#define FERTILIZER_PARAM_FNH4N_IDX         6
#define FERTILIZER_PARAM_BACTPDB_IDX       7
#define FERTILIZER_PARAM_BATTLPDB_IDX      8
#define FERTILIZER_PARAM_BACKTKDDB_IDX     9
#define FERTILIZER_PARAM_MANURE_IDX        10

/// Fertilizer ID
#define FERTILIZER_ID_UREA                 4
/// Field index in LanduseLookup table
#define LANDUSE_PARAM_COUNT                49
#define LANDUSE_PARAM_LANDUSE_ID_IDX       1
#define LANDUSE_PARAM_CN2A_IDX             2
#define LANDUSE_PARAM_CN2B_IDX             3
#define LANDUSE_PARAM_CN2C_IDX             4
#define LANDUSE_PARAM_CN2D_IDX             5
#define LANDUSE_PARAM_ROOT_DEPTH_IDX       6
#define LANDUSE_PARAM_MANNING_IDX          7
#define LANDUSE_PARAM_INTERC_MAX_IDX       8
#define LANDUSE_PARAM_INTERC_MIN_IDX       9
#define LANDUSE_PARAM_SHC_IDX              10
#define LANDUSE_PARAM_SOIL_T10_IDX         11
#define LANDUSE_PARAM_USLE_C_IDX           12
#define LANDUSE_PARAM_PET_FR_IDX           13
#define LANDUSE_PARAM_PRC_ST1_IDX          14
#define LANDUSE_PARAM_PRC_ST2_IDX          15
#define LANDUSE_PARAM_PRC_ST3_IDX          16
#define LANDUSE_PARAM_PRC_ST4_IDX          17
#define LANDUSE_PARAM_PRC_ST5_IDX          18
#define LANDUSE_PARAM_PRC_ST6_IDX          19
#define LANDUSE_PARAM_PRC_ST7_IDX          20
#define LANDUSE_PARAM_PRC_ST8_IDX          21
#define LANDUSE_PARAM_PRC_ST9_IDX          22
#define LANDUSE_PARAM_PRC_ST10_IDX         23
#define LANDUSE_PARAM_PRC_ST11_IDX         24
#define LANDUSE_PARAM_PRC_ST12_IDX         25
#define LANDUSE_PARAM_SC_ST1_IDX           26
#define LANDUSE_PARAM_SC_ST2_IDX           27
#define LANDUSE_PARAM_SC_ST3_IDX           28
#define LANDUSE_PARAM_SC_ST4_IDX           29
#define LANDUSE_PARAM_SC_ST5_IDX           30
#define LANDUSE_PARAM_SC_ST6_IDX           31
#define LANDUSE_PARAM_SC_ST7_IDX           32
#define LANDUSE_PARAM_SC_ST8_IDX           33
#define LANDUSE_PARAM_SC_ST9_IDX           34
#define LANDUSE_PARAM_SC_ST10_IDX          35
#define LANDUSE_PARAM_SC_ST11_IDX          36
#define LANDUSE_PARAM_SC_ST12_IDX          37
#define LANDUSE_PARAM_DSC_ST1_IDX          38
#define LANDUSE_PARAM_DSC_ST2_IDX          39
#define LANDUSE_PARAM_DSC_ST3_IDX          40
#define LANDUSE_PARAM_DSC_ST4_IDX          41
#define LANDUSE_PARAM_DSC_ST5_IDX          42
#define LANDUSE_PARAM_DSC_ST6_IDX          43
#define LANDUSE_PARAM_DSC_ST7_IDX          44
#define LANDUSE_PARAM_DSC_ST8_IDX          45
#define LANDUSE_PARAM_DSC_ST9_IDX          46
#define LANDUSE_PARAM_DSC_ST10_IDX         47
#define LANDUSE_PARAM_DSC_ST11_IDX         48
#define LANDUSE_PARAM_DSC_ST12_IDX         49

#define LANDUSE_ID_WATR                    18
#define LANDUSE_ID_PADDY                   33

/// irrigation source code:
#define IRR_SRC_RCH                    1 /// divert water from reach
#define IRR_SRC_RES                    2 /// divert water from reservoir
#define IRR_SRC_SHALLOW                3 ///  divert water from shallow aquifer
#define IRR_SRC_DEEP                   4 /// divert water from deep aquifer
#define IRR_SRC_OUTWTSD                5 /// divert water from source outside watershed

#endif /* SEIMS_BMP_TEXT_H */
