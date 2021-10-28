#include "api.h"

#include "ImplicitKinematicWave.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new ImplicitKinematicWave_OL();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_OL_ROUTING[0], MCLS_OL_ROUTING[1]);
    mdi.SetDescription(M_IKW_OL[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("IKW_OL.chm");
    mdi.SetID(M_IKW_OL[0]);
    mdi.SetName(M_IKW_OL[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_TimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_CellSize[0], UNIT_NON_DIM, Tag_CellSize[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SLOPE[0], UNIT_PERCENT, VAR_SLOPE[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHWIDTH[0], UNIT_LEN_M, VAR_CHWIDTH[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK[0], UNIT_NON_DIM, VAR_STREAM_LINK[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MANNING[0], UNIT_NON_DIM, VAR_MANNING[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_FLOWDIR[0], UNIT_NON_DIM, VAR_FLOWDIR[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_FLOWIN_INDEX_D8[0], UNIT_NON_DIM, Tag_FLOWIN_INDEX_D8[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(Tag_ROUTING_LAYERS[0], UNIT_NON_DIM, Tag_ROUTING_LAYERS[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(Tag_FLOWOUT_INDEX_D8[0], UNIT_NON_DIM, Tag_FLOWOUT_INDEX_D8[1], Source_ParameterDB, DT_Array1D);

    mdi.AddInput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INFILCAPSURPLUS[0], UNIT_DEPTH_MM, VAR_INFILCAPSURPLUS[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INFIL[0], UNIT_DEPTH_MM, VAR_INFIL[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_ACC_INFIL[0], UNIT_DEPTH_MM, VAR_ACC_INFIL[1], Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_QOVERLAND[0], UNIT_FLOW_CMS, VAR_QOVERLAND[1], DT_Raster1D);
    mdi.AddOutput(VAR_Reinfiltration[0], UNIT_DEPTH_MM, VAR_Reinfiltration[1], DT_Raster1D);
    mdi.AddOutput(VAR_FLOWWIDTH[0], UNIT_LEN_M, VAR_FLOWWIDTH[1], DT_Raster1D);
    mdi.AddOutput("ChWidth", "m", "Flow length of overland plane",
                  DT_Raster1D);  //Flowlen add by Wu hui  /// TODO Figure out what's meaning? LJ
    mdi.AddOutput(VAR_RadianSlope[0], UNIT_NON_DIM, VAR_RadianSlope[1], DT_Raster1D);

    mdi.AddOutput(VAR_ID_OUTLET[0], UNIT_NON_DIM, VAR_ID_OUTLET[1], DT_Single);

    // set the dependencies
    mdi.AddDependency(MCLS_DEP[0], MCLS_DEP[1]);

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
//mdi.AddOutput("CELLH", "mm", "Water depth in the downslope boundary of cells", DT_Raster);
