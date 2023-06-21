#include "api.h"

#include "SurfaceRunoffDump.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SurfaceRunoffDump();
}

/// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    mdi.SetAuthor("Yujing Wang");
    mdi.SetClass(MCLS_SUR_RUNOFF[0], MCLS_SUR_RUNOFF[1]);
    mdi.SetDescription(M_SUR_DUMP[1]);
    mdi.SetID(M_SUR_DUMP[0]);
    mdi.SetName(M_SUR_DUMP[0]);
    mdi.SetVersion("0.1");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddParameter(Tag_CellSize[0], UNIT_NON_DIM, Tag_CellSize[1], Source_ParameterDB, DT_SingleInt);
    /// Set inputs from other modules (Source_Module or Source_Module_Optional)
    mdi.AddInput(VAR_PCP[0], UNIT_DEPTH_MM, VAR_PCP[1], Source_Module, DT_Raster1D);

    /// Set output variables of the current module
    mdi.AddOutput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], DT_Raster1D);


    /// Write out the XML file.
    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
