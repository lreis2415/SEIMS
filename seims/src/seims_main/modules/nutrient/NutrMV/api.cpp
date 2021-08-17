#include "api.h"

#include "NutrientMovementViaWater.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new NutrientMovementViaWater();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    mdi.SetAuthor("Huiran Gao, Liangjun Zhu");
    mdi.SetClass(MCLS_NutRemv[0], MCLS_NutRemv[1]);
    mdi.SetDescription(M_NUTRMV[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_NUTRMV[0]);
    mdi.SetName(M_NUTRMV[0]);
    mdi.SetVersion("1.0");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    // set the parameters
    mdi.AddParameter(VAR_CSWAT[0], UNIT_NON_DIM, VAR_CSWAT[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_QTILE[0], UNIT_NON_DIM, VAR_QTILE[1], Source_ParameterDB_Optional, DT_Single);
    mdi.AddParameter(VAR_PHOSKD[0], UNIT_NON_DIM, VAR_PHOSKD[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_PPERCO[0], UNIT_NON_DIM, VAR_PPERCO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NPERCO[0], UNIT_NON_DIM, VAR_NPERCO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ISEP_OPT[0], UNIT_NON_DIM, VAR_ISEP_OPT[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_COD_N[0], UNIT_NON_DIM, VAR_COD_N[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_COD_K[0], UNIT_NON_DIM, VAR_COD_K[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SOL_UL[0], UNIT_DEPTH_MM, VAR_SOL_UL[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_ANION_EXCL[0], UNIT_NON_DIM, VAR_ANION_EXCL[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_CRK[0], UNIT_NON_DIM, VAR_SOL_CRK[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(VAR_SOILDEPTH[0], UNIT_DEPTH_MM, VAR_SOILDEPTH[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DISTSTREAM[0], UNIT_LEN_M, VAR_DISTSTREAM[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_FLOWOUT_INDEX_D8[0], UNIT_NON_DIM, Tag_FLOWOUT_INDEX_D8[1], Source_ParameterDB, DT_Array1D);

    mdi.AddParameter(VAR_SOL_NO3[0], UNIT_CONT_KGHA, VAR_SOL_NO3[1], Source_Module, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SOLP[0], UNIT_CONT_KGHA, VAR_SOL_SOLP[1], Source_Module, DT_Raster2D);

    mdi.AddParameter(VAR_SOL_CBN[0], UNIT_PERCENT, VAR_SOL_CBN[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_BD[0], UNIT_DENSITY, VAR_SOL_BD[1], Source_ParameterDB, DT_Raster2D);

    // parameters for subbasin sum
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SUBBASIN_PARAM[0], UNIT_NON_DIM, VAR_SUBBASIN_PARAM[1], Source_ParameterDB, DT_Subbasin);
    mdi.AddParameter(VAR_STREAM_LINK[0], UNIT_NON_DIM, VAR_STREAM_LINK[1], Source_ParameterDB, DT_Raster1D);
    // for subsurface routing
    mdi.AddParameter(Tag_ROUTING_LAYERS[0], UNIT_NON_DIM, Tag_ROUTING_LAYERS[1], Source_ParameterDB, DT_Array2D);
    // set input from other modules
    //mdi.AddInput(VAR_WSHD_PLCH[0], UNIT_CONT_KGHA, VAR_WSHD_PLCH[1], Source_Module, DT_Single);
    //surface related inputs
    mdi.AddInput(VAR_OLFLOW[0], UNIT_DEPTH_MM, VAR_OLFLOW[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SEDYLD[0], UNIT_KG, VAR_SEDYLD[1], Source_Module, DT_Raster1D);
    //soil related inputs
    mdi.AddInput(VAR_PERCO[0], UNIT_DEPTH_MM, VAR_PERCO[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SSRU[0], UNIT_DEPTH_MM, VAR_SSRU[1], Source_Module, DT_Raster2D);
    //groundwater related inputs
    //mdi.AddInput(VAR_GW_Q[0], UNIT_DEPTH_MM, VAR_GW_Q[1], Source_Module, DT_Raster1D);

    mdi.AddInput(VAR_SEDORGN[0], UNIT_CONT_KGHA, VAR_SEDORGN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);

    mdi.AddInput(VAR_SEDLOSS_C[0], UNIT_CONT_KGHA, VAR_SEDLOSS_C[1], Source_Module_Optional, DT_Raster1D);

    mdi.AddInput(VAR_CONV_WT[0], UNIT_NON_DIM, VAR_CONV_WT[1], Source_Module, DT_Raster2D);
    ///////////////////////////////////////////////////////////////
    // set the output variables
    mdi.AddOutput(VAR_WSHD_PLCH[0], UNIT_CONT_KGHA, VAR_WSHD_PLCH[1], DT_Single);
    //surface related
    mdi.AddOutput(VAR_SUR_NO3[0], UNIT_CONT_KGHA, VAR_SUR_NO3[1], DT_Raster1D);
    mdi.AddOutput(VAR_SUR_NH4[0], UNIT_CONT_KGHA, VAR_SUR_NH4[1], DT_Raster1D);
    mdi.AddOutput(VAR_SUR_SOLP[0], UNIT_CONT_KGHA, VAR_SUR_SOLP[1], DT_Raster1D);
    mdi.AddOutput(VAR_SUR_COD[0], UNIT_CONT_KGHA, VAR_SUR_COD[1], DT_Raster1D);
    mdi.AddOutput(VAR_CHL_A[0], UNIT_CONCENTRATION, VAR_CHL_A[1], DT_Raster1D);
    mdi.AddOutput(VAR_LATNO3[0], UNIT_CONT_KGHA, VAR_LATNO3[1], DT_Raster1D);
    //to groundwater
    mdi.AddOutput(VAR_PERCO_N_GW[0], UNIT_KG, VAR_PERCO_N_GW[1], DT_Array1D);
    mdi.AddOutput(VAR_PERCO_P_GW[0], UNIT_KG, VAR_PERCO_P_GW[1], DT_Array1D);
    //to channel
    mdi.AddOutput(VAR_LATNO3_TOCH[0], UNIT_KG, VAR_LATNO3_TOCH[1], DT_Array1D);
    mdi.AddOutput(VAR_SUR_NO3_TOCH[0], UNIT_KG, VAR_SUR_NO3_TOCH[1], DT_Array1D);
    mdi.AddOutput(VAR_SUR_NH4_TOCH[0], UNIT_KG, VAR_SUR_NH4_TOCH[1], DT_Array1D);
    mdi.AddOutput(VAR_SUR_SOLP_TOCH[0], UNIT_KG, VAR_SUR_SOLP_TOCH[1], DT_Array1D);
    mdi.AddOutput(VAR_SUR_COD_TOCH[0], UNIT_KG, VAR_SUR_COD_TOCH[1], DT_Array1D);

    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
