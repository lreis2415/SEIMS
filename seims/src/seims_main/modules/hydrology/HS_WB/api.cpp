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
    mdi.SetClass(MCLS_HS_HYDRO, MCLSDESC_HS_HYDRO);
    mdi.SetDescription(MDESC_HS_WB);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_HS_WB);
    mdi.SetName(MID_HS_WB);
    mdi.SetVersion("0.3");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("HS_WB.chm");

    mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILDEPTH, UNIT_DEPTH_MM, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK, UNIT_NON_DIM, DESC_STREAM_LINK, Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(VAR_POROST, UNIT_VOL_FRA_M3M3, DESC_POROST, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_FIELDCAP, UNIT_VOL_FRA_M3M3, DESC_FIELDCAP, Source_ParameterDB, DT_Raster2D);
    //mdi.AddParameter(Tag_RchParam, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Array2D);
    // add reach information
    mdi.AddParameter(VAR_REACH_PARAM, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Reach);

    mdi.AddInput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QOVERLAND, UNIT_FLOW_CMS, DESC_QOVERLAND, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QSOIL, UNIT_FLOW_CMS, DESC_QSOIL, Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_SOWB, UNIT_DEPTH_MM, DESC_SOWB, DT_Array2D);
    mdi.AddOutput(VAR_SBOF, UNIT_FLOW_CMS, DESC_SBOF, DT_Array1D);
    mdi.AddOutput(VAR_SBIF, UNIT_FLOW_CMS, DESC_SBIF, DT_Array1D);

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
