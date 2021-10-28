#include "api.h"

#include "IUH_SED_OL.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new IUH_SED_OL();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    // set the information properties
    mdi.SetAuthor("Junzhi Liu, Liangjun Zhu");
    mdi.SetClass(MCLS_OL_ROUTING[0], MCLS_OL_ROUTING[1]);
    mdi.SetDescription(M_IUH_SED_OL[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("");
    mdi.SetID(M_IUH_SED_OL[0]);
    mdi.SetName(M_IUH_SED_OL[0]);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_TimeStep[0], UNIT_HOUR, Tag_TimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_OL_IUH[0], UNIT_NON_DIM, VAR_OL_IUH[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddInput(VAR_SOER[0], UNIT_KG, VAR_SOER[1], Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_SED_TO_CH[0], UNIT_KG, VAR_SED_TO_CH[1], DT_Array1D);
    mdi.AddOutput(VAR_SEDYLD[0], UNIT_KG, VAR_SEDYLD[1], DT_Raster1D);

    // write out the XML file.
    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
