#include "api.h"
#include "SNO_SIMPLE.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SNO_SIMPLE();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    mdi.SetAuthor("Your Name");
    mdi.SetClass(MCLS_SNOW[0], MCLS_SNOW[1]);
    mdi.SetDescription(M_SNO_SIMPLE[1]);
    mdi.SetEmail("");
    mdi.SetID(M_SNO_SIMPLE[0]);
    mdi.SetName(M_SNO_SIMPLE[0]);
    mdi.SetVersion("1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("SnowMeltSimple.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddInput(VAR_POTENTIAL_MELT[0], UNIT_DEPTH_MM, VAR_POTENTIAL_MELT[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_SNME[0], UNIT_DEPTH_MM, VAR_SNME[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
