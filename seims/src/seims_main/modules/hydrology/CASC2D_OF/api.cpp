#include "api.h"

#include "CASC2D_OF.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new CASC2D_OF();
}

/// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    mdi.SetAuthor("Liangjun Zhu");
    mdi.SetClass("TEST", "Functionality test of the module template!");
    mdi.SetDescription("Template of SEIMS module");
    mdi.SetID("CASC2D_OF");
    mdi.SetName("CASC2D_OF");
    mdi.SetVersion("1.0");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    /// Set parameters from database (Source_ParameterDB or Source_ParameterDB_Optional)

    /// Parameters with basic data types
    //mdi.AddParameter("SingleValueParam", "UNIT", "DESC", Source_ParameterDB, DT_Single);
    //mdi.AddParameter("OptioanlParam", "UNIT", "DESC", Source_ParameterDB_Optional, DT_Single);
    //mdi.AddParameter("1DArrayParam", "UNIT", "DESC", Source_ParameterDB, DT_Array1D);
    //mdi.AddParameter("1DRasterParam", "UNIT", "DESC", Source_ParameterDB, DT_Raster1D);
    //mdi.AddParameter("2DArrayParam", "UNIT", "DESC", Source_ParameterDB, DT_Array2D);
    //mdi.AddParameter("2DRasterParam", "UNIT", "DESC", Source_ParameterDB, DT_Raster2D);

    /// Parameters with complex data types
    mdi.AddParameter(VAR_DEM[0], UNIT_NON_DIM, VAR_DEM[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MANNING[0], UNIT_NON_DIM, VAR_MANNING[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_HillSlopeTimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_STREAM_LINK[0], UNIT_NON_DIM, VAR_STREAM_LINK[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_FLOWOUT_INDEX[0], UNIT_NON_DIM, Tag_FLOWOUT_INDEX[1], Source_ParameterDB, DT_Array1D);
    mdi.AddParameter(Tag_FLOWIN_INDEX[0], UNIT_NON_DIM, Tag_FLOWIN_INDEX[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_SLOPE[0], UNIT_PERCENT, VAR_SLOPE[1], Source_ParameterDB, DT_Raster1D);

    //mdi.AddParameter(VAR_SUR_SDEP, UNIT_DEPTH_MM, DESC_SUR_SDEP, Source_ParameterDB, DT_Array1D);
    mdi.AddParameter(REACH_DEPTH, UNIT_NON_DIM, "DESC_REACH_DEPTH_SPATIAL", Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHWIDTH[0], UNIT_LEN_M, VAR_CHWIDTH[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Type_RasterPositionData, UNIT_NON_DIM, "DESC_RasterPositionData", Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[0], Source_ParameterDB, DT_Reach);

    /// Set inputs from other modules (Source_Module or Source_Module_Optional)

    //mdi.AddInput(VAR_EXCP, UNIT_DEPTH_MM, DESC_EXCP, Source_Module, DT_Raster1D); //Excess precipitation
    //mdi.AddInput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, Source_Module, DT_Raster1D);
    //mdi.AddInput(VAR_CH_V, UNIT_SPEED_MS, DESC_CH_V, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], Source_Module, DT_Raster1D);

    /// Set output variables of the current module
    mdi.AddOutput(VAR_QOVERLAND[0], UNIT_FLOW_CMS, VAR_QOVERLAND[1], DT_Raster1D);
    mdi.AddOutput("OUTLET_Q", UNIT_FLOW_CMS, "outlet flow speed(spatially for output of casc2d module", DT_Single);
    mdi.AddOutput("OUTLET_V", UNIT_VOL_M3, "outlet flow volume(spatially for output of casc2d module)", DT_Single);
    mdi.AddOutput(VAR_SUR_WRT_DEPTH[0], UNIT_DEPTH_MM, VAR_SUR_WRT_DEPTH[1], DT_Raster1D);
    mdi.AddOutput(VAR_CH_WRT_DEPTH[0], UNIT_LEN_M, VAR_CH_WRT_DEPTH[1], DT_Raster1D);
    /// Set In/Output variables with transferred data type


    /// Write out the XML file.
    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
