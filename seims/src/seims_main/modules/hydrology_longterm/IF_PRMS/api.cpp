#include "api.h"
#include "IF_PRMS.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new IF_PRMS();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfoHillslope mdi;

    mdi.SetAuthor("Yujing Wang");
    mdi.SetClass(MCLS_INTERFLOW[0], MCLS_INTERFLOW[1]);
    mdi.SetDescription(M_IF_PRMS[1]);
    mdi.SetEmail("");
    mdi.SetID(M_IF_PRMS[0]);
    mdi.SetName(M_IF_PRMS[0]);
    mdi.SetVersion("1");
    mdi.SetWebsite("");
    mdi.SetHelpfile("IF_PRMS.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddParameter(VAR_SUBBSNID_NUM[0], UNIT_NON_DIM, VAR_SUBBSNID_NUM[1], Source_ParameterDB, DT_SingleInt);
    mdi.AddParameter(VAR_SUBBSN[0], UNIT_NON_DIM, VAR_SUBBSN[1], Source_ParameterDB, DT_Raster1DInt);
    mdi.AddParameter(VAR_CELL_AREA[0], UNIT_AREA_M2, VAR_CELL_AREA[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_DEPTH_MM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1DInt);

    mdi.AddParameter(VAR_MAX_IF_RATE[0], UNIT_AREA_M2, VAR_MAX_IF_RATE[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POROST[0], UNIT_DEPTH_MM, VAR_POROST[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_AWC_AMOUNT[0], UNIT_DEPTH_MM, VAR_SOL_AWC_AMOUNT[1], Source_ParameterDB, DT_Raster2D);
    
    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);

    mdi.AddOutput(VAR_SBIF[0], UNIT_FLOW_CMS, VAR_SBIF[1], DT_Array1D);
    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
