#include "api.h"

#include "HydroVarsUpdate.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new HydroVarsUpdate();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass("HydroVarsUpdate", "HydroVarsUpdate");
    mdi.SetDescription("HydroVarsUpdate");
    mdi.SetEmail("");
    mdi.SetID("HydroVarsUpdate");
    mdi.SetName("HydroVarsUpdate");
    mdi.SetVersion("0.1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("HydroVarsUpdate.chm");
    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_SUBBASIN_PARAM[0], UNIT_NON_DIM, VAR_SUBBASIN_PARAM[1], Source_ParameterDB, DT_Subbasin);
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1DInt);

    mdi.AddInput(VAR_PET[0], UNIT_DEPTH_MM, VAR_PET[1], Source_Module_Optional, DT_Raster1D);

    mdi.AddOutput(VAR_SBPET[0], UNIT_DEPTH_MM, VAR_SBPET[1], DT_Array1D);
    mdi.AddOutput(VAR_SBGS[0], UNIT_DEPTH_MM, VAR_SBGS[1], DT_Array1D);

    // write out the XML file.
    string res = mdi.GetXMLDocument();
    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
