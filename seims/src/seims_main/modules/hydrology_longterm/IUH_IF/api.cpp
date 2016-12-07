#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "IUH_IF.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new IUH_IF();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Wu Hui");
    mdi.SetClass(MCLS_INTERFLOW, MCLSDESC_INTERFLOW);
    mdi.SetDescription(MDESC_IUH_IF);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("IUH_IF.chm");
    mdi.SetID(MID_IUH_IF);
    mdi.SetName(MID_IUH_IF);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_TimeStep, UNIT_HOUR, DESC_TIMESTEP, File_Input, DT_Single);
    mdi.AddParameter(Tag_CellSize, UNIT_NON_DIM, DESC_CellSize, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_OL_IUH, UNIT_NON_DIM, DESC_OL_IUH, Source_ParameterDB, DT_Array2D);

    mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);

    //mdi.AddParameter("uhminCell","","start time of IUH for each grid cell","ParameterDB_Discharge",DT_Array1D);
    //mdi.AddParameter("uhmaxCell","","end time of IUH for each grid cell","ParameterDB_Discharge",DT_Array1D);
    mdi.AddInput(VAR_SSRU, UNIT_DEPTH_MM, DESC_SSRU, Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_SBIF, UNIT_FLOW_CMS, DESC_SBIF, DT_Array1D);

    // set the dependencies
    mdi.AddDependency("SSR_DA", "Subsurface Runoff module");

    // write out the XML file.

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}