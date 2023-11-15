#include "api.h"
#include "RainSnowHBV.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new RainSnowHBV();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    mdi.SetAuthor("Yujing Wang");
    mdi.SetClass(MCLS_CLIMATE[0], MCLS_CLIMATE[1]);
    mdi.SetDescription(M_RAINSNOW_HBV[1]);
    mdi.SetEmail("");
    mdi.SetID(M_RAINSNOW_HBV[0]);
    mdi.SetName(M_RAINSNOW_HBV[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("RAINSNOW_HBV.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PCP[0], UNIT_DEPTH_MM, VAR_PCP[1], Source_Module, DT_Raster1D);

    mdi.AddParameter(VAR_T_SNOW[0], UNIT_TEMP_DEG, VAR_T_SNOW[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T_RAIN_SNOW_DELTA[0], UNIT_TEMP_DEG, VAR_T_RAIN_SNOW_DELTA[1], Source_ParameterDB, DT_Single);

    mdi.AddOutput(VAR_SNOW_LIQUID[0], UNIT_DEPTH_MM, VAR_SNOW_LIQUID[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
