#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "NutrientinGroundwater.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new NutrientinGroundwater();
}

//! function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;
    mdi.SetAuthor("Huiran Gao; Liang-Jun Zhu");
    mdi.SetClass(MCLS_NUTRGW, MCLSDESC_NUTRGW);
    mdi.SetDescription(MDESC_NUTRGW);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MDESC_NUTRGW);
    mdi.SetName(MDESC_NUTRGW);
    mdi.SetVersion("1.0");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("NutGW.html");

    // set the parameters
	mdi.AddParameter(Tag_TimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Config, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);

	// parameters for subbasin sum
	mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_SUBBASIN_PARAM, UNIT_NON_DIM, DESC_SUBBASIN_PARAM, Source_ParameterDB, DT_Subbasin);
	mdi.AddParameter(VAR_SOILLAYERS, UNIT_NON_DIM, DESC_SOILLAYERS, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_GW0, UNIT_DEPTH_MM, DESC_GW0, Source_ParameterDB, DT_Single);
	// add reach information
	mdi.AddParameter(VAR_REACH_PARAM, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Reach);
    // set the input from other modules
    mdi.AddInput(VAR_SBQG, UNIT_FLOW_CMS, DESC_SBQG, Source_Module, DT_Array1D);
	mdi.AddInput(VAR_SBGS, UNIT_DEPTH_MM, DESC_SBGS, Source_Module, DT_Array1D);
	
	mdi.AddInput(VAR_SOL_NO3, UNIT_CONT_KGHA, DESC_SOL_NO3, Source_Module, DT_Raster2D);
	mdi.AddInput(VAR_SOL_SOLP, UNIT_CONT_KGHA, DESC_SOL_SOLP, Source_Module, DT_Raster2D);

	mdi.AddInput(VAR_PERCO_N_GW, UNIT_KG, DESC_PERCO_N, Source_Module, DT_Array1D);
	mdi.AddInput(VAR_PERCO_P_GW, UNIT_KG, DESC_PERCO_P, Source_Module, DT_Array1D);

    // set the output variables
	mdi.AddOutput(VAR_GWNO3_CONC, UNIT_CONCENTRATION, DESC_GWNO3_CONC, DT_Array1D);
	mdi.AddOutput(VAR_GWSOLP_CONC, UNIT_CONCENTRATION, DESC_GWSOLP, DT_Array1D);
    mdi.AddOutput(VAR_NO3GW_TOCH, UNIT_KG, DESC_NO3GW_CH, DT_Array1D);
    mdi.AddOutput(VAR_MINPGW_TOCH, UNIT_KG, DESC_MINPGW_CH, DT_Array1D);
	mdi.AddOutput(VAR_GWNO3, UNIT_KG, DESC_GWNO3, DT_Array1D);
	mdi.AddOutput(VAR_GWSOLP, UNIT_KG, DESC_GWSOLP_CONC, DT_Array1D);

    string res = mdi.GetXMLDocument();
    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}