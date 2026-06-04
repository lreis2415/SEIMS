#include "api.h"

#include "InterFlow_IKW.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new InterFlow_IKW();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;
    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_INTERFLOW[0], MCLS_INTERFLOW[1]);
    mdi.SetDescription(M_IKW_IF[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("");
    mdi.SetID(M_IKW_IF[0]);
    mdi.SetName(M_IKW_IF[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    //mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_HillSlopeTimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_HillSlopeTimeStep[1], File_Input, DT_SingleInt);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SLOPE[0], UNIT_PERCENT, VAR_SLOPE[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHWIDTH[0], UNIT_LEN_M, VAR_CHWIDTH[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK[0], UNIT_NON_DIM, VAR_STREAM_LINK[1], Source_ParameterDB, DT_Raster1DInt);
    //mdi.AddParameter(Tag_FLOWIN_INDEX[0], UNIT_NON_DIM, Tag_FLOWIN_INDEX[1], Source_ParameterDB, DT_Array2D);
    //mdi.AddParameter(Tag_ROUTING_LAYERS[0], UNIT_NON_DIM, Tag_ROUTING_LAYERS[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(Tag_FLOWIN_INDEX[0], UNIT_NON_DIM, Tag_FLOWIN_INDEX[1], Source_ParameterDB, DT_Array2DInt);
    mdi.AddParameter(Tag_ROUTING_LAYERS[0], UNIT_NON_DIM, Tag_ROUTING_LAYERS[1], Source_ParameterDB, DT_Array2DInt);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1DInt);
    mdi.AddParameter(VAR_SOILDEPTH[0], UNIT_DEPTH_MM, VAR_SOILDEPTH[1], Source_ParameterDB, DT_Raster2D);

    mdi.AddParameter(VAR_CONDUCT[0], UNIT_WTRDLT_MMH, VAR_CONDUCT[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POROST[0], UNIT_STRG_M3M, VAR_POROST[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POREIDX[0], UNIT_NON_DIM, VAR_POREIDX[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_FIELDCAP[0], UNIT_STRG_M3M, VAR_FIELDCAP[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_MOIST_IN[0], UNIT_VOL_FRA_M3M3, VAR_MOIST_IN[1],
                     Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(VAR_KI[0], UNIT_NON_DIM, VAR_KI[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter("MOIST_IN_REF", UNIT_NON_DIM,
                     "Reference for MOIST_IN initialization: 0 field capacity, 1 porosity",
                     Source_ParameterDB_Optional, DT_Single);
    mdi.AddParameter(VAR_FAST_RATIO[0], UNIT_NON_DIM, VAR_FAST_RATIO[1],
                     Source_ParameterDB_Optional, DT_Single);
    mdi.AddParameter(VAR_ANISOTROPY[0], UNIT_NON_DIM, VAR_ANISOTROPY[1],
                     Source_ParameterDB_Optional, DT_Single);
    mdi.AddParameter(VAR_MACROPORE_FACTOR[0], UNIT_NON_DIM, VAR_MACROPORE_FACTOR[1],
                     Source_ParameterDB_Optional, DT_Single);
    mdi.AddParameter("IF_SUBSTEPS", UNIT_NON_DIM,
                     "Numerical substeps for lateral interflow routing within one hillslope timestep",
                     Source_ParameterDB_Optional, DT_Single);

    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_QSOIL[0], UNIT_FLOW_CMS, VAR_QSOIL[1], DT_Raster1D);

    mdi.AddOutput(VAR_RETURNFLOW[0], UNIT_DEPTH_MM, VAR_RETURNFLOW[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
