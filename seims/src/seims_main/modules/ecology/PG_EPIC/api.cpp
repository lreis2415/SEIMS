#include "api.h"

#include "Biomass_EPIC.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new Biomass_EPIC();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Liang-Jun Zhu");
    mdi.SetClass(MCLS_PG, MCLSDESC_PG);
    mdi.SetDescription(MDESC_PG_EPIC);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_PG_EPIC);
    mdi.SetName(MID_PG_EPIC);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    /// climate parameters
    mdi.AddParameter(VAR_DAYLEN_MIN, UNIT_HOUR, DESC_DAYLEN_MIN, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DORMHR, UNIT_HOUR, DESC_DORMHR, Source_ParameterDB, DT_Raster1D);
    /// Single values
    /// in SWAT, CO2 is assigned by subbasin
    mdi.AddParameter(VAR_CO2, UNIT_GAS_PPMV, DESC_CO2, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NUPDIS, UNIT_NON_DIM, DESC_NUPDIS, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_PUPDIS, UNIT_NON_DIM, DESC_PUPDIS, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NFIXCO, UNIT_NON_DIM, DESC_NFIXCO, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NFIXMX, UNIT_NON_DIM, DESC_NFIXMX, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TMEAN_ANN, UNIT_TEMP_DEG, DESC_TMEAN_ANN, Source_ParameterDB, DT_Raster1D);
    /// Soil parameters related raster data
    mdi.AddParameter(VAR_SOILLAYERS, UNIT_NON_DIM, DESC_SOILLAYERS, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_ZMX, UNIT_DEPTH_MM, DESC_SOL_ZMX, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_ALB, UNIT_NON_DIM, DESC_SOL_ALB, Source_ParameterDB, DT_Raster1D); /// soil surface
    mdi.AddParameter(VAR_SOILDEPTH, UNIT_DEPTH_MM, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILTHICK, UNIT_DEPTH_MM, DESC_SOILTHICK, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_AWC, UNIT_DEPTH_MM, DESC_SOL_AWC, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SUMAWC, UNIT_DEPTH_MM, DESC_SOL_SUMAWC, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_SUMSAT, UNIT_DEPTH_MM, DESC_SOL_SUMSAT, Source_ParameterDB, DT_Raster1D);

    /// Crop or land cover related parameters
    mdi.AddParameter(VAR_IDC, UNIT_NON_DIM, DESC_IDC, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_ALAIMIN, UNIT_AREA_RATIO, DESC_ALAIMIN, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BIO_E, UNIT_RAD_USE_EFFI, DESC_BIO_E, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BIOEHI, UNIT_RAD_USE_EFFI, DESC_BIOEHI, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BIOLEAF, UNIT_NON_DIM, DESC_BIOLEAF, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BLAI, UNIT_AREA_RATIO, DESC_BLAI, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BMX_TREES, UNIT_CONT_TONHA, DESC_BMX_TREES, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BN1, UNIT_NON_DIM, DESC_BN1, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BN2, UNIT_NON_DIM, DESC_BN2, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BN3, UNIT_NON_DIM, DESC_BN3, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BP1, UNIT_NON_DIM, DESC_BP1, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BP2, UNIT_NON_DIM, DESC_BP2, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BP3, UNIT_NON_DIM, DESC_BP3, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHTMX, UNIT_LEN_M, DESC_CHTMX, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CO2HI, UNIT_GAS_PPMV, DESC_CO2HI, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DLAI, UNIT_NON_DIM, DESC_DLAI, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_EXT_COEF, UNIT_NON_DIM, DESC_EXT_COEF, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_FRGRW1, UNIT_NON_DIM, DESC_FRGRW1, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_FRGRW2, UNIT_NON_DIM, DESC_FRGRW2, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_HVSTI, UNIT_CONT_RATIO, DESC_HVSTI, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_LAIMX1, UNIT_NON_DIM, DESC_LAIMX1, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_LAIMX2, UNIT_NON_DIM, DESC_LAIMX2, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MAT_YRS, UNIT_YEAR, DESC_MAT_YRS, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_T_BASE, UNIT_TEMP_DEG, DESC_T_BASE, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_T_OPT, UNIT_TEMP_DEG, DESC_T_OPT, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_WAVP, UNIT_RAD_USE_EFFI, DESC_WAVP, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_RSDIN, UNIT_CONT_KGHA, DESC_SOL_RSDIN, Source_ParameterDB, DT_Raster1D);
    /// Management related parameters, may saved as DT_Array2D with ICNUM as index. IF not provided, these should be initialized in intialOutputs()
    mdi.AddParameter(VAR_IGRO, UNIT_NON_DIM, DESC_IGRO, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_EPCO, UNIT_NON_DIM, DESC_EPCO, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_TREEYRS, UNIT_YEAR, DESC_TREEYRS, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_LAIINIT, UNIT_CONT_RATIO, DESC_LAIINIT, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BIOINIT, UNIT_CONT_KGHA, DESC_BIOINIT, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PHUPLT, UNIT_HEAT_UNIT, DESC_PHUPLT, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHT, UNIT_LEN_M, DESC_CHT, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DORMI, UNIT_NON_DIM, DESC_DORMI, Source_ParameterDB, DT_Raster1D);
    /// nutrient
    mdi.AddParameter(VAR_SOL_NO3, UNIT_CONT_KGHA, DESC_SOL_NO3, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SOLP, UNIT_CONT_KGHA, DESC_SOL_SOLP, Source_ParameterDB, DT_Raster2D);

    /// climate parameters INPUT
    mdi.AddInput(VAR_TMEAN, UNIT_TEMP_DEG, DESC_TMEAN, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMIN, UNIT_TEMP_DEG, DESC_TMIN, Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_SolarRadiation, UNIT_SR, DESC_SR, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DAYLEN, UNIT_HOUR, DESC_DAYLEN, Source_Module, DT_Raster1D); /// from PET modules
    /// water related INPUT
    mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_SW, UNIT_DEPTH_MM, DESC_SOL_SW, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PET, UNIT_DEPTH_MM, DESC_PET, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_VPD, UNIT_PRESSURE, DESC_VPD, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PPT, UNIT_DEPTH_MM, DESC_PPT, Source_Module, DT_Raster1D);   /// maximum plant et
    mdi.AddInput(VAR_SOET, UNIT_DEPTH_MM, DESC_SOET, Source_Module, DT_Raster1D); /// actual soil evaporation
    mdi.AddInput(VAR_SNAC, UNIT_DEPTH_MM, DESC_SNAC, Source_Module, DT_Raster1D);

    mdi.AddInput(VAR_SOL_COV, UNIT_CONT_KGHA, DESC_SOL_COV, Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_SOL_RSD, UNIT_CONT_KGHA, DESC_SOL_RSD, Source_Module_Optional, DT_Raster2D);

    /// plant related inputs
    mdi.AddInput(VAR_BIOTARG, UNIT_CONT_KGHA, DESC_BIOTARG, Source_Module_Optional, DT_Raster1D);

    /// residue cover should be output of plant growth module.
    mdi.AddOutput(VAR_SOL_COV, UNIT_CONT_KGHA, DESC_SOL_COV, DT_Raster1D);
    mdi.AddOutput(VAR_SOL_RSD, UNIT_CONT_KGHA, DESC_SOL_RSD, DT_Raster2D);
    // set the output variables
    mdi.AddOutput(VAR_IGRO, UNIT_NON_DIM, DESC_IGRO, DT_Raster1D);
    mdi.AddOutput(VAR_HVSTI_ADJ, UNIT_CONT_RATIO, DESC_HVSTI_ADJ, DT_Raster1D);
    //mdi.AddOutput(VAR_CHT, UNIT_LEN_M, DESC_CHT, DT_Raster1D);
    mdi.AddOutput(VAR_ALBDAY, UNIT_NON_DIM, DESC_ALBDAY, DT_Raster1D);
    mdi.AddOutput(VAR_DORMI, UNIT_NON_DIM, DESC_DORMI, DT_Raster1D);

    mdi.AddOutput(VAR_LAST_SOILRD, UNIT_DEPTH_MM, DESC_LAST_SOILRD, DT_Raster1D);
    mdi.AddOutput(VAR_LAIYRMAX, UNIT_AREA_RATIO, DESC_LAIYRMAX, DT_Raster1D);
    mdi.AddOutput(VAR_LAIDAY, UNIT_AREA_RATIO, DESC_LAIDAY, DT_Raster1D);
    mdi.AddOutput(VAR_ROOTDEPTH, UNIT_DEPTH_MM, DESC_ROOTDEPTH, DT_Raster1D);

    mdi.AddOutput(VAR_LAIMAXFR, UNIT_NON_DIM, DESC_LAIMAXFR, DT_Raster1D);
    mdi.AddOutput(VAR_OLAI, UNIT_AREA_RATIO, DESC_OLAI, DT_Raster1D);
    mdi.AddOutput(VAR_FR_PHU_ACC, UNIT_HEAT_UNIT, DESC_FR_PHU_ACC, DT_Raster1D);
    mdi.AddOutput(VAR_PLTET_TOT, UNIT_DEPTH_MM, DESC_PLTET_TOT, DT_Raster1D);
    mdi.AddOutput(VAR_PLTPET_TOT, UNIT_DEPTH_MM, DESC_PLTPET_TOT, DT_Raster1D);
    mdi.AddOutput(VAR_AET_PLT, UNIT_DEPTH_MM, DESC_AET_PLT, DT_Raster1D);
    mdi.AddOutput(VAR_PLANT_N, UNIT_CONT_KGHA, DESC_PLANT_N, DT_Raster1D);
    mdi.AddOutput(VAR_PLANT_P, UNIT_CONT_KGHA, DESC_PLANT_P, DT_Raster1D);
    mdi.AddOutput(VAR_FR_PLANT_N, UNIT_NON_DIM, DESC_FR_PLANT_N, DT_Raster1D);
    mdi.AddOutput(VAR_FR_PLANT_P, UNIT_NON_DIM, DESC_FR_PLANT_P, DT_Raster1D);
    mdi.AddOutput(VAR_FR_ROOT, UNIT_NON_DIM, DESC_FR_ROOT, DT_Raster1D);
    mdi.AddOutput(VAR_FR_STRSWTR, UNIT_NON_DIM, DESC_FR_STRSWTR, DT_Raster1D);
    mdi.AddOutput(VAR_BIOMASS, UNIT_CONT_KGHA, DESC_BIOMASS, DT_Raster1D);

    // write out the XML file.
    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
