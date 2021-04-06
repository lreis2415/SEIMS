#include "api.h"

#include "DiffusiveWave.h"
#include "text.h"
#include "MetadataInfo.h"

//! Get instance of SimulationModule class
extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new DiffusiveWave();
}

extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_CH_ROUTING[0], MCLS_CH_ROUTING[1]);
    mdi.SetDescription(M_CH_DW[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("CH_DW.chm");
    mdi.SetID(M_CH_DW[0]);
    mdi.SetName(M_CH_DW[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_LayeringMethod[0], UNIT_NON_DIM, Tag_LayeringMethod[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_TimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_CellSize[0], UNIT_NON_DIM, Tag_CellSize[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    //mdi.AddParameter(VAR_CH_MANNING_FACTOR[0], UNIT_NON_DIM, VAR_CH_MANNING_FACTOR[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_DEM[0], UNIT_LEN_M, VAR_DEM[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SLOPE[0], UNIT_PERCENT, VAR_SLOPE[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_FLOWDIR[0], UNIT_NON_DIM, VAR_FLOWDIR[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHWIDTH[0], UNIT_LEN_M, VAR_CHWIDTH[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(Tag_FLOWOUT_INDEX_D8[0], UNIT_NON_DIM, Tag_FLOWOUT_INDEX_D8[1], Source_ParameterDB, DT_Array1D);
    mdi.AddParameter(Tag_FLOWIN_INDEX_D8[0], UNIT_NON_DIM, Tag_FLOWIN_INDEX_D8[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_STREAM_LINK[0], UNIT_NON_DIM, VAR_STREAM_LINK[1], Source_ParameterDB, DT_Raster1D);
    // add reach information
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Reach);
    // from other module
    mdi.AddInput(VAR_QOVERLAND[0], UNIT_FLOW_CMS, VAR_QOVERLAND[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QSOIL[0], UNIT_FLOW_CMS, VAR_QSOIL[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PCP[0], UNIT_DEPTH_MM, VAR_PCP[1], Source_Module, DT_Raster1D);

    // output
    mdi.AddOutput(VAR_QCH[0], UNIT_FLOW_CMS, VAR_QCH[1], DT_Array2D);
    mdi.AddOutput(VAR_QSUBBASIN[0], UNIT_FLOW_CMS, VAR_QSUBBASIN[1], DT_Array1D);
    mdi.AddOutput(VAR_HCH[0], UNIT_DEPTH_MM, VAR_HCH[1], DT_Array2D);

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
//mdi.AddParameter("ID_UPREACH", "", "the cell adjacent to channel of the upstream subbasin", "main", DT_Single);
//mdi.AddParameter("Q_UPREACH", "m2/s", "channel flow in discharge from the upstream subbasin", "main", DT_Single);

// reach information
//mdi.AddParameter("Reach_Layers", "", "Routing layers according to the flow direction"
//	"There are not flow relationships within each layer, and the first element in each layer is the number of cells in the layer", "subbasins.shp", DT_Array2D);
//mdi.AddParameter("Reach_Links", "", "id of cells in reach links", "streamlink.txt", DT_Array2D);
//mdi.AddParameter("Reach_UpStream", "", "id of reach upstream", "subbasin.shp", DT_Array1D);
//mdi.AddParameter("Manning_nCh", "", "Manning's n of channel cells", "manning.txt", DT_Array1D);

//mdi.AddInput("D_ID_OUTLET", "", "ID of watershed outlet", "Module", DT_Single);
//test output
/*mdi.AddOutput("CHWATH", "mm", "Water depth in the downslope boundary of cells", DT_Raster1D);
mdi.AddOutput("CHQCH", "m3/s", "Flux in the downslope boundary of cells", DT_Raster1D);
mdi.AddOutput(VAR_QOUTLET, UNIT_FLOW_CMS, DESC_QOUTLET, DT_Single);
*/
