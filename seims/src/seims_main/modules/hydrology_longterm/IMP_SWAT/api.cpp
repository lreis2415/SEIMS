#include "api.h"

#include "pothole_SWAT.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new IMP_SWAT();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    string res;

    mdi.SetAuthor("Liang-Jun Zhu");
    mdi.SetClass(MCLS_PADDY[0], MCLS_PADDY[1]);
    mdi.SetDescription(M_IMP_SWAT[1]);
    mdi.SetID(M_IMP_SWAT[0]);
    mdi.SetName(M_IMP_SWAT[0]);
    mdi.SetVersion("1.2");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    /// set parameters from database
    mdi.AddParameter(Tag_ROUTING_LAYERS[0], UNIT_NON_DIM, Tag_ROUTING_LAYERS[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_TimeStep[0], UNIT_DAY, Tag_TimeStep[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_EVLAI[0], UNIT_AREA_RATIO, VAR_EVLAI[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_POT_TILEMM[0], UNIT_DEPTH_MM, VAR_POT_TILEMM[1], Source_ParameterDB_Optional, DT_Single);
    mdi.AddParameter(VAR_POT_NO3DECAY[0], UNIT_PER_DAY, VAR_POT_NO3DECAY[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_POT_SOLPDECAY[0], UNIT_PER_DAY, VAR_POT_SOLPDECAY[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SLOPE[0], UNIT_PERCENT, VAR_SLOPE[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CONDUCT[0], UNIT_WTRDLT_MMH, VAR_CONDUCT[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_SUMAWC[0], UNIT_DEPTH_MM, VAR_SOL_SUMAWC[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_UL[0], UNIT_DEPTH_MM, VAR_SOL_UL[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POROST[0], UNIT_VOL_FRA_M3M3, VAR_POROST[1], Source_ParameterDB, DT_Raster2D);

    mdi.AddParameter(VAR_KV_PADDY[0], UNIT_PER_DAY, VAR_KV_PADDY[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_KN_PADDY[0], UNIT_PER_DAY, VAR_KN_PADDY[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_POT_K[0], UNIT_WTRDLT_MMH, VAR_POT_K[1], Source_ParameterDB, DT_Single);

    /// set input from other modules
    mdi.AddInput(VAR_IMPOUND_TRIG[0], UNIT_NON_DIM, VAR_IMPOUND_TRIG[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_POT_VOLMAXMM[0], UNIT_DEPTH_MM, VAR_POT_VOLMAXMM[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_POT_VOLLOWMM[0], UNIT_DEPTH_MM, VAR_POT_VOLLOWMM[1], Source_Module, DT_Raster1D);
    //mdi.AddInput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], Source_Module, DT_Raster1D);// m_pNet
    mdi.AddInput(VAR_LAIDAY[0], UNIT_AREA_RATIO, VAR_LAIDAY[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PET[0], UNIT_DEPTH_MM, VAR_PET[1], Source_Module, DT_Raster1D); ///PET
    // mdi.AddInput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], Source_Module, DT_Raster1D); /// should be VAR_FLOW_OL
    mdi.AddInput(VAR_OLFLOW[0], UNIT_DEPTH_MM, VAR_OLFLOW[1], Source_Module, DT_Raster1D);      /// m_surfaceRunoff
    mdi.AddInput(VAR_DEET[0], UNIT_DEPTH_MM, VAR_DEET[1], Source_Module_Optional, DT_Raster1D); /// m_depEvapor
    mdi.AddInput(VAR_DPST[0], UNIT_DEPTH_MM, VAR_DPST[1], Source_Module_Optional, DT_Raster1D); /// m_depStorage
    mdi.AddInput(VAR_SEDYLD[0], UNIT_KG, VAR_SEDYLD[1], Source_Module, DT_Raster1D);            /// m_sedYield
    /// m_sedToCh, may be updated in this module
    mdi.AddInput(VAR_SED_TO_CH[0], UNIT_KG, VAR_SED_TO_CH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SANDYLD[0], UNIT_KG, VAR_SANDYLD[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SILTYLD[0], UNIT_KG, VAR_SILTYLD[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_CLAYYLD[0], UNIT_KG, VAR_CLAYYLD[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SAGYLD[0], UNIT_KG, VAR_SAGYLD[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_LAGYLD[0], UNIT_KG, VAR_LAGYLD[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_SW[0], UNIT_DEPTH_MM, VAR_SOL_SW[1], Source_Module, DT_Raster1D); /// sol_sw in SWAT
    mdi.AddInput(VAR_SUR_NO3[0], UNIT_CONT_KGHA, VAR_SUR_NO3[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SUR_NH4[0], UNIT_CONT_KGHA, VAR_SUR_NH4[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SUR_SOLP[0], UNIT_CONT_KGHA, VAR_SUR_SOLP[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SUR_COD[0], UNIT_CONT_KGHA, VAR_SUR_COD[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SEDORGN[0], UNIT_CONT_KGHA, VAR_SEDORGN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SEDORGP[0], UNIT_CONT_KGHA, VAR_SEDORGP[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SEDMINPA[0], UNIT_CONT_KGHA, VAR_SEDMINPA[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SEDMINPS[0], UNIT_CONT_KGHA, VAR_SEDMINPS[1], Source_Module, DT_Raster1D);

    mdi.AddInput(VAR_SBOF[0], UNIT_FLOW_CMS, VAR_SBOF[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SUR_NO3_TOCH[0], UNIT_KG, VAR_SUR_NO3_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SUR_NH4_TOCH[0], UNIT_KG, VAR_SUR_NH4_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SUR_SOLP_TOCH[0], UNIT_KG, VAR_SUR_SOLP_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SUR_COD_TOCH[0], UNIT_KG, VAR_SUR_COD_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SEDORGN_TOCH[0], UNIT_KG, VAR_SEDORGN_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SEDORGP_TOCH[0], UNIT_KG, VAR_SEDORGP_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SEDMINPA_TOCH[0], UNIT_KG, VAR_SEDMINPA_TOCH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SEDMINPS_TOCH[0], UNIT_KG, VAR_SEDMINPS_TOCH[1], Source_Module, DT_Array1D);

    /// set the output variables
    mdi.AddOutput(VAR_POT_NO3[0], UNIT_KG, VAR_POT_NO3[1], DT_Raster1D);
    mdi.AddOutput(VAR_POT_NH4[0], UNIT_KG, VAR_POT_NH4[1], DT_Raster1D);
    mdi.AddOutput(VAR_POT_ORGN[0], UNIT_KG, VAR_POT_ORGN[1], DT_Raster1D);
    mdi.AddOutput(VAR_POT_SOLP[0], UNIT_PER_DAY, VAR_POT_SOLP[1], DT_Raster1D);
    mdi.AddOutput(VAR_POT_ORGP[0], UNIT_KG, VAR_POT_ORGP[1], DT_Raster1D);
    mdi.AddOutput(VAR_POT_AMINP[0], UNIT_KG, VAR_POT_AMINP[1], DT_Raster1D);
    mdi.AddOutput(VAR_POT_SMINP[0], UNIT_KG, VAR_POT_SMINP[1], DT_Raster1D);
    mdi.AddOutput(VAR_POT_SED[0], UNIT_KG, VAR_POT_SED[1], DT_Raster1D);
    mdi.AddOutput(VAR_POT_VOL[0], UNIT_DEPTH_MM, VAR_POT_VOL[1], DT_Raster1D);
    //mdi.AddOutput(VAR_POT_FLOWIN[0], UNIT_VOL_M3, VAR_POT_FLOWIN[1], DT_Raster1D);
    //mdi.AddOutput(VAR_POT_FLOWOUT[0], UNIT_VOL_M3, VAR_POT_FLOWOUT[1], DT_Raster1D);
    //mdi.AddOutput(VAR_POT_SEDIN[0], UNIT_KG, VAR_POT_SEDIN[1], DT_Raster1D);
    //mdi.AddOutput(VAR_POT_SEDOUT[0], UNIT_KG, VAR_POT_SEDOUT[1], DT_Raster1D);
    mdi.AddOutput(VAR_POT_SA[0], UNIT_AREA_HA, VAR_POT_SA[1], DT_Raster1D);

    /// write out the XML file.
    res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
