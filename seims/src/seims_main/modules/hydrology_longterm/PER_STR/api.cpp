#include "api.h"

#include "PER_STR.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new PER_STR();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu, Liangjun Zhu");
    mdi.SetClass(MCLS_PERCO[0], MCLS_PERCO[1]);
    mdi.SetDescription(M_PER_STR[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_PER_STR[0]);
    mdi.SetName(M_PER_STR[0]);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    mdi.AddParameter(Tag_TimeStep[0], UNIT_SECOND, UNIT_NON_DIM, File_Input, DT_Single);
    mdi.AddParameter(VAR_T_SOIL[0], UNIT_TEMP_DEG, VAR_T_SOIL[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_LEN_M, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_CONDUCT[0], UNIT_WTRDLT_MMH, VAR_CONDUCT[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_UL[0], UNIT_DEPTH_MM, VAR_SOL_UL[1], Source_ParameterDB, DT_Raster2D);   // m_sat
    mdi.AddParameter(VAR_SOL_AWC[0], UNIT_DEPTH_MM, VAR_SOL_AWC[1], Source_ParameterDB, DT_Raster2D); // m_fc
    mdi.AddInput(VAR_SOTE[0], UNIT_TEMP_DEG, VAR_SOTE[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INFIL[0], UNIT_DEPTH_MM, VAR_INFIL[1], Source_Module, DT_Raster1D);

    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_SW[0], UNIT_DEPTH_MM, VAR_SOL_SW[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_POT_VOL[0], UNIT_DEPTH_MM, VAR_POT_VOL[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_IMPOUND_TRIG[0], UNIT_NON_DIM, VAR_IMPOUND_TRIG[1], Source_Module_Optional, DT_Raster1D);
    // set the output variables
    mdi.AddOutput(VAR_PERCO[0], UNIT_DEPTH_MM, VAR_PERCO[1], DT_Raster2D);

    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
