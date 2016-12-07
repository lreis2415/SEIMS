#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "SNO_DD.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new SNO_DD();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Chunping Ou");
    mdi.SetClass(MCLS_SNOW, MCLSDESC_SNOW);
    mdi.SetDescription(MDESC_SNO_DD);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_SNO_DD);
    mdi.SetName(MID_SNO_DD);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("SNO_DD.chm");
	
    mdi.AddParameter(VAR_C_SNOW, UNIT_TEMP_FACTOR, DESC_C_SNOW, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_C_RAIN, UNIT_TEMP_FACTOR, DESC_C_RAIN, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T0, UNIT_TEMP_DEG, DESC_T0, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T_SNOW, UNIT_TEMP_DEG, DESC_T_SNOW, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_K_BLOW, UNIT_NON_DIM, DESC_K_BLOW, Source_ParameterDB, DT_Single);
    //mdi.AddParameter(VAR_SWE0, UNIT_DEPTH_MM, DESC_SWE0, Source_ParameterDB, DT_Single);

    mdi.AddInput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, Source_Module, DT_Raster1D); // from interception module
    //mdi.AddInput(VAR_SNSB, UNIT_DEPTH_MM, DESC_SNSB, Source_Module, DT_Raster1D); //from snow sublimation module
    mdi.AddInput(VAR_TMEAN, UNIT_TEMP_DEG, DESC_TMEAN, Source_Module, DT_Raster1D);

    // set the output variables
	mdi.AddOutput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, DT_Raster1D);
    mdi.AddOutput(VAR_SNME, UNIT_DEPTH_MM, DESC_SNME, DT_Raster1D);
    mdi.AddOutput(VAR_SNAC, UNIT_DEPTH_MM, DESC_SNAC, DT_Raster1D);

    // write out the XML file.
    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}