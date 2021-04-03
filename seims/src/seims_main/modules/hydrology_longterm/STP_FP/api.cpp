#include "api.h"

#include "SoilTemperatureFINPL.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SoilTemperatureFINPL();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu, Liangjun Zhu");
    mdi.SetClass(MCLS_SOLT[0], MCLS_SOLT[1]);
    mdi.SetDescription(M_STP_FP[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_STP_FP[0]);
    mdi.SetName(M_STP_FP[0]);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    /// from parameter database
    mdi.AddParameter(VAR_SOL_TA0[0], UNIT_NON_DIM, VAR_SOL_TA0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TA1[0], UNIT_NON_DIM, VAR_SOL_TA1[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TA2[0], UNIT_NON_DIM, VAR_SOL_TA2[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TA3[0], UNIT_NON_DIM, VAR_SOL_TA3[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TB1[0], UNIT_NON_DIM, VAR_SOL_TB1[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TB2[0], UNIT_NON_DIM, VAR_SOL_TB2[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TD1[0], UNIT_NON_DIM, VAR_SOL_TD1[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TD2[0], UNIT_NON_DIM, VAR_SOL_TD2[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_SOIL10[0], UNIT_NON_DIM, VAR_K_SOIL10[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SOIL_T10[0], UNIT_NON_DIM, VAR_SOIL_T10[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_LANDUSE[0], UNIT_NON_DIM, VAR_LANDUSE[1], Source_ParameterDB, DT_Raster1D);
    /// mean air temperature
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);

    /// output soil temperature
    mdi.AddOutput(VAR_SOTE[0], UNIT_TEMP_DEG, VAR_SOTE[1], DT_Raster1D);
    mdi.AddOutput(VAR_TMEAN1[0], UNIT_TEMP_DEG, VAR_TMEAN1[1], DT_Raster1D); /// mean air temperature of the day(d-1)
    mdi.AddOutput(VAR_TMEAN2[0], UNIT_TEMP_DEG, VAR_TMEAN2[1], DT_Raster1D); /// mean air temperature of the day(d-2)

    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
