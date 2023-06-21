#include "api.h"

#include "SNO_DD.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new SNO_DD();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Chunping Ou");
    mdi.SetClass(MCLS_SNOW[0], MCLS_SNOW[1]);
    mdi.SetDescription(M_SNO_DD[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_SNO_DD[0]);
    mdi.SetName(M_SNO_DD[0]);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("SNO_DD.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddParameter(VAR_C_SNOW[0], UNIT_TEMP_FACTOR, VAR_C_SNOW[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_C_RAIN[0], UNIT_TEMP_FACTOR, VAR_C_RAIN[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T0[0], UNIT_TEMP_DEG, VAR_T0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T_SNOW[0], UNIT_TEMP_DEG, VAR_T_SNOW[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_BLOW[0], UNIT_NON_DIM, VAR_K_BLOW[1], Source_ParameterDB, DT_Single);
    //mdi.AddParameter(VAR_SWE0[0], UNIT_DEPTH_MM, VAR_SWE0[1], Source_ParameterDB, DT_Single);

    mdi.AddInput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], Source_Module, DT_Raster1D); // from interception module
    //mdi.AddInput(VAR_SNSB[0], UNIT_DEPTH_MM, VAR_SNSB[1], Source_Module, DT_Raster1D); //from snow sublimation module
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], DT_Raster1D);
    mdi.AddOutput(VAR_SNME[0], UNIT_DEPTH_MM, VAR_SNME[1], DT_Raster1D);
    mdi.AddOutput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], DT_Raster1D);

    // write out the XML file.
    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
