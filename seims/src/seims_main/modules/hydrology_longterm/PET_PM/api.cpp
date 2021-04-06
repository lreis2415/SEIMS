#include "api.h"

#include "PETPenmanMonteith.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new PETPenmanMonteith();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu, Liangjun Zhu");
    mdi.SetClass(MCLS_PET[0], MCLS_PET[1]);
    mdi.SetDescription(M_PET_PM[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_PET_PM[0]);
    mdi.SetName(M_PET_PM[0]);
    mdi.SetVersion("1.0");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    // set the parameters
    mdi.AddParameter(VAR_CO2[0], UNIT_GAS_PPMV, VAR_CO2[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T_SNOW[0], UNIT_DEPTH_MM, VAR_T_SNOW[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_PET[0], UNIT_NON_DIM, VAR_K_PET[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_DEM[0], UNIT_LEN_M, CONS_IN_ELEV, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CELL_LAT[0], UNIT_LONLAT_DEG, VAR_CELL_LAT[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(VAR_GSI[0], UNIT_SPEED_MS, VAR_GSI[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_VPDFR[0], UNIT_PRESSURE, VAR_VPDFR[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_FRGMAX[0], UNIT_NON_DIM, VAR_FRGMAX[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PHUTOT[0], UNIT_HOUR, VAR_PHUTOT[1], Source_ParameterDB, DT_Raster1D);
    //Now, grow code is prepared as an input parameters, together with LAI_INTI, BIO_INIT, CURYR_INIT, etc.
    mdi.AddParameter(VAR_IGRO[0], UNIT_NON_DIM, VAR_IGRO[1], Source_ParameterDB, DT_Raster1D);

    // set the input from other modules
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMAX[0], UNIT_TEMP_DEG, VAR_TMAX[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMIN[0], UNIT_TEMP_DEG, VAR_TMIN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_RelativeAirMoisture, UNIT_PERCENT, DESC_RM, Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_SolarRadiation, UNIT_SR, DESC_SR, Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_WindSpeed, UNIT_SPEED_MS, VAR_WS[1], Source_Module, DT_Raster1D);

    /// these three parameters all from plant growth module, e.g., PG_EPIC
    mdi.AddParameter(VAR_CHT[0], UNIT_LEN_M, VAR_CHT[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddInput(VAR_LAIDAY[0], UNIT_AREA_RATIO, VAR_LAIDAY[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_ALBDAY[0], UNIT_NON_DIM, VAR_ALBDAY[1], Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_DAYLEN[0], UNIT_HOUR, VAR_DAYLEN[1], DT_Raster1D);
    mdi.AddOutput(VAR_PHUBASE[0], UNIT_NON_DIM, VAR_PHUBASE[1], DT_Raster1D);
    mdi.AddOutput(VAR_VPD[0], UNIT_PRESSURE, VAR_VPD[1], DT_Raster1D);
    mdi.AddOutput(VAR_PET[0], UNIT_WTRDLT_MMD, VAR_PET[1], DT_Raster1D);
    mdi.AddOutput(VAR_PPT[0], UNIT_WTRDLT_MMD, VAR_PPT[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
