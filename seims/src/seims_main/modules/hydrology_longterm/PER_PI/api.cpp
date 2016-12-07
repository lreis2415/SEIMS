#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "PER_PI.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new PER_PI();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_PERCO, MCLSDESC_PERCO);
    mdi.SetDescription(MDESC_PER_PI);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_PER_PI);
    mdi.SetName(MID_PER_PI);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("PER_PI.chm");

    mdi.AddParameter(Tag_TimeStep, UNIT_SECOND, UNIT_NON_DIM, File_Input, DT_Single);
    mdi.AddParameter(VAR_T_SOIL, UNIT_TEMP_DEG, DESC_T_SOIL, Source_ParameterDB, DT_Single);

    //mdi.AddParameter(VAR_SOILDEPTH, UNIT_LEN_M, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_SOILLAYERS, UNIT_NON_DIM, DESC_SOILLAYERS, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_SOILTHICK, UNIT_DEPTH_MM, DESC_SOILTHICK, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_CONDUCT, UNIT_WTRDLT_MMH, DESC_CONDUCT, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POREIDX, UNIT_NON_DIM, DESC_POREIDX, Source_ParameterDB, DT_Raster2D);
    //mdi.AddParameter(VAR_POROST, UNIT_NON_DIM, DESC_POROST, Source_ParameterDB, DT_Raster2D); => VAR_SOL_UL
    //mdi.AddParameter(VAR_FIELDCAP, UNIT_WAT_RATIO, DESC_FIELDCAP, Source_ParameterDB, DT_Raster2D); => VAR_SOL_AWC
	//mdi.AddParameter(VAR_WILTPOINT,UNIT_WAT_RATIO, DESC_WILTPOINT, Source_ParameterDB, DT_Raster2D); => VAR_SOL_WPMM
	mdi.AddParameter(VAR_SOL_UL, UNIT_DEPTH_MM, DESC_SOL_UL, Source_ParameterDB, DT_Raster2D); // m_sat
	mdi.AddParameter(VAR_SOL_AWC, UNIT_DEPTH_MM, DESC_SOL_AWC, Source_ParameterDB, DT_Raster2D); // m_fc
	mdi.AddParameter(VAR_SOL_WPMM, UNIT_DEPTH_MM, DESC_SOL_WPMM, Source_ParameterDB, DT_Raster2D); // m_wp
	mdi.AddInput(VAR_IMPOUND_TRIG, UNIT_NON_DIM, DESC_IMPOUND_TRIG, Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_SOTE, UNIT_TEMP_DEG, DESC_SOTE, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INFIL, UNIT_DEPTH_MM, DESC_INFIL, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module, DT_Raster2D);
	mdi.AddInput(VAR_SOL_SW, UNIT_DEPTH_MM, DESC_SOL_SW, Source_Module, DT_Raster1D);
    // set the output variables
    mdi.AddOutput(VAR_PERCO, UNIT_DEPTH_MM, DESC_PERCO, DT_Raster2D);

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}