#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "SoilTemperatureFINPL.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new SoilTemperatureFINPL();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_SOLT, MCLSDESC_SOLT);
    mdi.SetDescription(MDESC_STP_FP);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_STP_FP);
    mdi.SetName(MID_STP_FP);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("STP_FP.html");

    /// from parameter database
    mdi.AddParameter(VAR_SOL_TA0, UNIT_NON_DIM, DESC_SOL_TA0, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TA1, UNIT_NON_DIM, DESC_SOL_TA1, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TA2, UNIT_NON_DIM, DESC_SOL_TA2, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TA3, UNIT_NON_DIM, DESC_SOL_TA3, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TB1, UNIT_NON_DIM, DESC_SOL_TB1, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TB2, UNIT_NON_DIM, DESC_SOL_TB2, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TD1, UNIT_NON_DIM, DESC_SOL_TD1, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_TD2, UNIT_NON_DIM, DESC_SOL_TD2, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_SOIL10, UNIT_NON_DIM, DESC_K_SOIL10, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SOIL_T10, UNIT_NON_DIM, DESC_SOIL_T10, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_LANDUSE, UNIT_NON_DIM, DESC_LANDUSE, Source_ParameterDB, DT_Raster1D);
	/// mean air temperature
    mdi.AddInput(DataType_MeanTemperature, UNIT_TEMP_DEG, DESC_TMEAN, Source_Module, DT_Raster1D);
	/// mean air temperature of the day(d-1)
    mdi.AddInput(VAR_TMEAN1, UNIT_TEMP_DEG, DESC_TMEAN1, Source_Module, DT_Raster1D);  
	/// mean air temperature of the day(d-2)
    mdi.AddInput(VAR_TMEAN2, UNIT_TEMP_DEG, DESC_TMEAN2, Source_Module, DT_Raster1D);
    /// output soil temperature
    mdi.AddOutput(VAR_SOTE, UNIT_TEMP_DEG, DESC_SOTE, DT_Raster1D);
    mdi.AddOutput(VAR_TMEAN1, UNIT_TEMP_DEG, DESC_TMEAN1, DT_Raster1D);
    mdi.AddOutput(VAR_TMEAN2, UNIT_TEMP_DEG, DESC_TMEAN2, DT_Raster1D);
    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}

