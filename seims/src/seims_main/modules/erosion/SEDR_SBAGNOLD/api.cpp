#include "api.h"

#include "SEDR_SBAGNOLD.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SEDR_SBAGNOLD();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    string res;
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Liangjun Zhu; Wu Hui; Junzhi Liu");
    mdi.SetClass(MCLS_SED_ROUTING[0], MCLS_SED_ROUTING[1]);
    mdi.SetDescription(M_SEDR_SBAGNOLD[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("");
    mdi.SetID(M_SEDR_SBAGNOLD[0]);
    mdi.SetName(M_SEDR_SBAGNOLD[0]);
    mdi.SetVersion("2.0");
    mdi.SetWebsite(SEIMS_SITE);

    // Input parameters from database

    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_Single);
#ifdef STORM_MODE
    mdi.AddParameter(Tag_ChannelTimeStep,UNIT_SECOND,Tag_TimeStep[1],File_Input,DT_Single);
#else
    mdi.AddParameter(Tag_TimeStep[0], UNIT_SECOND, Tag_TimeStep[1], File_Input, DT_Single); // daily model
#endif /* STORM_MODE */
    mdi.AddParameter(VAR_VCD[0], UNIT_NON_DIM, VAR_VCD[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_P_RF[0], UNIT_NON_DIM, VAR_P_RF[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SPCON[0], UNIT_NON_DIM, VAR_SPCON[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SPEXP[0], UNIT_NON_DIM, VAR_SPEXP[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CHS0[0], UNIT_STRG_M3M, VAR_CHS0[1], Source_ParameterDB, DT_Single);

    /// load reach parameters
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Reach);
    /// load point source loading sediment from Scenario
    mdi.AddParameter(VAR_SCENARIO[0], UNIT_NON_DIM, VAR_SCENARIO[1], Source_ParameterDB, DT_Scenario);

    // Input from other modules

    mdi.AddInput(VAR_SED_TO_CH[0], UNIT_KG, VAR_SED_TO_CH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SAND_TO_CH[0], UNIT_KG, VAR_SAND_TO_CH[1], Source_Module_Optional, DT_Array1D);
    mdi.AddInput(VAR_SILT_TO_CH[0], UNIT_KG, VAR_SILT_TO_CH[1], Source_Module_Optional, DT_Array1D);
    mdi.AddInput(VAR_CLAY_TO_CH[0], UNIT_KG, VAR_CLAY_TO_CH[1], Source_Module_Optional, DT_Array1D);
    mdi.AddInput(VAR_SAG_TO_CH[0], UNIT_KG, VAR_SAG_TO_CH[1], Source_Module_Optional, DT_Array1D);
    mdi.AddInput(VAR_LAG_TO_CH[0], UNIT_KG, VAR_LAG_TO_CH[1], Source_Module_Optional, DT_Array1D);
    mdi.AddInput(VAR_GRAVEL_TO_CH[0], UNIT_KG, VAR_GRAVEL_TO_CH[1], Source_Module_Optional, DT_Array1D);

    mdi.AddInput(VAR_QRECH[0], UNIT_FLOW_CMS, VAR_QRECH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_CHST[0], UNIT_VOL_M3, VAR_CHST[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_RTE_WTROUT[0], UNIT_VOL_M3, VAR_RTE_WTROUT[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_CHBTMWIDTH[0], UNIT_LEN_M, VAR_CHBTMWIDTH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_CHWTRDEPTH[0], UNIT_LEN_M, VAR_CHWTRDEPTH[1], Source_Module, DT_Array1D);
    mdi.AddInput(VAR_CHWTRWIDTH[0], UNIT_LEN_M, VAR_CHWTRWIDTH[1], Source_Module, DT_Array1D);

    // Outputs

    mdi.AddInOutput(VAR_SED_RECH[0], UNIT_KG, VAR_SED_RECH[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_SED_RECHConc[0], UNIT_SEDCONC, VAR_SED_RECH[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_SAND_RECH[0], UNIT_KG, VAR_SAND_RECH[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_SILT_RECH[0], UNIT_KG, VAR_SILT_RECH[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CLAY_RECH[0], UNIT_KG, VAR_CLAY_RECH[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_SAG_RECH[0], UNIT_KG, VAR_SAG_RECH[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_LAG_RECH[0], UNIT_KG, VAR_LAG_RECH[1], DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_GRAVEL_RECH[0], UNIT_KG, VAR_GRAVEL_RECH[1], DT_Array1D, TF_SingleValue);

    mdi.AddOutput(VAR_RCH_BANKERO[0], UNIT_KG, VAR_RCH_BANKERO[1], DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEG[0], UNIT_KG, VAR_RCH_DEG[1], DT_Array1D);

    mdi.AddOutput(VAR_RCH_DEP[0], UNIT_KG, VAR_RCH_DEP[1], DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPNEW[0], UNIT_KG, VAR_RCH_DEPNEW[1], DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPSAND[0], UNIT_KG, VAR_RCH_DEPSAND[1], DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPSILT[0], UNIT_KG, VAR_RCH_DEPSILT[1], DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPCLAY[0], UNIT_KG, VAR_RCH_DEPCLAY[1], DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPSAG[0], UNIT_KG, VAR_RCH_DEPSAG[1], DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPLAG[0], UNIT_KG, VAR_RCH_DEPLAG[1], DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPGRAVEL[0], UNIT_KG, VAR_RCH_DEPGRAVEL[1], DT_Array1D);

    mdi.AddOutput(VAR_FLDPLN_DEP[0], UNIT_KG, VAR_FLDPLN_DEP[1], DT_Array1D);
    mdi.AddOutput(VAR_FLDPLN_DEPNEW[0], UNIT_KG, VAR_FLDPLN_DEPNEW[1], DT_Array1D);
    mdi.AddOutput(VAR_FLDPLN_DEPSILT[0], UNIT_KG, VAR_FLDPLN_DEPSILT[1], DT_Array1D);
    mdi.AddOutput(VAR_FLDPLN_DEPCLAY[0], UNIT_KG, VAR_FLDPLN_DEPCLAY[1], DT_Array1D);

    mdi.AddOutput(VAR_SEDSTO_CH[0], UNIT_KG, VAR_SEDSTO_CH[1], DT_Array1D);
    mdi.AddOutput(VAR_SANDSTO_CH[0], UNIT_KG, VAR_SANDSTO_CH[1], DT_Array1D);
    mdi.AddOutput(VAR_SILTSTO_CH[0], UNIT_KG, VAR_SILTSTO_CH[1], DT_Array1D);
    mdi.AddOutput(VAR_CLAYSTO_CH[0], UNIT_KG, VAR_CLAYSTO_CH[1], DT_Array1D);
    mdi.AddOutput(VAR_SAGSTO_CH[0], UNIT_KG, VAR_SAGSTO_CH[1], DT_Array1D);
    mdi.AddOutput(VAR_LAGSTO_CH[0], UNIT_KG, VAR_LAGSTO_CH[1], DT_Array1D);
    mdi.AddOutput(VAR_GRAVELSTO_CH[0], UNIT_KG, VAR_GRAVELSTO_CH[1], DT_Array1D);

    // set the dependencies
    mdi.AddDependency(MCLS_OL_EROSION[0], MCLS_OL_EROSION[1]);
    mdi.AddDependency(MCLS_CH_ROUTING[0], MCLS_CH_ROUTING[1]);

    res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
