#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "Interception.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new clsPI_STORM();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res;

    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Alex Storey, Junzhi Liu");
    mdi.SetClass(MCLS_INTERC, MCLSDESC_INTERC);
    mdi.SetDescription(MDESC_PI_STORM);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_PI_STORM);
    mdi.SetName(MID_PI_STORM);
    mdi.SetVersion("0.4");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    // set the input variables (time series)
    mdi.AddInput(VAR_PCP, UNIT_DEPTH_MM, DESC_PCP, Source_Module, DT_Raster1D);
    mdi.AddParameter(VAR_SLOPE, UNIT_PERCENT, DESC_SLOPE, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_HillSlopeTimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single);
    // set the parameters (non-time series)
    mdi.AddParameter(VAR_INTERC_MAX, UNIT_DEPTH_MM, DESC_INTERC_MAX, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_INTERC_MIN, UNIT_DEPTH_MM, DESC_INTERC_MIN, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PI_B, UNIT_NON_DIM, DESC_PI_B, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_INIT_IS, UNIT_NON_DIM, DESC_INIT_IS, Source_ParameterDB, DT_Single);

    // set the output variables
    mdi.AddOutput(VAR_INLO, UNIT_DEPTH_MM, DESC_INLO, DT_Raster1D);
    mdi.AddOutput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, DT_Raster1D);
    // set the dependencies
    mdi.AddDependency(MCLS_CLIMATE, MCLSDESC_CLIMATE);

    // write out the XML file.
    res = mdi.GetXMLDocument();

    //return res;

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
//mdi.AddInput("D_PET","mm", "Potential Evapotranspiration", "Module", DT_Raster1D);
//mdi.AddOutput("INET","mm", "Evaporation From Interception Storage", DT_Raster1D);
//mdi.AddParameter("K_pet","", "Correction Factor of PET", "ParameterDB_Interception", Single);