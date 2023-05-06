#include "api.h"

#include "GR4J.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new GR4J();
}

/// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    mdi.SetAuthor("Yujing Wang");
    mdi.SetClass(MCLS_CONCEPTUAL_MODEL[0], MCLS_CONCEPTUAL_MODEL[1]);
    mdi.SetDescription(CM_GR4J[1]);
    mdi.SetID(CM_GR4J[0]);
    mdi.SetName(CM_GR4J[0]);
    mdi.SetVersion("0.1");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    /// Set parameters from database (Source_ParameterDB or Source_ParameterDB_Optional)

    /// Parameters with basic data types
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter("GR4J_X2", UNIT_WTRDLT_MMD, "GR4J_X2", Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter("GR4J_X3", UNIT_DEPTH_MM, "GR4J_X3", Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter("GR4J_X4", UNIT_NON_DIM, "GR4J_X4", Source_ParameterDB, DT_Raster1D);

    /// Set inputs from other modules (Source_Module or Source_Module_Optional)
    mdi.AddInput(VAR_PCP[0], UNIT_DEPTH_MM, VAR_PCP[1], Source_Module, DT_Raster1D);  //from interception module
    mdi.AddInput(VAR_PET[0], UNIT_WTRDLT_MMD, VAR_PET[1], Source_Module, DT_Raster1D);  //from interception module

    /// Set output variables of the current module
    mdi.AddOutput(VAR_RTE_WTROUT[0], UNIT_DEPTH_MM, VAR_EXCP[1], DT_Array1D);

    /// Set In/Output variables with transferred data type

    /// Write out the XML file.
    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
