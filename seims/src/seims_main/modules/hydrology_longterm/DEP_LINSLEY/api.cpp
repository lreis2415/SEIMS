#include "api.h"

#include "DepressionLinsley.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new DepressionFSDaily();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    // set the information properties
    mdi.SetAuthor("Junzhi Liu, Liangjun Zhu");
    mdi.SetClass(MCLS_DEP, MCLSDESC_DEP);
    mdi.SetDescription(MDESC_DEP_LINSLEY);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("DEP_LINSLEY.chm");
    mdi.SetID(MID_DEP_LINSLEY);
    mdi.SetName(MID_DEP_LINSLEY);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(VAR_DEPREIN, UNIT_NON_DIM, DESC_DEPREIN, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DEPRESSION, UNIT_DEPTH_MM, DESC_DEPRESSION, Source_ParameterDB, DT_Raster1D);

    mdi.AddInput(VAR_INET, UNIT_DEPTH_MM, DESC_INET, Source_Module, DT_Raster1D); //Evaporation from intercepted storage
    mdi.AddInput(VAR_PET, UNIT_DEPTH_MM, DESC_PET, Source_Module, DT_Raster1D);   //PET
    mdi.AddInput(VAR_EXCP, UNIT_DEPTH_MM, DESC_EXCP, Source_Module, DT_Raster1D); //Excess precipitation
    mdi.AddInput(VAR_IMPOUND_TRIG, UNIT_NON_DIM, DESC_IMPOUND_TRIG, Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_POT_VOL, UNIT_DEPTH_MM, DESC_POT_VOL, Source_Module_Optional, DT_Raster1D);

    mdi.AddOutput(VAR_DPST, UNIT_DEPTH_MM, DESC_DPST, DT_Raster1D);
    mdi.AddOutput(VAR_DEET, UNIT_DEPTH_MM, DESC_DEET, DT_Raster1D);
    mdi.AddOutput(VAR_SURU, UNIT_DEPTH_MM, DESC_SURU, DT_Raster1D);

    // set the dependencies
    mdi.AddDependency(MCLS_CLIMATE, MCLSDESC_CLIMATE);
    mdi.AddDependency(MCLS_INTERC, MCLSDESC_INTERC);
    mdi.AddDependency(MCLS_SUR_RUNOFF, MCLSDESC_SUR_RUNOFF);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
