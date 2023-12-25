#include "api.h"

#include "OLR_CIUH_GAMMA.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new OLR_CIUH_GAMMA();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfoHillslope mdi;

    // set the information properties
    mdi.SetAuthor("Wang Yu-Jing");
    mdi.SetClass(MCLS_OL_ROUTING[0], MCLS_OL_ROUTING[1]);
    mdi.SetDescription(M_OLR_CIUH_GAMMA[1]);
    mdi.SetEmail("");
    mdi.SetID(M_OLR_CIUH_GAMMA[0]);
    mdi.SetName(M_OLR_CIUH_GAMMA[0]);
    mdi.SetVersion("1.0");
    mdi.SetWebsite("");
    mdi.SetHelpfile("OLR_CIUH_GAMMA.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1DInt);
    mdi.AddParameter(VAR_CELL_AREA[0], UNIT_AREA_M2, VAR_CELL_AREA[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_GAMMA_SCALE[0], UNIT_NON_DIM, VAR_GAMMA_SCALE[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_GAMMA_SHAPE[0], UNIT_NON_DIM, VAR_GAMMA_SHAPE[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddInput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_SBOF[0], UNIT_FLOW_CMS, VAR_SBOF[1], DT_Array1D);
    
    // write out the XML file.
    string res = mdi.GetXMLDocument();
    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
