#include "api.h"
#include "GREL_HBV.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new GREL_HBV();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    mdi.SetAuthor("Yujing Wang");
    mdi.SetClass(MCLS_GLAC[0], MCLS_GLAC[1]);
    mdi.SetDescription(M_GREL_HBV[1]);
    mdi.SetEmail("");
    mdi.SetID(M_GREL_HBV[0]);
    mdi.SetName(M_GREL_HBV[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("GlacierReleaseHBV.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddInput(VAR_GLAC_MELT[0], UNIT_DEPTH_MM, VAR_GLAC_MELT[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], Source_Module, DT_Raster1D);

    mdi.AddParameter(VAR_LANDUSE[0], UNIT_NON_DIM, VAR_LANDUSE[1], Source_ParameterDB, DT_Raster1DInt);
    mdi.AddParameter(VAR_GLAC_STOR_COEF[0], UNIT_PER_DAY, VAR_GLAC_STOR_COEF[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_HBV_GREL_KMIN[0], UNIT_PER_DAY, VAR_HBV_GREL_KMIN[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_HBV_GREL_AG[0], UNIT_PER_MM, VAR_HBV_GREL_AG[1], Source_ParameterDB, DT_Single);

    mdi.AddOutput(VAR_GLAC_REL[0], UNIT_DEPTH_MM, VAR_GLAC_REL[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
