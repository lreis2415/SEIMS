#include "api.h"

#include "PETPriestleyTaylor.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new PETPriestleyTaylor();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu, Liangjun Zhu");
    mdi.SetClass(MCLS_PET[0], MCLS_PET[1]);
    mdi.SetDescription(M_PET_PT[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_PET_PT[0]);
    mdi.SetName(M_PET_PT[0]);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddParameter(VAR_T_SNOW[0], UNIT_TEMP_DEG, VAR_T_SNOW[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_PET[0], UNIT_NON_DIM, VAR_K_PET[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_DEM[0], UNIT_LEN_M, VAR_DEM[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CELL_LAT[0], UNIT_LONLAT_DEG, VAR_CELL_LAT[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PHUTOT[0], UNIT_HOUR, VAR_PHUTOT[1], Source_ParameterDB, DT_Raster1D);

    //These five inputs are read from ITP module
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMAX[0], UNIT_TEMP_DEG, VAR_TMAX[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMIN[0], UNIT_TEMP_DEG, VAR_TMIN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_RelativeAirMoisture, UNIT_PERCENT, DESC_RM, Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_SolarRadiation, UNIT_SR, DESC_SR, Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_DAYLEN[0], UNIT_HOUR, VAR_DAYLEN[1], DT_Raster1D);
    mdi.AddOutput(VAR_PHUBASE[0], UNIT_HEAT_UNIT, VAR_PHUBASE[1], DT_Raster1D);
    mdi.AddOutput(VAR_VPD[0], UNIT_PRESSURE, VAR_VPD[1], DT_Raster1D);
    mdi.AddOutput(VAR_PET[0], UNIT_WTRDLT_MMD, VAR_PET[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
