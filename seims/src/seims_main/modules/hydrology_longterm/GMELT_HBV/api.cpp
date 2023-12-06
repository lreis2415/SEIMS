#include "api.h"
#include "GMELT_HBV.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new GMELT_HBV();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    mdi.SetAuthor("Yujing Wang");
    mdi.SetClass(MCLS_GLAC[0], MCLS_GLAC[1]);
    mdi.SetDescription(M_GMELT_HBV[1]);
    mdi.SetEmail("");
    mdi.SetID(M_GMELT_HBV[0]);
    mdi.SetName(M_GMELT_HBV[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("GMELT_HBV.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddInput(VAR_POTENTIAL_MELT[0], UNIT_DEPTH_MM, VAR_POTENTIAL_MELT[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], Source_Module, DT_Raster1D);
    mdi.AddParameter(VAR_LANDUSE[0], UNIT_NON_DIM, VAR_LANDUSE[1], Source_ParameterDB, DT_Raster1DInt);
    mdi.AddParameter(VAR_HBV_GLAC_MELT_CORR[0], UNIT_NON_DIM, VAR_HBV_GLAC_MELT_CORR[1], Source_ParameterDB, DT_Single);

    mdi.AddOutput(VAR_GLAC_MELT[0], UNIT_DEPTH_MM, VAR_GLAC_MELT[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
