#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "PER_STR.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new PER_STR();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_PERCO, MCLSDESC_PERCO);
    mdi.SetDescription(MDESC_PER_STR);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_PER_STR);
    mdi.SetName(MID_PER_STR);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("PER_STR.chm");


    mdi.AddParameter(Tag_TimeStep, UNIT_SECOND, UNIT_NON_DIM, File_Input, DT_Single);
    mdi.AddParameter(VAR_T_SOIL, UNIT_TEMP_DEG, DESC_T_SOIL, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_SOILLAYERS, UNIT_NON_DIM, DESC_SOILLAYERS, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILTHICK, UNIT_LEN_M, DESC_SOILTHICK, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_CONDUCT, UNIT_WTRDLT_MMH, DESC_CONDUCT, Source_ParameterDB, DT_Raster2D);
 //   mdi.AddParameter(VAR_POROST, UNIT_STRG_M3M, DESC_POROST, Source_ParameterDB, DT_Raster2D);
 //   mdi.AddParameter(VAR_FIELDCAP, UNIT_STRG_M3M, DESC_FIELDCAP, Source_ParameterDB, DT_Raster2D);
	//mdi.AddParameter(VAR_WILTPOINT,UNIT_WAT_RATIO, DESC_WILTPOINT, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_SOL_UL, UNIT_DEPTH_MM, DESC_SOL_UL, Source_ParameterDB, DT_Raster2D); // m_sat
	mdi.AddParameter(VAR_SOL_AWC, UNIT_DEPTH_MM, DESC_SOL_AWC, Source_ParameterDB, DT_Raster2D); // m_fc
    mdi.AddInput(VAR_SOTE, UNIT_TEMP_DEG, DESC_SOTE, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INFIL, UNIT_DEPTH_MM, DESC_INFIL, Source_Module, DT_Raster1D);

    mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module, DT_Raster2D);
	mdi.AddInput(VAR_SOL_SW, UNIT_DEPTH_MM, DESC_SOL_SW, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_SURU, UNIT_DEPTH_MM, DESC_SURU, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_POT_VOL, UNIT_DEPTH_MM, DESC_POT_VOL, Source_Module_Optional, DT_Raster1D);
    // set the output variables
    mdi.AddOutput(VAR_PERCO, UNIT_DEPTH_MM, DESC_PERCO, DT_Raster2D);

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}