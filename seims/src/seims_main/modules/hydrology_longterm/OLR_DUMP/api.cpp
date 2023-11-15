#include "api.h"

#include "OverlandRoutingDump.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new OverlandRoutingDump();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfoHillslope mdi;

    // set the information properties
    mdi.SetAuthor("Wang Yu-Jing");
    mdi.SetClass(MCLS_OL_ROUTING[0], MCLS_OL_ROUTING[1]);
    mdi.SetDescription(M_OLR_DUMP[1]);
    mdi.SetEmail("");
    mdi.SetID(M_OLR_DUMP[0]);
    mdi.SetName(M_OLR_DUMP[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("OverlandRoutingDump.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddParameter(Tag_CellSize[0], UNIT_NON_DIM, Tag_CellSize[1], Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1DInt);
    mdi.AddParameter(Tag_TimeStep[0], UNIT_HOUR, Tag_TimeStep[1], File_Input, DT_SingleInt);
    mdi.AddParameter(VAR_CELL_AREA[0], UNIT_AREA_M2, VAR_CELL_AREA[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddInput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_SBOF[0], UNIT_FLOW_CMS, VAR_SBOF[1], DT_Array1D);
    mdi.AddOutput(VAR_SBIF[0], UNIT_FLOW_CMS, VAR_SBIF[1], DT_Array1D);
    mdi.AddOutput(VAR_SBQG[0], UNIT_FLOW_CMS, VAR_SBQG[1], DT_Array1D);
    
    // write out the XML file.
    string res = mdi.GetXMLDocument();
    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
