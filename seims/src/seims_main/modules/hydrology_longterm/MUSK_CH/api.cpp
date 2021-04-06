#include "api.h"

#include "MUSK_CH.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new MUSK_CH();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    string res;
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu; Liangjun Zhu");
    mdi.SetClass(MCLS_CH_ROUTING[0], MCLS_CH_ROUTING[1]);
    mdi.SetDescription(M_MUSK_CH[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("");
    mdi.SetID(M_MUSK_CH[0]);
    mdi.SetName(M_MUSK_CH[0]);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_ChannelTimeStep, UNIT_SECOND, Tag_TimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_OUTLETID[0], UNIT_NON_DIM, VAR_OUTLETID[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_EP_CH[0], UNIT_WTRDLT_MMH, VAR_EP_CH[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BNK0[0], UNIT_STRG_M3M, VAR_BNK0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CHS0_PERC[0], UNIT_NON_DIM, VAR_CHS0_PERC[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_A_BNK[0], UNIT_NON_DIM, VAR_A_BNK[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_B_BNK[0], UNIT_NON_DIM, VAR_B_BNK[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MSK_X[0], UNIT_NON_DIM, VAR_MSK_X[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MSK_CO1[0], UNIT_NON_DIM, VAR_MSK_CO1[1], Source_ParameterDB, DT_Single);
    // add reach information
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Reach);
    // add BMPs management operations, such as point source discharge
    mdi.AddParameter(VAR_SCENARIO[0], UNIT_NON_DIM, VAR_SCENARIO[1], Source_ParameterDB, DT_Scenario);

    // Inputs from other modules
    mdi.AddInput(VAR_SBPET[0], UNIT_DEPTH_MM, VAR_SBPET[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBGS[0], UNIT_DEPTH_MM, VAR_SBGS[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBOF[0], UNIT_FLOW_CMS, VAR_SBOF[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBIF[0], UNIT_FLOW_CMS, VAR_SBIF[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBQG[0], UNIT_FLOW_CMS, VAR_SBQG[1], Source_Module, DT_Array1D);

    // Outputs
    mdi.AddInOutput(VAR_QRECH[0], UNIT_FLOW_CMS, VAR_QRECH[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_QS[0], UNIT_NON_DIM, VAR_QS[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_QI[0], UNIT_NON_DIM, VAR_QI[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_QG[0], UNIT_NON_DIM, VAR_QG[1], DT_Array1D, TF_SingleValue);

    mdi.AddOutput(VAR_CHST[0], UNIT_VOL_M3, VAR_CHST[1], DT_Array1D);
    mdi.AddOutput(VAR_RTE_WTRIN[0], UNIT_VOL_M3, VAR_RTE_WTRIN[1], DT_Array1D);
    mdi.AddOutput(VAR_RTE_WTROUT[0], UNIT_VOL_M3, VAR_RTE_WTROUT[1], DT_Array1D);
    mdi.AddOutput(VAR_BKST[0], UNIT_VOL_M3, VAR_BKST[1], DT_Array1D);

    mdi.AddOutput(VAR_CHWTRDEPTH[0], UNIT_LEN_M, VAR_CHWTRDEPTH[1], DT_Array1D);
    mdi.AddOutput(VAR_CHWTRWIDTH[0], UNIT_LEN_M, VAR_CHWTRWIDTH[1], DT_Array1D);
    mdi.AddOutput(VAR_CHBTMWIDTH[0], UNIT_LEN_M, VAR_CHBTMWIDTH[1], DT_Array1D);
    mdi.AddOutput(VAR_CHCROSSAREA[0], UNIT_AREA_M2, VAR_CHCROSSAREA[1], DT_Array1D);

    res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
