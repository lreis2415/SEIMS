#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "SSM_PE.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new SSM_PE();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Chunping Ou");
    mdi.SetClass(MCLS_SNO_SB, MCLSDESC_SNO_SB);
    mdi.SetDescription(MDESC_SSM_PE);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_SSM_PE);
    mdi.SetName(MID_SSM_PE);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("SSM_PE.chm");

    mdi.AddParameter(VAR_K_SUBLI, UNIT_NON_DIM, DESC_K_SUBLI, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_BLOW, UNIT_NON_DIM, DESC_K_BLOW, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T0, UNIT_TEMP_DEG, DESC_T0, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T_SNOW, UNIT_TEMP_DEG, DESC_T_SNOW, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SWE0, UNIT_DEPTH_MM, DESC_SWE0, Source_ParameterDB, DT_Single);

    mdi.AddInput(VAR_PET, UNIT_DEPTH_MM, DESC_PET, Source_Module, DT_Raster1D);  // from PET module
    mdi.AddInput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, Source_Module, DT_Raster1D); // from interception module
    mdi.AddInput(VAR_SNAC, UNIT_DEPTH_MM, DESC_SNAC, Source_Module, DT_Raster1D); // from snow melt module
    mdi.AddInput(VAR_SWE, UNIT_DEPTH_MM, DESC_SWE, Source_Module, DT_Single); // from snow water balance module
    // SNRD should be added as input when snow redistribution module is accomplished. LJ
    //mdi.AddInput(VAR_SNRD, UNIT_DEPTH_MM, DESC_SNRD, Source_Module, DT_Raster1D);	 // from snow redistribution module
    mdi.AddInput(VAR_TMEAN, UNIT_TEMP_DEG, DESC_TMEAN, Source_Module, DT_Raster1D); // from interpolation module
    
	// set the output variables
    mdi.AddOutput(VAR_SNSB, UNIT_DEPTH_MM, DESC_SNSB, DT_Raster1D);

    // write out the XML file.

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}