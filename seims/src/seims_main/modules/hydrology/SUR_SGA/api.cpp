#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "StormGreenAmpt.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new StormGreenAmpt();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_SUR_RUNOFF, MCLSDESC_SUR_RUNOFF);
    mdi.SetDescription(MDESC_SUR_SGA);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("SUR_SGA.chm");
    mdi.SetID(MID_SUR_SGA);
    mdi.SetName(MID_SUR_SGA);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_HillSlopeTimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single);

    mdi.AddParameter(VAR_CONDUCT, UNIT_WTRDLT_MMH, DESC_CONDUCT, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_MOIST_IN, UNIT_VOL_FRA_M3M3, DESC_MOIST_IN, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CLAY, UNIT_PERCENT, DESC_CLAY, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SAND, UNIT_PERCENT, DESC_SAND, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILDEPTH, UNIT_LEN_M, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_FIELDCAP, UNIT_VOL_FRA_M3M3, DESC_FIELDCAP, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POROST, UNIT_NON_DIM, DESC_POROST, Source_ParameterDB, DT_Raster2D);

    mdi.AddInput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DPST, UNIT_DEPTH_MM, DESC_DPST, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SURU, UNIT_DEPTH_MM, DESC_SURU, Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, DT_Raster2D);
    mdi.AddOutput(VAR_INFIL, UNIT_DEPTH_MM, DESC_INFIL, DT_Raster1D);
    mdi.AddOutput(VAR_INFILCAPSURPLUS, UNIT_DEPTH_MM, DESC_INFILCAPSURPLUS, DT_Raster1D);
    mdi.AddOutput(VAR_ACC_INFIL, UNIT_DEPTH_MM, DESC_ACC_INFIL, DT_Raster1D);

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
//mdi.AddParameter("t_soil","degree Celsius","threshold soil freezing temperature","ParameterDB_WaterBalance",DT_Single);

//mdi.AddParameter("T0","degree Celsius","snowmelt threshold temperature","ParameterDB_Snow",DT_Single);
//mdi.AddParameter("T_snow","degree Celsius","snowfall temperature","ParameterDB_Snow",DT_Single);
//mdi.AddParameter("s_frozen", "m3/m3", "frozen soil moisture","ParameterDB_WaterBalance", DT_Single);

//mdi.AddInput("D_TEMP","oC","Air temperature","Module", DT_Raster);
//mdi.AddInput("D_SOMO","m3/m3","The soil moisture","Module", DT_Raster);
//mdi.AddInput("D_SNAC","mm","The snow accumulation","Module",DT_Raster);
//mdi.AddInput("D_SNME","mm","The snowmelt","Module",DT_Raster);

//mdi.AddOutput("EXCP", "mm","The excess precipitation", DT_Raster);