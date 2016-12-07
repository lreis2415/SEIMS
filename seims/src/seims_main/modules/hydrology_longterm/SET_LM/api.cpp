#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "SET_LM.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new SET_LM();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res;
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Chunping Ou");
    mdi.SetClass("Evapotranspiration from soil",
                 "Calculate the amount of the evapotranspiration from soil water reservoir within the time step.");
    mdi.SetDescription(
            "The method relating ET linearly with actual soil moisture used in the original WetSpa will be the default method to estimate actual ET from the soil water reservoir.");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID("SET_LM");
    mdi.SetName("SET_LM");
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("SET_LM.chm");

    mdi.AddParameter(VAR_T_SOIL, UNIT_TEMP_DEG, DESC_T_SOIL, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SOILDEPTH, UNIT_LEN_M, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(VAR_FIELDCAP, UNIT_VOL_FRA_M3M3, DESC_FIELDCAP, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_WILTPOINT, UNIT_VOL_FRA_M3M3, DESC_WILTPOINT, Source_ParameterDB, DT_Array2D);

    // set the parameters (non-time series)
    mdi.AddInput(VAR_PET, UNIT_DEPTH_MM, DESC_PET, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INET, UNIT_DEPTH_MM, DESC_INET, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DEET, UNIT_DEPTH_MM, DESC_DEET, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOTE, UNIT_TEMP_DEG, DESC_SOTE, Source_Module, DT_Raster1D);

    mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module, DT_Raster2D);

    // set the output variables
    mdi.AddOutput(VAR_SOET, UNIT_DEPTH_MM, DESC_SOET, DT_Raster1D);

    // write out the XML file.
    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}