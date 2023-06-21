#include "api.h"

#include "SOL_WB.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SOL_WB();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    mdi.SetAuthor("Chunping Ou, Liangjun Zhu");
    mdi.SetClass(MCLS_SOIL[0], MCLS_SOIL[1]);
    mdi.SetDescription(M_SOL_WB[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_SOL_WB[0]);
    mdi.SetName(M_SOL_WB[0]);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_SOL_ZMX[0], UNIT_DEPTH_MM, VAR_SOL_ZMX[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1DInt);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster1D);
    /// add DT_Subbasin
    mdi.AddParameter(VAR_SUBBASIN_PARAM[0], UNIT_NON_DIM, VAR_SUBBASIN_PARAM[1], Source_ParameterDB, DT_Subbasin);

    mdi.AddInput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INFIL[0], UNIT_DEPTH_MM, VAR_INFIL[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOET[0], UNIT_DEPTH_MM, VAR_SOET[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_REVAP[0], UNIT_DEPTH_MM, VAR_REVAP[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SSRU[0], UNIT_DEPTH_MM, VAR_SSRU[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_PERCO[0], UNIT_DEPTH_MM, VAR_PERCO[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);
    //variables used to output
    mdi.AddInput(VAR_PCP[0], UNIT_DEPTH_MM, VAR_PCP[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INLO[0], UNIT_DEPTH_MM, VAR_INLO[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DPST[0], UNIT_DEPTH_MM, VAR_DPST[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INET[0], UNIT_DEPTH_MM, VAR_INET[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DEET[0], UNIT_DEPTH_MM, VAR_DEET[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_RG[0], UNIT_DEPTH_MM, VAR_RG[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SNSB[0], UNIT_DEPTH_MM, VAR_SNSB[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOTE[0], UNIT_TEMP_DEG, VAR_SOTE[1], Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_SOWB[0], UNIT_DEPTH_MM, VAR_SOWB[1], DT_Array2D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
