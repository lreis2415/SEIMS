#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "SEDR_SBAGNOLD.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new SEDR_SBAGNOLD();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Wu Hui; Junzhi Liu; LiangJun Zhu");
    mdi.SetClass(MCLS_SED_ROUTING, MCLSDESC_SED_ROUTING);
    mdi.SetDescription(MDESC_SEDR_SBAGNOLD);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("SEDR_SBAGNOLD.chm");
    mdi.SetID(MID_SEDR_SBAGNOLD);
    mdi.SetName(MID_SEDR_SBAGNOLD);
    mdi.SetVersion("1.0");
    mdi.SetWebsite(SEIMS_SITE);
#ifdef STORM_MODEL
    mdi.AddParameter(Tag_ChannelTimeStep,UNIT_SECOND,DESC_TIMESTEP,File_Input,DT_Single);
#else
    mdi.AddParameter(Tag_TimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single); // daily model
#endif
	mdi.AddParameter(VAR_VCD, UNIT_NON_DIM, DESC_VCD, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_P_RF, UNIT_NON_DIM, DESC_P_RF, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SPCON, UNIT_NON_DIM, DESC_SPCON, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SPEXP, UNIT_NON_DIM, DESC_SPEXP, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_VCRIT, UNIT_SPEED_MS, DESC_VCRIT, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CHS0, UNIT_STRG_M3M, DESC_CHS0, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_SED_CHI0, UNIT_DENSITY, DESC_SED_CHI0, Source_ParameterDB, DT_Single);
	/// load reach parameters, the previous version Tag_RchParam is deprecated!
	mdi.AddParameter(VAR_REACH_PARAM, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Reach);
	/// load point source loading sediment from Scenario
	mdi.AddParameter(VAR_SCENARIO, UNIT_NON_DIM, DESC_SCENARIO, Source_ParameterDB, DT_Scenario);
    
	mdi.AddInput(VAR_SED_TO_CH, UNIT_KG, DESC_SED_TO_CH, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_QRECH, UNIT_FLOW_CMS, DESC_QRECH, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_CHST, UNIT_VOL_M3, DESC_CHST, Source_Module, DT_Array1D);
	mdi.AddInput(VAR_PRECHST, UNIT_VOL_M3, DESC_PRECHST, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_CHWTDEPTH, UNIT_LEN_M, DESC_CHWTDEPTH, Source_Module, DT_Array1D);
	mdi.AddInput(VAR_PRECHWTDEPTH, UNIT_LEN_M, DESC_PRECHWTDEPTH, Source_Module, DT_Array1D);
	mdi.AddInput(VAR_CHWTWIDTH, UNIT_LEN_M, DESC_CHWTWIDTH, Source_Module, DT_Array1D);
    
	mdi.AddOutput(VAR_SED_RECH, UNIT_KG, DESC_SED_RECH, DT_Array1D);
	mdi.AddOutput(VAR_SED_RECHConc, UNIT_SEDCONC, DESC_SED_RECH, DT_Array1D);
    mdi.AddOutput(VAR_SED_OUTLET, UNIT_KG, DESC_SED_OUTLET, DT_Single);

	mdi.AddOutput(VAR_RCH_BANKERO, UNIT_KG, DESC_RCH_BANKERO, DT_Array1D);
	mdi.AddOutput(VAR_RCH_DEG, UNIT_KG, DESC_RCH_DEG, DT_Array1D);
	mdi.AddOutput(VAR_RCH_DEP, UNIT_KG, DESC_RCH_DEP, DT_Array1D);
	mdi.AddOutput(VAR_FLPLAIN_DEP, UNIT_KG, DESC_FLPLAIN_DEP, DT_Array1D);
    // set the dependencies
    mdi.AddDependency(MCLS_OL_EROSION, MCLSDESC_OL_EROSION);
    mdi.AddDependency(MCLS_CH_ROUTING, MCLSDESC_CH_ROUTING); 

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
