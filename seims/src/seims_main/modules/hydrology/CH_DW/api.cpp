#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "DiffusiveWave.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"


//! Get instance of SimulationModule class
extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new DiffusiveWave();
}

extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_CH_ROUTING, MCLSDESC_CH_ROUTING);
    mdi.SetDescription(MDESC_CH_DW);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("CH_DW.chm");
    mdi.SetID(MID_CH_DW);
    mdi.SetName(MID_CH_DW);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_HillSlopeTimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single);
    mdi.AddParameter(Tag_CellSize, UNIT_NON_DIM, DESC_CellSize, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_CH_MANNING_FACTOR, UNIT_NON_DIM, DESC_CH_MANNING_FACTOR, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_DEM, UNIT_LEN_M, DESC_DEM, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SLOPE, UNIT_PERCENT, DESC_SLOPE, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_FLOWDIR, UNIT_NON_DIM, DESC_FLOWDIR, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHWIDTH, UNIT_LEN_M, DESC_CHWIDTH, Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(Tag_FLOWOUT_INDEX_D8, UNIT_NON_DIM, DESC_FLOWOUT_INDEX_D8, Source_ParameterDB, DT_Array1D);
    mdi.AddParameter(Tag_FLOWIN_INDEX_D8, UNIT_NON_DIM, DESC_FLOWIN_INDEX_D8, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_STREAM_LINK, UNIT_NON_DIM, DESC_STREAM_LINK, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_ReachParameter, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Array2D);

    // from other module
    mdi.AddInput(VAR_QOVERLAND, UNIT_FLOW_CMS, DESC_QOVERLAND, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QSOIL, UNIT_FLOW_CMS, DESC_QSOIL, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PCP, UNIT_DEPTH_MM, DESC_PCP, Source_Module, DT_Raster1D);

    // output
    mdi.AddOutput(VAR_QCH, UNIT_FLOW_CMS, DESC_QCH, DT_Array2D);
    mdi.AddOutput(VAR_QOUTLET, UNIT_FLOW_CMS, DESC_QOUTLET, DT_Single);
    mdi.AddOutput(VAR_QSUBBASIN, UNIT_FLOW_CMS, DESC_QSUBBASIN, DT_Array1D);
    mdi.AddOutput(VAR_HCH, UNIT_DEPTH_MM, DESC_HCH, DT_Array2D);

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
mdi.AddOutput("CHQCH", "m3/s", "Flux in the downslope boundary of cells", DT_Raster1D);*/
