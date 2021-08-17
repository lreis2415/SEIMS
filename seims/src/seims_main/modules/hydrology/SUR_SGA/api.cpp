#include "api.h"

#include "StormGreenAmpt.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new StormGreenAmpt();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_SUR_RUNOFF[0], MCLS_SUR_RUNOFF[1]);
    mdi.SetDescription(M_SUR_SGA[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("");
    mdi.SetID(M_SUR_SGA[0]);
    mdi.SetName(M_SUR_SGA[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_TimeStep[1], File_Input, DT_Single);

    mdi.AddParameter(VAR_CONDUCT[0], UNIT_WTRDLT_MMH, VAR_CONDUCT[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_MOIST_IN[0], UNIT_VOL_FRA_M3M3, VAR_MOIST_IN[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CLAY[0], UNIT_PERCENT, VAR_CLAY[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SAND[0], UNIT_PERCENT, VAR_SAND[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILDEPTH[0], UNIT_LEN_M, VAR_SOILDEPTH[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_FIELDCAP[0], UNIT_VOL_FRA_M3M3, VAR_FIELDCAP[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POROST[0], UNIT_NON_DIM, VAR_POROST[1], Source_ParameterDB, DT_Raster2D);

    mdi.AddInput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DPST[0], UNIT_DEPTH_MM, VAR_DPST[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], DT_Raster2D);
    mdi.AddOutput(VAR_INFIL[0], UNIT_DEPTH_MM, VAR_INFIL[1], DT_Raster1D);
    mdi.AddOutput(VAR_INFILCAPSURPLUS[0], UNIT_DEPTH_MM, VAR_INFILCAPSURPLUS[1], DT_Raster1D);
    mdi.AddOutput(VAR_ACC_INFIL[0], UNIT_DEPTH_MM, VAR_ACC_INFIL[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
