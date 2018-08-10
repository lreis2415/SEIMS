#include "api.h"

#include "RiceGrowth_ORYZA.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new ORYZA();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Fang Shen");
    mdi.SetClass(MCLS_PG, MCLSDESC_PG);
    mdi.SetDescription(MDESC_PG_ORYZA);
    mdi.SetID(MID_PG_ORYZA);
    mdi.SetName(MID_PG_ORYZA);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    // climate, read from ITP module
    mdi.AddInput(VAR_TMEAN, UNIT_TEMP_DEG, DESC_TMEAN, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMAX, UNIT_TEMP_DEG, DESC_TMAX, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMIN, UNIT_TEMP_DEG, DESC_TMIN, Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_SolarRadiation, UNIT_SR, DESC_SR, Source_Module, DT_Raster1D);

    /// Single values,rice parameter
    mdi.AddParameter(VAR_CO2, UNIT_GAS_PPMV, DESC_CO2, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TBD, UNIT_TEMP_DEG, DESC_TBD, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TOD, UNIT_TEMP_DEG, DESC_TOD, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TMD, UNIT_TEMP_DEG, DESC_TMD, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DVRJ, UNIT_DVR, DESC_DVRJ, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DVRI, UNIT_DVR, DESC_DVRI, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DVRP, UNIT_DVR, DESC_DVRP, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DVRR, UNIT_DVR, DESC_DVRR, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MOPP, UNIT_HOUR, DESC_MOPP, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_PPSE, UNIT_PER_HOUR, DESC_PPSE, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SHCKD, UNIT_NON_DIM, DESC_SHCKD, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_KNF, UNIT_NON_DIM, DESC_KNF, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RGRLMX, UNIT_DVR, DESC_RGRLMX, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RGRLMN, UNIT_DVR, DESC_RGRLMN, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NH, UNIT_SOW_HILL, DESC_NH, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NPLH, UNIT_SOW_PLANT, DESC_NPLH, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NPLSB, UNIT_SOW_SEEDBED, DESC_NPLSB, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LAPE, UNIT_LAPE, DESC_LAPE, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ZRTTR, UNIT_LEN_M, DESC_ZRTTR, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TMPSB, UNIT_TEMP_DEG, DESC_TMPSB, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AFSH, UNIT_NON_DIM, DESC_AFLV, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BFSH, UNIT_NON_DIM, DESC_BFLV, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AFLV, UNIT_NON_DIM, DESC_AFSH, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BFLV, UNIT_NON_DIM, DESC_BFSH, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AFSO, UNIT_NON_DIM, DESC_AFSO, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BFSO, UNIT_NON_DIM, DESC_BFSO, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ADRLV, UNIT_NON_DIM, DESC_ADRLV, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BDRLV, UNIT_NON_DIM, DESC_BDRLV, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TCLSTR, UNIT_PER_DAY, DESC_TCLSTR, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_Q10, UNIT_NON_DIM, DESC_Q10, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TREF, UNIT_TEMP_DEG, DESC_TREF, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MAINLV, UNIT_MAIN, DESC_MAINLV, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MAINST, UNIT_MAIN, DESC_MAINST, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MAINSO, UNIT_MAIN, DESC_MAINSO, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MAINRT, UNIT_MAIN, DESC_MAINRT, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CRGLV, UNIT_CRG, DESC_CRGLV, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CRGST, UNIT_CRG, DESC_CRGST, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CRGSTR, UNIT_CRG, DESC_CRGSTR, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CRGSO, UNIT_CRG, DESC_CRGSO, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CRGRT, UNIT_CRG, DESC_CRGRT, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FSTR, UNIT_NON_DIM, DESC_FSTR, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LRSTR, UNIT_NON_DIM, DESC_LRSTR, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ASLA, UNIT_NON_DIM, DESC_ASLA, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BSLA, UNIT_NON_DIM, DESC_BSLA, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CSLA, UNIT_NON_DIM, DESC_CSLA, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DSLA, UNIT_NON_DIM, DESC_DSLA, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SLAMX, UNIT_SLA, DESC_SLAMX, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FCRT, UNIT_CRG, DESC_FCRT, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FCST, UNIT_CRG, DESC_FCST, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FCLV, UNIT_CRG, DESC_FCLV, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FCSTR, UNIT_CRG, DESC_FCSTR, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FCSO, UNIT_CRG, DESC_FCSO, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_WGRMX, UNIT_GRAIN_WEIGHT, DESC_WGRMX, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_GZRT, UNIT_ROOT_RATIO, DESC_GZRT, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ZRTMCD, UNIT_LEN_M, DESC_ZRTMCD, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FRPAR, UNIT_NON_DIM, DESC_FRPAR, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SPGF, UNIT_NON_DIM, DESC_SPGF, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NMAXL, UNIT_NON_DIM, DESC_NMAXL, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NMINL, UNIT_NON_DIM, DESC_NMINL, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RFNLV, UNIT_CRG, DESC_RFNLV, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RFNST, UNIT_CRG, DESC_RFNST, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RFNRT, UNIT_NON_DIM, DESC_RFNRT, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TCNTRF, UNIT_DAY, DESC_TCNTRF, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NMAXSO, UNIT_CRG, DESC_NMAXSO, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ANMINSO, UNIT_NON_DIM, DESC_ANMINSO, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BNMINSO, UNIT_NON_DIM, DESC_BNMINSO, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SHCKL, UNIT_NON_DIM, DESC_SHCKL, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SBDUR, UNIT_DAY, DESC_SBDUR, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LLLS, UNIT_PRESSURE, DESC_LLLS, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ULLS, UNIT_PRESSURE, DESC_ULLS, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LLLE, UNIT_PRESSURE, DESC_LLLE, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ULLE, UNIT_PRESSURE, DESC_ULLE, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LLDL, UNIT_PRESSURE, DESC_LLDL, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ULDL, UNIT_PRESSURE, DESC_ULDL, Source_ParameterDB, DT_Single);

    /// Soil parameters related raster data
    mdi.AddParameter(VAR_SOILLAYERS, UNIT_NON_DIM, DESC_SOILLAYERS, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_ZMX, UNIT_DEPTH_MM, DESC_SOL_ZMX, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_ALB, UNIT_NON_DIM, DESC_SOL_ALB, Source_ParameterDB, DT_Raster1D); /// soil surface
    mdi.AddParameter(VAR_SOILDEPTH, UNIT_DEPTH_MM, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILTHICK, UNIT_DEPTH_MM, DESC_SOILTHICK, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_AWC, UNIT_DEPTH_MM, DESC_SOL_AWC, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_UL, UNIT_DEPTH_MM, DESC_SOL_UL, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_WPMM, UNIT_DEPTH_MM, DESC_SOL_WPMM, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SUMAWC, UNIT_DEPTH_MM, DESC_SOL_SUMAWC, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_SUMSAT, UNIT_DEPTH_MM, DESC_SOL_SUMSAT, Source_ParameterDB, DT_Raster1D);

    /// water related INPUT
    mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_SW, UNIT_DEPTH_MM, DESC_SOL_SW, Source_Module, DT_Raster1D);
    //mdi.AddInput(VAR_PET, UNIT_DEPTH_MM, DESC_PET, Source_Module, DT_Raster1D);
    //mdi.AddInput(VAR_VPD, UNIT_PRESSURE, DESC_VPD, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PPT, UNIT_DEPTH_MM, DESC_PPT, Source_Module, DT_Raster1D); /// maximum plant et
    //mdi.AddInput(VAR_SOET, UNIT_DEPTH_MM, DESC_SOET, Source_Module, DT_Raster1D);/// actual soil evaporation
    mdi.AddInput(VAR_SNAC, UNIT_DEPTH_MM, DESC_SNAC, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOL_COV, UNIT_CONT_KGHA, DESC_SOL_COV, Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_SOL_RSD, UNIT_CONT_KGHA, DESC_SOL_RSD, Source_Module_Optional, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_RSDIN, UNIT_CONT_KGHA, DESC_SOL_RSDIN, Source_ParameterDB, DT_Raster1D);

    // latitude related, compute sun radiation
    mdi.AddParameter(VAR_CELL_LAT, UNIT_LONLAT_DEG, DESC_CELL_LAT, Source_ParameterDB, DT_Raster1D);

    /// Management related parameters, may saved as DT_Array2D with ICNUM as index. IF not provided, these should be initialized in intialOutputs()
    //mdi.AddParameter(VAR_IGRO, UNIT_NON_DIM, DESC_IGRO, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_EPCO, UNIT_NON_DIM, DESC_EPCO, Source_ParameterDB, DT_Raster1D);

    /// nutrient
    mdi.AddParameter(VAR_SOL_NO3, UNIT_CONT_KGHA, DESC_SOL_NO3, Source_ParameterDB, DT_Raster2D);

    /// residue cover should be output of plant growth module.
    mdi.AddOutput(VAR_SOL_COV, UNIT_CONT_KGHA, DESC_SOL_COV, DT_Raster1D);
    mdi.AddOutput(VAR_SOL_RSD, UNIT_CONT_KGHA, DESC_SOL_RSD, DT_Raster2D);
    mdi.AddOutput(VAR_ALBDAY, UNIT_NON_DIM, DESC_ALBDAY, DT_Raster1D);
    mdi.AddOutput(VAR_AET_PLT, UNIT_DEPTH_MM, DESC_AET_PLT, DT_Raster1D);

    // rice
    mdi.AddParameter(VAR_CROPSTA, UNIT_NON_DIM, DESC_CROPSTA, Source_ParameterDB, DT_Raster1D);
    mdi.AddInput(VAR_LAIDAY, UNIT_AREA_RATIO, DESC_LAIDAY, Source_Module, DT_Raster1D);


    // rice related, changed with days
    mdi.AddOutput(VAR_CROPSTA, UNIT_NON_DIM, DESC_CROPSTA, DT_Raster1D);
    mdi.AddOutput(VAR_TS, UNIT_PHENOLOGY, DESC_TS, DT_Raster1D);
    mdi.AddOutput(VAR_WLVG, UNIT_CONT_KGHA, DESC_WLVG, DT_Raster1D);
    mdi.AddOutput(VAR_WLVD, UNIT_CONT_KGHA, DESC_WLVD, DT_Raster1D);
    mdi.AddOutput(VAR_WSTS, UNIT_CONT_KGHA, DESC_WSTS, DT_Raster1D);
    mdi.AddOutput(VAR_WSTR, UNIT_CONT_KGHA, DESC_WSTR, DT_Raster1D);
    mdi.AddOutput(VAR_WSO, UNIT_CONT_KGHA, DESC_WSO, DT_Raster1D);
    mdi.AddOutput(VAR_WRT, UNIT_CONT_KGHA, DESC_WRT, DT_Raster1D);
    mdi.AddOutput(VAR_WRR, UNIT_CONT_KGHA, DESC_WRR, DT_Raster1D);
    mdi.AddOutput(VAR_NGR, UNIT_NUMBERHA, DESC_NGR, DT_Raster1D);
    mdi.AddOutput(VAR_NSP, UNIT_NUMBERHA, DESC_NSP, DT_Raster1D);
    mdi.AddOutput(VAR_TNASS, UNIT_CONT_KGHA, DESC_TNASS, DT_Raster1D);
    mdi.AddOutput(VAR_WST, UNIT_CONT_KGHA, DESC_WST, DT_Raster1D);
    mdi.AddOutput(VAR_WLV, UNIT_CONT_KGHA, DESC_WLV, DT_Raster1D);
    mdi.AddOutput(VAR_WAGT, UNIT_CONT_KGHA, DESC_WAGT, DT_Raster1D);
    mdi.AddOutput(VAR_ZRT, UNIT_LEN_M, DESC_ZRT, DT_Raster1D);
    mdi.AddOutput(VAR_DVS, UNIT_NON_DIM, DESC_DVS, DT_Raster1D);
    mdi.AddOutput(VAR_ANCRF, UNIT_CONT_KGHA, DESC_ANCRF, DT_Raster1D);

    // set the output variables
    //mdi.AddOutput(VAR_IGRO, UNIT_NON_DIM, DESC_IGRO, DT_Raster1D);
    mdi.AddOutput(VAR_LAST_SOILRD, UNIT_DEPTH_MM, DESC_LAST_SOILRD, DT_Raster1D);
    mdi.AddOutput(VAR_LAIDAY, UNIT_AREA_RATIO, DESC_LAIDAY, DT_Raster1D);
    mdi.AddOutput(VAR_PLANT_N, UNIT_CONT_KGHA, DESC_PLANT_N, DT_Raster1D);
    mdi.AddOutput(VAR_PLANT_P, UNIT_CONT_KGHA, DESC_PLANT_P, DT_Raster1D);
    mdi.AddOutput(VAR_FR_PLANT_N, UNIT_NON_DIM, DESC_FR_PLANT_N, DT_Raster1D);
    mdi.AddOutput(VAR_FR_PLANT_P, UNIT_NON_DIM, DESC_FR_PLANT_P, DT_Raster1D);
    mdi.AddOutput(VAR_FR_STRSWTR, UNIT_NON_DIM, DESC_FR_STRSWTR, DT_Raster1D);
    mdi.AddOutput(VAR_BIOMASS, UNIT_CONT_KGHA, DESC_BIOMASS, DT_Raster1D);
    mdi.AddOutput(VAR_FR_ROOT, UNIT_NON_DIM, DESC_FR_ROOT, DT_Raster1D);

    /// write out the XML file.
    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
