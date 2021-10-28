#include "api.h"

#include "SUR_MR.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SUR_MR();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu, Zhiqiang Yu, Liangjun Zhu");
    mdi.SetClass(MCLS_SUR_RUNOFF[0], MCLS_SUR_RUNOFF[1]);
    mdi.SetDescription(M_SUR_MR[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("");
    mdi.SetID(M_SUR_MR[0]);
    mdi.SetName(M_SUR_MR[0]);
    mdi.SetVersion("1.5");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_HillSlopeTimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(VAR_T_SOIL[0], UNIT_TEMP_DEG, VAR_T_SOIL[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_RUN[0], UNIT_NON_DIM, VAR_K_RUN[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_P_MAX[0], UNIT_DEPTH_MM, VAR_P_MAX[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_S_FROZEN[0], UNIT_WAT_RATIO, VAR_S_FROZEN[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_RUNOFF_CO[0], UNIT_NON_DIM, VAR_RUNOFF_CO[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MOIST_IN[0], UNIT_PERCENT, VAR_MOIST_IN[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOL_AWC[0], UNIT_DEPTH_MM, VAR_SOL_AWC[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_UL[0], UNIT_DEPTH_MM, VAR_SOL_UL[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_SUMSAT[0], UNIT_DEPTH_MM, VAR_SOL_SUMSAT[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddInput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DPST[0], UNIT_DEPTH_MM, VAR_DPST[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOTE[0], UNIT_TEMP_DEG, VAR_SOTE[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_IMPOUND_TRIG[0], UNIT_NON_DIM, VAR_IMPOUND_TRIG[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_POT_VOL[0], UNIT_DEPTH_MM, VAR_POT_VOL[1], Source_Module_Optional, DT_Raster1D);

    mdi.AddOutput(VAR_EXCP[0], UNIT_DEPTH_MM, VAR_EXCP[1], DT_Raster1D);
    mdi.AddOutput(VAR_INFIL[0], UNIT_DEPTH_MM, VAR_INFIL[1], DT_Raster1D);
    mdi.AddOutput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], DT_Raster2D);
    mdi.AddOutput(VAR_SOL_SW[0], UNIT_DEPTH_MM, VAR_SOL_SW[1], DT_Raster1D);

    // write out the XML file.
    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
