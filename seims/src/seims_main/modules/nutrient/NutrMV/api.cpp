#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "NutrientMovementViaWater.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new NutrientMovementViaWater();
}

//! function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;
    mdi.SetAuthor("Huiran Gao");
    mdi.SetClass(MCLS_NutRemv, MCLSDESC_NutRemv);
    mdi.SetDescription(MDESC_NUTRMV);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MDESC_NUTRMV);
    mdi.SetName(MDESC_NUTRMV);
    mdi.SetVersion("1.0");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("NutRemv.html");

    // set the parameters
	mdi.AddParameter(VAR_CSWAT, UNIT_NON_DIM, DESC_CSWAT, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_QTILE, UNIT_NON_DIM, DESC_QTILE, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_PHOSKD, UNIT_NON_DIM, DESC_PHOSKD, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_PPERCO, UNIT_NON_DIM, DESC_PPERCO, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NPERCO, UNIT_NON_DIM, DESC_NPERCO, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_ISEP_OPT, UNIT_NON_DIM, DESC_ISEP_OPT, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_COD_N, UNIT_NON_DIM, DESC_COD_N, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_COD_K, UNIT_NON_DIM, DESC_COD_K, Source_ParameterDB, DT_Single);

	mdi.AddParameter(VAR_SOL_UL, UNIT_DEPTH_MM, DESC_SOL_UL, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_ANION_EXCL, UNIT_NON_DIM, DESC_ANION_EXCL, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_CRK, UNIT_NON_DIM, DESC_SOL_CRK, Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(VAR_SOILDEPTH, UNIT_DEPTH_MM, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_SOILTHICK,UNIT_DEPTH_MM, DESC_SOILTHICK, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_SOILLAYERS, UNIT_NON_DIM, DESC_SOILLAYERS, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_DISTSTREAM, UNIT_LEN_M, DESC_DISTSTREAM, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(Tag_FLOWOUT_INDEX_D8, UNIT_NON_DIM, DESC_FLOWOUT_INDEX_D8, Source_ParameterDB, DT_Array1D);

    mdi.AddParameter(VAR_SOL_NO3, UNIT_CONT_KGHA, DESC_SOL_NO3, Source_Module, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SOLP, UNIT_CONT_KGHA, DESC_SOL_SOLP, Source_Module, DT_Raster2D);
	
	mdi.AddParameter(VAR_SOL_CBN, UNIT_PERCENT, DESC_SOL_CBN, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_SOL_BD, UNIT_DENSITY, DESC_SOL_BD, Source_ParameterDB, DT_Raster2D);

	// parameters for subbasin sum
	mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_SUBBASIN_PARAM, UNIT_NON_DIM, DESC_SUBBASIN_PARAM, Source_ParameterDB, DT_Subbasin);
	mdi.AddParameter(VAR_STREAM_LINK, UNIT_NON_DIM, DESC_STREAM_LINK, Source_ParameterDB, DT_Raster1D);
	// for subsurface routing
	mdi.AddParameter(Tag_ROUTING_LAYERS, UNIT_NON_DIM, DESC_ROUTING_LAYERS, Source_ParameterDB, DT_Array2D);
    // set input from other modules
    //mdi.AddInput(VAR_WSHD_PLCH, UNIT_CONT_KGHA, DESC_WSHD_PLCH, Source_Module, DT_Single);
	//surface related inputs
    mdi.AddInput(VAR_OLFLOW, UNIT_DEPTH_MM, DESC_OLFLOW, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SEDYLD, UNIT_KG, DESC_SEDYLD, Source_Module, DT_Raster1D);
	//soil related inputs
	mdi.AddInput(VAR_PERCO, UNIT_DEPTH_MM, DESC_PERCO, Source_Module, DT_Raster2D);
	mdi.AddInput(VAR_SSRU, UNIT_DEPTH_MM, DESC_SSRU, Source_Module, DT_Raster2D);
	//groundwater related inputs
	//mdi.AddInput(VAR_GW_Q, UNIT_DEPTH_MM, DESC_GW_Q, Source_Module, DT_Raster1D);

    mdi.AddInput(VAR_SEDORGN, UNIT_CONT_KGHA, DESC_SEDORGN, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMEAN, UNIT_TEMP_DEG, DESC_TMEAN, Source_Module, DT_Raster1D);

	mdi.AddInput(VAR_SEDLOSS_C, UNIT_CONT_KGHA, DESC_SEDLOSS_C, Source_Module_Optional, DT_Raster1D);

	mdi.AddInput(VAR_CONV_WT, UNIT_NON_DIM, DESC_CONV_WT, Source_Module, DT_Raster2D);
	///////////////////////////////////////////////////////////////
    // set the output variables
    mdi.AddOutput(VAR_WSHD_PLCH, UNIT_CONT_KGHA, DESC_WSHD_PLCH, DT_Single);
	//surface related
    mdi.AddOutput(VAR_SUR_NO3, UNIT_CONT_KGHA, DESC_SUR_NO3, DT_Raster1D);
	mdi.AddOutput(VAR_SUR_NH4, UNIT_CONT_KGHA, DESC_SUR_NH4, DT_Raster1D);
    mdi.AddOutput(VAR_SUR_SOLP, UNIT_CONT_KGHA, DESC_SUR_SOLP, DT_Raster1D);
	mdi.AddOutput(VAR_SUR_COD, UNIT_CONT_KGHA, DESC_SUR_COD, DT_Raster1D);
    mdi.AddOutput(VAR_CHL_A, UNIT_CONCENTRATION, DESC_CHL_A, DT_Raster1D);
	mdi.AddOutput(VAR_LATNO3, UNIT_CONT_KGHA, DESC_LATNO3, DT_Raster1D);
	//to groundwater
	mdi.AddOutput(VAR_PERCO_N_GW, UNIT_KG, DESC_PERCO_N, DT_Array1D);
	mdi.AddOutput(VAR_PERCO_P_GW, UNIT_KG, DESC_PERCO_P, DT_Array1D);
	//to channel
	mdi.AddOutput(VAR_LATNO3_TOCH, UNIT_KG, DESC_LATNO3_CH, DT_Array1D);
	mdi.AddOutput(VAR_SUR_NO3_TOCH, UNIT_KG, DESC_SUR_NO3_ToCH, DT_Array1D);
	mdi.AddOutput(VAR_SUR_NH4_TOCH, UNIT_KG, DESC_SUR_NH4_ToCH, DT_Array1D);
	mdi.AddOutput(VAR_SUR_SOLP_TOCH, UNIT_KG, DESC_SUR_SOLP_ToCH, DT_Array1D);
	mdi.AddOutput(VAR_SUR_COD_TOCH, UNIT_KG, DESC_SUR_COD_ToCH, DT_Array1D);

    string res = mdi.GetXMLDocument();
    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}

    //mdi.AddParameter(VAR_LDRAIN, UNIT_NON_DIM, DESC_LDRAIN, Source_ParameterDB, DT_Raster1D);
    //mdi.AddOutput(VAR_SOL_NO3, UNIT_CONT_KGHA, DESC_SOL_NO3, DT_Raster2D);
    //mdi.AddOutput(VAR_SOL_SOLP, UNIT_CONT_KGHA, DESC_SOL_SOLP, DT_Raster2D);