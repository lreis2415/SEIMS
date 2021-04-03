#include "api.h"

#include "Interception_MCS.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new clsPI_MCS();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Liangjun Zhu");
    mdi.SetClass(MCLS_INTERC[0], MCLS_INTERC[1]);
    mdi.SetDescription(M_PI_MCS[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_PI_MCS[0]);
    mdi.SetName(M_PI_MCS[0]);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    // set the input variables (time series)
    mdi.AddInput(VAR_PCP[0], UNIT_DEPTH_MM, VAR_PCP[1], Source_Module, DT_Raster1D); /// ITP_P
#ifndef STORM_MODE
    mdi.AddInput(VAR_PET[0], UNIT_DEPTH_MM, VAR_PET[1], Source_Module, DT_Raster1D); /// PET
#else
    mdi.AddParameter(VAR_SLOPE[0], UNIT_PERCENT, VAR_SLOPE[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_TimeStep[1], File_Input, DT_Single);
#endif

    // set the parameters (non-time series)
    mdi.AddParameter(VAR_INTERC_MAX[0], UNIT_DEPTH_MM, VAR_INTERC_MAX[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_INTERC_MIN[0], UNIT_DEPTH_MM, VAR_INTERC_MIN[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PI_B[0], UNIT_NON_DIM, VAR_PI_B[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_INIT_IS[0], UNIT_NON_DIM, VAR_INIT_IS[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LANDUSE[0], UNIT_NON_DIM, VAR_LANDUSE[1], Source_ParameterDB, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_INLO[0], UNIT_DEPTH_MM, VAR_INLO[1], DT_Raster1D);
#ifndef STORM_MODE
    mdi.AddOutput(VAR_INET[0], UNIT_DEPTH_MM, VAR_INET[1], DT_Raster1D);
#endif
    mdi.AddOutput(VAR_CANSTOR[0], UNIT_DEPTH_MM, VAR_CANSTOR[1], DT_Raster1D);
    mdi.AddOutput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], DT_Raster1D);

    // set the dependencies
    mdi.AddDependency(MCLS_CLIMATE[0], MCLS_CLIMATE[1]);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
