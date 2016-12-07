#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "GWaterReservoir.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new GWaterReservoir();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_GW, MCLSDESC_GW);
    mdi.SetDescription(MDESC_GW_RSVR);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("GW_RSVR.chm");
    mdi.SetID(MID_GW_RSVR);
    mdi.SetName(MID_GW_RSVR);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_RchParam, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Array2D);

    mdi.AddParameter(Tag_HillSlopeTimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_GW_KG, UNIT_NON_DIM, DESC_GW_KG, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_Base_ex, UNIT_NON_DIM, DESC_BASE_EX, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_GW0, UNIT_DEPTH_MM, DESC_GW0, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_GWMAX, UNIT_DEPTH_MM, DESC_GWMAX, Source_ParameterDB, DT_Single);

    mdi.AddInput(VAR_PERCO, UNIT_DEPTH_MM, DESC_PERCO, Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_SBQG, UNIT_FLOW_CMS, DESC_SBQG, DT_Array1D);
    mdi.AddOutput(VAR_SBGS, UNIT_DEPTH_MM, DESC_SBGS, DT_Array1D);

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
	