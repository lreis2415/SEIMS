#include "api.h"

#include "PETHargreaves.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new PETHargreaves();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu, Liangjun Zhu");
    mdi.SetClass(MCLS_PET, MCLSDESC_PET);
    mdi.SetDescription(MDESC_PET_H);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_PET_H);
    mdi.SetName(MID_PET_H);
    mdi.SetVersion("2.0");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    // Parameters from Database (non-time series)
    mdi.AddParameter(VAR_K_PET, UNIT_NON_DIM, DESC_PET_K, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_PET_HCOEF, UNIT_NON_DIM, DESC_PET_HCOEF, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CELL_LAT, UNIT_LONLAT_DEG, DESC_CELL_LAT, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PHUTOT, UNIT_HOUR, DESC_PHUTOT, Source_ParameterDB, DT_Raster1D);

    // Inputs from the output of other modules
    mdi.AddInput(VAR_TMEAN, UNIT_TEMP_DEG, DESC_TMEAN, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMAX, UNIT_TEMP_DEG, DESC_TMAX, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMIN, UNIT_TEMP_DEG, DESC_TMIN, Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_RelativeAirMoisture, UNIT_PERCENT, DESC_RM, Source_Module, DT_Raster1D);

    // Output variables
    mdi.AddOutput(VAR_DAYLEN, UNIT_HOUR, DESC_DAYLEN, DT_Raster1D);
    mdi.AddOutput(VAR_PHUBASE, UNIT_NON_DIM, DESC_PHUBASE, DT_Raster1D);
    mdi.AddOutput(VAR_PET, UNIT_WTRDLT_MMD, DESC_PET, DT_Raster1D);
    mdi.AddOutput(VAR_VPD, UNIT_PRESSURE, DESC_VPD, DT_Raster1D);

    // set the dependencies module classes
    mdi.AddDependency(MCLS_CLIMATE, MCLSDESC_CLIMATE);

    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
