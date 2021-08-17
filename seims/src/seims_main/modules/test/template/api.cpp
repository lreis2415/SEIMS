#include "api.h"

#include "template.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new ModuleTemplate();
}

/// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    mdi.SetAuthor("Liangjun Zhu");
    mdi.SetClass("TEST", "Functionality test of the module template!");
    mdi.SetDescription("Template of SEIMS module");
    mdi.SetID("ModuleTemplate");
    mdi.SetName("ModuleTemplate");
    mdi.SetVersion("1.0");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    /// Set parameters from database (Source_ParameterDB or Source_ParameterDB_Optional)

    /// Parameters with basic data types
    mdi.AddParameter("SingleValueParam", "UNIT", "DESC", Source_ParameterDB, DT_Single);
    mdi.AddParameter("OptioanlParam", "UNIT", "DESC", Source_ParameterDB_Optional, DT_Single);
    mdi.AddParameter("1DArrayParam", "UNIT", "DESC", Source_ParameterDB, DT_Array1D);
    mdi.AddParameter("1DRasterParam", "UNIT", "DESC", Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter("2DArrayParam", "UNIT", "DESC", Source_ParameterDB, DT_Array2D);
    mdi.AddParameter("2DRasterParam", "UNIT", "DESC", Source_ParameterDB, DT_Raster2D);

    /// Parameters with complex data types
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Reach);
    mdi.AddParameter(VAR_SUBBASIN_PARAM[0], UNIT_NON_DIM, VAR_SUBBASIN_PARAM[1], Source_ParameterDB, DT_Subbasin);
    mdi.AddParameter(VAR_SCENARIO[0], UNIT_NON_DIM, VAR_SCENARIO[1], Source_ParameterDB, DT_Scenario);

    /// Set inputs from other modules (Source_Module or Source_Module_Optional)
    mdi.AddInput("SingleInput", "UNIT", "DESC", Source_ParameterDB, DT_Single);
    mdi.AddInput("1DArrayInput", "UNIT", "DESC", Source_ParameterDB, DT_Array1D);
    mdi.AddInput("1DRasterInput", "UNIT", "DESC", Source_ParameterDB, DT_Raster1D);
    mdi.AddInput("2DArrayInput", "UNIT", "DESC", Source_ParameterDB, DT_Array2D);
    mdi.AddInput("2DRasterInput", "UNIT", "DESC", Source_ParameterDB, DT_Raster2D);

    /// Set output variables of the current module
    mdi.AddOutput("SingleOutput", "UNIT", "DESC", DT_Single);
    mdi.AddOutput("1DArrayOutput", "UNIT", "DESC", DT_Array1D);
    mdi.AddOutput("1DRasterOutput", "UNIT", "DESC", DT_Raster1D);
    mdi.AddOutput("2DArrayOutput", "UNIT", "DESC", DT_Array2D);
    mdi.AddOutput("2DRasterOutput", "UNIT", "DESC", DT_Raster2D);

    /// Set In/Output variables with transferred data type
    mdi.AddInOutput("1DArrayOutputSingleInOutput", "UNIT", "DESC", DT_Array1D, TF_SingleValue);
    mdi.AddInOutput("2DArrayOutput1DArrayInOutput", "UNIT", "DESC", DT_Array2D, TF_OneArray1D);

    /// Write out the XML file.
    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
