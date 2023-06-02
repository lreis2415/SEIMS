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
    // TODO: check if commented parameters are needed
    mdi.AddParameter(Tag_TimeStep[0], UNIT_HOUR, Tag_TimeStep[1], File_Input, DT_SingleInt);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_SingleInt);
    // mdi.AddParameter(VAR_OUTLETID[0], UNIT_NON_DIM, VAR_OUTLETID[1], Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_SUBBASIN_PARAM[0], UNIT_NON_DIM, VAR_SUBBASIN_PARAM[1], Source_ParameterDB, DT_Subbasin);
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1DInt);

    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POROST[0], UNIT_NON_DIM, VAR_POROST[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter("GR4J_X2", UNIT_WTRDLT_MMD, "GR4J_X2", Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter("GR4J_X3", UNIT_DEPTH_MM, "GR4J_X3", Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter("GR4J_X4", UNIT_NON_DIM, "GR4J_X4", Source_ParameterDB, DT_Raster1D);

    /// Set inputs from other modules (Source_Module or Source_Module_Optional)
    mdi.AddInput(VAR_PCP[0], UNIT_DEPTH_MM, VAR_PCP[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PET[0], UNIT_WTRDLT_MMD, VAR_PET[1], Source_Module, DT_Raster1D);

    /// Set output variables of the current module
    mdi.AddOutput(VAR_SBOF[0], UNIT_FLOW_CMS, VAR_SBOF[1], DT_Array1D);
    mdi.AddOutput(VAR_SBIF[0], UNIT_FLOW_CMS, VAR_SBIF[1], DT_Array1D);
    mdi.AddOutput(VAR_SBQG[0], UNIT_FLOW_CMS, VAR_SBQG[1], DT_Array1D);

    mdi.AddOutput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], DT_Raster2D);
    //mdi.AddOutput(VAR_QRECH[0], UNIT_FLOW_CMS, VAR_QRECH[1], DT_Array1D);

    /// Set In/Output variables with transferred data type

    /// Write out the XML file.
    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
