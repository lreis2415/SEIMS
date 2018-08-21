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
    mdi.SetClass(MCLS_CH_ROUTING, MCLSDESC_CH_ROUTING);
    mdi.SetDescription(MDESC_MUSK_CH);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("");
    mdi.SetID(MID_MUSK_CH);
    mdi.SetName(MID_MUSK_CH);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_ChannelTimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single);
    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_OUTLETID, UNIT_NON_DIM, DESC_OUTLETID, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_EP_CH, UNIT_WTRDLT_MMH, DESC_EP_CH, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BNK0, UNIT_STRG_M3M, DESC_BNK0, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CHS0_PERC, UNIT_NON_DIM, DESC_CHS0_PERC, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_A_BNK, UNIT_NON_DIM, DESC_A_BNK, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_B_BNK, UNIT_NON_DIM, DESC_B_BNK, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MSK_X, UNIT_NON_DIM, DESC_MSK_X, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MSK_CO1, UNIT_NON_DIM, DESC_MSK_CO1, Source_ParameterDB, DT_Single);
    // add reach information
    mdi.AddParameter(VAR_REACH_PARAM, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Reach);
    // add BMPs management operations, such as point source discharge
    mdi.AddParameter(VAR_SCENARIO, UNIT_NON_DIM, DESC_SCENARIO, Source_ParameterDB, DT_Scenario);

    // Inputs from other modules
    mdi.AddInput(VAR_SBPET, UNIT_DEPTH_MM, DESC_SBPET, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBGS, UNIT_DEPTH_MM, DESC_SBGS, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBOF, UNIT_FLOW_CMS, DESC_SBOF, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBIF, UNIT_FLOW_CMS, DESC_SBIF, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBQG, UNIT_FLOW_CMS, DESC_SBQG, Source_Module, DT_Array1D);

    // Outputs
    mdi.AddInOutput(VAR_QRECH, UNIT_FLOW_CMS, DESC_QRECH, DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_QS, UNIT_NON_DIM, DESC_QS, DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_QI, UNIT_NON_DIM, DESC_QI, DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_QG, UNIT_NON_DIM, DESC_QG, DT_Array1D, TF_SingleValue);

    mdi.AddOutput(VAR_CHST, UNIT_VOL_M3, DESC_CHST, DT_Array1D);
    mdi.AddOutput(VAR_RTE_WTRIN, UNIT_VOL_M3, DESC_RTE_WTRIN, DT_Array1D);
    mdi.AddOutput(VAR_RTE_WTROUT, UNIT_VOL_M3, DESC_RTE_WTROUT, DT_Array1D);
    mdi.AddOutput(VAR_BKST, UNIT_VOL_M3, DESC_BKST, DT_Array1D);

    mdi.AddOutput(VAR_CHWTRDEPTH, UNIT_LEN_M, DESC_CHWTDEPTH, DT_Array1D);
    mdi.AddOutput(VAR_CHWTRWIDTH, UNIT_LEN_M, DESC_CHWTWIDTH, DT_Array1D);
    mdi.AddOutput(VAR_CHBTMWIDTH, UNIT_LEN_M, DESC_CHBTMWIDTH, DT_Array1D);
    mdi.AddOutput(VAR_CHCROSSAREA, UNIT_AREA_M2, DESC_CHCROSSAREA, DT_Array1D);

    res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
