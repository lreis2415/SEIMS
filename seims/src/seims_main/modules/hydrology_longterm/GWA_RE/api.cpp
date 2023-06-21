#include "api.h"

#include "ReservoirMethod.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new ReservoirMethod();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Wu Hui; Zhiqiang Yu; Liang-Jun Zhu");
    mdi.SetClass(MCLS_GW[0], MCLS_GW[1]);
    mdi.SetDescription(M_GWA_RE[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("GWA_RE.chm");
    mdi.SetID(M_GWA_RE[0]);
    mdi.SetName(M_GWA_RE[0]);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddParameter(Tag_TimeStep[0], UNIT_SECOND, Tag_TimeStep[1], File_Config, DT_SingleInt);
    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_SingleInt);
    /// add DT_Subbasin
    mdi.AddParameter(VAR_SUBBASIN_PARAM[0], UNIT_NON_DIM, VAR_SUBBASIN_PARAM[1], Source_ParameterDB, DT_Subbasin);

    mdi.AddParameter(VAR_GW0[0], UNIT_DEPTH_MM, VAR_GW0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_GWMAX[0], UNIT_DEPTH_MM, VAR_GWMAX[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DF_COEF[0], UNIT_NON_DIM, VAR_DF_COEF[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_KG[0], UNIT_NON_DIM, VAR_KG[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_Base_ex[0], UNIT_NON_DIM, VAR_Base_ex[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SOILDEPTH[0], UNIT_DEPTH_MM, VAR_SOILDEPTH[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1DInt);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SLOPE[0], UNIT_PERCENT, VAR_SLOPE[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddInput(VAR_INET[0], UNIT_DEPTH_MM, VAR_INET[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DEET[0], UNIT_DEPTH_MM, VAR_DEET[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOET[0], UNIT_DEPTH_MM, VAR_SOET[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_AET_PLT[0], UNIT_DEPTH_MM, VAR_AET_PLT[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PET[0], UNIT_DEPTH_MM, VAR_PET[1], Source_Module, DT_Raster1D);
    // VAR_GWNEW[0] is OPTIONALLY from IUH_CH or other channel routing module
    mdi.AddInput(VAR_GWNEW[0], UNIT_DEPTH_MM, VAR_GWNEW[1], Source_Module_Optional, DT_Array1D);
    // VAR_PERCO[0] is from percolation modules
    mdi.AddInput(VAR_PERCO[0], UNIT_DEPTH_MM, VAR_PERCO[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);

    mdi.AddOutput(VAR_GWWB[0], UNIT_NON_DIM, DESC_NONE, DT_Array2D);
    mdi.AddOutput(VAR_REVAP[0], UNIT_DEPTH_MM, VAR_REVAP[1], DT_Raster1D);              //used by soil water balance module
    mdi.AddOutput(VAR_RG[0], UNIT_DEPTH_MM, VAR_RG[1], DT_Array1D);     //used by soil water balance module
    mdi.AddOutput(VAR_SBQG[0], UNIT_FLOW_CMS, VAR_SBQG[1], DT_Array1D); //used by channel flow routing module
    mdi.AddOutput(VAR_SBPET[0], UNIT_DEPTH_MM, VAR_SBPET[1], DT_Array1D);
    mdi.AddOutput(VAR_SBGS[0], UNIT_DEPTH_MM, VAR_SBGS[1], DT_Array1D);

    res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
