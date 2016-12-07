#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "DepressionFS.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new DepressionFS();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_DEP, MCLSDESC_DEP);
    mdi.SetDescription(MDESC_DEP_FS);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("DEP_FS.chm");
    mdi.SetID(MID_DEP_FS);
    mdi.SetName(MID_DEP_FS);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(VAR_DEPREIN, UNIT_NON_DIM, DESC_DEPREIN, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DEPRESSION, UNIT_DEPTH_MM, DESC_DEPRESSION, Source_ParameterDB, DT_Raster1D);

#ifndef STORM_MODE
    mdi.AddInput(VAR_PET, UNIT_DEPTH_MM, DESC_PET, Source_Module, DT_Raster1D);    //PET
    mdi.AddInput(VAR_INLO, UNIT_DEPTH_MM, DESC_INLO, Source_Module, DT_Raster1D);
    mdi.AddOutput(VAR_DEET, UNIT_DEPTH_MM, DESC_DEET, DT_Raster1D);
#endif
    //mdi.AddInput("D_INFIL","mm","Infiltration calculated in the infiltration module", "Module", DT_Raster);							//Infiltration
    mdi.AddOutput(VAR_DPST, UNIT_DEPTH_MM, DESC_DPST, DT_Raster1D);
    mdi.AddOutput(VAR_SURU, UNIT_DEPTH_MM, DESC_SURU, DT_Raster1D);
    mdi.AddOutput(VAR_STCAPSURPLUS, UNIT_DEPTH_MM, DESC_STCAPSURPLUS, DT_Raster1D);

    res = mdi.GetXMLDocument();
    //return res;

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}