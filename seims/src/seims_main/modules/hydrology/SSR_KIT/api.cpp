#include "api.h"

#include "MetadataInfo.h"
#include "SSR_KIT.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SSR_KIT();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    mdi.SetAuthor("SEIMS contributors");
    mdi.SetClass(MCLS_INTERFLOW[0], MCLS_INTERFLOW[1]);
    mdi.SetDescription(M_SSR_KIT[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("");
    mdi.SetID(M_SSR_KIT[0]);
    mdi.SetName(M_SSR_KIT[0]);
    mdi.SetVersion("1.0");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_HillSlopeTimeStep[1],
        File_Input, DT_SingleInt);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1],
        Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SLOPE[0], UNIT_PERCENT, VAR_SLOPE[1],
        Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHWIDTH[0], UNIT_LEN_M, VAR_CHWIDTH[1],
        Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1],
        Source_ParameterDB, DT_Reach);
    mdi.AddParameter(VAR_STREAM_LINK[0], UNIT_NON_DIM, VAR_STREAM_LINK[1],
        Source_ParameterDB, DT_Raster1DInt);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1],
        Source_ParameterDB, DT_Raster1DInt);
    mdi.AddParameter(VAR_SOILDEPTH[0], UNIT_DEPTH_MM, VAR_SOILDEPTH[1],
        Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_CONDUCT[0], UNIT_WTRDLT_MMH, VAR_CONDUCT[1],
        Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POROST[0], UNIT_STRG_M3M, VAR_POROST[1],
        Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_FIELDCAP[0], UNIT_STRG_M3M, VAR_FIELDCAP[1],
        Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(Tag_FLOWIN_INDEX[0], UNIT_NON_DIM, Tag_FLOWIN_INDEX[1],
        Source_ParameterDB, DT_Array2DInt);
    mdi.AddParameter(Tag_FLOWIN_FRACTION[0], UNIT_NON_DIM, Tag_FLOWIN_FRACTION[1],
        Source_ParameterDB_Optional, DT_Array2D);
    mdi.AddParameter(VAR_DHSVM_KH_RATIO[0], UNIT_NON_DIM, VAR_DHSVM_KH_RATIO[1],
        Source_ParameterDB_Optional, DT_Single);
    mdi.AddParameter(VAR_DHSVM_K_DECAY_FACTOR[0], UNIT_NON_DIM,
        VAR_DHSVM_K_DECAY_FACTOR[1], Source_ParameterDB_Optional, DT_Single);
    mdi.AddParameter(VAR_DHSVM_DEPTH_THRESHOLD_RATIO[0], UNIT_NON_DIM,
        VAR_DHSVM_DEPTH_THRESHOLD_RATIO[1],
        Source_ParameterDB_Optional, DT_Single);
    mdi.AddParameter(VAR_DHSVM_MAX_DRAIN_FRAC[0], UNIT_NON_DIM,
        VAR_DHSVM_MAX_DRAIN_FRAC[1], Source_ParameterDB_Optional, DT_Single);
    mdi.AddParameter(VAR_DHSVM_MAX_SUBSTEPS[0], UNIT_NON_DIM,
        VAR_DHSVM_MAX_SUBSTEPS[1], Source_ParameterDB_Optional, DT_Single);
    mdi.AddParameter(VAR_DHSVM_GRADIENT_MODE[0], UNIT_NON_DIM,
        VAR_DHSVM_GRADIENT_MODE[1],
        Source_ParameterDB_Optional, DT_SingleInt);

    // SOL_ST is volumetric soil moisture in the storm model despite its legacy unit label.
    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INFILCAPSURPLUS[0], UNIT_DEPTH_MM, VAR_INFILCAPSURPLUS[1],
        Source_Module_Optional, DT_Raster1D);

    mdi.AddOutput(VAR_QSOIL[0], UNIT_FLOW_CMS, VAR_QSOIL[1], DT_Raster1D);
    mdi.AddOutput(VAR_RETURNFLOW[0], UNIT_DEPTH_MM, VAR_RETURNFLOW[1], DT_Raster1D);
    mdi.AddOutput(VAR_DHSVM_WT_DEPTH[0], UNIT_DEPTH_MM, VAR_DHSVM_WT_DEPTH[1], DT_Raster1D);
    mdi.AddOutput(VAR_DHSVM_SAT_DEPTH[0], UNIT_DEPTH_MM, VAR_DHSVM_SAT_DEPTH[1], DT_Raster1D);
    mdi.AddOutput(VAR_DHSVM_QOUT[0], UNIT_FLOW_CMS, VAR_DHSVM_QOUT[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
