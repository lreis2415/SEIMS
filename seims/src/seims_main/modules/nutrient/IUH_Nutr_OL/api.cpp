#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "IUH_NUTR_OL.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new IUH_NUTR_OL();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Wu Hui; Zhiqiang Yu");
    mdi.SetClass(MCLS_OL_ROUTING, MCLSDESC_OL_ROUTING);
    mdi.SetDescription(MDESC_IUH_NUTR_OL);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("IUH_NUTR_OL.chm");
    mdi.SetID(MID_IUH_NUTR_OL);
    mdi.SetName(MID_IUH_NUTR_OL);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_TimeStep, UNIT_TIMESTEP_HOUR, DESC_TIMESTEP, File_Input, DT_Single);
    mdi.AddParameter(Tag_CellSize, UNIT_NON_DIM, DESC_CellSize, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_OL_IUH, UNIT_NON_DIM, DESC_OL_IUH, Source_ParameterDB, DT_Array2D);

    mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_SUBBASIN_PARAM, UNIT_NON_DIM, DESC_SUBBASIN_PARAM, Source_ParameterDB, DT_Subbasin);
    //mdi.AddParameter("uhminCell","","start time of IUH for each grid cell","ParameterDB_Discharge",DT_Array1D);
    //mdi.AddParameter("uhmaxCell","","end time of IUH for each grid cell","ParameterDB_Discharge",DT_Array1D);

	mdi.AddInput(VAR_SOER, UNIT_KG, DESC_SOER, Source_Module, DT_Raster1D);

	mdi.AddOutput(VAR_SED_TO_CH, UNIT_KG, DESC_SED_TO_CH, DT_Array1D);
	mdi.AddOutput(VAR_SED_OL, UNIT_KG, DESC_SED_OL, DT_Raster1D);

    // set the dependencies
    //mdi.AddDependency("DEP_FS", "Depression Storage module");

    // write out the XML file.

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}