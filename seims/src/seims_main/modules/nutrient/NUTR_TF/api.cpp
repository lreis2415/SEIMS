#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "Nutrient_Transformation.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new Nutrient_Transformation();
}

//! function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;
    mdi.SetAuthor("Huiran Gao, LiangJun Zhu");
    mdi.SetClass(MCLS_NUTRCYC, MCLSDESC_NUTRCYC);
    mdi.SetDescription(MDESC_NUTR_TF);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MDESC_NUTR_TF);
    mdi.SetName(MDESC_NUTR_TF);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("NUTR_TF.html");

    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CMN, UNIT_NON_DIM, DESC_CMN, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CDN, UNIT_NON_DIM, DESC_CDN, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_NACTFR, UNIT_NON_DIM, DESC_NACTFR, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_SDNCO, UNIT_NON_DIM, DESC_SDNCO, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_PSP, UNIT_NON_DIM, DESC_PSP, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_SSP, UNIT_NON_DIM, DESC_SSP, Source_ParameterDB, DT_Single);

	/// idplt in SWAT is a lookup array. in SEIMS, use landcover
    mdi.AddParameter(VAR_LCC, UNIT_NON_DIM, DESC_LCC, Source_ParameterDB, DT_Raster1D);  
    mdi.AddParameter(VAR_PL_RSDCO, UNIT_NON_DIM, DESC_PL_RSDCO, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_SOILLAYERS, UNIT_NON_DIM, DESC_SOILLAYERS, Source_ParameterDB, DT_Raster1D);
	//mdi.AddParameter(VAR_A_DAYS, UNIT_NON_DIM, DESC_A_DAYS, Source_ParameterDB, DT_Raster1D);
	//mdi.AddParameter(VAR_B_DAYS, UNIT_NON_DIM, DESC_B_DAYS, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILDEPTH, UNIT_DEPTH_MM, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_SOILTHICK, UNIT_DEPTH_MM, DESC_SOILTHICK, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_SOL_RSDIN, UNIT_CONT_KGHA, DESC_SOL_RSDIN, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_CSWAT, UNIT_NON_DIM, DESC_CSWAT, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_SOL_CBN, UNIT_PERCENT, DESC_SOL_CBN, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_CLAY, UNIT_PERCENT, DESC_CLAY, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_ROCK, UNIT_PERCENT, DESC_ROCK, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_SOL_BD, UNIT_DENSITY, DESC_SOL_BD, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_AWC, UNIT_DEPTH_MM, DESC_SOL_AWC, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_SOL_WPMM, UNIT_DEPTH_MM, DESC_SOL_WPMM, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_NO3, UNIT_CONT_KGHA, DESC_SOL_NO3, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_NH4, UNIT_CONT_KGHA, DESC_SOL_NH4, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SORGN, UNIT_CONT_KGHA, DESC_SOL_SORGN, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_HORGP, UNIT_CONT_KGHA, DESC_SOL_HORGP, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SOLP, UNIT_CONT_KGHA, DESC_SOL_SOLP, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_SOL_UL, UNIT_DEPTH_MM, DESC_SOL_UL, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_POROST, UNIT_VOL_FRA_M3M3, DESC_POROST, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_SAND, UNIT_PERCENT, DESC_SAND, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_SOL_UL, UNIT_DEPTH_MM, DESC_SOL_UL, Source_ParameterDB, DT_Raster2D);
	
	mdi.AddInput(VAR_SOL_COV, UNIT_CONT_KGHA, DESC_SOL_COV, Source_Module_Optional, DT_Raster1D);
	mdi.AddInput(VAR_SOL_RSD, UNIT_CONT_KGHA, DESC_SOL_RSD, Source_Module_Optional, DT_Raster2D);

	// from other modules
	mdi.AddInput(VAR_SOTE, UNIT_TEMP_DEG, DESC_SOTE, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module, DT_Raster2D);

	/// tillage operation during CENTURY model
	mdi.AddInput(VAR_TILLAGE_DAYS, UNIT_DAY, DESC_TILLAGE_DAYS, Source_Module_Optional, DT_Raster1D);
	mdi.AddInput(VAR_TILLAGE_DEPTH, UNIT_DAY, DESC_TILLAGE_DEPTH, Source_Module_Optional, DT_Raster1D);
	mdi.AddInput(VAR_TILLAGE_SWITCH, UNIT_DAY, DESC_TILLAGE_SWITCH, Source_Module_Optional, DT_Raster1D);
	mdi.AddInput(VAR_TILLAGE_FACTOR, UNIT_DAY, DESC_TILLAGE_FACTOR, Source_Module_Optional, DT_Raster1D);

	mdi.AddOutput(VAR_SOL_COV, UNIT_CONT_KGHA, DESC_SOL_COV, DT_Raster1D);
	mdi.AddOutput(VAR_SOL_RSD, UNIT_CONT_KGHA, DESC_SOL_RSD, DT_Raster2D);

    // set the output variables
	
    mdi.AddOutput(VAR_HMNTL, UNIT_CONT_KGHA, DESC_HMNTL, DT_Raster1D);
    mdi.AddOutput(VAR_HMPTL, UNIT_CONT_KGHA, DESC_HMPTL, DT_Raster1D);
    mdi.AddOutput(VAR_RMN2TL, UNIT_CONT_KGHA, DESC_RMN2TL, DT_Raster1D);
    mdi.AddOutput(VAR_RMPTL, UNIT_CONT_KGHA, DESC_RMPTL, DT_Raster1D);
    mdi.AddOutput(VAR_RWNTL, UNIT_CONT_KGHA, DESC_RWNTL, DT_Raster1D);
    mdi.AddOutput(VAR_WDNTL, UNIT_CONT_KGHA, DESC_WDNTL, DT_Raster1D);
    mdi.AddOutput(VAR_RMP1TL, UNIT_CONT_KGHA, DESC_RMP1TL, DT_Raster1D);
    mdi.AddOutput(VAR_ROCTL, UNIT_CONT_KGHA, DESC_ROCTL, DT_Raster1D);
	/// watershed statistics, total 10
    mdi.AddOutput(VAR_WSHD_DNIT, UNIT_CONT_KGHA, DESC_WSHD_DNIT, DT_Single);
    mdi.AddOutput(VAR_WSHD_HMN, UNIT_CONT_KGHA, DESC_WSHD_HMN, DT_Single);
    mdi.AddOutput(VAR_WSHD_HMP, UNIT_CONT_KGHA, DESC_WSHD_HMP, DT_Single);
    mdi.AddOutput(VAR_WSHD_RMN, UNIT_CONT_KGHA, DESC_WSHD_RMN, DT_Single);
    mdi.AddOutput(VAR_WSHD_RMP, UNIT_CONT_KGHA, DESC_WSHD_RMP, DT_Single);
    mdi.AddOutput(VAR_WSHD_RWN, UNIT_CONT_KGHA, DESC_WSHD_RWN, DT_Single);
    mdi.AddOutput(VAR_WSHD_NITN, UNIT_CONT_KGHA, DESC_WSHD_NITN, DT_Single);
    mdi.AddOutput(VAR_WSHD_VOLN, UNIT_CONT_KGHA, DESC_WSHD_VOLN, DT_Single);
    mdi.AddOutput(VAR_WSHD_PAL, UNIT_CONT_KGHA, DESC_WSHD_PAL, DT_Single);
    mdi.AddOutput(VAR_WSHD_PAS, UNIT_CONT_KGHA, DESC_WSHD_PAS, DT_Single);

    mdi.AddOutput(VAR_SOL_AORGN, UNIT_CONT_KGHA, DESC_SOL_AORGN, DT_Raster2D);
    mdi.AddOutput(VAR_SOL_FORGN, UNIT_CONT_KGHA, DESC_SOL_FORGN, DT_Raster2D);
    mdi.AddOutput(VAR_SOL_NO3, UNIT_CONT_KGHA, DESC_SOL_NO3, DT_Raster2D);
    mdi.AddOutput(VAR_SOL_NH4, UNIT_CONT_KGHA, DESC_SOL_NH4, DT_Raster2D);
    mdi.AddOutput(VAR_SOL_SORGN, UNIT_CONT_KGHA, DESC_SOL_SORGN, DT_Raster2D);

	mdi.AddOutput(VAR_SOL_FORGP, UNIT_CONT_KGHA, DESC_SOL_FORGP, DT_Raster2D);
    mdi.AddOutput(VAR_SOL_HORGP, UNIT_CONT_KGHA, DESC_SOL_HORGP, DT_Raster2D);
    mdi.AddOutput(VAR_SOL_SOLP, UNIT_CONT_KGHA, DESC_SOL_SOLP, DT_Raster2D);
    mdi.AddOutput(VAR_SOL_ACTP, UNIT_CONT_KGHA, DESC_SOL_ACTP, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_STAP, UNIT_CONT_KGHA, DESC_SOL_STAP, DT_Raster2D);

	/***** Outputs of CENTURY model *******/
	mdi.AddOutput(VAR_SOL_WOC, UNIT_CONT_KGHA, DESC_SOL_WOC, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_WON, UNIT_CONT_KGHA, DESC_SOL_WON, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_BM, UNIT_CONT_KGHA, DESC_SOL_BM, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_BMC, UNIT_CONT_KGHA, DESC_SOL_BMC, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_BMN, UNIT_CONT_KGHA, DESC_SOL_BMN, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_HP, UNIT_CONT_KGHA, DESC_SOL_HP, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_HS, UNIT_CONT_KGHA, DESC_SOL_HS, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_HSC, UNIT_CONT_KGHA, DESC_SOL_HSC, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_HSN, UNIT_CONT_KGHA, DESC_SOL_HSN, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_HPC, UNIT_CONT_KGHA, DESC_SOL_HPC, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_HPN, UNIT_CONT_KGHA, DESC_SOL_HPN, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_LM, UNIT_CONT_KGHA, DESC_SOL_LM, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_LMC, UNIT_CONT_KGHA, DESC_SOL_LMC, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_LMN, UNIT_CONT_KGHA, DESC_SOL_LMN, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_LSC, UNIT_CONT_KGHA, DESC_SOL_LSC, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_LSN, UNIT_CONT_KGHA, DESC_SOL_LSN, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_LS, UNIT_CONT_KGHA, DESC_SOL_LS, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_LSL, UNIT_CONT_KGHA, DESC_SOL_LSL, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_LSLC, UNIT_CONT_KGHA, DESC_SOL_LSLC, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_LSLNC, UNIT_CONT_KGHA, DESC_SOL_LSLNC, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_RNMN, UNIT_CONT_KGHA, DESC_SOL_RNMN, DT_Raster2D);
	mdi.AddOutput(VAR_SOL_RSPC, UNIT_CONT_KGHA, DESC_SOL_RSPC, DT_Raster2D);

	mdi.AddOutput(VAR_CONV_WT, UNIT_NON_DIM, DESC_CONV_WT, DT_Raster2D);
    string res = mdi.GetXMLDocument();
    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}