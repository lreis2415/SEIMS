#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "IKW_CH.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new ImplicitKinematicWave_CH();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_CH_ROUTING, MCLSDESC_CH_ROUTING);
    mdi.SetDescription(MDESC_IKW_CH);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("IKW_CH.chm");
    mdi.SetID(MID_IKW_CH);
    mdi.SetName(MID_IKW_CH);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_HillSlopeTimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_CH_MANNING_FACTOR, UNIT_NON_DIM, DESC_CH_MANNING_FACTOR, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_FLOWDIR, UNIT_NON_DIM, DESC_FLOWDIR, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHWIDTH, UNIT_LEN_M, DESC_CHWIDTH, Source_ParameterDB, DT_Raster1D);
    // reach information
    mdi.AddParameter(VAR_CH_MANNING_FACTOR, UNIT_NON_DIM, DESC_CH_MANNING_FACTOR, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_FLOWOUT_INDEX_D8, UNIT_NON_DIM, DESC_FLOWOUT_INDEX_D8, Source_ParameterDB, DT_Array1D);
    mdi.AddParameter(Tag_FLOWIN_INDEX_D8, UNIT_NON_DIM, DESC_FLOWIN_INDEX_D8, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_STREAM_LINK, UNIT_NON_DIM, DESC_STREAM_LINK, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_ReachParameter, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Array2D);
    // from other module
    mdi.AddInput(VAR_RadianSlope, UNIT_NON_DIM, DESC_RadianSlope, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QOVERLAND, UNIT_FLOW_CMS, DESC_QOVERLAND, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QSOIL, UNIT_FLOW_CMS, DESC_QSOIL, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PCP, UNIT_DEPTH_MM, DESC_PCP, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SBQG, UNIT_FLOW_CMS, DESC_SBQG, Source_Module, DT_Array1D);

    // output
    mdi.AddOutput(VAR_QRECH, UNIT_FLOW_CMS, DESC_QRECH, DT_Array1D);
    mdi.AddOutput(VAR_QOUTLET, UNIT_FLOW_CMS, DESC_QOUTLET, DT_Single);
    mdi.AddOutput(VAR_QTOTAL, UNIT_FLOW_CMS, DESC_QTOTAL, DT_Single);
    mdi.AddOutput(VAR_QSUBBASIN, UNIT_FLOW_CMS, DESC_QSUBBASIN, DT_Array1D);
    mdi.AddOutput(VAR_HCH, UNIT_DEPTH_MM, DESC_HCH, DT_Array2D);

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
//mdi.AddOutput("SBOF_IKW", "mm","Overland flow to streams for each subbasin", DT_Array1D);
//test output
//mdi.AddOutput("CHWATH", "mm", "Water depth in the downslope boundary of cells", DT_Raster1D);
//mdi.AddOutput("CHQCH", "m3/s", "Flux in the downslope boundary of cells", DT_Raster1D);
