#include "api.h"

#include "HS_WB.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new HS_WB();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_HS_HYDRO[0], MCLS_HS_HYDRO[1]);
    mdi.SetDescription(M_HS_WB[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_HS_WB[0]);
    mdi.SetName(M_HS_WB[0]);
    mdi.SetVersion("0.3");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("HS_WB.chm");

    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILDEPTH[0], UNIT_DEPTH_MM, VAR_SOILDEPTH[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK[0], UNIT_NON_DIM, VAR_STREAM_LINK[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(VAR_POROST[0], UNIT_VOL_FRA_M3M3, VAR_POROST[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_FIELDCAP[0], UNIT_VOL_FRA_M3M3, VAR_FIELDCAP[1], Source_ParameterDB, DT_Raster2D);
    //mdi.AddParameter(Tag_RchParam, UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Array2D);
    // add reach information
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Reach);

    mdi.AddInput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QOVERLAND[0], UNIT_FLOW_CMS, VAR_QOVERLAND[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QSOIL[0], UNIT_FLOW_CMS, VAR_QSOIL[1], Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_SOWB[0], UNIT_DEPTH_MM, VAR_SOWB[1], DT_Array2D);
    mdi.AddOutput(VAR_SBOF[0], UNIT_FLOW_CMS, VAR_SBOF[1], DT_Array1D);
    mdi.AddOutput(VAR_SBIF[0], UNIT_FLOW_CMS, VAR_SBIF[1], DT_Array1D);

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
