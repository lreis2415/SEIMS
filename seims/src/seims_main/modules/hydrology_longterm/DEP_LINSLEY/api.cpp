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
    mdi.SetClass(MCLS_DEP[0], MCLS_DEP[1]);
    mdi.SetDescription(M_DEP_LINSLEY[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("DEP_LINSLEY.chm");
    mdi.SetID(M_DEP_LINSLEY[0]);
    mdi.SetName(M_DEP_LINSLEY[0]);
    mdi.SetVersion("1.3");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(VAR_DEPREIN[0], UNIT_NON_DIM, VAR_DEPREIN[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DEPRESSION[0], UNIT_DEPTH_MM, VAR_DEPRESSION[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddInput(VAR_INET[0], UNIT_DEPTH_MM, VAR_INET[1], Source_Module, DT_Raster1D); //Evaporation from intercepted storage
    mdi.AddInput(VAR_PET[0], UNIT_DEPTH_MM, VAR_PET[1], Source_Module, DT_Raster1D);   //PET
    mdi.AddInput(VAR_EXCP[0], UNIT_DEPTH_MM, VAR_EXCP[1], Source_Module, DT_Raster1D); //Excess precipitation
    mdi.AddInput(VAR_IMPOUND_TRIG[0], UNIT_NON_DIM, VAR_IMPOUND_TRIG[1], Source_Module_Optional, DT_Raster1DInt);
    mdi.AddInput(VAR_POT_VOL[0], UNIT_DEPTH_MM, VAR_POT_VOL[1], Source_Module_Optional, DT_Raster1D);

    mdi.AddOutput(VAR_DPST[0], UNIT_DEPTH_MM, VAR_DPST[1], DT_Raster1D);
    mdi.AddOutput(VAR_DEET[0], UNIT_DEPTH_MM, VAR_DEET[1], DT_Raster1D);
    mdi.AddOutput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], DT_Raster1D);

    // set the dependencies
    mdi.AddDependency(MCLS_CLIMATE[0], MCLS_CLIMATE[1]);
    mdi.AddDependency(MCLS_INTERC[0], MCLS_INTERC[1]);
    mdi.AddDependency(MCLS_SUR_RUNOFF[0], MCLS_SUR_RUNOFF[1]);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
