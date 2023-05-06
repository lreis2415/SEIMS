#include "api.h"

#include "SUR_GR4J.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SUR_GR4J();
}

/// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    mdi.SetAuthor("Yujing Wang");
    mdi.SetClass(MCLS_SUR_RUNOFF[0], MCLS_SUR_RUNOFF[1]);
    mdi.SetDescription(M_SUR_GR4J[1]);
    mdi.SetID(M_SUR_GR4J[0]);
    mdi.SetName(M_SUR_GR4J[0]);
    mdi.SetVersion("0.1");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    /// Set parameters from database (Source_ParameterDB or Source_ParameterDB_Optional)

    /// Parameters with basic data types
    mdi.AddParameter("TopSoilThickness", UNIT_DEPTH_MM, "TopSoilThickness", Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter("TopSoilPorosity", UNIT_DEPTH_MM, "TopSoilPorosity", Source_ParameterDB, DT_Raster1D);

    /// Set inputs from other modules (Source_Module or Source_Module_Optional)
    mdi.AddInput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], Source_Module, DT_Raster1D);  //from interception module

    /// Set output variables of the current module
    mdi.AddOutput(VAR_EXCP[0], UNIT_DEPTH_MM, VAR_EXCP[1], DT_Raster1D);
    mdi.AddOutput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], DT_Raster1D);
    // mdi.AddOutput(VAR_INFIL[0], UNIT_DEPTH_MM, VAR_INFIL[1], DT_Raster1D);

    /// Set In/Output variables with transferred data type

    /// Write out the XML file.
    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
