#include "api.h"

#include "VARS_NEPR.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new VARS_NEPR();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfoHillslope mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass("VARS_NEPR", "VARS_NEPR");
    mdi.SetDescription("VARS_NEPR");
    mdi.SetEmail("");
    mdi.SetID("VARS_NEPR");
    mdi.SetName("VARS_NEPR");
    mdi.SetVersion("1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("VARS_NEPR.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddInput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DPST[0], UNIT_DEPTH_MM, VAR_DPST[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_SNME[0], UNIT_DEPTH_MM, VAR_SNME[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_GLAC_REL[0], UNIT_DEPTH_MM, VAR_GLAC_REL[1], Source_Module_Optional, DT_Raster1D);

    // write out the XML file.
    string res = mdi.GetXMLDocument();
    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
