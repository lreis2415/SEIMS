#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "IKW_REACH.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new IKW_REACH();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
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

    mdi.AddParameter(Tag_ChannelTimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single);
    mdi.AddParameter(VAR_K_CHB, UNIT_WTRDLT_MMH, DESC_K_CHB, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_BANK, UNIT_WTRDLT_MMH, DESC_K_BANK, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_EP_CH, UNIT_WTRDLT_MMH, DESC_EP_CH, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BNK0, UNIT_STRG_M3M, DESC_BNK0, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CHS0, UNIT_STRG_M3M, DESC_CHS0, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_VSEEP0, UNIT_FLOW_CMS, DESC_VSEEP0, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_A_BNK, UNIT_NON_DIM, DESC_A_BNK, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_B_BNK, UNIT_NON_DIM, DESC_B_BNK, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MSK_X, UNIT_NON_DIM, DESC_MSK_X, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MSK_CO1, UNIT_NON_DIM, DESC_MSK_CO1, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_QUPREACH, UNIT_NON_DIM, DESC_QUPREACH, Source_ParameterDB, DT_Single);
    //mdi.AddParameter(VAR_MSF, UNIT_NON_DIM, DESC_MSF, Source_ParameterDB, DT_Single);

    mdi.AddParameter(Tag_RchParam, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Array2D);
    //mdi.AddParameter(VAR_VDIV, UNIT_VOL_M3, DESC_VDIV, DT_Array1D);
    //mdi.AddParameter(VAR_VPOINT, UNIT_VOL_M3, DESC_VPOINT, "diversionloss.txt", DT_Array1D);
    mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);

    mdi.AddInput(VAR_SBOF, UNIT_FLOW_CMS, DESC_SBOF, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBIF, UNIT_FLOW_CMS, DESC_SBIF, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBQG, UNIT_FLOW_CMS, DESC_SBQG, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBPET, UNIT_DEPTH_MM, DESC_SBPET, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBGS, UNIT_DEPTH_MM, DESC_SBGS, Source_Module, DT_Array1D);

    mdi.AddOutput(VAR_QRECH, UNIT_FLOW_CMS, DESC_QRECH, DT_Array1D);
	mdi.AddOutput(VAR_QOUTLET, UNIT_FLOW_CMS, DESC_QOUTLET, DT_Single);
	mdi.AddOutput(VAR_QS, UNIT_FLOW_CMS, DESC_QS, DT_Array1D);
    mdi.AddOutput(VAR_QSOUTLET, UNIT_FLOW_CMS, DESC_QSOUTLET, DT_Single);
    mdi.AddOutput(VAR_QI, UNIT_FLOW_CMS, DESC_QI, DT_Array1D);
    mdi.AddOutput(VAR_QG, UNIT_FLOW_CMS, DESC_QG, DT_Array1D);
    mdi.AddOutput(VAR_CHST, UNIT_VOL_M3, DESC_CHST, DT_Array1D);
    mdi.AddOutput(VAR_BKST, UNIT_VOL_M3, DESC_BKST, DT_Array1D);
    mdi.AddOutput(VAR_SEEPAGE, UNIT_VOL_M3, DESC_SEEPAGE, DT_Array1D);
    mdi.AddOutput(VAR_CHWTDEPTH, UNIT_LEN_M, DESC_CHWTDEPTH, DT_Array1D);
    //mdi.AddOutput(VAR_C_WABA, UNIT_NON_DIM, DESC_C_WABA, DT_Array2D);

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}