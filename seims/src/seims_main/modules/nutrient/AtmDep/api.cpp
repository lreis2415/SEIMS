#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "AtmosphericDeposition.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new AtmosphericDeposition();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Huiran Gao");
    mdi.SetClass(MCLS_ATMDEP, MCLSDESC_ATMDEP);
    mdi.SetDescription(MDESC_ATMDEP);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_ATMDEP);
    mdi.SetName(MID_ATMDEP);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    //mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RCN, UNIT_DENSITY, DESC_RCN, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RCA, UNIT_DENSITY, DESC_RCA, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DRYDEP_NO3, UNIT_CONT_KGHA, DESC_DRYDEP_NO3, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DRYDEP_NH4, UNIT_CONT_KGHA, DESC_DRYDEP_NH4, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SOL_NH4, UNIT_CONT_KGHA, DESC_SOL_NH4, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_NO3, UNIT_CONT_KGHA, DESC_SOL_NO3, Source_ParameterDB, DT_Raster2D);

    // set input from other modules
    mdi.AddInput(VAR_PCP, UNIT_DEPTH_MM, DESC_PCP, Source_Module, DT_Raster1D);

    // set output
    mdi.AddOutput(VAR_WSHD_RNO3, UNIT_CONT_KGHA, DESC_WSHD_RNO3, DT_Single);

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
