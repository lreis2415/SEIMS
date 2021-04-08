#include "api.h"

#include "NutrientinGroundwater.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new NutrientinGroundwater();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    mdi.SetAuthor("Huiran Gao; Liangjun Zhu");
    mdi.SetClass(MCLS_NUTRGW[0], MCLS_NUTRGW[1]);
    mdi.SetDescription(M_NUTRGW[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_NUTRGW[0]);
    mdi.SetName(M_NUTRGW[0]);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    // set the parameters
    mdi.AddParameter(Tag_TimeStep[0], UNIT_SECOND, Tag_TimeStep[1], File_Config, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_Single);
    // parameters for subbasin sum
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SUBBASIN_PARAM[0], UNIT_NON_DIM, VAR_SUBBASIN_PARAM[1], Source_ParameterDB, DT_Subbasin);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_GW0[0], UNIT_DEPTH_MM, VAR_GW0[1], Source_ParameterDB, DT_Single);
    // add reach information
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Reach);
    // set the input from other modules
    mdi.AddInput(VAR_SBQG[0], UNIT_FLOW_CMS, VAR_SBQG[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBGS[0], UNIT_DEPTH_MM, VAR_SBGS[1], Source_Module, DT_Array1D);

    mdi.AddInput(VAR_SOL_NO3[0], UNIT_CONT_KGHA, VAR_SOL_NO3[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_SOLP[0], UNIT_CONT_KGHA, VAR_SOL_SOLP[1], Source_Module, DT_Raster2D);

    mdi.AddInput(VAR_PERCO_N_GW[0], UNIT_KG, VAR_PERCO_N_GW[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_PERCO_P_GW[0], UNIT_KG, VAR_PERCO_P_GW[1], Source_Module, DT_Array1D);

    // set the output variables
    mdi.AddOutput(VAR_GWNO3_CONC[0], UNIT_CONCENTRATION, VAR_GWNO3_CONC[1], DT_Array1D);
    mdi.AddOutput(VAR_GWSOLP_CONC[0], UNIT_CONCENTRATION, VAR_GWSOLP_CONC[1], DT_Array1D);
    mdi.AddOutput(VAR_NO3GW_TOCH[0], UNIT_KG, VAR_NO3GW_TOCH[1], DT_Array1D);
    mdi.AddOutput(VAR_MINPGW_TOCH[0], UNIT_KG, VAR_MINPGW_TOCH[1], DT_Array1D);
    mdi.AddOutput(VAR_GWNO3[0], UNIT_KG, VAR_GWNO3[1], DT_Array1D);
    mdi.AddOutput(VAR_GWSOLP[0], UNIT_KG, VAR_GWSOLP[1], DT_Array1D);

    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
