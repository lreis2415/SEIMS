#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"
#include "PETPriestleyTaylor.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new PETPriestleyTaylor();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu, LiangJun Zhu");
    mdi.SetClass(MCLS_PET, MCLSDESC_PET);
    mdi.SetDescription(MDESC_PET_PT);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_PET_PT);
    mdi.SetName(MID_PET_PT);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("PET_PT.html");

    //This temperature is used to determine the value of variable m_snow
    //if T_MEAN is larger than T_snow, then m_snow = 0;
    //else m_snow = 1.
    mdi.AddParameter(VAR_T_SNOW, UNIT_TEMP_DEG, DESC_T_SNOW, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_PET, UNIT_NON_DIM, DESC_PET_K, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_DEM, UNIT_LEN_M, CONS_IN_ELEV, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CELL_LAT, UNIT_LONLAT_DEG, DESC_CELL_LAT, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PHUTOT, UNIT_HOUR, DESC_PHUTOT, Source_ParameterDB, DT_Raster1D);
    //These five inputs are read from ITP module
    mdi.AddInput(DataType_MeanTemperature, UNIT_TEMP_DEG, DESC_MAXTEMP, Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_MinimumTemperature, UNIT_TEMP_DEG, DESC_MINTEMP, Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_MaximumTemperature, UNIT_TEMP_DEG, DESC_MAXTEMP, Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_RelativeAirMoisture, UNIT_PERCENT, DESC_RM, Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_SolarRadiation, UNIT_SR, DESC_SR, Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_DAYLEN, UNIT_HOUR, DESC_DAYLEN, DT_Raster1D);
    mdi.AddOutput(VAR_PHUBASE, UNIT_HEAT_UNIT, DESC_PHUBASE, DT_Raster1D);
    mdi.AddOutput(VAR_VPD, UNIT_PRESSURE, DESC_VPD, DT_Raster1D);
    mdi.AddOutput(VAR_PET, UNIT_WTRDLT_MMD, DESC_PET, DT_Raster1D);
    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
