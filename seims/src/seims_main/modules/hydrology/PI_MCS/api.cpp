#include "Interception_MCS.h"

#include "api.h"

extern "C" SEIMS_MODULE_API SimulationModule *

GetInstance() {
    return new clsPI_MCS();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Liangjun Zhu");
    mdi.SetClass(MCLS_INTERC, MCLSDESC_INTERC);
    mdi.SetDescription(MDESC_PI_MCS);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_PI_MSC);
    mdi.SetName(MID_PI_MSC);
    mdi.SetVersion("1.0");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    // set the input variables (time series)
    mdi.AddInput(VAR_PCP, UNIT_DEPTH_MM, DESC_PCP, Source_Module, DT_Raster1D); /// ITP_P
#ifndef STORM_MODE
    mdi.AddInput(VAR_PET, UNIT_DEPTH_MM, DESC_PET, Source_Module, DT_Raster1D); /// PET
#else
    mdi.AddParameter(VAR_SLOPE, UNIT_PERCENT, DESC_SLOPE, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_HillSlopeTimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single);
#endif

    // set the parameters (non-time series)
    mdi.AddParameter(VAR_INTERC_MAX, UNIT_DEPTH_MM, DESC_INTERC_MAX, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_INTERC_MIN, UNIT_DEPTH_MM, DESC_INTERC_MIN, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PI_B, UNIT_NON_DIM, DESC_PI_B, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_INIT_IS, UNIT_NON_DIM, DESC_INIT_IS, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LANDUSE, UNIT_NON_DIM, DESC_LANDUSE, Source_ParameterDB, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_INLO, UNIT_DEPTH_MM, DESC_INLO, DT_Raster1D);
#ifndef STORM_MODE
    mdi.AddOutput(VAR_INET, UNIT_DEPTH_MM, DESC_INET, DT_Raster1D);
#endif
    mdi.AddOutput(VAR_CANSTOR, UNIT_DEPTH_MM, DESC_CANSTOR, DT_Raster1D);
    mdi.AddOutput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, DT_Raster1D);

    // set the dependencies
    mdi.AddDependency(MCLS_CLIMATE, MCLSDESC_CLIMATE);

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}