#ifndef TEXT_H
#define TEXT_H

// string constants used in the code
// TODO - you may consider moving these to an external resource file 
//        to make it easier to change them later 
#define App_Config_File "app.config"

#define Source_ParameterDB "ParameterDB"
#define Source_File "File"

//for contant input variables
#define Contant_Input_Elevation						"Elevation"
#define Contant_Input_Latitude						"Latitude"
#define Contant_Input_Xpr							"xpr"
#define Contant_Input_Ypr							"ypr"
//#define Contant_Input_FlowdiversionProperty			"FlowDiversion_property"
//#define Contant_Input_PointsourceProperty			"PointSource_property"
//#define Contant_Input_ReservoirProperty				"Reservoir_property"
//#define Contant_Input_ReservoirRatingCurve			"Reservoir_RatingCurve"
//#define Contant_Input_ReservoirOperationSchedual	"Reservoir_OperationSchedual"

//for data type
#define DataType_Precipitation "P"					//1
#define DataType_MinimumTemperature "TMIN"			//2
#define DataType_MaximumTemperature "TMAX"			//3
#define DataType_PotentialEvapotranspiration "PET"	//4
#define DataType_SolarRadiation "SR"				//5
#define DataType_WindSpeed "WS"						//6
#define DataType_RelativeAirMoisture "RM"			//7

#define Tag_Elevation "Elevation"
#define Tag_Configuration "Configuration"
#define Tag_ProjectsPath "ProjectsPath"

#ifndef linux
    #define Tag_ModuleDirectoryName "modules\\"
    #define Tag_ModuleExt ".dll"
#else
    #define Tag_ModuleDirectoryName "modules/"
    #define Tag_ModuleExt ".so"
#endif

#define Tag_OutputID "OUTPUTID"
#define Tag_Interval "INTERVAL"
#define Tag_SiteCount "SITECOUNT"
#define Tag_SiteName "SITENAME"
#define Tag_ReachName "REACHNAME"
#define Tag_StartTime "STARTTIME"
#define Tag_EndTime "ENDTIME"
#define Tag_FileName "FILENAME"
#define Tag_Type "TYPE"
#define Tag_Count "COUNT"
#define Tag_DataType "DATA_TYPE"
#define Tag_Weight "WEIGHT"
#define Tag_VerticalInterpolation "VERTICALINTERPOLATION"
#define Tag_SubbasinCount "SUBBASINCOUNT"
#define Tag_SubbasinId "SUBBASINID"
#define Tag_ReservoirCount "RESERVOIRCOUNT"
#define Tag_ReservoirId "RESERVOIRID"
#define Tag_SubbasinSelected "subbasinSelected"
#define Tag_CellSize "CELLSIZE"
#define Tag_Mask "MASK"
#define Tag_TimeStep "STORM_DT"
#define Tag_CellWidth "CellWidth"
#define Tag_PStat "P_STAT"
#define Tag_FLOWIN_INDEX "FLOWIN_INDEX"
#define Tag_FLOWOUT_INDEX "FLOWOUT_INDEX"
#define Tag_ROUTING_LAYERS "ROUTING_LAYERS"
#define Tag_ReachParameter "ReachParameter"
#define Tag_QOUTLET "QOUTLET"
#define Tag_SEDOUTLET "SEDOUTLET"
#define Tag_QSUBBASIN "QSUBBASIN"

#define Tag_SEDOUTLET "SEDOUTLET"

#define ModID_PET_RD "PET_RD"
#define ModId_ITP_AU "ITP_AU"
#define ModId_PI_MSM "PI_MSM"

//for reach hydroclimate data
#define DIVERSION	"flowdiversion"
#define POINTSOURCE "pointsource"
#define RESERVOIR	"reservoir"
#define FLOW		"flow"
#define SDEIDMENT	"sediment"
#define ORGANICN	"organicN"
#define ORGANICP	"organicP"
#define NO3			"NO3"
#define NH4			"NH4"
#define NO2			"NO2"
#define MINERALP	"mineralP"

//! string constants used in the code
#define MapWindowRasterExtension ".asc"
#define RasterExtension ".asc"
#define TextExtension ".txt"
#define File_HydroClimateDB "HydroClimate.db3"
#define File_ParameterDB "Parameter.db3"
#define File_BMPDB "BMP.db3"
#define Table_LapseRate "lapse_rate"
#define Table_WGN "wgt"
#define File_Config "config.fig"
#define File_Input "file.in"
#define File_Output "file.out"
#define File_Mask "mask.asc"
#define File_DEM "dem.asc"

#define NAME_MASK "mask"
//#define Tag_SiteCount "SITECOUNT"
//#define Tag_SiteName "SITENAME"
//#define Tag_StartTime "STARTTIME"
//#define Tag_EndTime "ENDTIME"
//#define Tag_Interval "INTERVAL"

//! define string constants used in the code
#define Tag_NoDataValue "NoDataValue"
#define Tag_SubbasinSelected "subbasinSelected"

#define Type_Scenario "SCENARIO"
#define Type_LapseRateArray "LAPSERATEARRAY"
#define Type_SiteInformation "SITEINFORMATION"
#define Type_MapWindowRaster "MAPWINDOWRASTER"
#define Type_Array1DDateValue "ARRAY1DDATEVALUE"
#define Type_Array3D "ARRAY3D"
#define Type_Array2D "ARRAY2D"
#define Type_Array1D "ARRAY1D"
#define Type_Single "SINGLE"

#define Print_D_PREC "D_PREC"
#define Print_D_POET "D_POET"
#define Print_D_TEMP "D_TEMP"
#define Print_D_INLO "D_INLO"
#define Print_D_INET "D_INET"
#define Print_D_NEPR "D_NEPR"

#define ModID_PI_MSM "PI_MSM"
#define ModID_ITP_AU "ITP_AU"

#define PARAMETERS_TABLE "parameters"
#define LANDUSE_TABLE "LanduseLookup"
#define SOIL_TABLE "SoilLookup"
#define STATIONS_TABLE "stations"
#define TOPOLOGY_TABLE "reaches"
#define SPATIAL_TABLE "spatial.files"
#define MEASUREMENT_TABLE "measurement"

#ifndef linux
#define CH_ROUTING_MODULE "IKW_CH"
#else
#define CH_ROUTING_MODULE "libIKW_CH"
#endif

#endif

