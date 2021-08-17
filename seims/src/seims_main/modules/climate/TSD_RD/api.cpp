#include "api.h"

#include "text.h"
#include "MetadataInfo.h"
#include "clsTSD_RD.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new clsTSD_RD();
}

extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;
    mdi.SetAuthor("Zhiqiang Yu");
    mdi.SetClass(MCLS_CLIMATE[0], MCLS_CLIMATE[1]);
    mdi.SetDescription(M_TSD_RD[1]);
    mdi.SetID(M_TSD_RD[0]);
    mdi.SetName(M_TSD_RD[0]);
    mdi.SetVersion("1.0");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    mdi.AddParameter(VAR_TSD_DT[0], UNIT_NON_DIM, VAR_TSD_DT[1], File_Config, DT_Single);

    /// set the input variables (time series), and T means time series. D means distribution.
    mdi.AddInput(DataType_Prefix_TS, UNIT_NON_DIM, DESC_TSD_CLIMATE, Source_HydroClimateDB, DT_Array1D);

    /// set the output variables
    mdi.AddOutput(DataType_Prefix_TS, UNIT_NON_DIM, DESC_TSD_CLIMATE, DT_Array1D);

    /// write out the XML file.
    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
