#include "api.h"
#include "PMELT_DD.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new PMELT_DD();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    mdi.SetAuthor("Yujing Wang");
    mdi.SetClass(MCLS_GLAC[0], MCLS_GLAC[1]);
    mdi.SetDescription(M_PMELT_DD[1]);
    mdi.SetEmail("");
    mdi.SetID(M_PMELT_DD[0]);
    mdi.SetName(M_PMELT_DD[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("PMELT_DD.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddParameter(VAR_MELT_FACTOR[0], UNIT_NON_DIM, VAR_MELT_FACTOR[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_T0[0], UNIT_TEMP_DEG, VAR_T0[1], Source_ParameterDB, DT_Single);

    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_POTENTIAL_MELT[0], UNIT_DEPTH_MM, VAR_POTENTIAL_MELT[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}