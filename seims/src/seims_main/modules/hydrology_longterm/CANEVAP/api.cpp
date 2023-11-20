#include "api.h"
#include "CanopyEvaporation.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new CanopyEvaporation();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    mdi.SetAuthor("Yujing Wang");
    mdi.SetClass(MCLS_INTERC[0], MCLS_INTERC[1]);
    mdi.SetDescription(M_CAN_EVAP[1]);
    mdi.SetEmail("");
    mdi.SetID(M_CAN_EVAP[0]);
    mdi.SetName(M_CAN_EVAP[0]);
    mdi.SetVersion("1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("CanopyEvaporation.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddInput(VAR_CANSTOR[0], UNIT_DEPTH_MM, VAR_CANSTOR[1], Source_Module, DT_Raster1D);
    mdi.AddOutput(VAR_CANEVAP[0], UNIT_DEPTH_MM, VAR_CANEVAP[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
