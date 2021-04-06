#include "api.h"

#include "IUH_IF.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new IUH_IF();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Wu Hui");
    mdi.SetClass(MCLS_INTERFLOW[0], MCLS_INTERFLOW[1]);
    mdi.SetDescription(M_IUH_IF[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("IUH_IF.chm");
    mdi.SetID(M_IUH_IF[0]);
    mdi.SetName(M_IUH_IF[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_TimeStep[0], UNIT_HOUR, Tag_TimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_CellSize[0], UNIT_NON_DIM, Tag_CellSize[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_OL_IUH[0], UNIT_NON_DIM, VAR_OL_IUH[1], Source_ParameterDB, DT_Array2D);

    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1D);

    //mdi.AddParameter("uhminCell","","start time of IUH for each grid cell","ParameterDB_Discharge",DT_Array1D);
    //mdi.AddParameter("uhmaxCell","","end time of IUH for each grid cell","ParameterDB_Discharge",DT_Array1D);
    mdi.AddInput(VAR_SSRU[0], UNIT_DEPTH_MM, VAR_SSRU[1], Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_SBIF[0], UNIT_FLOW_CMS, VAR_SBIF[1], DT_Array1D);

    // set the dependencies
    mdi.AddDependency("SSR_DA", "Subsurface Runoff module");

    // write out the XML file.

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
