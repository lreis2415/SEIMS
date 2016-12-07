#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "ImplicitKinematicWave.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new ImplicitKinematicWave_OL();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_OL_ROUTING, MCLSDESC_OL_ROUTING);
    mdi.SetDescription(MDESC_IKW_OL);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("IKW_OL.chm");
    mdi.SetID(MID_IKW_OL);
    mdi.SetName(MID_IKW_OL);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_HillSlopeTimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single);
    mdi.AddParameter(Tag_CellSize, UNIT_NON_DIM, DESC_CellSize, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SLOPE, UNIT_PERCENT, DESC_SLOPE, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHWIDTH, UNIT_LEN_M, DESC_CHWIDTH, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK, UNIT_NON_DIM, DESC_STREAM_LINK, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MANNING, UNIT_NON_DIM, DESC_MANNING, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_FLOWDIR, UNIT_NON_DIM, DESC_FLOWDIR, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_FLOWIN_INDEX_D8, UNIT_NON_DIM, DESC_FLOWIN_INDEX_D8, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(Tag_ROUTING_LAYERS, UNIT_NON_DIM, DESC_ROUTING_LAYERS, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(Tag_FLOWOUT_INDEX_D8, UNIT_NON_DIM, DESC_FLOWOUT_INDEX_D8, Source_ParameterDB, DT_Array1D);

    mdi.AddInput(VAR_SURU, UNIT_DEPTH_MM, DESC_SURU, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INFILCAPSURPLUS, UNIT_DEPTH_MM, DESC_INFILCAPSURPLUS, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INFIL, UNIT_DEPTH_MM, DESC_INFIL, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_ACC_INFIL, UNIT_DEPTH_MM, DESC_ACC_INFIL, Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_QOVERLAND, UNIT_FLOW_CMS, DESC_QOVERLAND, DT_Raster1D);
    mdi.AddOutput(VAR_Reinfiltration, UNIT_DEPTH_MM, DESC_Reinfiltration, DT_Raster1D);
    mdi.AddOutput(VAR_FLOWWIDTH, UNIT_LEN_M, DESC_FLOWWIDTH, DT_Raster1D);
    mdi.AddOutput("ChWidth", "m", "Flow length of overland plane",
                  DT_Raster1D);  //Flowlen add by Wu hui  /// TODO Figure out what's meaning? LJ
    mdi.AddOutput(VAR_RadianSlope, UNIT_NON_DIM, DESC_RadianSlope, DT_Raster1D);

    mdi.AddOutput(VAR_ID_OUTLET, UNIT_NON_DIM, DESC_ID_OUTLET, DT_Single);

    // set the dependencies
    mdi.AddDependency(MCLS_DEP, MCLSDESC_DEP);

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
//mdi.AddOutput("CELLH", "mm", "Water depth in the downslope boundary of cells", DT_Raster);