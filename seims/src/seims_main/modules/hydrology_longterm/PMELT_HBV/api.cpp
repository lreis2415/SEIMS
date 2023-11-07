#include "api.h"
#include "PMELT_HBV.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new PMELT_HBV();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    mdi.SetAuthor("Yujing Wang");
    mdi.SetClass(MCLS_GLAC[0], MCLS_GLAC[1]);
    mdi.SetDescription(M_PMELT_HBV[1]);
    mdi.SetEmail("");
    mdi.SetID(M_PMELT_HBV[0]);
    mdi.SetName(M_PMELT_HBV[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("PMELT_HBV.chm");

    mdi.AddParameter(VAR_MA[0], UNIT_NON_DIM, VAR_MA[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MA_MIN[0], UNIT_NON_DIM, VAR_MA_MIN[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AM[0], UNIT_NON_DIM, VAR_AM[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MRF[0], UNIT_NON_DIM, VAR_MRF[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FC[0], UNIT_NON_DIM, VAR_FC[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MELT_TEMP[0], UNIT_TEMP_DEG, VAR_MELT_TEMP[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SLOPE_CORR[0], UNIT_NON_DIM, VAR_SLOPE_CORR[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RAIN_ENERGY[0], UNIT_ENERGY, VAR_RAIN_ENERGY[1], Source_ParameterDB, DT_Single);

    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_POTENTIAL_MELT[0], UNIT_DEPTH_MM, VAR_POTENTIAL_MELT[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}