#include "api.h"

#include "NutrCH_QUAL2E.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new NutrCH_QUAL2E();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    mdi.SetAuthor("Huiran Gao, Liangjun Zhu");
    mdi.SetClass(MCLS_NutCHRout[0], MCLS_NutCHRout[1]);
    mdi.SetDescription(M_NUTRCH_QUAL2E[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_NUTRCH_QUAL2E[0]);
    mdi.SetName(M_NUTRCH_QUAL2E[0]);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    // set the parameters
    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_ChannelTimeStep, UNIT_SECOND, Tag_TimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(VAR_RNUM1[0], UNIT_NON_DIM, VAR_RNUM1[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_IGROPT[0], UNIT_NON_DIM, VAR_IGROPT[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AI0[0], UNIT_NUTR_RATIO, VAR_AI0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AI1[0], UNIT_NUTR_RATIO, VAR_AI1[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AI2[0], UNIT_NUTR_RATIO, VAR_AI2[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AI3[0], UNIT_NUTR_RATIO, VAR_AI3[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AI4[0], UNIT_NUTR_RATIO, VAR_AI4[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AI5[0], UNIT_NUTR_RATIO, VAR_AI5[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AI6[0], UNIT_NUTR_RATIO, VAR_AI6[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LAMBDA0[0], UNIT_NON_DIM, VAR_LAMBDA0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LAMBDA1[0], UNIT_NON_DIM, VAR_LAMBDA1[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LAMBDA2[0], UNIT_NON_DIM, VAR_LAMBDA2[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_L[0], UNIT_SR, VAR_K_L[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_N[0], UNIT_CONCENTRATION, VAR_K_N[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_P[0], UNIT_CONCENTRATION, VAR_K_P[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_P_N[0], UNIT_CONCENTRATION, VAR_P_N[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TFACT[0], UNIT_NON_DIM, VAR_TFACT[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MUMAX[0], UNIT_PER_DAY, VAR_MUMAX[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RHOQ[0], UNIT_PER_DAY, VAR_RHOQ[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_COD_N[0], UNIT_NON_DIM, VAR_COD_N[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_COD_K[0], UNIT_NON_DIM, VAR_COD_K[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_STREAM_LINK[0], UNIT_NON_DIM, VAR_STREAM_LINK[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CH_ONCO[0], UNIT_CONCENTRATION, VAR_CH_ONCO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CH_OPCO[0], UNIT_CONCENTRATION, VAR_CH_OPCO[1], Source_ParameterDB, DT_Single);
    // add reach information
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Reach);
    // add BMPs management operations, such as point source discharge
    mdi.AddParameter(VAR_SCENARIO[0], UNIT_NON_DIM, VAR_SCENARIO[1], Source_ParameterDB, DT_Scenario);

    // set the input variables
    mdi.AddInput(DataType_SolarRadiation, UNIT_SR, DESC_SR, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DAYLEN[0], UNIT_HOUR, VAR_DAYLEN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOTE[0], UNIT_TEMP_DEG, VAR_SOTE[1], Source_Module, DT_Raster1D);

    mdi.AddInput(VAR_QRECH[0], UNIT_FLOW_CMS, VAR_QRECH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_CHST[0], UNIT_VOL_M3, VAR_CHST[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_RTE_WTRIN[0], UNIT_VOL_M3, VAR_RTE_WTRIN[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_RTE_WTROUT[0], UNIT_VOL_M3, VAR_RTE_WTROUT[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_CHWTRDEPTH[0], UNIT_LEN_M, VAR_CHWTRDEPTH[1], Source_Module, DT_Array1D);
    /// input from hillslope
    //nutrient from surface water
    mdi.AddInput(VAR_SUR_NO3_TOCH[0], UNIT_KG, VAR_SUR_NO3_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SUR_NH4_TOCH[0], UNIT_KG, VAR_SUR_NH4_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SUR_SOLP_TOCH[0], UNIT_KG, VAR_SUR_SOLP_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SUR_COD_TOCH[0], UNIT_KG, VAR_SUR_COD_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SEDORGN_TOCH[0], UNIT_KG, VAR_SEDORGN_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SEDORGP_TOCH[0], UNIT_KG, VAR_SEDORGP_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SEDMINPA_TOCH[0], UNIT_KG, VAR_SEDMINPA_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SEDMINPS_TOCH[0], UNIT_KG, VAR_SEDMINPS_TOCH[1], Source_Module, DT_Array1D);
    //nutrient from interflow
    mdi.AddInput(VAR_LATNO3_TOCH[0], UNIT_KG, VAR_LATNO3_TOCH[1], Source_Module, DT_Array1D);
    //nutrient from ground water
    mdi.AddInput(VAR_NO3GW_TOCH[0], UNIT_KG, VAR_NO3GW_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_MINPGW_TOCH[0], UNIT_KG, VAR_MINPGW_TOCH[1], Source_Module, DT_Array1D);
    // channel erosion
    mdi.AddInput(VAR_RCH_DEG[0], UNIT_KG, VAR_RCH_DEG[1], Source_Module_Optional, DT_Array1D);
    // set the output variables
    /// 1. Amount (kg) outputs
    mdi.AddInOutput(VAR_CH_ALGAE[0], UNIT_KG, VAR_CH_ALGAE[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_ORGN[0], UNIT_KG, VAR_CH_ORGN[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_ORGP[0], UNIT_KG, VAR_CH_ORGP[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_NH4[0], UNIT_KG, VAR_CH_NH4[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_NO2[0], UNIT_KG, VAR_CH_NO2[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_NO3[0], UNIT_KG, VAR_CH_NO3[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_SOLP[0], UNIT_KG, VAR_CH_SOLP[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_COD[0], UNIT_KG, VAR_CH_COD[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_CHLORA[0], UNIT_KG, VAR_CH_CHLORA[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_DOX[0], UNIT_KG, VAR_CH_DOX[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_TN[0], UNIT_KG, VAR_CH_TN[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_TP[0], UNIT_KG, VAR_CH_TP[1], DT_Array1D, TF_SingleValue);
    /// 2. Concentration (mg/L) outputs
    mdi.AddInOutput(VAR_CH_ALGAEConc[0], UNIT_CONCENTRATION, VAR_CH_ALGAE[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_ORGNConc[0], UNIT_CONCENTRATION, VAR_CH_ORGN[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_ORGPConc[0], UNIT_CONCENTRATION, VAR_CH_ORGP[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_NH4Conc[0], UNIT_CONCENTRATION, VAR_CH_NH4[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_NO2Conc[0], UNIT_CONCENTRATION, VAR_CH_NO2[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_NO3Conc[0], UNIT_CONCENTRATION, VAR_CH_NO3[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_SOLPConc[0], UNIT_CONCENTRATION, VAR_CH_SOLP[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_CODConc[0], UNIT_CONCENTRATION, VAR_CH_COD[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_CHLORAConc[0], UNIT_CONCENTRATION, VAR_CH_CHLORA[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_DOXConc[0], UNIT_CONCENTRATION, VAR_CH_DOX[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_TNConc[0], UNIT_CONCENTRATION, VAR_CH_TNConc[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CH_TPConc[0], UNIT_CONCENTRATION, VAR_CH_TPConc[1], DT_Array1D, TF_SingleValue);
    // 3. point source loadings
    mdi.AddOutput(VAR_PTTN2CH[0], UNIT_KG, VAR_PTTN2CH[1], DT_Array1D);
    mdi.AddOutput(VAR_PTTP2CH[0], UNIT_KG, VAR_PTTP2CH[1], DT_Array1D);
    mdi.AddOutput(VAR_PTCOD2CH[0], UNIT_KG, VAR_PTCOD2CH[1], DT_Array1D);
    // 4. nutrient stored in reaches
    mdi.AddInOutput(VAR_CHSTR_NH4[0], UNIT_KG, VAR_CHSTR_NH4[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CHSTR_NO3[0], UNIT_KG, VAR_CHSTR_NO3[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CHSTR_TN[0], UNIT_KG, VAR_CHSTR_TN[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CHSTR_TP[0], UNIT_KG, VAR_CHSTR_TP[1], DT_Array1D, TF_SingleValue);

    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
