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
    mdi.SetClass(MCLS_PG[0], MCLS_PG[1]);
    mdi.SetDescription(M_PG_ORYZA[1]);
    mdi.SetID(M_PG_ORYZA[0]);
    mdi.SetName(M_PG_ORYZA[0]);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    // climate, read from ITP module
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMAX[0], UNIT_TEMP_DEG, VAR_TMAX[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMIN[0], UNIT_TEMP_DEG, VAR_TMIN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_SolarRadiation, UNIT_SR, DESC_SR, Source_Module, DT_Raster1D);

    /// Single values,rice parameter
    mdi.AddParameter(VAR_CO2[0], UNIT_GAS_PPMV, VAR_CO2[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TBD[0], UNIT_TEMP_DEG, VAR_TBD[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TOD[0], UNIT_TEMP_DEG, VAR_TOD[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TMD[0], UNIT_TEMP_DEG, VAR_TMD[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DVRJ[0], UNIT_DVR, VAR_DVRJ[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DVRI[0], UNIT_DVR, VAR_DVRI[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DVRP[0], UNIT_DVR, VAR_DVRP[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DVRR[0], UNIT_DVR, VAR_DVRR[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MOPP[0], UNIT_HOUR, VAR_MOPP[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_PPSE[0], UNIT_PER_HOUR, VAR_PPSE[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SHCKD[0], UNIT_NON_DIM, VAR_SHCKD[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_KNF[0], UNIT_NON_DIM, VAR_KNF[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RGRLMX[0], UNIT_DVR, VAR_RGRLMX[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RGRLMN[0], UNIT_DVR, VAR_RGRLMN[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NH[0], UNIT_SOW_HILL, VAR_NH[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NPLH[0], UNIT_SOW_PLANT, VAR_NPLH[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NPLSB[0], UNIT_SOW_SEEDBED, VAR_NPLSB[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LAPE[0], UNIT_LAPE, VAR_LAPE[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ZRTTR[0], UNIT_LEN_M, VAR_ZRTTR[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TMPSB[0], UNIT_TEMP_DEG, VAR_TMPSB[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AFSH[0], UNIT_NON_DIM, VAR_AFSH[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BFSH[0], UNIT_NON_DIM, VAR_BFSH[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AFLV[0], UNIT_NON_DIM, VAR_AFLV[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BFLV[0], UNIT_NON_DIM, VAR_BFLV[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_AFSO[0], UNIT_NON_DIM, VAR_AFSO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BFSO[0], UNIT_NON_DIM, VAR_BFSO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ADRLV[0], UNIT_NON_DIM, VAR_ADRLV[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BDRLV[0], UNIT_NON_DIM, VAR_BDRLV[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TCLSTR[0], UNIT_PER_DAY, VAR_TCLSTR[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_Q10[0], UNIT_NON_DIM, VAR_Q10[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TREF[0], UNIT_TEMP_DEG, VAR_TREF[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MAINLV[0], UNIT_MAIN, VAR_MAINLV[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MAINST[0], UNIT_MAIN, VAR_MAINST[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MAINSO[0], UNIT_MAIN, VAR_MAINSO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_MAINRT[0], UNIT_MAIN, VAR_MAINRT[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CRGLV[0], UNIT_CRG, VAR_CRGLV[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CRGST[0], UNIT_CRG, VAR_CRGST[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CRGSTR[0], UNIT_CRG, VAR_CRGSTR[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CRGSO[0], UNIT_CRG, VAR_CRGSO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CRGRT[0], UNIT_CRG, VAR_CRGRT[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FSTR[0], UNIT_NON_DIM, VAR_FSTR[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LRSTR[0], UNIT_NON_DIM, VAR_LRSTR[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ASLA[0], UNIT_NON_DIM, VAR_ASLA[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BSLA[0], UNIT_NON_DIM, VAR_BSLA[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CSLA[0], UNIT_NON_DIM, VAR_CSLA[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DSLA[0], UNIT_NON_DIM, VAR_DSLA[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SLAMX[0], UNIT_SLA, VAR_SLAMX[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FCRT[0], UNIT_CRG, VAR_FCRT[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FCST[0], UNIT_CRG, VAR_FCST[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FCLV[0], UNIT_CRG, VAR_FCLV[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FCSTR[0], UNIT_CRG, VAR_FCSTR[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FCSO[0], UNIT_CRG, VAR_FCSO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_WGRMX[0], UNIT_GRAIN_WEIGHT, VAR_WGRMX[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_GZRT[0], UNIT_ROOT_RATIO, VAR_GZRT[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ZRTMCD[0], UNIT_LEN_M, VAR_ZRTMCD[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_FRPAR[0], UNIT_NON_DIM, VAR_FRPAR[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SPGF[0], UNIT_NON_DIM, VAR_SPGF[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NMAXL[0], UNIT_NON_DIM, VAR_NMAXL[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NMINL[0], UNIT_NON_DIM, VAR_NMINL[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RFNLV[0], UNIT_CRG, VAR_RFNLV[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RFNST[0], UNIT_CRG, VAR_RFNST[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RFNRT[0], UNIT_NON_DIM, VAR_RFNRT[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_TCNTRF[0], UNIT_DAY, VAR_TCNTRF[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_NMAXSO[0], UNIT_CRG, VAR_NMAXSO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ANMINSO[0], UNIT_NON_DIM, VAR_ANMINSO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_BNMINSO[0], UNIT_NON_DIM, VAR_BNMINSO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SHCKL[0], UNIT_NON_DIM, VAR_SHCKL[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SBDUR[0], UNIT_DAY, VAR_SBDUR[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LLLS[0], UNIT_PRESSURE, VAR_LLLS[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ULLS[0], UNIT_PRESSURE, VAR_ULLS[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LLLE[0], UNIT_PRESSURE, VAR_LLLE[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ULLE[0], UNIT_PRESSURE, VAR_ULLE[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LLDL[0], UNIT_PRESSURE, VAR_LLDL[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_ULDL[0], UNIT_PRESSURE, VAR_ULDL[1], Source_ParameterDB, DT_Single);

    /// Soil parameters related raster data
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_ZMX[0], UNIT_DEPTH_MM, VAR_SOL_ZMX[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_ALB[0], UNIT_NON_DIM, VAR_SOL_ALB[1], Source_ParameterDB, DT_Raster1D); /// soil surface
    mdi.AddParameter(VAR_SOILDEPTH[0], UNIT_DEPTH_MM, VAR_SOILDEPTH[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_AWC[0], UNIT_DEPTH_MM, VAR_SOL_AWC[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_UL[0], UNIT_DEPTH_MM, VAR_SOL_UL[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_WPMM[0], UNIT_DEPTH_MM, VAR_SOL_WPMM[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SUMAWC[0], UNIT_DEPTH_MM, VAR_SOL_SUMAWC[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_SUMSAT[0], UNIT_DEPTH_MM, VAR_SOL_SUMSAT[1], Source_ParameterDB, DT_Raster1D);

    /// water related INPUT
    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_SW[0], UNIT_DEPTH_MM, VAR_SOL_SW[1], Source_Module, DT_Raster1D);
    //mdi.AddInput(VAR_PET, UNIT_DEPTH_MM, VAR_PET[1], Source_Module, DT_Raster1D);
    //mdi.AddInput(VAR_VPD[0], UNIT_PRESSURE, VAR_VPD[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PPT[0], UNIT_DEPTH_MM, VAR_PPT[1], Source_Module, DT_Raster1D); /// maximum plant et
    //mdi.AddInput(VAR_SOET[0], UNIT_DEPTH_MM, VAR_SOET[1], Source_Module, DT_Raster1D);/// actual soil evaporation
    mdi.AddInput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOL_COV[0], UNIT_CONT_KGHA, VAR_SOL_COV[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_SOL_RSD[0], UNIT_CONT_KGHA, VAR_SOL_RSD[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_RSDIN[0], UNIT_CONT_KGHA, VAR_SOL_RSDIN[1], Source_ParameterDB, DT_Raster1D);

    // latitude related, compute sun radiation
    mdi.AddParameter(VAR_CELL_LAT[0], UNIT_LONLAT_DEG, VAR_CELL_LAT[1], Source_ParameterDB, DT_Raster1D);

    /// Management related parameters, may saved as DT_Array2D with ICNUM as index. IF not provided, these should be initialized in intialOutputs()
    //mdi.AddParameter(VAR_IGRO[0], UNIT_NON_DIM, VAR_IGRO[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_EPCO[0], UNIT_NON_DIM, VAR_EPCO[1], Source_ParameterDB, DT_Raster1D);

    /// nutrient
    mdi.AddParameter(VAR_SOL_NO3[0], UNIT_CONT_KGHA, VAR_SOL_NO3[1], Source_ParameterDB, DT_Raster2D);

    /// residue cover should be output of plant growth module.
    mdi.AddOutput(VAR_SOL_COV[0], UNIT_CONT_KGHA, VAR_SOL_COV[1], DT_Raster1D);
    mdi.AddOutput(VAR_SOL_RSD[0], UNIT_CONT_KGHA, VAR_SOL_RSD[1], DT_Raster2D);
    mdi.AddOutput(VAR_ALBDAY[0], UNIT_NON_DIM, VAR_ALBDAY[1], DT_Raster1D);
    mdi.AddOutput(VAR_AET_PLT[0], UNIT_DEPTH_MM, VAR_AET_PLT[1], DT_Raster1D);

    // rice
    mdi.AddParameter(VAR_CROPSTA[0], UNIT_NON_DIM, VAR_CROPSTA[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddInput(VAR_LAIDAY[0], UNIT_AREA_RATIO, VAR_LAIDAY[1], Source_Module, DT_Raster1D);


    // rice related, changed with days
    mdi.AddOutput(VAR_CROPSTA[0], UNIT_NON_DIM, VAR_CROPSTA[1], DT_Raster1D);
    mdi.AddOutput(VAR_TS[0], UNIT_PHENOLOGY, VAR_TS[1], DT_Raster1D);
    mdi.AddOutput(VAR_WLVG[0], UNIT_CONT_KGHA, VAR_WLVG[1], DT_Raster1D);
    mdi.AddOutput(VAR_WLVD[0], UNIT_CONT_KGHA, VAR_WLVD[1], DT_Raster1D);
    mdi.AddOutput(VAR_WSTS[0], UNIT_CONT_KGHA, VAR_WSTS[1], DT_Raster1D);
    mdi.AddOutput(VAR_WSTR[0], UNIT_CONT_KGHA, VAR_WSTR[1], DT_Raster1D);
    mdi.AddOutput(VAR_WSO[0], UNIT_CONT_KGHA, VAR_WSO[1], DT_Raster1D);
    mdi.AddOutput(VAR_WRT[0], UNIT_CONT_KGHA, VAR_WRT[1], DT_Raster1D);
    mdi.AddOutput(VAR_WRR[0], UNIT_CONT_KGHA, VAR_WRR[1], DT_Raster1D);
    mdi.AddOutput(VAR_NGR[0], UNIT_NUMBERHA, VAR_NGR[1], DT_Raster1D);
    mdi.AddOutput(VAR_NSP[0], UNIT_NUMBERHA, VAR_NSP[1], DT_Raster1D);
    mdi.AddOutput(VAR_TNASS[0], UNIT_CONT_KGHA, VAR_TNASS[1], DT_Raster1D);
    mdi.AddOutput(VAR_WST[0], UNIT_CONT_KGHA, VAR_WST[1], DT_Raster1D);
    mdi.AddOutput(VAR_WLV[0], UNIT_CONT_KGHA, VAR_WLV[1], DT_Raster1D);
    mdi.AddOutput(VAR_WAGT[0], UNIT_CONT_KGHA, VAR_WAGT[1], DT_Raster1D);
    mdi.AddOutput(VAR_ZRT[0], UNIT_LEN_M, VAR_ZRT[1], DT_Raster1D);
    mdi.AddOutput(VAR_DVS[0], UNIT_NON_DIM, VAR_DVS[1], DT_Raster1D);
    mdi.AddOutput(VAR_ANCRF[0], UNIT_CONT_KGHA, VAR_ANCRF[1], DT_Raster1D);

    // set the output variables
    //mdi.AddOutput(VAR_IGRO[0], UNIT_NON_DIM, VAR_IGRO[1], DT_Raster1D);
    mdi.AddOutput(VAR_LAST_SOILRD[0], UNIT_DEPTH_MM, VAR_LAST_SOILRD[1], DT_Raster1D);
    mdi.AddOutput(VAR_LAIDAY[0], UNIT_AREA_RATIO, VAR_LAIDAY[1], DT_Raster1D);
    mdi.AddOutput(VAR_PLANT_N[0], UNIT_CONT_KGHA, VAR_PLANT_N[1], DT_Raster1D);
    mdi.AddOutput(VAR_PLANT_P[0], UNIT_CONT_KGHA, VAR_PLANT_P[1], DT_Raster1D);
    mdi.AddOutput(VAR_FR_PLANT_N[0], UNIT_NON_DIM, VAR_FR_PLANT_N[1], DT_Raster1D);
    mdi.AddOutput(VAR_FR_PLANT_P[0], UNIT_NON_DIM, VAR_FR_PLANT_P[1], DT_Raster1D);
    mdi.AddOutput(VAR_FR_STRSWTR[0], UNIT_NON_DIM, VAR_FR_STRSWTR[1], DT_Raster1D);
    mdi.AddOutput(VAR_BIOMASS[0], UNIT_CONT_KGHA, VAR_BIOMASS[1], DT_Raster1D);
    mdi.AddOutput(VAR_FR_ROOT[0], UNIT_NON_DIM, VAR_FR_ROOT[1], DT_Raster1D);

    /// write out the XML file.
    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
