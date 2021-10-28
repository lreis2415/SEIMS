#include "api.h"

#include "SSR_DA.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SSR_DA();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    mdi.SetAuthor("Zhiqiang Yu; Junzhi Liu; Liangjun Zhu");
    mdi.SetClass(MCLS_INTERFLOW[0], MCLS_INTERFLOW[1]);
    mdi.SetDescription(M_SSR_DA[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_SSR_DA[0]);
    mdi.SetName(M_SSR_DA[0]);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");

    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_TimeStep[0], UNIT_SECOND, Tag_TimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(VAR_KI[0], UNIT_NON_DIM, VAR_KI[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T_SOIL[0], UNIT_TEMP_DEG, VAR_T_SOIL[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_CONDUCT[0], UNIT_WTRDLT_MMH, VAR_CONDUCT[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_UL[0], UNIT_DEPTH_MM, VAR_SOL_UL[1], Source_ParameterDB, DT_Raster2D); // m_sat
    mdi.AddParameter(VAR_POREIDX[0], UNIT_NON_DIM, VAR_POREIDX[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_AWC[0], UNIT_DEPTH_MM, VAR_SOL_AWC[1], Source_ParameterDB, DT_Raster2D);   // m_fc
    mdi.AddParameter(VAR_SOL_WPMM[0], UNIT_DEPTH_MM, VAR_SOL_WPMM[1], Source_ParameterDB, DT_Raster2D); // m_wp
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SLOPE[0], UNIT_PERCENT, VAR_SLOPE[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHWIDTH[0], UNIT_LEN_M, VAR_CHWIDTH[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK[0], UNIT_NON_DIM, VAR_STREAM_LINK[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_FLOWIN_INDEX[0], UNIT_NON_DIM, Tag_FLOWIN_INDEX[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(Tag_FLOWIN_FRACTION[0], UNIT_NON_DIM, Tag_FLOWIN_FRACTION[1], Source_ParameterDB_Optional, DT_Array2D);
    mdi.AddParameter(Tag_ROUTING_LAYERS[0], UNIT_NON_DIM, Tag_ROUTING_LAYERS[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddInput(VAR_SOTE[0], UNIT_TEMP_DEG, VAR_SOTE[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_SW[0], UNIT_DEPTH_MM, VAR_SOL_SW[1], Source_Module, DT_Raster1D);
    // set the output variables
    mdi.AddOutput(VAR_SSRU[0], UNIT_DEPTH_MM, VAR_SSRU[1], DT_Raster2D);
    mdi.AddOutput(VAR_SSRUVOL[0], UNIT_VOL_M3, VAR_SSRUVOL[1], DT_Raster2D);

    mdi.AddOutput(VAR_SBIF[0], UNIT_FLOW_CMS, VAR_SBIF[1], DT_Array1D);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
