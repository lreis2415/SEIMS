#include "api.h"

#include "Interpolate.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new Interpolate();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    // set the information properties
    mdi.SetAuthor("Junzhi Liu, Liangjun Zhu");
    //mdi.SetClass(MCLS_CLIMATE, MCLSDESC_CLIMATE);
    mdi.SetClass(MCLS_CLIMATE[0], MCLS_CLIMATE[1]);
    mdi.SetDescription(M_ITP[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_ITP[0]);
    mdi.SetName(M_ITP[0]);
    mdi.SetVersion("1.0");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    //from parameter database, e.g., Weight_P, Weight_PET, Weight_T.
    mdi.AddParameter(Tag_Weight[0], UNIT_NON_DIM, Tag_Weight[1], Source_ParameterDB, DT_Array1D);
    // from config.fig, e.g. Interpolation_P_1
    mdi.AddParameter(Tag_VerticalInterpolation[0], UNIT_NON_DIM, Tag_VerticalInterpolation[1], File_Config, DT_Single);
    // these three parameters are just read when it will do vertical interpolation
    //  from spatial database
    mdi.AddParameter(VAR_DEM[0], UNIT_LEN_M, VAR_DEM[1], Source_ParameterDB_Optional, DT_Raster1D);
    /// from stations table
    mdi.AddParameter(Tag_StationElevation, UNIT_LEN_M, Tag_StationElevation, Source_HydroClimateDB_Optional, DT_Array1D);
    // TODO, Lapse_rate should be prepared by users according to the study area and imported into HydroClimate database.
    mdi.AddParameter(Tag_LapseRate, UNIT_LAP_RATE, Tag_LapseRate, Source_HydroClimateDB_Optional, DT_Array2D);

    // This is the climate data of all sites.
    // T means time series and it is same with first part of output id, e.g T_P. It may be P,PET,TMean, TMin or TMax data.
    mdi.AddInput(DataType_Prefix_TS, UNIT_NON_DIM, DESC_NONE, Source_Module, DT_Array1D);

    /// Must be "D". This is used to match with output id in file.out with data type.
    mdi.AddOutput(DataType_Prefix_DIS, UNIT_NON_DIM, DESC_NONE, DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
