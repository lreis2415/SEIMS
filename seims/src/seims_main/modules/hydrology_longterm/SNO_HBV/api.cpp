#include "api.h"
#include "SNO_HBV.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SNO_HBV();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    mdi.SetAuthor("Your Name");
    mdi.SetClass(MCLS_SNOW[0], MCLS_SNOW[1]);
    mdi.SetDescription(M_SNO_HBV[1]);
    mdi.SetEmail("");
    mdi.SetID(M_SNO_HBV[0]);
    mdi.SetName(M_SNO_HBV[0]);
    mdi.SetVersion("1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("SnowMeltSimple.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddInput(VAR_POTENTIAL_MELT[0], UNIT_DEPTH_MM, VAR_POTENTIAL_MELT[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SNOWFALL[0], UNIT_DEPTH_MM, VAR_SNOWFALL[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMEAN[0], UNIT_DEPTH_MM, VAR_TMEAN[1], Source_Module, DT_Raster1D);

    mdi.AddParameter(VAR_SNOW_REFREEZE_FACTOR[0], UNIT_NON_DIM, VAR_SNOW_REFREEZE_FACTOR[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_T0[0], UNIT_NON_DIM, VAR_T0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SWI[0], UNIT_NON_DIM, VAR_SWI[1], Source_ParameterDB, DT_Single);

    mdi.AddOutput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], DT_Raster1D);
    mdi.AddOutput(VAR_SNME[0], UNIT_DEPTH_MM, VAR_SNME[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
