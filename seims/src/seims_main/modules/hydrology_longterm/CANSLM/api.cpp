#include "api.h"
#include "CanopySublimation.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new CanopySublimation();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    mdi.SetAuthor("Yujing Wang");
    mdi.SetClass(MCLS_EVAP[0], MCLS_EVAP[1]);
    mdi.SetDescription(M_CANSLM[1]);
    mdi.SetEmail("");
    mdi.SetID(M_CANSLM[0]);
    mdi.SetName(M_CANSLM[0]);
    mdi.SetVersion("1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("CanopySublimation.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddInput(VAR_CANSTOR_SNOW[0], UNIT_DEPTH_MM, VAR_CANSTOR_SNOW[1], Source_Module, DT_Raster1D);
    mdi.AddOutput(VAR_CANSLM[0], UNIT_DEPTH_MM, VAR_CANSLM[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
