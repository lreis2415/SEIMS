#include "api.h"

#include "NutrientTransportSediment.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new NutrientTransportSediment();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    mdi.SetAuthor("Huiran Gao, Liangjun Zhu");
    mdi.SetClass(MCLS_NUTRCYC[0], MCLS_NUTRCYC[1]);
    mdi.SetDescription(M_NUTRSED[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_NUTRSED[0]);
    mdi.SetName(M_NUTRSED[0]);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    // set the parameters
    mdi.AddParameter(VAR_CSWAT[0], UNIT_NON_DIM, VAR_CSWAT[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SOL_BD[0], UNIT_DENSITY, VAR_SOL_BD[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_ROCK[0], UNIT_PERCENT, VAR_ROCK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_UL[0], UNIT_DEPTH_MM, VAR_SOL_UL[1], Source_ParameterDB, DT_Raster2D);
    // parameters for subbasin sum
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SUBBASIN_PARAM[0], UNIT_NON_DIM, VAR_SUBBASIN_PARAM[1], Source_ParameterDB, DT_Subbasin);

    // set input from other modules
    mdi.AddInput(VAR_OLFLOW[0], UNIT_DEPTH_MM, VAR_OLFLOW[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SEDYLD[0], UNIT_KG, VAR_SEDYLD[1], Source_Module, DT_Raster1D);

    mdi.AddInput(VAR_SOL_AORGN[0], UNIT_CONT_KGHA, VAR_SOL_AORGN[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_SORGN[0], UNIT_CONT_KGHA, VAR_SOL_SORGN[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_FORGN[0], UNIT_CONT_KGHA, VAR_SOL_FORGN[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_FORGP[0], UNIT_CONT_KGHA, VAR_SOL_FORGP[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_HORGP[0], UNIT_CONT_KGHA, VAR_SOL_HORGP[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_ACTP[0], UNIT_CONT_KGHA, VAR_SOL_ACTP[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_STAP[0], UNIT_CONT_KGHA, VAR_SOL_STAP[1], Source_Module, DT_Raster2D);
    /// for C-FARM one carbon model
    mdi.AddInput(VAR_SOL_MP[0], UNIT_CONT_KGHA, VAR_SOL_MP[1], Source_Module_Optional, DT_Raster2D);

    /// for CENTURY C/N cycling model, set as optional inputs
    mdi.AddInput(VAR_SOL_LSN[0], UNIT_CONT_KGHA, VAR_SOL_LSN[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LMN[0], UNIT_CONT_KGHA, VAR_SOL_LMN[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_HPN[0], UNIT_CONT_KGHA, VAR_SOL_HPN[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_HSN[0], UNIT_CONT_KGHA, VAR_SOL_HSN[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_HPC[0], UNIT_CONT_KGHA, VAR_SOL_HPC[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_HSC[0], UNIT_CONT_KGHA, VAR_SOL_HSC[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LM[0], UNIT_CONT_KGHA, VAR_SOL_LM[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LMC[0], UNIT_CONT_KGHA, VAR_SOL_LMC[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSC[0], UNIT_CONT_KGHA, VAR_SOL_LSC[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LS[0], UNIT_CONT_KGHA, VAR_SOL_LS[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSL[0], UNIT_CONT_KGHA, VAR_SOL_LSL[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSLC[0], UNIT_CONT_KGHA, VAR_SOL_LSLC[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_LSLNC[0], UNIT_CONT_KGHA, VAR_SOL_LSLNC[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_BMC[0], UNIT_CONT_KGHA, VAR_SOL_BMC[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_WOC[0], UNIT_CONT_KGHA, VAR_SOL_WOC[1], Source_Module_Optional, DT_Raster2D);

    mdi.AddInput(VAR_SSRU[0], UNIT_DEPTH_MM, VAR_SSRU[1], Source_Module_Optional, DT_Raster2D);   //m_sol_laterq
    mdi.AddInput(VAR_PERCO[0], UNIT_DEPTH_MM, VAR_PERCO[1], Source_Module_Optional, DT_Raster2D); //m_sol_perco
    /// end CENTURY variables

    // set the output variables
    // output surface runoff related variables
    mdi.AddOutput(VAR_SEDORGN[0], UNIT_CONT_KGHA, VAR_SEDORGN[1], DT_Raster1D);
    mdi.AddOutput(VAR_SEDORGP[0], UNIT_CONT_KGHA, VAR_SEDORGP[1], DT_Raster1D);
    mdi.AddOutput(VAR_SEDMINPA[0], UNIT_CONT_KGHA, VAR_SEDMINPA[1], DT_Raster1D);
    mdi.AddOutput(VAR_SEDMINPS[0], UNIT_CONT_KGHA, VAR_SEDMINPS[1], DT_Raster1D);

    // to channel
    mdi.AddOutput(VAR_SEDORGN_TOCH[0], UNIT_KG, VAR_SEDORGN_TOCH[1], DT_Array1D);
    mdi.AddOutput(VAR_SEDORGP_TOCH[0], UNIT_KG, VAR_SEDORGP_TOCH[1], DT_Array1D);
    mdi.AddOutput(VAR_SEDMINPA_TOCH[0], UNIT_KG, VAR_SEDMINPA_TOCH[1], DT_Array1D);
    mdi.AddOutput(VAR_SEDMINPS_TOCH[0], UNIT_KG, VAR_SEDMINPS_TOCH[1], DT_Array1D);

    /// outputs of CENTURY C/N cycling model
    mdi.AddOutput(VAR_SOL_LATERAL_C[0], UNIT_CONT_KGHA, VAR_SOL_LATERAL_C[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_PERCO_C[0], UNIT_CONT_KGHA, VAR_SOL_PERCO_C[1], DT_Raster2D);
    mdi.AddOutput(VAR_LATERAL_C[0], UNIT_CONT_KGHA, VAR_LATERAL_C[1], DT_Raster1D);
    mdi.AddOutput(VAR_PERCO_C[0], UNIT_CONT_KGHA, VAR_PERCO_C[1], DT_Raster1D);
    mdi.AddOutput(VAR_SEDLOSS_C[0], UNIT_CONT_KGHA, VAR_SEDLOSS_C[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
