#include "api.h"

#include "IO_test.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new IO_TEST();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    string res;

    mdi.SetAuthor("Liangjun Zhu");
    mdi.SetClass("TEST", "Base functionality test!");
    mdi.SetDescription("Module test.");
    mdi.SetID("IO_TEST");
    mdi.SetName("IO_TEST");
    mdi.SetVersion("1.0");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    mdi.AddParameter(VAR_CN2[0], UNIT_NON_DIM, VAR_CN2[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CONDUCT[0], UNIT_WTRDLT_MMH, VAR_CONDUCT[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster2D);
    ///test add scenario data
    mdi.AddParameter(VAR_SCENARIO[0], UNIT_NON_DIM, VAR_SCENARIO[1], Source_ParameterDB, DT_Scenario);
    ///test reaches information
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Reach);
    /// set the output variables
    mdi.AddOutput("CN2_M", UNIT_NON_DIM, VAR_CN2[1], DT_Raster1D);
    mdi.AddOutput("K_M", UNIT_WTRDLT_MMH, VAR_CONDUCT[1], DT_Raster2D);
    /// write out the XML file.
    res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
