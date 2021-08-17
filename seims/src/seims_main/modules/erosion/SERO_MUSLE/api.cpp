#include "api.h"

#include "SoilErosion_MUSLE.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SERO_MUSLE();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Liangjun Zhu, Zhiqiang Yu");
    mdi.SetClass(MCLS_OL_EROSION, MCLSDESC_OL_EROSION);
    mdi.SetDescription(MDESC_SERO_MUSLE);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_SERO_MUSLE);
    mdi.SetName(MID_SERO_MUSLE);
    mdi.SetVersion("1.3");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ROCK, UNIT_PERCENT, DESC_ROCK, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_USLE_K, UNIT_NON_DIM, DESC_USLE_K, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_USLE_P, UNIT_NON_DIM, DESC_USLE_P, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_ACC, UNIT_NON_DIM, DESC_ACC, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SLOPE, UNIT_PERCENT, DESC_SLOPE, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SLPLEN, UNIT_LEN_M, DESC_SLPLEN, Source_ParameterDB_Optional, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK, UNIT_NON_DIM, DESC_STREAM_LINK, Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(VAR_DETACH_SAND, UNIT_NON_DIM, DESC_DETACH_SAND, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DETACH_SILT, UNIT_NON_DIM, DESC_DETACH_SILT, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DETACH_CLAY, UNIT_NON_DIM, DESC_DETACH_CLAY, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DETACH_SAG, UNIT_NON_DIM, DESC_DETACH_SAG, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DETACH_LAG, UNIT_NON_DIM, DESC_DETACH_LAG, Source_ParameterDB, DT_Raster1D);

    // C-Factor related
    mdi.AddParameter(VAR_ICFAC, UNIT_NON_DIM, DESC_ICFAC, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_USLE_C, UNIT_NON_DIM, DESC_USLE_C, Source_ParameterDB, DT_Raster1D);
    // Update USLE_C factor by average minimum C factor for the land cover (icfac = 0)
    mdi.AddParameter(VAR_LANDCOVER, UNIT_NON_DIM, DESC_LANDCOVER, Source_ParameterDB, DT_Raster1D);
    mdi.AddInput(VAR_SOL_COV, UNIT_CONT_KGHA, DESC_SOL_COV, Source_Module_Optional, DT_Raster1D);
    // Update USLE_C factor by the new calculation method from RUSLE without the ave. min. C (icfac = 1)
    mdi.AddParameter(VAR_RSDCOV_COEF, UNIT_NON_DIM, DESC_RSDCOV_COEF, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CHT, UNIT_LEN_M, DESC_CHT, Source_ParameterDB, DT_Raster1D);
    mdi.AddInput(VAR_LAIDAY, UNIT_AREA_RATIO, DESC_LAIDAY, Source_Module_Optional, DT_Raster1D);

    //input from other module
    mdi.AddInput(VAR_SURU, UNIT_DEPTH_MM, DESC_SURU, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SNAC, UNIT_DEPTH_MM, DESC_SNAC, Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_USLE_L, UNIT_NON_DIM, DESC_USLE_L, DT_Raster1D);
    mdi.AddOutput(VAR_USLE_S, UNIT_NON_DIM, DESC_USLE_S, DT_Raster1D);
    mdi.AddOutput(VAR_USLE_C, UNIT_NON_DIM, DESC_USLE_C, DT_Raster1D);
    mdi.AddOutput(VAR_USLE_K, UNIT_NON_DIM, DESC_USLE_K, DT_Raster2D);

    mdi.AddOutput(VAR_SOER, UNIT_KG, DESC_SOER, DT_Raster1D);
    mdi.AddOutput(VAR_SANDYLD, UNIT_KG, DESC_SANDYLD, DT_Raster1D);
    mdi.AddOutput(VAR_SILTYLD, UNIT_KG, DESC_SILTYLD, DT_Raster1D);
    mdi.AddOutput(VAR_CLAYYLD, UNIT_KG, DESC_CLAYYLD, DT_Raster1D);
    mdi.AddOutput(VAR_SAGYLD, UNIT_KG, DESC_SAGYLD, DT_Raster1D);
    mdi.AddOutput(VAR_LAGYLD, UNIT_KG, DESC_LAGYLD, DT_Raster1D);

    // write out the XML file
    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
