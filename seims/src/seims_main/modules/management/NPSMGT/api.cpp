#include "api.h"

#include "NonPointSource_Management.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new NPS_Management();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    string res;

    mdi.SetAuthor("Liangjun Zhu");
    mdi.SetClass(MCLS_MGT[0], MCLS_MGT[1]);
    mdi.SetDescription(M_NPSMGT[1]);
    mdi.SetID(M_NPSMGT[0]);
    mdi.SetName(M_NPSMGT[0]);
    mdi.SetVersion("1.0");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    /// set parameters from database
    mdi.AddParameter(Tag_TimeStep[0], UNIT_SECOND, Tag_TimeStep[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SCENARIO[0], UNIT_NON_DIM, VAR_SCENARIO[1], Source_ParameterDB, DT_Scenario);
    /// set input from other modules, all set to be optional
    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_NH4[0], UNIT_CONT_KGHA, VAR_SOL_NH4[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_NO3[0], UNIT_CONT_KGHA, VAR_SOL_NO3[1], Source_Module_Optional, DT_Raster2D);
    mdi.AddInput(VAR_SOL_SOLP[0], UNIT_CONT_KGHA, VAR_SOL_SOLP[1], Source_Module_Optional, DT_Raster2D);
    /// outputs

    /// write out the XML file.
    res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
