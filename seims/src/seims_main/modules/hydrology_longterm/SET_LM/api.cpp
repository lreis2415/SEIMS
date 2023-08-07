#include "api.h"

#include "SET_LM.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new SET_LM();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    string res;
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Chunping Ou, Liangjun Zhu");
    mdi.SetClass(MCLS_AET[0], MCLS_AET[1]);
    mdi.SetDescription(M_SET_LM[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_SET_LM[0]);
    mdi.SetName(M_SET_LM[0]);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddParameter(VAR_T_SOIL[0], UNIT_TEMP_DEG, VAR_T_SOIL[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SOL_AWC_AMOUNT[0], UNIT_DEPTH_MM, VAR_SOL_AWC_AMOUNT[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1DInt);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);

    // set the parameters (non-time series)
    mdi.AddInput(VAR_PET[0], UNIT_DEPTH_MM, VAR_PET[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INET[0], UNIT_DEPTH_MM, VAR_INET[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DEET[0], UNIT_DEPTH_MM, VAR_DEET[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOTE[0], UNIT_TEMP_DEG, VAR_SOTE[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PPT[0], UNIT_DEPTH_MM, VAR_PPT[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);

    // set the output variables
    mdi.AddOutput(VAR_SOET[0], UNIT_DEPTH_MM, VAR_SOET[1], DT_Raster1D);

    // write out the XML file.
    res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
