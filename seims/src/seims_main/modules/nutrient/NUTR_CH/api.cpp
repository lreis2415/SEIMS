#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "NUTR_CH.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new NUTR_CH();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_SED_ROUTING, MCLSDESC_SED_ROUTING);
    mdi.SetDescription(MDESC_NUTR_CH);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("NUTR_CH.chm");
    mdi.SetID(MID_NUTR_CH);
    mdi.SetName(MID_NUTR_CH);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);
#ifdef STORM_MODEL
    mdi.AddParameter(Tag_ChannelTimeStep,UNIT_SECOND,DESC_TIMESTEP,File_Input,DT_Single);  //for long term model // this method is daily time interval based.
#else
	/// in Module Settings: time_t SettingsInput::getDtDaily() const{return 86400;}
    mdi.AddParameter(Tag_TimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single); 
#endif
    
    mdi.AddParameter(Tag_RchParam, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Array2D);

    /// input from hillslope
	//nutrient from surface water
	mdi.AddInput(VAR_SUR_NO3_TOCH, UNIT_KG, DESC_SUR_NO3_CH, Source_Module, DT_Array1D);
	mdi.AddInput(VAR_SUR_SOLP_TOCH, UNIT_KG, DESC_SUR_SOLP_CH, Source_Module, DT_Array1D);
	mdi.AddInput(VAR_SEDORGN_TOCH, UNIT_KG, DESC_SEDORGN_CH, Source_Module, DT_Array1D);
	mdi.AddInput(VAR_SEDORGP_TOCH, UNIT_KG, DESC_SEDORGP_CH, Source_Module, DT_Array1D);
	//mdi.AddInput(VAR_SEDMINPA_TOCH, UNIT_KG, DESC_SEDMINPA_CH, Source_Module, DT_Array1D);
	//mdi.AddInput(VAR_SEDMINPS_TOCH, UNIT_KG, DESC_SEDMINPS_CH, Source_Module, DT_Array1D);
	//nutrient from interflow
	mdi.AddInput(VAR_LATNO3_TOCH, UNIT_KG, DESC_LATNO3_CH, Source_Module, DT_Array1D);
	//nutrient from ground water
	mdi.AddInput(VAR_NO3GW_TOCH, UNIT_KG, DESC_NO3GW_CH, Source_Module, DT_Array1D);
	mdi.AddInput(VAR_MINPGW_TOCH, UNIT_KG, DESC_MINPGW_CH, Source_Module, DT_Raster1D);

	///////////////////////////////
    mdi.AddInput(VAR_QRECH, UNIT_FLOW_CMS, DESC_QRECH, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_CHST, UNIT_VOL_M3, DESC_CHST, Source_Module, DT_Array1D);
    
	// output
    mdi.AddOutput(VAR_CH_NO3, UNIT_CONCENTRATION, DESC_CH_NO3, DT_Array1D);
    mdi.AddOutput(VAR_CH_SOLP, UNIT_CONCENTRATION, DESC_CH_SOLP, DT_Array1D);
	mdi.AddOutput(VAR_CH_ORGN, UNIT_CONCENTRATION, DESC_CH_ORGN, DT_Array1D);
	mdi.AddOutput(VAR_CH_ORGP, UNIT_CONCENTRATION, DESC_CH_SOLP, DT_Array1D);

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}

//mdi.AddParameter("Vseep0","m3/s","the initial volume of transmission loss to the deep aquifer over the time interval","ParameterDB_Discharge", DT_Single);

//TODO: add later...
//mdi.AddParameter("Vdiv","m3","diversion loss of the river reach", "diversionloss.txt",DT_Array1D);
//mdi.AddParameter("Vpoint","m3"," point source discharge", "diversionloss.txt",DT_Array1D);

//mdi.AddInput("CROSS_AREA", "m2", "the cross-sectional area of flow in the channel","Module", DT_Array1D);
//mdi.AddParameter("subbasin","","subbasin grid","ParameterDB_Discharge", DT_Raster1D);

//mdi.AddInput("SEEPAGE", "m3", "seepage", "Module",DT_Array1D);
//mdi.AddInput("C_WABA","","Channel water balance in a text format for each reach and at each time step","Module",DT_Array2D);
//mdi.AddOutput("DEPOUTLET", "ton", "sediment concentration at the watershed outlet", DT_Single);
