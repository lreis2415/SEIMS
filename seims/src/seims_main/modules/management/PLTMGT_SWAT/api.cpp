#include "managementOperation_SWAT.h"

#include "api.h"

extern "C" SEIMS_MODULE_API SimulationModule *

GetInstance() {
    return new MGTOpt_SWAT();
}

/// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;
    string res;

    mdi.SetAuthor("Liang-Jun Zhu");
    mdi.SetClass(MCLS_MGT, MCLSDESC_MGT);
    mdi.SetDescription(MDESC_PLTMGT_SWAT);
    mdi.SetID(MID_PLTMGT_SWAT);
    mdi.SetName(MID_PLTMGT_SWAT);
    mdi.SetVersion("0.1");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    /// set parameters from database
    mdi.AddParameter(VAR_CSWAT, UNIT_NON_DIM, DESC_CSWAT, Source_ParameterDB, DT_Single);
    //mdi.AddParameter(VAR_BACT_SWF, UNIT_NON_DIM, DESC_BACT_SWF, Source_ParameterDB, DT_Single); ///TODO
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
    /// basic parameters
    mdi.AddParameter(VAR_SUBBASIN_PARAM, UNIT_NON_DIM, DESC_SUBBASIN_PARAM, Source_ParameterDB, DT_Subbasin);
    mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);
    //mdi.AddParameter(VAR_MGT_FIELD, UNIT_NON_DIM, DESC_MGT_FIELD, Source_ParameterDB, DT_Raster1D); // remove by lj, 08/16/17
    /// soil
    mdi.AddParameter(VAR_SOILLAYERS, UNIT_NON_DIM, DESC_SOILLAYERS, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_ZMX, UNIT_DEPTH_MM, DESC_SOL_ZMX, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_SUMAWC, UNIT_DEPTH_MM, DESC_SOL_SUMAWC, Source_ParameterDB, DT_Raster1D); /// m_soilSumFC
    mdi.AddParameter(VAR_SOILDEPTH, UNIT_DEPTH_MM, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILTHICK, UNIT_DEPTH_MM, DESC_SOILTHICK, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_BD, UNIT_DENSITY, DESC_SOL_BD, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_CBN, UNIT_PERCENT, DESC_SOL_CBN, Source_ParameterDB, DT_Raster2D);
    /// for 1-C-FARM on carbon pool model
    mdi.AddParameter(VAR_SOL_N, UNIT_CONT_KGHA, DESC_SOL_N, Source_ParameterDB, DT_Raster2D);

    mdi.AddParameter(VAR_CLAY, UNIT_PERCENT, DESC_CLAY, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SILT, UNIT_PERCENT, DESC_SILT, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SAND, UNIT_PERCENT, DESC_SAND, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_ROCK, UNIT_PERCENT, DESC_ROCK, Source_ParameterDB, DT_Raster2D);
    /// landuse/landcover
    mdi.AddParameter(VAR_IDC, UNIT_NON_DIM, DESC_IDC, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_LANDUSE, UNIT_NON_DIM, DESC_LANDUSE, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_LANDCOVER, UNIT_NON_DIM, DESC_LANDCOVER, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CN2, UNIT_NON_DIM, DESC_CN2, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_HVSTI, UNIT_CONT_RATIO, DESC_HVSTI, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_WSYF, UNIT_CONT_RATIO, DESC_WSYF, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PHUPLT, UNIT_HEAT_UNIT, DESC_PHUPLT, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_T_BASE, UNIT_TEMP_DEG, DESC_T_BASE, Source_ParameterDB, DT_Raster1D);
    /// lookup table as 2D array, such as crop, management, landuse, tillage, etc.
    mdi.AddParameter(VAR_LANDUSE_LOOKUP, UNIT_NON_DIM, DESC_LANDUSE_LOOKUP, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_CROP_LOOKUP, UNIT_NON_DIM, DESC_CROP_LOOKUP, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_TILLAGE_LOOKUP, UNIT_NON_DIM, DESC_TILLAGE_LOOKUP, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_FERTILIZER_LOOKUP, UNIT_NON_DIM, DESC_FERTILIZER_LOOKUP, Source_ParameterDB, DT_Array2D);

    /// set scenario data
    mdi.AddParameter(VAR_SCENARIO, UNIT_NON_DIM, DESC_SCENARIO, Source_ParameterDB, DT_Scenario);

    mdi.AddParameter(VAR_SOL_SORGN, UNIT_CONT_KGHA, DESC_SOL_SORGN, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_HORGP, UNIT_CONT_KGHA, DESC_SOL_HORGP, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SOLP, UNIT_CONT_KGHA, DESC_SOL_SOLP, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_NH4, UNIT_CONT_KGHA, DESC_SOL_NH4, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_NO3, UNIT_CONT_KGHA, DESC_SOL_NO3, Source_Module, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_AWC, UNIT_DEPTH_MM, DESC_SOL_AWC, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_UL, UNIT_DEPTH_MM, DESC_SOL_UL, Source_ParameterDB, DT_Raster2D);
    /// set input from other modules
    /// soil properties
    mdi.AddInput(VAR_SOL_AORGN, UNIT_CONT_KGHA, DESC_SOL_AORGN, Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_FORGN, UNIT_CONT_KGHA, DESC_SOL_FORGN, Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_FORGP, UNIT_CONT_KGHA, DESC_SOL_FORGP, Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_ACTP, UNIT_CONT_KGHA, DESC_SOL_ACTP, Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_STAP, UNIT_CONT_KGHA, DESC_SOL_STAP, Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_RSD, UNIT_CONT_KGHA, DESC_SOL_RSD, Source_Module, DT_Raster2D);

    /// landuse/landcover
    mdi.AddInput(VAR_PHUBASE, UNIT_HEAT_UNIT, DESC_PHUBASE, Source_Module, DT_Raster1D); /// PET modules
    mdi.AddInput(VAR_IGRO, UNIT_NON_DIM, DESC_IGRO, Source_Module, DT_Raster1D); /// BIO_EPIC module
    mdi.AddInput(VAR_FR_PHU_ACC, UNIT_HEAT_UNIT, DESC_FR_PHU_ACC, Source_Module, DT_Raster1D); /// BIO_EPIC module
    mdi.AddParameter(VAR_TREEYRS, UNIT_YEAR, DESC_TREEYRS, Source_ParameterDB,
                     DT_Raster1D); /// m_curYearMat, from ParameterDB
    mdi.AddInput(VAR_HVSTI_ADJ, UNIT_CONT_RATIO, DESC_HVSTI_ADJ, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_LAIDAY, UNIT_AREA_RATIO, DESC_LAIDAY, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DORMI, UNIT_NON_DIM, DESC_DORMI, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_LAIMAXFR, UNIT_NON_DIM, DESC_LAIMAXFR, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_OLAI, UNIT_AREA_RATIO, DESC_OLAI, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PLANT_N, UNIT_CONT_KGHA, DESC_PLANT_N, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PLANT_P, UNIT_CONT_KGHA, DESC_PLANT_P, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_FR_PLANT_N, UNIT_NON_DIM, DESC_FR_PLANT_N, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_FR_PLANT_P, UNIT_NON_DIM, DESC_FR_PLANT_P, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PLTET_TOT, UNIT_DEPTH_MM, DESC_PLTET_TOT, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PLTPET_TOT, UNIT_DEPTH_MM, DESC_PLTPET_TOT, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_FR_ROOT, UNIT_NON_DIM, DESC_FR_ROOT, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_BIOMASS, UNIT_CONT_KGHA, DESC_BIOMASS, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_LAST_SOILRD, UNIT_DEPTH_MM, DESC_LAST_SOILRD, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_FR_STRSWTR, UNIT_NON_DIM, DESC_FR_STRSWTR, Source_Module, DT_Raster1D);

    /// groundwater table, currently, shallow and deep aquifer are not distinguished
    //mdi.AddInput(VAR_DEEPST, UNIT_DEPTH_MM, DESC_DEEPST, Source_Module, DT_Raster1D);
    //mdi.AddInput(VAR_SHALLST, UNIT_DEPTH_MM, DESC_SHALLST, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SBGS, UNIT_DEPTH_MM, DESC_SBGS, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_POT_VOL, UNIT_DEPTH_MM, DESC_POT_VOL, Source_Module_Optional, DT_Raster1D); /// IMP_SWAT
    mdi.AddInput(VAR_POT_NO3, UNIT_KG, DESC_POT_NO3, Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_POT_NH4, UNIT_KG, DESC_POT_NH4, Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_POT_SOLP, UNIT_PER_DAY, DESC_POT_SOLP, Source_Module_Optional, DT_Raster1D);

    mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_SW, UNIT_DEPTH_MM, DESC_SOL_SW, Source_Module, DT_Raster1D); /// sol_sw in SWAT

    /**** set inputs for CENTURY C/N cycling model derived from NUTR_TF module, if CSWAT = 2. As optional inputs. ****/
    /// for fertilizer operation
    mdi.AddInput(VAR_SOL_HSN, UNIT_CONT_KGHA, DESC_SOL_HSN, Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LM, UNIT_CONT_KGHA, DESC_SOL_LM, Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LMC, UNIT_CONT_KGHA, DESC_SOL_LMC, Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LMN, UNIT_CONT_KGHA, DESC_SOL_LMN, Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSC, UNIT_CONT_KGHA, DESC_SOL_LSC, Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSN, UNIT_CONT_KGHA, DESC_SOL_LSN, Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LS, UNIT_CONT_KGHA, DESC_SOL_LS, Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSL, UNIT_CONT_KGHA, DESC_SOL_LSL, Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSLC, UNIT_CONT_KGHA, DESC_SOL_LSLC, Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSLNC, UNIT_CONT_KGHA, DESC_SOL_LSLNC, Source_Module_Optional, DT_Raster2D);

    //mdi.AddInput(VAR_SOL_WON, UNIT_CONT_KGHA, DESC_SOL_WON, Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_BM, UNIT_CONT_KGHA, DESC_SOL_BM, Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_BMC, UNIT_CONT_KGHA, DESC_SOL_BMC, Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_BMN, UNIT_CONT_KGHA, DESC_SOL_BMN, Source_Module, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_HP, UNIT_CONT_KGHA, DESC_SOL_HP, Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_HS, UNIT_CONT_KGHA, DESC_SOL_HS, Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_HSC, UNIT_CONT_KGHA, DESC_SOL_HSC, Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_HPC, UNIT_CONT_KGHA, DESC_SOL_HPC, Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_HPN, UNIT_CONT_KGHA, DESC_SOL_HPN, Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_RNMN, UNIT_CONT_KGHA, DESC_SOL_RNMN, Source_Module_Optional, DT_Raster2D);
    //mdi.AddInput(VAR_SOL_RSPC, UNIT_CONT_KGHA, DESC_SOL_RSPC, Source_Module_Optional, DT_Raster2D);

    mdi.AddInput(VAR_POT_SA, UNIT_AREA_HA, DESC_POT_SA, Source_Module_Optional, DT_Raster1D);

    /// set the output variables

    ///// outputs of plant operation. NO NEED TO OUTPUT?
    mdi.AddOutput(VAR_BIOTARG, UNIT_CONT_KGHA, DESC_BIOTARG, DT_Raster1D);
    mdi.AddOutput(VAR_HVSTI_TARG, UNIT_NON_DIM, DESC_HVSTI_TARG, DT_Raster1D);
    ///// outputs of irrigation / autoIrrigation operation
    mdi.AddOutput(VAR_IRR_FLAG, UNIT_NON_DIM, DESC_IRR_FLAG, DT_Raster1D);
    mdi.AddOutput(VAR_IRR_WTR, UNIT_DEPTH_MM, DESC_IRR_WTR, DT_Raster1D);
    mdi.AddOutput(VAR_IRR_SURFQ, UNIT_DEPTH_MM, DESC_IRR_SURFQ, DT_Raster1D);
    /// defined in auto irrigation operation
    mdi.AddOutput(VAR_AWTR_STRS_ID, UNIT_NON_DIM, DESC_AWTR_STRS_ID, DT_Raster1D);
    mdi.AddOutput(VAR_AWTR_STRS_TRIG, UNIT_NON_DIM, DESC_AWTR_STRS_TRIG, DT_Raster1D);
    mdi.AddOutput(VAR_AIRR_SOURCE, UNIT_NON_DIM, DESC_AIRR_SOURCE, DT_Raster1D);
    mdi.AddOutput(VAR_AIRR_LOCATION, UNIT_NON_DIM, DESC_AIRR_LOCATION, DT_Raster1D);
    mdi.AddOutput(VAR_AIRR_EFF, UNIT_NON_DIM, DESC_AIRR_EFF, DT_Raster1D);
    mdi.AddOutput(VAR_AIRRWTR_DEPTH, UNIT_DEPTH_MM, DESC_AIRRWTR_DEPTH, DT_Raster1D);
    mdi.AddOutput(VAR_AIRRSURF_RATIO, UNIT_NON_DIM, DESC_AIRRSURF_RATIO, DT_Raster1D);
    /// outputs of fertilizer / auto fertilizer operations
    mdi.AddOutput(VAR_AFERT_ID, UNIT_NON_DIM, DESC_AFERT_ID, DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_NSTRSID, UNIT_NON_DIM, DESC_AFERT_NSTRSID, DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_NSTRS, UNIT_NON_DIM, DESC_AFERT_NSTRS, DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_MAXN, UNIT_CONT_KGHA, DESC_AFERT_MAXN, DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_AMAXN, UNIT_CONT_KGHA, DESC_AFERT_AMAXN, DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_NYLDT, UNIT_NON_DIM, DESC_AFERT_NYLDT, DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_FRTEFF, UNIT_NON_DIM, DESC_AFERT_FRTEFF, DT_Raster1D);
    mdi.AddOutput(VAR_AFERT_FRTSURF, UNIT_NON_DIM, DESC_AFERT_FRTSURF, DT_Raster1D);
    /// for 1-C-FARM on carbon pool model
    mdi.AddOutput(VAR_SOL_MC, UNIT_CONT_KGHA, DESC_SOL_MC, DT_Raster2D);
    mdi.AddOutput(VAR_SOL_MN, UNIT_CONT_KGHA, DESC_SOL_MN, DT_Raster2D);
    mdi.AddOutput(VAR_SOL_MP, UNIT_CONT_KGHA, DESC_SOL_MP, DT_Raster2D);
    //// outputs of grazing operation
    mdi.AddOutput(VAR_GRZ_DAYS, UNIT_NON_DIM, DESC_GRZ_DAYS, DT_Raster1D);
    mdi.AddOutput(VAR_GRZ_FLAG, UNIT_NON_DIM, DESC_GRZ_FLAG, DT_Raster1D);
    //// output of impound/release operation
    mdi.AddOutput(VAR_IMPOUND_TRIG, UNIT_NON_DIM, DESC_IMPOUND_TRIG, DT_Raster1D);
    mdi.AddOutput(VAR_POT_VOLMAXMM, UNIT_DEPTH_MM, DESC_POT_VOLMAXMM, DT_Raster1D);
    mdi.AddOutput(VAR_POT_VOLLOWMM, UNIT_DEPTH_MM, DESC_POT_VOLLOWMM, DT_Raster1D);
    /// outputs of tillage operation during CENTURY model
    mdi.AddOutput(VAR_TILLAGE_DAYS, UNIT_DAY, DESC_TILLAGE_DAYS, DT_Raster1D);
    mdi.AddOutput(VAR_TILLAGE_DEPTH, UNIT_DAY, DESC_TILLAGE_DEPTH, DT_Raster1D);
    mdi.AddOutput(VAR_TILLAGE_SWITCH, UNIT_DAY, DESC_TILLAGE_SWITCH, DT_Raster1D);
    mdi.AddOutput(VAR_TILLAGE_FACTOR, UNIT_DAY, DESC_TILLAGE_FACTOR, DT_Raster1D);

    /// write out the XML file.
    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}


