#include "api.h"

#include "Nutrient_Transformation.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new Nutrient_Transformation();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    mdi.SetAuthor("Huiran Gao, Liangjun Zhu");
    mdi.SetClass(MCLS_NUTRCYC[0], MCLS_NUTRCYC[1]);
    mdi.SetDescription(M_NUTR_TF[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_NUTR_TF[0]);
    mdi.SetName(M_NUTR_TF[0]);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CMN[0], UNIT_NON_DIM, VAR_CMN[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CDN[0], UNIT_NON_DIM, VAR_CDN[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NACTFR[0], UNIT_NON_DIM, VAR_NACTFR[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SDNCO[0], UNIT_NON_DIM, VAR_SDNCO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_PSP[0], UNIT_NON_DIM, VAR_PSP[1], Source_ParameterDB, DT_Single);

    /// idplt in SWAT is a lookup array. in SEIMS, use landcover
    mdi.AddParameter(VAR_LANDCOVER[0], UNIT_NON_DIM, VAR_LANDCOVER[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PL_RSDCO[0], UNIT_NON_DIM, VAR_PL_RSDCO[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILDEPTH[0], UNIT_DEPTH_MM, VAR_SOILDEPTH[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_RSDIN[0], UNIT_CONT_KGHA, VAR_SOL_RSDIN[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CSWAT[0], UNIT_NON_DIM, VAR_CSWAT[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_CBN[0], UNIT_PERCENT, VAR_SOL_CBN[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_CLAY[0], UNIT_PERCENT, VAR_CLAY[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_ROCK[0], UNIT_PERCENT, VAR_ROCK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_BD[0], UNIT_DENSITY, VAR_SOL_BD[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_AWC[0], UNIT_DEPTH_MM, VAR_SOL_AWC[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_WPMM[0], UNIT_DEPTH_MM, VAR_SOL_WPMM[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_NO3[0], UNIT_CONT_KGHA, VAR_SOL_NO3[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_NH4[0], UNIT_CONT_KGHA, VAR_SOL_NH4[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SORGN[0], UNIT_CONT_KGHA, VAR_SOL_SORGN[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_HORGP[0], UNIT_CONT_KGHA, VAR_SOL_HORGP[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SOLP[0], UNIT_CONT_KGHA, VAR_SOL_SOLP[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_UL[0], UNIT_DEPTH_MM, VAR_SOL_UL[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POROST[0], UNIT_VOL_FRA_M3M3, VAR_POROST[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SAND[0], UNIT_PERCENT, VAR_SAND[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_UL[0], UNIT_DEPTH_MM, VAR_SOL_UL[1], Source_ParameterDB, DT_Raster2D);

    mdi.AddInput(VAR_SOL_COV[0], UNIT_CONT_KGHA, VAR_SOL_COV[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_SOL_RSD[0], UNIT_CONT_KGHA, VAR_SOL_RSD[1], Source_Module_Optional, DT_Raster2D);

    // from other modules
    mdi.AddInput(VAR_SOTE[0], UNIT_TEMP_DEG, VAR_SOTE[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);

    /// tillage operation during CENTURY model
    mdi.AddInput(VAR_TILLAGE_DAYS[0], UNIT_DAY, VAR_TILLAGE_DAYS[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_TILLAGE_DEPTH[0], UNIT_DAY, VAR_TILLAGE_DEPTH[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_TILLAGE_SWITCH[0], UNIT_DAY, VAR_TILLAGE_SWITCH[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_TILLAGE_FACTOR[0], UNIT_DAY, VAR_TILLAGE_FACTOR[1], Source_Module_Optional, DT_Raster1D);

    mdi.AddOutput(VAR_SOL_COV[0], UNIT_CONT_KGHA, VAR_SOL_COV[1], DT_Raster1D);
    mdi.AddOutput(VAR_SOL_RSD[0], UNIT_CONT_KGHA, VAR_SOL_RSD[1], DT_Raster2D);

    // set the output variables

    mdi.AddOutput(VAR_HMNTL[0], UNIT_CONT_KGHA, VAR_HMNTL[1], DT_Raster1D);
    mdi.AddOutput(VAR_HMPTL[0], UNIT_CONT_KGHA, VAR_HMPTL[1], DT_Raster1D);
    mdi.AddOutput(VAR_RMN2TL[0], UNIT_CONT_KGHA, VAR_RMN2TL[1], DT_Raster1D);
    mdi.AddOutput(VAR_RMPTL[0], UNIT_CONT_KGHA, VAR_RMPTL[1], DT_Raster1D);
    mdi.AddOutput(VAR_RWNTL[0], UNIT_CONT_KGHA, VAR_RWNTL[1], DT_Raster1D);
    mdi.AddOutput(VAR_WDNTL[0], UNIT_CONT_KGHA, VAR_WDNTL[1], DT_Raster1D);
    mdi.AddOutput(VAR_RMP1TL[0], UNIT_CONT_KGHA, VAR_RMP1TL[1], DT_Raster1D);
    mdi.AddOutput(VAR_ROCTL[0], UNIT_CONT_KGHA, VAR_ROCTL[1], DT_Raster1D);
    /// watershed statistics, total 10
    mdi.AddOutput(VAR_WSHD_DNIT[0], UNIT_CONT_KGHA, VAR_WSHD_DNIT[1], DT_Single);
    mdi.AddOutput(VAR_WSHD_HMN[0], UNIT_CONT_KGHA, VAR_WSHD_HMN[1], DT_Single);
    mdi.AddOutput(VAR_WSHD_HMP[0], UNIT_CONT_KGHA, VAR_WSHD_HMP[1], DT_Single);
    mdi.AddOutput(VAR_WSHD_RMN[0], UNIT_CONT_KGHA, VAR_WSHD_RMN[1], DT_Single);
    mdi.AddOutput(VAR_WSHD_RMP[0], UNIT_CONT_KGHA, VAR_WSHD_RMP[1], DT_Single);
    mdi.AddOutput(VAR_WSHD_RWN[0], UNIT_CONT_KGHA, VAR_WSHD_RWN[1], DT_Single);
    mdi.AddOutput(VAR_WSHD_NITN[0], UNIT_CONT_KGHA, VAR_WSHD_NITN[1], DT_Single);
    mdi.AddOutput(VAR_WSHD_VOLN[0], UNIT_CONT_KGHA, VAR_WSHD_VOLN[1], DT_Single);
    mdi.AddOutput(VAR_WSHD_PAL[0], UNIT_CONT_KGHA, VAR_WSHD_PAL[1], DT_Single);
    mdi.AddOutput(VAR_WSHD_PAS[0], UNIT_CONT_KGHA, VAR_WSHD_PAS[1], DT_Single);

    mdi.AddOutput(VAR_SOL_AORGN[0], UNIT_CONT_KGHA, VAR_SOL_AORGN[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_FORGN[0], UNIT_CONT_KGHA, VAR_SOL_FORGN[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_NO3[0], UNIT_CONT_KGHA, VAR_SOL_NO3[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_NH4[0], UNIT_CONT_KGHA, VAR_SOL_NH4[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_SORGN[0], UNIT_CONT_KGHA, VAR_SOL_SORGN[1], DT_Raster2D);

    mdi.AddOutput(VAR_SOL_FORGP[0], UNIT_CONT_KGHA, VAR_SOL_FORGP[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_HORGP[0], UNIT_CONT_KGHA, VAR_SOL_HORGP[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_SOLP[0], UNIT_CONT_KGHA, VAR_SOL_SOLP[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_ACTP[0], UNIT_CONT_KGHA, VAR_SOL_ACTP[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_STAP[0], UNIT_CONT_KGHA, VAR_SOL_STAP[1], DT_Raster2D);

    /***** Outputs of CENTURY model *******/
    mdi.AddOutput(VAR_SOL_WOC[0], UNIT_CONT_KGHA, VAR_SOL_WOC[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_WON[0], UNIT_CONT_KGHA, VAR_SOL_WON[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_BM[0], UNIT_CONT_KGHA, VAR_SOL_BM[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_BMC[0], UNIT_CONT_KGHA, VAR_SOL_BMC[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_BMN[0], UNIT_CONT_KGHA, VAR_SOL_BMN[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_HP[0], UNIT_CONT_KGHA, VAR_SOL_HP[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_HS[0], UNIT_CONT_KGHA, VAR_SOL_HS[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_HSC[0], UNIT_CONT_KGHA, VAR_SOL_HSC[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_HSN[0], UNIT_CONT_KGHA, VAR_SOL_HSN[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_HPC[0], UNIT_CONT_KGHA, VAR_SOL_HPC[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_HPN[0], UNIT_CONT_KGHA, VAR_SOL_HPN[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_LM[0], UNIT_CONT_KGHA, VAR_SOL_LM[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_LMC[0], UNIT_CONT_KGHA, VAR_SOL_LMC[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_LMN[0], UNIT_CONT_KGHA, VAR_SOL_LMN[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_LSC[0], UNIT_CONT_KGHA, VAR_SOL_LSC[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_LSN[0], UNIT_CONT_KGHA, VAR_SOL_LSN[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_LS[0], UNIT_CONT_KGHA, VAR_SOL_LS[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_LSL[0], UNIT_CONT_KGHA, VAR_SOL_LSL[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_LSLC[0], UNIT_CONT_KGHA, VAR_SOL_LSLC[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_LSLNC[0], UNIT_CONT_KGHA, VAR_SOL_LSLNC[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_RNMN[0], UNIT_CONT_KGHA, VAR_SOL_RNMN[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_RSPC[0], UNIT_CONT_KGHA, VAR_SOL_RSPC[1], DT_Raster2D);

    mdi.AddOutput(VAR_CONV_WT[0], UNIT_NON_DIM, VAR_CONV_WT[1], DT_Raster2D);

    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
