#include "api.h"

#include "SSM_PE.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new SSM_PE();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Chunping Ou");
    mdi.SetClass(MCLS_SNO_SB[0], MCLS_SNO_SB[1]);
    mdi.SetDescription(M_SSM_PE[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_SSM_PE[0]);
    mdi.SetName(M_SSM_PE[0]);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("SSM_PE.chm");

    mdi.AddParameter(VAR_K_SUBLI[0], UNIT_NON_DIM, VAR_K_SUBLI[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_BLOW[0], UNIT_NON_DIM, VAR_K_BLOW[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T0[0], UNIT_TEMP_DEG, VAR_T0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T_SNOW[0], UNIT_TEMP_DEG, VAR_T_SNOW[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SWE0[0], UNIT_DEPTH_MM, VAR_SWE0[1], Source_ParameterDB, DT_Single);

    mdi.AddInput(VAR_PET[0], UNIT_DEPTH_MM, VAR_PET[1], Source_Module, DT_Raster1D);  // from PET module
    mdi.AddInput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], Source_Module, DT_Raster1D); // from interception module
    mdi.AddInput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], Source_Module, DT_Raster1D); // from snow melt module
    mdi.AddInput(VAR_SWE[0], UNIT_DEPTH_MM, VAR_SWE[1], Source_Module, DT_Single); // from snow water balance module
    // SNRD should be added as input when snow redistribution module is accomplished. LJ
    //mdi.AddInput(VAR_SNRD[0], UNIT_DEPTH_MM, VAR_SNRD[1], Source_Module, DT_Raster1D);	 // from snow redistribution module
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D); // from interpolation module

    // set the output variables
    mdi.AddOutput(VAR_SNSB[0], UNIT_DEPTH_MM, VAR_SNSB[1], DT_Raster1D);

    // write out the XML file.

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
