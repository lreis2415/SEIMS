#include "api.h"

#include "OverlandRoutingDump.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new OverlandRoutingDump();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass("Unsaturated flow", "Calculate the flow of soil water within multi-layers.");
    mdi.SetDescription("OverlandRoutingDump");
    mdi.SetEmail("");
    mdi.SetID("OverlandRoutingDump");
    mdi.SetName("OverlandRoutingDump");
    mdi.SetVersion("0.1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("OverlandRoutingDump.chm");

    // write out the XML file.
    string res = mdi.GetXMLDocument();
    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_OUTLETID[0], UNIT_NON_DIM, VAR_OUTLETID[1], Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_SUBBASIN_PARAM[0], UNIT_NON_DIM, VAR_SUBBASIN_PARAM[1], Source_ParameterDB, DT_Subbasin);

    mdi.AddOutput(VAR_SBPET[0], UNIT_DEPTH_MM, VAR_SBPET[1], DT_Array1D);
    mdi.AddOutput(VAR_SBGS[0], UNIT_DEPTH_MM, VAR_SBGS[1], DT_Array1D);

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
