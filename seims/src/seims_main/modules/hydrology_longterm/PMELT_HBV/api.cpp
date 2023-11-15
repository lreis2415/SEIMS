#include "api.h"
#include "PMELT_HBV.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new PMELT_HBV();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    mdi.SetAuthor("Yujing Wang");
    mdi.SetClass(MCLS_GLAC[0], MCLS_GLAC[1]);
    mdi.SetDescription(M_PMELT_HBV[1]);
    mdi.SetEmail("");
    mdi.SetID(M_PMELT_HBV[0]);
    mdi.SetName(M_PMELT_HBV[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("PMELT_HBV.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);
    
    mdi.AddParameter(VAR_CELL_LAT[0], UNIT_NON_DIM, VAR_CELL_LAT[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MELT_FACTOR[0], UNIT_NON_DIM, VAR_MELT_FACTOR[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MIN_MELT_FACTOR[0], UNIT_NON_DIM, VAR_MIN_MELT_FACTOR[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_HBV_MELT_FOREST_CORR[0], UNIT_NON_DIM, VAR_HBV_MELT_FOREST_CORR[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_HBV_MELT_ASPECT_CORR[0], UNIT_NON_DIM, VAR_HBV_MELT_ASPECT_CORR[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LANDUSE[0], UNIT_NON_DIM, VAR_LANDUSE[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_FOREST_COV[0], UNIT_NON_DIM, VAR_FOREST_COV[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_T0[0], UNIT_TEMP_DEG, VAR_T0[1], Source_ParameterDB, DT_Single);
    
    mdi.AddParameter(VAR_SLOPE[0], UNIT_NON_DIM, VAR_SLOPE[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_ASPECT[0], UNIT_NON_DIM, VAR_ASPECT[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddOutput(VAR_POTENTIAL_MELT[0], UNIT_DEPTH_MM, VAR_POTENTIAL_MELT[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
