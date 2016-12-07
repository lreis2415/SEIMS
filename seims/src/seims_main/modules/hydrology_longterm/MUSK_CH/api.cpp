#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "MUSK_CH.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new MUSK_CH();
}
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Liu Junzhi");
    mdi.SetClass(MCLS_CH_ROUTING, MCLSDESC_CH_ROUTING);
    mdi.SetDescription(MDESC_MUSK_CH);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("MUSK_CH.chm");
    mdi.SetID(MID_MUSK_CH);
    mdi.SetName(MID_MUSK_CH);
    mdi.SetVersion("0.2");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_ChannelTimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single);
    mdi.AddParameter(VAR_K_CHB, UNIT_WTRDLT_MMH, DESC_K_CHB, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_BANK, UNIT_WTRDLT_MMH, DESC_K_BANK, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_EP_CH, UNIT_WTRDLT_MMH, DESC_EP_CH, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BNK0, UNIT_STRG_M3M, DESC_BNK0, Source_ParameterDB, DT_Single);
    //mdi.AddParameter(VAR_CHS0, UNIT_STRG_M3M, DESC_CHS0, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_CHS0_PERC, UNIT_NON_DIM, DESC_CHS0_PERC, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_VSEEP0, UNIT_FLOW_CMS, DESC_VSEEP0, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_A_BNK, UNIT_NON_DIM, DESC_A_BNK, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_B_BNK, UNIT_NON_DIM, DESC_B_BNK, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MSK_X, UNIT_NON_DIM, DESC_MSK_X, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MSK_CO1, UNIT_NON_DIM, DESC_MSK_CO1, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_VSF, UNIT_NON_DIM, DESC_VSF, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_GWRQ, UNIT_FLOW_CMS, DESC_GWRQ, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);
	// add reach information
	mdi.AddParameter(VAR_REACH_PARAM, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Reach);
	// add BMPs management operations, such as point source discharge
	mdi.AddParameter(VAR_SCENARIO, UNIT_NON_DIM, DESC_SCENARIO, Source_ParameterDB, DT_Scenario);

    mdi.AddInput(VAR_SBOF, UNIT_FLOW_CMS, DESC_SBOF, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBIF, UNIT_FLOW_CMS, DESC_SBIF, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBQG, UNIT_FLOW_CMS, DESC_SBQG, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBPET, UNIT_DEPTH_MM, DESC_SBPET, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SBGS, UNIT_DEPTH_MM, DESC_SBGS, Source_Module, DT_Array1D);

    mdi.AddOutput(VAR_QRECH, UNIT_FLOW_CMS, DESC_QRECH, DT_Array1D);
    mdi.AddOutput(VAR_QOUTLET, UNIT_FLOW_CMS, DESC_QOUTLET, DT_Single);
    mdi.AddOutput(VAR_QSOUTLET, UNIT_FLOW_CMS, DESC_QSOUTLET, DT_Single);
    mdi.AddOutput(VAR_QS, UNIT_NON_DIM, DESC_QS, DT_Array1D);
    mdi.AddOutput(VAR_QI, UNIT_NON_DIM, DESC_QI, DT_Array1D);
    mdi.AddOutput(VAR_QG, UNIT_NON_DIM, DESC_QG, DT_Array1D);
    mdi.AddOutput(VAR_CHST, UNIT_VOL_M3, DESC_CHST, DT_Array1D);
	mdi.AddOutput(VAR_PRECHST, UNIT_VOL_M3, DESC_PRECHST, DT_Array1D);
    mdi.AddOutput(VAR_BKST, UNIT_VOL_M3, DESC_BKST, DT_Array1D);
    mdi.AddOutput(VAR_SEEPAGE, UNIT_VOL_M3, DESC_SEEPAGE, DT_Array1D);
    mdi.AddOutput(VAR_CHWTDEPTH, UNIT_LEN_M, DESC_CHWTDEPTH, DT_Array1D);
	mdi.AddOutput(VAR_CHWTWIDTH, UNIT_LEN_M, DESC_CHWTWIDTH, DT_Array1D);
	mdi.AddOutput(VAR_CHBTMWIDTH, UNIT_LEN_M, DESC_CHBTMWIDTH, DT_Array1D);
	mdi.AddOutput(VAR_PRECHWTDEPTH, UNIT_LEN_M, DESC_PRECHWTDEPTH, DT_Array1D);

    res = mdi.GetXMLDocument();
    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
    //mdi.AddOutput(VAR_C_WABA, UNIT_NON_DIM, DESC_C_WABA, DT_Array2D);

//mdi.AddParameter(Tag_RchParam, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Array2D);// replaced by lj
// add basic subbasins information. (NO NEED SINCE DT_Reach is enough)
//mdi.AddParameter(VAR_SUBBASIN_PARAM, UNIT_NON_DIM, DESC_SUBBASIN_PARAM, Source_ParameterDB, DT_Subbasin);