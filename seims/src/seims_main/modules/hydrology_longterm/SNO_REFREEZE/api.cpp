#include "api.h"
#include "SnowRefreeze.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SnowRefreeze();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    mdi.SetAuthor("Yujing Wang");
    mdi.SetClass(MCLS_SNOW[0], MCLS_SNOW[1]);
    mdi.SetDescription(M_SNOW_REFREEZE[1]);
    mdi.SetEmail("");
    mdi.SetID(M_SNOW_REFREEZE[0]);
    mdi.SetName(M_SNOW_REFREEZE[0]);
    mdi.SetVersion("1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("SnowRefreeze.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddInput(VAR_SNOW_LIQUID[0], UNIT_DEPTH_MM, VAR_SNOW_LIQUID[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], Source_Module, DT_Raster1D);
    //mdi.AddOutput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
