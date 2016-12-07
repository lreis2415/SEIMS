#include <stdio.h>
#include <string>
#include "util.h"
#include "api.h"
#include "NonPointSource_Management.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new NPS_Management();
}

/// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;
    string res;

    mdi.SetAuthor("Liang-Jun Zhu");
    mdi.SetClass(MCLS_MGT, MCLSDESC_MGT);
    mdi.SetDescription(MDESC_NPSMGT);
    mdi.SetID(MID_NPSMGT);
    mdi.SetName(MID_NPSMGT);
    mdi.SetVersion("0.1");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("NPSMGT.html");
    /// set parameters from database
	mdi.AddParameter(Tag_TimeStep, UNIT_SECOND, DESC_TIMESTEP, Source_ParameterDB, DT_Single);
	mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SCENARIO, UNIT_NON_DIM, DESC_SCENARIO, Source_ParameterDB, DT_Scenario);
	mdi.AddParameter(VAR_MGT_FIELD, UNIT_NON_DIM, DESC_MGT_FIELD, Source_ParameterDB, DT_Raster1D);
    /// set input from other modules, all set to be optional
    mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module_Optional, DT_Raster2D);
	mdi.AddInput(VAR_SOL_NH4, UNIT_CONT_KGHA, DESC_SOL_NH4, Source_Module_Optional, DT_Raster2D);
	mdi.AddInput(VAR_SOL_NO3, UNIT_CONT_KGHA, DESC_SOL_NO3, Source_Module_Optional, DT_Raster2D);
	mdi.AddInput(VAR_SOL_SOLP, UNIT_CONT_KGHA, DESC_SOL_SOLP, Source_Module_Optional, DT_Raster2D);
    /// outputs

    /// write out the XML file.
    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}