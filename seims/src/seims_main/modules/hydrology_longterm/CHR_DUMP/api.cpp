#include "api.h"

#include "ChannelRoutingDump.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new ChannelRoutingDump();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Wang Yu-Jing");
    mdi.SetClass(MCLS_CH_ROUTING[0], MCLS_CH_ROUTING[1]);
    mdi.SetDescription(M_CHR_DUMP[1]);
    mdi.SetEmail("");
    mdi.SetID(M_CHR_DUMP[0]);
    mdi.SetName(M_CHR_DUMP[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("ChannelRoutingDump.chm");

    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_OUTLETID[0], UNIT_NON_DIM, VAR_OUTLETID[1], Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Reach);

    mdi.AddInput(VAR_SBOF[0], UNIT_DEPTH_MM, VAR_SBOF[1], Source_Module, DT_Array1D);

    mdi.AddInOutput(VAR_QRECH[0], UNIT_FLOW_CMS, VAR_QRECH[1], DT_Array1D, TF_SingleValue);
    
    // write out the XML file.
    string res = mdi.GetXMLDocument();
    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
