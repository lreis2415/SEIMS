/*!
 * \author Junzhi Liu, LiangJun Zhu
 * \date Jan. 2010
 * \revised date Apr. 2016
 */
#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "Interpolate.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new Interpolate();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu, Liangjun Zhu");
    mdi.SetClass(MCLS_CLIMATE, MCLSDESC_CLIMATE);
    mdi.SetDescription(MDESC_ITP);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_ITP);
    mdi.SetName(MID_ITP);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    //from parameter database, e.g., Weight_P, Weight_PET, Weight_T.
    mdi.AddParameter(Tag_Weight, UNIT_NON_DIM, DESC_WEIGHT_ITP, Source_ParameterDB, DT_Array1D);
    // from config.fig, e.g. Interpolation_P_1
    mdi.AddParameter(Tag_VerticalInterpolation, UNIT_NON_DIM, DESC_VER_ITP, File_Config, DT_Single);
    // these three parameters are just read when it will do vertical interpolation
    mdi.AddParameter(VAR_DEM, UNIT_LEN_M, DESC_DEM, Source_ParameterDB, DT_Raster1D);//from spatial database
    mdi.AddParameter(Tag_StationElevation, UNIT_LEN_M, Tag_StationElevation, Source_HydroClimateDB,
                     DT_Array1D);/// from stations table
    // Lapse_rate is the combined lapse rate table name in HydroClimate database. 
    // TODO, currently, LapseRate is defined in ModuleFactory.cpp and not imported into MongoDB. By LJ
    mdi.AddParameter(VAR_LAP_RATE, UNIT_LAP_RATE, DESC_LAP_RATE, Source_HydroClimateDB, DT_Array2D);

    // This is the climate data of all sites.
    // T means time series and it is same with first part of output id, e.g T_P. It may be P,PET,TMean, TMin or TMax data.
    mdi.AddInput(DataType_Prefix_TS, UNIT_NON_DIM, DESC_NONE, Source_Module, DT_Array1D);

    /// Must be "D". This is used to match with output id in file.out with data type.
    mdi.AddOutput(DataType_Prefix_DIS, UNIT_NON_DIM, DESC_NONE, DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}