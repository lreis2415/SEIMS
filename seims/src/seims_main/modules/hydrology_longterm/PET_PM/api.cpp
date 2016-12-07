#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "PETPenmanMonteith.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"
/** \defgroup PET_PM
 * \ingroup Hydrology_longterm
 * \brief Calculate potential evapotranspiration using Penman-Monteith method
 *
 */
//! Get instance of SimulationModule class
extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new PETPenmanMonteith();
}

/*!
 * \ingroup PET_H
 * \brief function to return the XML Metadata document string
 */
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_PET, MCLSDESC_PET);
    mdi.SetDescription(MDESC_PET_PM);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_PET_PM);
    mdi.SetName(MID_PET_PM);
    mdi.SetVersion("1.0");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("PET_PM.html");

    // set the parameters
    mdi.AddParameter(VAR_CO2, UNIT_GAS_PPMV, DESC_CO2, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T_SNOW, UNIT_DEPTH_MM, DESC_T_SNOW, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_PET, UNIT_NON_DIM, DESC_PET_K, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_DEM, UNIT_LEN_M, CONS_IN_ELEV, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CELL_LAT, UNIT_LONLAT_DEG, DESC_CELL_LAT, Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(VAR_GSI, UNIT_SPEED_MS, DESC_GSI, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_VPDFR, UNIT_PRESSURE, DESC_VPDFR, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_FRGMAX, UNIT_NON_DIM, DESC_FRGMAX, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PHUTOT, UNIT_HOUR, DESC_PHUTOT, Source_ParameterDB, DT_Raster1D);
    //Now, grow code is prepared as an input parameters, together with LAI_INTI, BIO_INIT, CURYR_INIT, etc.
    mdi.AddParameter(VAR_IGRO, UNIT_NON_DIM, DESC_IGRO, Source_ParameterDB, DT_Raster1D);

    // set the input from other modules
    mdi.AddInput(DataType_MeanTemperature, UNIT_TEMP_DEG, DESC_MAXTEMP, Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_MinimumTemperature, UNIT_TEMP_DEG, DESC_MINTEMP, Source_Module, DT_Array1D);
    mdi.AddInput(DataType_MaximumTemperature, UNIT_TEMP_DEG, DESC_MAXTEMP, Source_Module, DT_Array1D);
    mdi.AddInput(DataType_RelativeAirMoisture, UNIT_PERCENT, DESC_RM, Source_Module, DT_Array1D);
    mdi.AddInput(DataType_SolarRadiation, UNIT_SR, DESC_SR, Source_Module, DT_Array1D);
    mdi.AddInput(DataType_WindSpeed, UNIT_SPEED_MS, DESC_WS, Source_Module, DT_Array1D);

    /// these three parameters all from plant growth module, e.g., BIO_EPIC
    mdi.AddInput(VAR_CHT, UNIT_LEN_M, DESC_CHT, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_LAIDAY, UNIT_AREA_RATIO, DESC_LAIDAY, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_ALBDAY, UNIT_NON_DIM, DESC_ALBDAY, Source_Module, DT_Array1D);

    // set the output variables
    mdi.AddOutput(VAR_DAYLEN, UNIT_HOUR, DESC_DAYLEN, DT_Raster1D);
    mdi.AddOutput(VAR_PHUBASE, UNIT_NON_DIM, DESC_PHUBASE, DT_Raster1D);
    mdi.AddOutput(VAR_VPD, UNIT_PRESSURE, DESC_VPD, DT_Raster1D);
    mdi.AddOutput(VAR_PET, UNIT_WTRDLT_MMD, DESC_PET, DT_Raster1D);
    mdi.AddOutput(VAR_PPT, UNIT_WTRDLT_MMD, DESC_PPT, DT_Raster1D);
    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}