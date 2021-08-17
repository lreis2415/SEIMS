#include "api.h"

#include "IKW_CH.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new ImplicitKinematicWave_CH();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    string res = "";
    MetadataInfo mdi;
    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_CH_ROUTING[0], MCLS_CH_ROUTING[1]);
    mdi.SetDescription(M_IKW_CH[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("");
    mdi.SetID(M_IKW_CH[0]);
    mdi.SetName(M_IKW_CH[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_LayeringMethod[0], UNIT_NON_DIM, Tag_LayeringMethod[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_TimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_FLOWDIR[0], UNIT_NON_DIM, VAR_FLOWDIR[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHWIDTH[0], UNIT_LEN_M, VAR_CHWIDTH[1], Source_ParameterDB, DT_Raster1D);
    // reach information
    //mdi.AddParameter(VAR_CH_MANNING_FACTOR[0], UNIT_NON_DIM, VAR_CH_MANNING_FACTOR[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_FLOWOUT_INDEX_D8[0], UNIT_NON_DIM, Tag_FLOWOUT_INDEX_D8[1], Source_ParameterDB, DT_Array1D);
    mdi.AddParameter(Tag_FLOWIN_INDEX_D8[0], UNIT_NON_DIM, Tag_FLOWIN_INDEX_D8[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_STREAM_LINK[0], UNIT_NON_DIM, VAR_STREAM_LINK[1], Source_ParameterDB, DT_Raster1D);
    // add reach information
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Reach);

    // from other module
    mdi.AddInput(VAR_RadianSlope[0], UNIT_NON_DIM, VAR_RadianSlope[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QOVERLAND[0], UNIT_FLOW_CMS, VAR_QOVERLAND[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QSOIL[0], UNIT_FLOW_CMS, VAR_QSOIL[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PCP[0], UNIT_DEPTH_MM, VAR_PCP[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SBQG[0], UNIT_FLOW_CMS, VAR_SBQG[1], Source_Module, DT_Array1D);

    // output
    mdi.AddOutput(VAR_QRECH[0], UNIT_FLOW_CMS, VAR_QRECH[1], DT_Array1D);
    //mdi.AddOutput(VAR_QOUTLET, UNIT_FLOW_CMS, DESC_QOUTLET, DT_Single); // Deprecated!
    mdi.AddOutput(VAR_QTOTAL[0], UNIT_FLOW_CMS, VAR_QTOTAL[1], DT_Single);
    mdi.AddOutput(VAR_QSUBBASIN[0], UNIT_FLOW_CMS, VAR_QSUBBASIN[1], DT_Array1D);
    mdi.AddOutput(VAR_HCH[0], UNIT_DEPTH_MM, VAR_HCH[1], DT_Array2D);

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
//mdi.AddOutput("SBOF_IKW", "mm","Overland flow to streams for each subbasin", DT_Array1D);
//test output
//mdi.AddOutput("CHWATH", "mm", "Water depth in the downslope boundary of cells", DT_Raster1D);
//mdi.AddOutput("CHQCH", "m3/s", "Flux in the downslope boundary of cells", DT_Raster1D);
