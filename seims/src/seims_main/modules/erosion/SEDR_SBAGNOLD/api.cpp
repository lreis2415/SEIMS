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
    mdi.SetClass(MCLS_SED_ROUTING, MCLSDESC_SED_ROUTING);
    mdi.SetDescription(MDESC_SEDR_SBAGNOLD);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("");
    mdi.SetID(MID_SEDR_SBAGNOLD);
    mdi.SetName(MID_SEDR_SBAGNOLD);
    mdi.SetVersion("2.0");
    mdi.SetWebsite(SEIMS_SITE);

    // Input parameters from database

    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_Single);
#ifdef STORM_MODE
    mdi.AddParameter(Tag_ChannelTimeStep,UNIT_SECOND,DESC_TIMESTEP,File_Input,DT_Single);
#else
    mdi.AddParameter(Tag_TimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single); // daily model
#endif /* STORM_MODE */
    mdi.AddParameter(VAR_VCD, UNIT_NON_DIM, DESC_VCD, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_P_RF, UNIT_NON_DIM, DESC_P_RF, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SPCON, UNIT_NON_DIM, DESC_SPCON, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SPEXP, UNIT_NON_DIM, DESC_SPEXP, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CHS0, UNIT_STRG_M3M, DESC_CHS0, Source_ParameterDB, DT_Single);

    /// load reach parameters
    mdi.AddParameter(VAR_REACH_PARAM, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Reach);
    /// load point source loading sediment from Scenario
    mdi.AddParameter(VAR_SCENARIO, UNIT_NON_DIM, DESC_SCENARIO, Source_ParameterDB, DT_Scenario);

    // Input from other modules

    mdi.AddInput(VAR_SED_TO_CH, UNIT_KG, DESC_SED_TO_CH, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_SAND_TO_CH, UNIT_KG, DESC_SAND_TO_CH, Source_Module_Optional, DT_Array1D);
    mdi.AddInput(VAR_SILT_TO_CH, UNIT_KG, DESC_SILT_TO_CH, Source_Module_Optional, DT_Array1D);
    mdi.AddInput(VAR_CLAY_TO_CH, UNIT_KG, DESC_CLAY_TO_CH, Source_Module_Optional, DT_Array1D);
    mdi.AddInput(VAR_SAG_TO_CH, UNIT_KG, DESC_SAG_TO_CH, Source_Module_Optional, DT_Array1D);
    mdi.AddInput(VAR_LAG_TO_CH, UNIT_KG, DESC_LAG_TO_CH, Source_Module_Optional, DT_Array1D);
    mdi.AddInput(VAR_GRAVEL_TO_CH, UNIT_KG, DESC_GRAVEL_TO_CH, Source_Module_Optional, DT_Array1D);

    mdi.AddInput(VAR_QRECH, UNIT_FLOW_CMS, DESC_QRECH, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_CHST, UNIT_VOL_M3, DESC_CHST, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_RTE_WTROUT, UNIT_VOL_M3, DESC_RTE_WTROUT, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_CHBTMWIDTH, UNIT_LEN_M, DESC_CHBTMWIDTH, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_CHWTRDEPTH, UNIT_LEN_M, DESC_CHWTDEPTH, Source_Module, DT_Array1D);
    mdi.AddInput(VAR_CHWTRWIDTH, UNIT_LEN_M, DESC_CHWTWIDTH, Source_Module, DT_Array1D);

    // Outputs

    mdi.AddInOutput(VAR_SED_RECH, UNIT_KG, DESC_SED_RECH, DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_SED_RECHConc, UNIT_SEDCONC, DESC_SED_RECH, DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_SAND_RECH, UNIT_KG, DESC_SAND_RECH, DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_SILT_RECH, UNIT_KG, DESC_SILT_RECH, DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_CLAY_RECH, UNIT_KG, DESC_CLAY_RECH, DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_SAG_RECH, UNIT_KG, DESC_SAG_RECH, DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_LAG_RECH, UNIT_KG, DESC_LAG_RECH, DT_Array1D, TF_SingleValue);
    mdi.AddInOutput(VAR_GRAVEL_RECH, UNIT_KG, DESC_GRAVEL_RECH, DT_Array1D, TF_SingleValue);

    mdi.AddOutput(VAR_RCH_BANKERO, UNIT_KG, DESC_RCH_BANKERO, DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEG, UNIT_KG, DESC_RCH_DEG, DT_Array1D);

    mdi.AddOutput(VAR_RCH_DEP, UNIT_KG, DESC_RCH_DEP, DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPNEW, UNIT_KG, DESC_RCH_DEPNEW, DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPSAND, UNIT_KG, DESC_RCH_DEPSAND, DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPSILT, UNIT_KG, DESC_RCH_DEPSILT, DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPCLAY, UNIT_KG, DESC_RCH_DEPCLAY, DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPSAG, UNIT_KG, DESC_RCH_DEPSAG, DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPLAG, UNIT_KG, DESC_RCH_DEPLAG, DT_Array1D);
    mdi.AddOutput(VAR_RCH_DEPGRAVEL, UNIT_KG, DESC_RCH_DEPGRAVEL, DT_Array1D);

    mdi.AddOutput(VAR_FLDPLN_DEP, UNIT_KG, DESC_FLDPLN_DEP, DT_Array1D);
    mdi.AddOutput(VAR_FLDPLN_DEPNEW, UNIT_KG, DESC_FLDPLN_DEPNEW, DT_Array1D);
    mdi.AddOutput(VAR_FLDPLN_DEPSILT, UNIT_KG, DESC_FLDPLN_DEPSILT, DT_Array1D);
    mdi.AddOutput(VAR_FLDPLN_DEPCLAY, UNIT_KG, DESC_FLDPLN_DEPCLAY, DT_Array1D);

    mdi.AddOutput(VAR_SEDSTO_CH, UNIT_KG, DESC_SEDSTO_CH, DT_Array1D);
    mdi.AddOutput(VAR_SANDSTO_CH, UNIT_KG, DESC_SANDSTO_CH, DT_Array1D);
    mdi.AddOutput(VAR_SILTSTO_CH, UNIT_KG, DESC_SILTSTO_CH, DT_Array1D);
    mdi.AddOutput(VAR_CLAYSTO_CH, UNIT_KG, DESC_CLAYSTO_CH, DT_Array1D);
    mdi.AddOutput(VAR_SAGSTO_CH, UNIT_KG, DESC_SAGSTO_CH, DT_Array1D);
    mdi.AddOutput(VAR_LAGSTO_CH, UNIT_KG, DESC_LAGSTO_CH, DT_Array1D);
    mdi.AddOutput(VAR_GRAVELSTO_CH, UNIT_KG, DESC_GRAVELSTO_CH, DT_Array1D);

    // set the dependencies
    mdi.AddDependency(MCLS_OL_EROSION, MCLSDESC_OL_EROSION);
    mdi.AddDependency(MCLS_CH_ROUTING, MCLSDESC_CH_ROUTING);

    res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
