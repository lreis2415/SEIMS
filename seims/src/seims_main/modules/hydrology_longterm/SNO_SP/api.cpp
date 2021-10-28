#include "api.h"

#include "SNO_SP.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SNO_SP();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Zhiqiang Yu, Liangjun Zhu");
    mdi.SetClass(MCLS_SNOW[0], MCLS_SNOW[1]);
    mdi.SetDescription(M_SNO_SP[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_SNO_SP[0]);
    mdi.SetName(M_SNO_SP[0]);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    mdi.AddParameter(VAR_T0[0], UNIT_TEMP_DEG, VAR_T0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_BLOW[0], UNIT_NON_DIM, VAR_K_BLOW[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T_SNOW[0], UNIT_TEMP_DEG, VAR_T_SNOW[1], Source_ParameterDB, DT_Single);
    // I don't think VAR_SWE0[0] is useful. By LJ
    // mdi.AddParameter(VAR_SWE0[0], UNIT_DEPTH_MM, VAR_SWE0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LAG_SNOW[0], UNIT_NON_DIM, VAR_LAG_SNOW[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_C_SNOW6[0], UNIT_MELT_FACTOR, VAR_C_SNOW6[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_C_SNOW12[0], UNIT_MELT_FACTOR, VAR_C_SNOW12[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SNOCOVMX[0], UNIT_DEPTH_MM, VAR_SNOCOVMX[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SNO50COV[0], UNIT_NON_DIM, VAR_SNO50COV[1], Source_ParameterDB, DT_Single);
    // Net precipitation is updated after snow accumulation.
    mdi.AddInput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], Source_Module, DT_Raster1D); // from interception module
    //TODO: SNAC is used as unknown variable in the execute() funtion, so why it here as Input, otherwise in Output? By LJ
    //mdi.AddInput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], Source_Module,DT_Raster1D);	 // from snow water balance module
    // I think VAR_SWE[0] is useless either. By LJ
    // mdi.AddInput(VAR_SWE[0], UNIT_DEPTH_MM, VAR_SWE[1], Source_Module, DT_Single);  // from snow water balance module
    // TODO: SNRD currently have not been implemented, therefore initialized as zero. By LJ
    // mdi.AddInput(VAR_SNRD[0], UNIT_DEPTH_MM, VAR_SNRD[1], Source_Module, DT_Raster1D); // from snow redistribution module
    // Snow sublimation will be considered in actual evpotranspiration module (AET_PTH), so no need to set as Input. By LJ
    // mdi.AddInput(VAR_SNSB[0], UNIT_DEPTH_MM, VAR_SNSB[1], Source_Module, DT_Raster1D); //from snow sublimation module
    mdi.AddInput(VAR_TMAX[0], UNIT_TEMP_DEG, VAR_TMAX[1], Source_Module, DT_Raster1D); // from interpolation module
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_SNME[0], UNIT_DEPTH_MM, VAR_SNME[1], DT_Raster1D);
    mdi.AddOutput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], DT_Raster1D);

    // write out the XML file.
    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
