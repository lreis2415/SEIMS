#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "clsPI_MSM.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new clsPI_MSM();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Alex Storey");
    mdi.SetClass(MCLS_INTERC, MCLSDESC_INTERC);
    mdi.SetDescription(MDESC_PI_MSM);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_PI_MSM);
    mdi.SetName(MID_PI_MSM);
    mdi.SetVersion("0.4");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("PI_MSM.chm");

    // set the input variables (time series)
    mdi.AddInput(VAR_PCP, UNIT_DEPTH_MM, DESC_PCP, Source_Module, DT_Raster1D);/// ITP_P
    mdi.AddInput(VAR_PET, UNIT_DEPTH_MM, DESC_PET, Source_Module, DT_Raster1D); ///PET

    // set the parameters (non-time series)
    mdi.AddParameter(VAR_INTERC_MAX, UNIT_DEPTH_MM, DESC_INTERC_MAX, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_INTERC_MIN, UNIT_DEPTH_MM, DESC_INTERC_MIN, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PI_B, UNIT_NON_DIM, DESC_PI_B, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_INIT_IS, UNIT_NON_DIM, DESC_INIT_IS, Source_ParameterDB, DT_Single);

    // set the output variables
    mdi.AddOutput(VAR_INLO, UNIT_DEPTH_MM, DESC_INLO, DT_Raster1D);
    mdi.AddOutput(VAR_INET, UNIT_DEPTH_MM, DESC_INET, DT_Raster1D);
	mdi.AddOutput(VAR_CANSTOR, UNIT_DEPTH_MM, DESC_CANSTOR, DT_Raster1D);
    mdi.AddOutput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, DT_Raster1D);

    // set the dependencies
    mdi.AddDependency(MCLS_CLIMATE, MCLSDESC_CLIMATE);

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}