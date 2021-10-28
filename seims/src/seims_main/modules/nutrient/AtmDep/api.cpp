#include "api.h"

#include "AtmosphericDeposition.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new AtmosphericDeposition();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Huiran Gao, Liangjun Zhu");
    mdi.SetClass(MCLS_ATMDEP[0], MCLS_ATMDEP[1]);
    mdi.SetDescription(M_ATMDEP[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_ATMDEP[0]);
    mdi.SetName(M_ATMDEP[0]);
    mdi.SetVersion("1.0");
    mdi.SetWebsite(SEIMS_SITE);

    //mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RCN[0], UNIT_DENSITY, VAR_RCN[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_RCA[0], UNIT_DENSITY, VAR_RCA[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DRYDEP_NO3[0], UNIT_CONT_KGHA, VAR_DRYDEP_NO3[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DRYDEP_NH4[0], UNIT_CONT_KGHA, VAR_DRYDEP_NH4[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SOL_NH4[0], UNIT_CONT_KGHA, VAR_SOL_NH4[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_NO3[0], UNIT_CONT_KGHA, VAR_SOL_NO3[1], Source_ParameterDB, DT_Raster2D);

    // set input from other modules
    mdi.AddInput(VAR_PCP[0], UNIT_DEPTH_MM, VAR_PCP[1], Source_Module, DT_Raster1D);

    res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
