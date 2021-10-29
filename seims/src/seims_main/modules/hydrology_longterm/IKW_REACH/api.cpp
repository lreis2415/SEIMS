#include "api.h"

#include "IKW_REACH.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new IKW_REACH();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Liu Junzhi");
    mdi.SetClass("Channelflow", "channel flow routing using kinermatic wave method.");
    mdi.SetDescription("channel flow routing using variable storage method.");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("IKW_REACH.chm");
    mdi.SetID("IKW_REACH");
    mdi.SetName("MUSK_CH");
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    //mdi.AddParameter(Tag_LayeringMethod[0], UNIT_NON_DIM, Tag_LayeringMethod[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_ChannelTimeStep[0], UNIT_SECOND, Tag_ChannelTimeStep[1], File_Input, DT_Single);
    //mdi.AddParameter(VAR_K_CHB, UNIT_WTRDLT_MMH, DESC_K_CHB, Source_ParameterDB, DT_Single);
    //mdi.AddParameter(VAR_K_BANK, UNIT_WTRDLT_MMH, DESC_K_BANK, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_EP_CH[0], UNIT_WTRDLT_MMH, VAR_EP_CH[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BNK0[0], UNIT_STRG_M3M, VAR_BNK0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CHS0[0], UNIT_STRG_M3M, VAR_CHS0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_VSEEP0[0], UNIT_FLOW_CMS, VAR_VSEEP0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_A_BNK[0], UNIT_NON_DIM, VAR_A_BNK[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_B_BNK[0], UNIT_NON_DIM, VAR_B_BNK[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MSK_X[0], UNIT_NON_DIM, VAR_MSK_X[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MSK_CO1[0], UNIT_NON_DIM, VAR_MSK_CO1[1], Source_ParameterDB, DT_Single);
    //mdi.AddParameter(VAR_QUPREACH, UNIT_NON_DIM, DESC_QUPREACH, Source_ParameterDB, DT_Single);
    //mdi.AddParameter(VAR_MSF[0], UNIT_NON_DIM, VAR_MSF[1], Source_ParameterDB, DT_Single);

    //mdi.AddParameter(Tag_RchParam, UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Array2D);
    // add reach information
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Reach);
    //mdi.AddParameter(VAR_VDIV[0], UNIT_VOL_M3, VAR_VDIV[1], DT_Array1D);
    //mdi.AddParameter(VAR_VPOINT, UNIT_VOL_M3, DESC_VPOINT, "diversionloss.txt", DT_Array1D);
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddInput(VAR_SBOF[0], UNIT_FLOW_CMS, VAR_SBOF[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBIF[0], UNIT_FLOW_CMS, VAR_SBIF[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBQG[0], UNIT_FLOW_CMS, VAR_SBQG[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBPET[0], UNIT_DEPTH_MM, VAR_SBPET[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBGS[0], UNIT_DEPTH_MM, VAR_SBGS[1], Source_Module, DT_Array1D);

    mdi.AddOutput(VAR_QRECH[0], UNIT_FLOW_CMS, VAR_QRECH[1], DT_Array1D);
    //mdi.AddOutput(VAR_QOUTLET, UNIT_FLOW_CMS, DESC_QOUTLET, DT_Single);
    mdi.AddOutput(VAR_QS[0], UNIT_FLOW_CMS, VAR_QS[1], DT_Array1D);
    //mdi.AddOutput(VAR_QSOUTLET, UNIT_FLOW_CMS, DESC_QSOUTLET, DT_Single);
    mdi.AddOutput(VAR_QI[0], UNIT_FLOW_CMS, VAR_QI[1], DT_Array1D);
    mdi.AddOutput(VAR_QG[0], UNIT_FLOW_CMS, VAR_QG[1], DT_Array1D);
    mdi.AddOutput(VAR_CHST[0], UNIT_VOL_M3, VAR_CHST[1], DT_Array1D);
    mdi.AddOutput(VAR_BKST[0], UNIT_VOL_M3, VAR_BKST[1], DT_Array1D);
    mdi.AddOutput(VAR_SEEPAGE[0], UNIT_VOL_M3, VAR_SEEPAGE[1], DT_Array1D);
    mdi.AddOutput(VAR_CHWTRDEPTH[0], UNIT_LEN_M, VAR_CHWTRDEPTH[1], DT_Array1D);
    //mdi.AddOutput(VAR_C_WABA[0], UNIT_NON_DIM, VAR_C_WABA[1], DT_Array2D);

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
