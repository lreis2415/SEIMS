#include "api.h"

#include "GWaterReservoir.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new GWaterReservoir();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;
    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_GW[0], MCLS_GW[1]);
    mdi.SetDescription(M_GW_RSVR[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("");
    mdi.SetID(M_GW_RSVR[0]);
    mdi.SetName(M_GW_RSVR[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1D);
    //mdi.AddParameter(Tag_RchParam, UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Array2D);
    // add reach information
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Reach);

    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_TimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_GW_KG[0], UNIT_NON_DIM, VAR_GW_KG[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_Base_ex[0], UNIT_NON_DIM, VAR_Base_ex[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_GW0[0], UNIT_DEPTH_MM, VAR_GW0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_GWMAX[0], UNIT_DEPTH_MM, VAR_GWMAX[1], Source_ParameterDB, DT_Single);

    mdi.AddInput(VAR_PERCO[0], UNIT_DEPTH_MM, VAR_PERCO[1], Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_SBQG[0], UNIT_FLOW_CMS, VAR_SBQG[1], DT_Array1D);
    mdi.AddOutput(VAR_SBGS[0], UNIT_DEPTH_MM, VAR_SBGS[1], DT_Array1D);

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}

