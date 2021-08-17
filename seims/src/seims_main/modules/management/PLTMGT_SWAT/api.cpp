#include "api.h"

#include "managementOperation_SWAT.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new MGTOpt_SWAT();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    string res;

    mdi.SetAuthor("Liangjun Zhu");
    mdi.SetClass(MCLS_MGT[0], MCLS_MGT[1]);
    mdi.SetDescription(M_PLTMGT_SWAT[1]);
    mdi.SetID(M_PLTMGT_SWAT[0]);
    mdi.SetName(M_PLTMGT_SWAT[0]);
    mdi.SetVersion("1.3");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    /// set parameters from database
    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CSWAT[0], UNIT_NON_DIM, VAR_CSWAT[1], Source_ParameterDB, DT_Single);
    //mdi.AddParameter(VAR_BACT_SWF[0], UNIT_NON_DIM, VAR_BACT_SWF[1], Source_ParameterDB, DT_Single); ///TODO
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    /// basic parameters
    mdi.AddParameter(VAR_SUBBASIN_PARAM[0], UNIT_NON_DIM, VAR_SUBBASIN_PARAM[1], Source_ParameterDB, DT_Subbasin);
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1D);
    /// soil
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_ZMX[0], UNIT_DEPTH_MM, VAR_SOL_ZMX[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_SUMAWC[0], UNIT_DEPTH_MM, VAR_SOL_SUMAWC[1], Source_ParameterDB, DT_Raster1D); /// m_soilSumFC
    mdi.AddParameter(VAR_SOILDEPTH[0], UNIT_DEPTH_MM, VAR_SOILDEPTH[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_BD[0], UNIT_DENSITY, VAR_SOL_BD[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_CBN[0], UNIT_PERCENT, VAR_SOL_CBN[1], Source_ParameterDB, DT_Raster2D);
    /// for 1-C-FARM on carbon pool model
    mdi.AddParameter(VAR_SOL_N[0], UNIT_CONT_KGHA, VAR_SOL_N[1], Source_ParameterDB, DT_Raster2D);

    mdi.AddParameter(VAR_CLAY[0], UNIT_PERCENT, VAR_CLAY[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SILT[0], UNIT_PERCENT, VAR_SILT[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SAND[0], UNIT_PERCENT, VAR_SAND[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_ROCK[0], UNIT_PERCENT, VAR_ROCK[1], Source_ParameterDB, DT_Raster2D);
    /// landuse/landcover
    mdi.AddParameter(VAR_IDC[0], UNIT_NON_DIM, VAR_IDC[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_LANDUSE[0], UNIT_NON_DIM, VAR_LANDUSE[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_LANDCOVER[0], UNIT_NON_DIM, VAR_LANDCOVER[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CN2[0], UNIT_NON_DIM, VAR_CN2[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_HVSTI[0], UNIT_CONT_RATIO, VAR_HVSTI[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_WSYF[0], UNIT_CONT_RATIO, VAR_WSYF[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PHUPLT[0], UNIT_HEAT_UNIT, VAR_PHUPLT[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_T_BASE[0], UNIT_TEMP_DEG, VAR_T_BASE[1], Source_ParameterDB, DT_Raster1D);
    /// lookup table as 2D array, such as crop, management, landuse, tillage, etc.
    mdi.AddParameter(VAR_LANDUSE_LOOKUP[0], UNIT_NON_DIM, VAR_LANDUSE_LOOKUP[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_CROP_LOOKUP[0], UNIT_NON_DIM, VAR_CROP_LOOKUP[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_TILLAGE_LOOKUP[0], UNIT_NON_DIM, VAR_TILLAGE_LOOKUP[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_FERTILIZER_LOOKUP[0], UNIT_NON_DIM, VAR_FERTILIZER_LOOKUP[1], Source_ParameterDB, DT_Array2D);

    /// set scenario data
    mdi.AddParameter(VAR_SCENARIO[0], UNIT_NON_DIM, VAR_SCENARIO[1], Source_ParameterDB, DT_Scenario);

    mdi.AddParameter(VAR_SOL_SORGN[0], UNIT_CONT_KGHA, VAR_SOL_SORGN[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_HORGP[0], UNIT_CONT_KGHA, VAR_SOL_HORGP[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SOLP[0], UNIT_CONT_KGHA, VAR_SOL_SOLP[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_NH4[0], UNIT_CONT_KGHA, VAR_SOL_NH4[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_NO3[0], UNIT_CONT_KGHA, VAR_SOL_NO3[1], Source_Module, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_AWC[0], UNIT_DEPTH_MM, VAR_SOL_AWC[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_UL[0], UNIT_DEPTH_MM, VAR_SOL_UL[1], Source_ParameterDB, DT_Raster2D);
    /// set input from other modules
    /// soil properties
    mdi.AddInput(VAR_SOL_AORGN[0], UNIT_CONT_KGHA, VAR_SOL_AORGN[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_FORGN[0], UNIT_CONT_KGHA, VAR_SOL_FORGN[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_FORGP[0], UNIT_CONT_KGHA, VAR_SOL_FORGP[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_ACTP[0], UNIT_CONT_KGHA, VAR_SOL_ACTP[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_STAP[0], UNIT_CONT_KGHA, VAR_SOL_STAP[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_RSD[0], UNIT_CONT_KGHA, VAR_SOL_RSD[1], Source_Module, DT_Raster2D);

    /// landuse/landcover
    mdi.AddInput(VAR_PHUBASE[0], UNIT_HEAT_UNIT, VAR_PHUBASE[1], Source_Module, DT_Raster1D);       /// PET modules
    mdi.AddInput(VAR_IGRO[0], UNIT_NON_DIM, VAR_IGRO[1], Source_Module, DT_Raster1D);               /// PG_EPIC module
    mdi.AddInput(VAR_FR_PHU_ACC[0], UNIT_HEAT_UNIT, VAR_FR_PHU_ACC[1], Source_Module, DT_Raster1D); /// PG_EPIC module
    mdi.AddParameter(VAR_TREEYRS[0], UNIT_YEAR, VAR_TREEYRS[1], Source_ParameterDB, DT_Raster1D);
    /// m_curYearMat, from ParameterDB
    mdi.AddInput(VAR_HVSTI_ADJ[0], UNIT_CONT_RATIO, VAR_HVSTI_ADJ[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_LAIDAY[0], UNIT_AREA_RATIO, VAR_LAIDAY[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DORMI[0], UNIT_NON_DIM, VAR_DORMI[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_LAIMAXFR[0], UNIT_NON_DIM, VAR_LAIMAXFR[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_OLAI[0], UNIT_AREA_RATIO, VAR_OLAI[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PLANT_N[0], UNIT_CONT_KGHA, VAR_PLANT_N[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PLANT_P[0], UNIT_CONT_KGHA, VAR_PLANT_P[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_FR_PLANT_N[0], UNIT_NON_DIM, VAR_FR_PLANT_N[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_FR_PLANT_P[0], UNIT_NON_DIM, VAR_FR_PLANT_P[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PLTET_TOT[0], UNIT_DEPTH_MM, VAR_PLTET_TOT[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PLTPET_TOT[0], UNIT_DEPTH_MM, VAR_PLTPET_TOT[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_FR_ROOT[0], UNIT_NON_DIM, VAR_FR_ROOT[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_BIOMASS[0], UNIT_CONT_KGHA, VAR_BIOMASS[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_LAST_SOILRD[0], UNIT_DEPTH_MM, VAR_LAST_SOILRD[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_FR_STRSWTR[0], UNIT_NON_DIM, VAR_FR_STRSWTR[1], Source_Module, DT_Raster1D);

    /// groundwater table, currently, shallow and deep aquifer are not distinguished
    //mdi.AddInput(VAR_DEEPST[0], UNIT_DEPTH_MM, VAR_DEEPST[1], Source_Module, DT_Raster1D);
    //mdi.AddInput(VAR_SHALLST[0], UNIT_DEPTH_MM, VAR_SHALLST[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SBGS[0], UNIT_DEPTH_MM, VAR_SBGS[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_POT_VOL[0], UNIT_DEPTH_MM, VAR_POT_VOL[1], Source_Module_Optional, DT_Raster1D); /// IMP_SWAT
    mdi.AddInput(VAR_POT_NO3[0], UNIT_KG, VAR_POT_NO3[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_POT_NH4[0], UNIT_KG, VAR_POT_NH4[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_POT_SOLP[0], UNIT_PER_DAY, VAR_POT_SOLP[1], Source_Module_Optional, DT_Raster1D);

    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_SW[0], UNIT_DEPTH_MM, VAR_SOL_SW[1], Source_Module, DT_Raster1D); /// sol_sw in SWAT

    /**** set inputs for CENTURY C/N cycling model derived from NUTR_TF module, if CSWAT = 2. As optional inputs. ****/
    /// for fertilizer operation
    mdi.AddInput(VAR_SOL_HSN[0], UNIT_CONT_KGHA, VAR_SOL_HSN[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LM[0], UNIT_CONT_KGHA, VAR_SOL_LM[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LMC[0], UNIT_CONT_KGHA, VAR_SOL_LMC[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LMN[0], UNIT_CONT_KGHA, VAR_SOL_LMN[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSC[0], UNIT_CONT_KGHA, VAR_SOL_LSC[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSN[0], UNIT_CONT_KGHA, VAR_SOL_LSN[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LS[0], UNIT_CONT_KGHA, VAR_SOL_LS[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSL[0], UNIT_CONT_KGHA, VAR_SOL_LSL[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSLC[0], UNIT_CONT_KGHA, VAR_SOL_LSLC[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSLNC[0], UNIT_CONT_KGHA, VAR_SOL_LSLNC[1], Source_Module_Optional, DT_Raster2D);

    //mdi.AddInput(VAR_SOL_WON[0], UNIT_CONT_KGHA, VAR_SOL_WON[1], Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_BM[0], UNIT_CONT_KGHA, VAR_SOL_BM[1], Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_BMC[0], UNIT_CONT_KGHA, VAR_SOL_BMC[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_BMN[0], UNIT_CONT_KGHA, VAR_SOL_BMN[1], Source_Module, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_HP[0], UNIT_CONT_KGHA, VAR_SOL_HP[1], Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_HS[0], UNIT_CONT_KGHA, VAR_SOL_HS[1], Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_HSC[0], UNIT_CONT_KGHA, VAR_SOL_HSC[1], Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_HPC[0], UNIT_CONT_KGHA, VAR_SOL_HPC[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_HPN[0], UNIT_CONT_KGHA, VAR_SOL_HPN[1], Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_RNMN[0], UNIT_CONT_KGHA, VAR_SOL_RNMN[1], Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_RSPC[0], UNIT_CONT_KGHA, VAR_SOL_RSPC[1], Source_Module_Optional, DT_Raster2D);

    mdi.AddInput(VAR_POT_SA[0], UNIT_AREA_HA, VAR_POT_SA[1], Source_Module_Optional, DT_Raster1D);

    /// set the output variables

    ///// outputs of plant operation.
    mdi.AddOutput(VAR_BIOTARG[0], UNIT_CONT_KGHA, VAR_BIOTARG[1], DT_Raster1D);
    mdi.AddOutput(VAR_HVSTI_TARG[0], UNIT_NON_DIM, VAR_HVSTI_TARG[1], DT_Raster1D);
    ///// outputs of irrigation / autoIrrigation operation
    mdi.AddOutput(VAR_IRR_FLAG[0], UNIT_NON_DIM, VAR_IRR_FLAG[1], DT_Raster1D);
    mdi.AddOutput(VAR_IRR_WTR[0], UNIT_DEPTH_MM, VAR_IRR_WTR[1], DT_Raster1D);
    mdi.AddOutput(VAR_IRR_SURFQ[0], UNIT_DEPTH_MM, VAR_IRR_SURFQ[1], DT_Raster1D);
    /// defined in auto irrigation operation
    mdi.AddOutput(VAR_AWTR_STRS_ID[0], UNIT_NON_DIM, VAR_AWTR_STRS_ID[1], DT_Raster1D);
    mdi.AddOutput(VAR_AWTR_STRS_TRIG[0], UNIT_NON_DIM, VAR_AWTR_STRS_TRIG[1], DT_Raster1D);
    mdi.AddOutput(VAR_AIRR_SOURCE[0], UNIT_NON_DIM, VAR_AIRR_SOURCE[1], DT_Raster1D);
    mdi.AddOutput(VAR_AIRR_LOCATION[0], UNIT_NON_DIM, VAR_AIRR_LOCATION[1], DT_Raster1D);
    mdi.AddOutput(VAR_AIRR_EFF[0], UNIT_NON_DIM, VAR_AIRR_EFF[1], DT_Raster1D);
    mdi.AddOutput(VAR_AIRRWTR_DEPTH[0], UNIT_DEPTH_MM, VAR_AIRRWTR_DEPTH[1], DT_Raster1D);
    mdi.AddOutput(VAR_AIRRSURF_RATIO[0], UNIT_NON_DIM, VAR_AIRRSURF_RATIO[1], DT_Raster1D);
    /// outputs of fertilizer / auto fertilizer operations
    mdi.AddOutput(VAR_AFERT_ID[0], UNIT_NON_DIM, VAR_AFERT_ID[1], DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_NSTRSID[0], UNIT_NON_DIM, VAR_AFERT_NSTRSID[1], DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_NSTRS[0], UNIT_NON_DIM, VAR_AFERT_NSTRS[1], DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_MAXN[0], UNIT_CONT_KGHA, VAR_AFERT_MAXN[1], DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_AMAXN[0], UNIT_CONT_KGHA, VAR_AFERT_AMAXN[1], DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_NYLDT[0], UNIT_NON_DIM, VAR_AFERT_NYLDT[1], DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_FRTEFF[0], UNIT_NON_DIM, VAR_AFERT_FRTEFF[1], DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_FRTSURF[0], UNIT_NON_DIM, VAR_AFERT_FRTSURF[1], DT_Raster1D);
    /// for 1-C-FARM on carbon pool model
    mdi.AddOutput(VAR_SOL_MC[0], UNIT_CONT_KGHA, VAR_SOL_MC[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_MN[0], UNIT_CONT_KGHA, VAR_SOL_MN[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_MP[0], UNIT_CONT_KGHA, VAR_SOL_MP[1], DT_Raster2D);
    //// outputs of grazing operation
    mdi.AddOutput(VAR_GRZ_DAYS[0], UNIT_NON_DIM, VAR_GRZ_DAYS[1], DT_Raster1D);
    mdi.AddOutput(VAR_GRZ_FLAG[0], UNIT_NON_DIM, VAR_GRZ_FLAG[1], DT_Raster1D);
    //// output of impound/release operation
    mdi.AddOutput(VAR_IMPOUND_TRIG[0], UNIT_NON_DIM, VAR_IMPOUND_TRIG[1], DT_Raster1D);
    mdi.AddOutput(VAR_POT_VOLMAXMM[0], UNIT_DEPTH_MM, VAR_POT_VOLMAXMM[1], DT_Raster1D);
    mdi.AddOutput(VAR_POT_VOLLOWMM[0], UNIT_DEPTH_MM, VAR_POT_VOLLOWMM[1], DT_Raster1D);
    /// outputs of tillage operation during CENTURY model
    mdi.AddOutput(VAR_TILLAGE_DAYS[0], UNIT_DAY, VAR_TILLAGE_DAYS[1], DT_Raster1D);
    mdi.AddOutput(VAR_TILLAGE_DEPTH[0], UNIT_DAY, VAR_TILLAGE_DEPTH[1], DT_Raster1D);
    mdi.AddOutput(VAR_TILLAGE_SWITCH[0], UNIT_DAY, VAR_TILLAGE_SWITCH[1], DT_Raster1D);
    mdi.AddOutput(VAR_TILLAGE_FACTOR[0], UNIT_DAY, VAR_TILLAGE_FACTOR[1], DT_Raster1D);

    /// write out the XML file.
    res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
