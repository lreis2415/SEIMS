#include "api.h"

#include "PETHargreaves.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new PETHargreaves();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu, Liangjun Zhu");
    mdi.SetClass(MCLS_PET[0], MCLS_PET[1]);
    mdi.SetDescription(M_PET_H[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_PET_H[0]);
    mdi.SetName(M_PET_H[0]);
    mdi.SetVersion("2.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    mdi.SetAbstractionTypeToConceptual();

    // Parameters from Database (non-time series)
    mdi.AddParameter(VAR_K_PET[0], UNIT_NON_DIM, VAR_K_PET[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_PET_HCOEF[0], UNIT_NON_DIM, VAR_PET_HCOEF[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CELL_LAT[0], UNIT_LONLAT_DEG, VAR_CELL_LAT[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_PHUTOT[0], UNIT_HOUR, VAR_PHUTOT[1], Source_ParameterDB, DT_Raster1D);

    // Inputs from the output of other modules
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMAX[0], UNIT_TEMP_DEG, VAR_TMAX[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMIN[0], UNIT_TEMP_DEG, VAR_TMIN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(DataType_RelativeAirMoisture, UNIT_PERCENT, DESC_RM, Source_Module, DT_Raster1D);

    // Output variables
    mdi.AddOutput(VAR_DAYLEN[0], UNIT_HOUR, VAR_DAYLEN[1], DT_Raster1D);
    mdi.AddOutput(VAR_PHUBASE[0], UNIT_NON_DIM, VAR_PHUBASE[1], DT_Raster1D);
    mdi.AddOutput(VAR_PET[0], UNIT_WTRDLT_MMD, VAR_PET[1], DT_Raster1D);
    mdi.AddOutput(VAR_VPD[0], UNIT_PRESSURE, VAR_VPD[1], DT_Raster1D);

    // set the dependencies module classes
    mdi.AddDependency(MCLS_CLIMATE[0], MCLS_CLIMATE[1]);

    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
