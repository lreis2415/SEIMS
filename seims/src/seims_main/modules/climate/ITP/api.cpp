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
    mdi.SetClass(MCLS_CLIMATE, MCLSDESC_CLIMATE);
    mdi.SetDescription(MDESC_ITP);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_ITP);
    mdi.SetName(MID_ITP);
    mdi.SetVersion("1.0");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    //from parameter database, e.g., Weight_P, Weight_PET, Weight_T.
    mdi.AddParameter(Tag_Weight, UNIT_NON_DIM, DESC_WEIGHT_ITP, Source_ParameterDB, DT_Array1D);
    // from config.fig, e.g. Interpolation_P_1
    mdi.AddParameter(Tag_VerticalInterpolation, UNIT_NON_DIM, DESC_VER_ITP, File_Config, DT_Single);
    // these three parameters are just read when it will do vertical interpolation
    //  from spatial database
    mdi.AddParameter(VAR_DEM, UNIT_LEN_M, DESC_DEM, Source_ParameterDB_Optional, DT_Raster1D);
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
