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
    mdi.SetClass(MCLS_PG[0], MCLS_PG[1]);
    mdi.SetDescription(M_PG_EPIC[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_PG_EPIC[0]);
    mdi.SetName(M_PG_EPIC[0]);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    /// climate parameters
    mdi.AddParameter(VAR_DAYLEN_MIN[0], UNIT_HOUR, VAR_DAYLEN_MIN[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DORMHR[0], UNIT_HOUR, VAR_DORMHR[1], Source_ParameterDB, DT_Raster1D);
    /// Single values
    /// in SWAT, CO2 is assigned by subbasin
    mdi.AddParameter(VAR_CO2[0], UNIT_GAS_PPMV, VAR_CO2[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NUPDIS[0], UNIT_NON_DIM, VAR_NUPDIS[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_PUPDIS[0], UNIT_NON_DIM, VAR_PUPDIS[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NFIXCO[0], UNIT_NON_DIM, VAR_NFIXCO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NFIXMX[0], UNIT_NON_DIM, VAR_NFIXMX[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TMEAN_ANN[0], UNIT_TEMP_DEG, VAR_TMEAN_ANN[1], Source_ParameterDB, DT_Raster1D);
    /// Soil parameters related raster data
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_ZMX[0], UNIT_DEPTH_MM, VAR_SOL_ZMX[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_ALB[0], UNIT_NON_DIM, VAR_SOL_ALB[1], Source_ParameterDB, DT_Raster1D); /// soil surface
    mdi.AddParameter(VAR_SOILDEPTH[0], UNIT_DEPTH_MM, VAR_SOILDEPTH[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_AWC[0], UNIT_DEPTH_MM, VAR_SOL_AWC[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SUMAWC[0], UNIT_DEPTH_MM, VAR_SOL_SUMAWC[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_SUMSAT[0], UNIT_DEPTH_MM, VAR_SOL_SUMSAT[1], Source_ParameterDB, DT_Raster1D);

    /// Crop or land cover related parameters
    mdi.AddParameter(VAR_IDC[0], UNIT_NON_DIM, VAR_IDC[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_ALAIMIN[0], UNIT_AREA_RATIO, VAR_ALAIMIN[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BIO_E[0], UNIT_RAD_USE_EFFI, VAR_BIO_E[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BIOEHI[0], UNIT_RAD_USE_EFFI, VAR_BIOEHI[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BIOLEAF[0], UNIT_NON_DIM, VAR_BIOLEAF[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BLAI[0], UNIT_AREA_RATIO, VAR_BLAI[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BMX_TREES[0], UNIT_CONT_TONHA, VAR_BMX_TREES[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BN1[0], UNIT_NON_DIM, VAR_BN1[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BN2[0], UNIT_NON_DIM, VAR_BN2[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BN3[0], UNIT_NON_DIM, VAR_BN3[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BP1[0], UNIT_NON_DIM, VAR_BP1[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BP2[0], UNIT_NON_DIM, VAR_BP2[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BP3[0], UNIT_NON_DIM, VAR_BP3[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHTMX[0], UNIT_LEN_M, VAR_CHTMX[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CO2HI[0], UNIT_GAS_PPMV, VAR_CO2HI[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DLAI[0], UNIT_NON_DIM, VAR_DLAI[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_EXT_COEF[0], UNIT_NON_DIM, VAR_EXT_COEF[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_FRGRW1[0], UNIT_NON_DIM, VAR_FRGRW1[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_FRGRW2[0], UNIT_NON_DIM, VAR_FRGRW2[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_HVSTI[0], UNIT_CONT_RATIO, VAR_HVSTI[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_LAIMX1[0], UNIT_NON_DIM, VAR_LAIMX1[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_LAIMX2[0], UNIT_NON_DIM, VAR_LAIMX2[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MAT_YRS[0], UNIT_YEAR, VAR_MAT_YRS[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_T_BASE[0], UNIT_TEMP_DEG, VAR_T_BASE[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_T_OPT[0], UNIT_TEMP_DEG, VAR_T_OPT[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_WAVP[0], UNIT_RAD_USE_EFFI, VAR_WAVP[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_RSDIN[0], UNIT_CONT_KGHA, VAR_SOL_RSDIN[1], Source_ParameterDB, DT_Raster1D);
    /// Management related parameters, may saved as DT_Array2D with ICNUM as index. IF not provided, these should be initialized in intialOutputs()
    mdi.AddParameter(VAR_IGRO[0], UNIT_NON_DIM, VAR_IGRO[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_EPCO[0], UNIT_NON_DIM, VAR_EPCO[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_TREEYRS[0], UNIT_YEAR, VAR_TREEYRS[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_LAIINIT[0], UNIT_CONT_RATIO, VAR_LAIINIT[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_BIOINIT[0], UNIT_CONT_KGHA, VAR_BIOINIT[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PHUPLT[0], UNIT_HEAT_UNIT, VAR_PHUPLT[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHT[0], UNIT_LEN_M, VAR_CHT[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_DORMI[0], UNIT_NON_DIM, VAR_DORMI[1], Source_ParameterDB, DT_Raster1D);
    /// nutrient
    mdi.AddParameter(VAR_SOL_NO3[0], UNIT_CONT_KGHA, VAR_SOL_NO3[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SOLP[0], UNIT_CONT_KGHA, VAR_SOL_SOLP[1], Source_ParameterDB, DT_Raster2D);

    /// climate parameters INPUT
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMIN[0], UNIT_TEMP_DEG, VAR_TMIN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_SolarRadiation, UNIT_SR, DESC_SR, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DAYLEN[0], UNIT_HOUR, VAR_DAYLEN[1], Source_Module, DT_Raster1D); /// from PET modules
    /// water related INPUT
    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_SW[0], UNIT_DEPTH_MM, VAR_SOL_SW[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PET[0], UNIT_DEPTH_MM, VAR_PET[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_VPD[0], UNIT_PRESSURE, VAR_VPD[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PPT[0], UNIT_DEPTH_MM, VAR_PPT[1], Source_Module, DT_Raster1D);   /// maximum plant et
    mdi.AddInput(VAR_SOET[0], UNIT_DEPTH_MM, VAR_SOET[1], Source_Module, DT_Raster1D); /// actual soil evaporation
    mdi.AddInput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], Source_Module, DT_Raster1D);

    mdi.AddInput(VAR_SOL_COV[0], UNIT_CONT_KGHA, VAR_SOL_COV[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_SOL_RSD[0], UNIT_CONT_KGHA, VAR_SOL_RSD[1], Source_Module_Optional, DT_Raster2D);

    /// plant related inputs
    mdi.AddInput(VAR_BIOTARG[0], UNIT_CONT_KGHA, VAR_BIOTARG[1], Source_Module_Optional, DT_Raster1D);

    /// residue cover should be output of plant growth module.
    mdi.AddOutput(VAR_SOL_COV[0], UNIT_CONT_KGHA, VAR_SOL_COV[1], DT_Raster1D);
    mdi.AddOutput(VAR_SOL_RSD[0], UNIT_CONT_KGHA, VAR_SOL_RSD[1], DT_Raster2D);
    // set the output variables
    mdi.AddOutput(VAR_IGRO[0], UNIT_NON_DIM, VAR_IGRO[1], DT_Raster1D);
    mdi.AddOutput(VAR_HVSTI_ADJ[0], UNIT_CONT_RATIO, VAR_HVSTI_ADJ[1], DT_Raster1D);
    //mdi.AddOutput(VAR_CHT[0], UNIT_LEN_M, VAR_CHT[0], DT_Raster1D);
    mdi.AddOutput(VAR_ALBDAY[0], UNIT_NON_DIM, VAR_ALBDAY[1], DT_Raster1D);
    mdi.AddOutput(VAR_DORMI[0], UNIT_NON_DIM, VAR_DORMI[1], DT_Raster1D);

    mdi.AddOutput(VAR_LAST_SOILRD[0], UNIT_DEPTH_MM, VAR_LAST_SOILRD[1], DT_Raster1D);
    mdi.AddOutput(VAR_LAIYRMAX[0], UNIT_AREA_RATIO, VAR_LAIYRMAX[1], DT_Raster1D);
    mdi.AddOutput(VAR_LAIDAY[0], UNIT_AREA_RATIO, VAR_LAIDAY[1], DT_Raster1D);
    mdi.AddOutput(VAR_ROOTDEPTH[0], UNIT_DEPTH_MM, VAR_ROOTDEPTH[1], DT_Raster1D);

    mdi.AddOutput(VAR_LAIMAXFR[0], UNIT_NON_DIM, VAR_LAIMAXFR[1], DT_Raster1D);
    mdi.AddOutput(VAR_OLAI[0], UNIT_AREA_RATIO, VAR_OLAI[1], DT_Raster1D);
    mdi.AddOutput(VAR_FR_PHU_ACC[0], UNIT_HEAT_UNIT, VAR_FR_PHU_ACC[1], DT_Raster1D);
    mdi.AddOutput(VAR_PLTET_TOT[0], UNIT_DEPTH_MM, VAR_PLTET_TOT[1], DT_Raster1D);
    mdi.AddOutput(VAR_PLTPET_TOT[0], UNIT_DEPTH_MM, VAR_PLTPET_TOT[1], DT_Raster1D);
    mdi.AddOutput(VAR_AET_PLT[0], UNIT_DEPTH_MM, VAR_AET_PLT[1], DT_Raster1D);
    mdi.AddOutput(VAR_PLANT_N[0], UNIT_CONT_KGHA, VAR_PLANT_N[1], DT_Raster1D);
    mdi.AddOutput(VAR_PLANT_P[0], UNIT_CONT_KGHA, VAR_PLANT_P[1], DT_Raster1D);
    mdi.AddOutput(VAR_FR_PLANT_N[0], UNIT_NON_DIM, VAR_FR_PLANT_N[1], DT_Raster1D);
    mdi.AddOutput(VAR_FR_PLANT_P[0], UNIT_NON_DIM, VAR_FR_PLANT_P[1], DT_Raster1D);
    mdi.AddOutput(VAR_FR_ROOT[0], UNIT_NON_DIM, VAR_FR_ROOT[1], DT_Raster1D);
    mdi.AddOutput(VAR_FR_STRSWTR[0], UNIT_NON_DIM, VAR_FR_STRSWTR[1], DT_Raster1D);
    mdi.AddOutput(VAR_BIOMASS[0], UNIT_CONT_KGHA, VAR_BIOMASS[1], DT_Raster1D);

    // write out the XML file.
    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
