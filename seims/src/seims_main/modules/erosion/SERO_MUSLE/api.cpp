#include "api.h"

#include "SoilErosion_MUSLE.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SERO_MUSLE();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    // set the information properties
    mdi.SetAuthor("Liangjun Zhu, Zhiqiang Yu");
    mdi.SetClass(MCLS_OL_EROSION[0], MCLS_OL_EROSION[1]);
    mdi.SetDescription(M_SERO_MUSLE[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_SERO_MUSLE[0]);
    mdi.SetName(M_SERO_MUSLE[0]);
    mdi.SetVersion("1.4");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ROCK[0], UNIT_PERCENT, VAR_ROCK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_USLE_K[0], UNIT_NON_DIM, VAR_USLE_K[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_USLE_P[0], UNIT_NON_DIM, VAR_USLE_P[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_ACC[0], UNIT_NON_DIM, VAR_ACC[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SLOPE[0], UNIT_PERCENT, VAR_SLOPE[1], Source_ParameterDB, DT_Raster1D);
    // mdi.AddParameter(VAR_SLPLEN[0], UNIT_LEN_M, VAR_SLPLEN[1], Source_ParameterDB_Optional, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK[0], UNIT_NON_DIM, VAR_STREAM_LINK[1], Source_ParameterDB, DT_Raster1DInt);

    mdi.AddParameter(VAR_DETACH_SAND[0], UNIT_NON_DIM, VAR_DETACH_SAND[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DETACH_SILT[0], UNIT_NON_DIM, VAR_DETACH_SILT[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DETACH_CLAY[0], UNIT_NON_DIM, VAR_DETACH_CLAY[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DETACH_SAG[0], UNIT_NON_DIM, VAR_DETACH_SAG[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DETACH_LAG[0], UNIT_NON_DIM, VAR_DETACH_LAG[1], Source_ParameterDB, DT_Raster1D);

    // C-Factor related
    mdi.AddParameter(VAR_ICFAC[0], UNIT_NON_DIM, VAR_ICFAC[1], Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_USLE_C[0], UNIT_NON_DIM, VAR_USLE_C[1], Source_ParameterDB, DT_Raster1D);
    // Update USLE_C factor by average minimum C factor for the land cover (icfac = 0)
    mdi.AddParameter(VAR_LANDCOVER[0], UNIT_NON_DIM, VAR_LANDCOVER[1], Source_ParameterDB, DT_Raster1DInt);
    mdi.AddInput(VAR_SOL_COV[0], UNIT_CONT_KGHA, VAR_SOL_COV[1], Source_Module_Optional, DT_Raster1D);
    // Update USLE_C factor by the new calculation method from RUSLE without the ave. min. C (icfac = 1)
    mdi.AddParameter(VAR_RSDCOV_COEF[0], UNIT_NON_DIM, VAR_RSDCOV_COEF[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CHT[0], UNIT_LEN_M, VAR_CHT[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddInput(VAR_LAIDAY[0], UNIT_AREA_RATIO, VAR_LAIDAY[1], Source_Module_Optional, DT_Raster1D);

    //input from other module
    mdi.AddInput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_USLE_L[0], UNIT_NON_DIM, VAR_USLE_L[1], DT_Raster1D);
    mdi.AddOutput(VAR_USLE_S[0], UNIT_NON_DIM, VAR_USLE_S[1], DT_Raster1D);
    mdi.AddOutput(VAR_USLE_C[0], UNIT_NON_DIM, VAR_USLE_C[1], DT_Raster1D);

    mdi.AddOutput(VAR_SOER[0], UNIT_KG, VAR_SOER[1], DT_Raster1D);
    mdi.AddOutput(VAR_SANDYLD[0], UNIT_KG, VAR_SANDYLD[1], DT_Raster1D);
    mdi.AddOutput(VAR_SILTYLD[0], UNIT_KG, VAR_SILTYLD[1], DT_Raster1D);
    mdi.AddOutput(VAR_CLAYYLD[0], UNIT_KG, VAR_CLAYYLD[1], DT_Raster1D);
    mdi.AddOutput(VAR_SAGYLD[0], UNIT_KG, VAR_SAGYLD[1], DT_Raster1D);
    mdi.AddOutput(VAR_LAGYLD[0], UNIT_KG, VAR_LAGYLD[1], DT_Raster1D);

    // write out the XML file
    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
