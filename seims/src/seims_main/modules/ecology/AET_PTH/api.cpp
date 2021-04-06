#include "api.h"

#include "AET_PriestleyTaylorHargreaves.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new AET_PT_H();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    string res;
    mdi.SetAuthor("Liangjun Zhu");
    mdi.SetClass(MCLS_AET[0], MCLS_AET[1]);
    mdi.SetDescription(M_AET_PTH[1]);
    mdi.SetID(M_AET_PTH[0]);
    mdi.SetName(M_AET_PTH[0]);
    mdi.SetVersion("1.1");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    /// set parameters from database
    mdi.AddParameter(VAR_ESCO[0], UNIT_NON_DIM, VAR_ESCO[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILLAYERS[0], UNIT_NON_DIM, VAR_SOILLAYERS[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILDEPTH[0], UNIT_DEPTH_MM, VAR_SOILDEPTH[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOILTHICK[0], UNIT_DEPTH_MM, VAR_SOILTHICK[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_AWC[0], UNIT_DEPTH_MM, VAR_SOL_AWC[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_NO3[0], UNIT_CONT_KGHA, VAR_SOL_NO3[1], Source_Module, DT_Raster2D);
    /// set input from other modules
    mdi.AddInput(DataType_MeanTemperature, UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_LAIDAY[0], UNIT_AREA_RATIO, VAR_LAIDAY[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PET[0], UNIT_WTRDLT_MMD, VAR_PET[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_INET[0], UNIT_DEPTH_MM, VAR_INET[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], Source_Module, DT_Raster1D); /// in swat, sno_hru
    mdi.AddInput(VAR_SNSB[0], UNIT_DEPTH_MM, VAR_SNSB[1], Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_SOL_COV[0], UNIT_CONT_KGHA, VAR_SOL_COV[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D); /// sol_st in SWAT
    mdi.AddInput(VAR_SOL_SW[0], UNIT_DEPTH_MM, VAR_SOL_SW[1], Source_Module, DT_Raster1D); /// sol_sw in SWAT

    /// set the output variables
    mdi.AddOutput(VAR_PPT[0], UNIT_DEPTH_MM, VAR_PPT[1], DT_Raster1D);
    mdi.AddOutput(VAR_SOET[0], UNIT_DEPTH_MM, VAR_SOET[1], DT_Raster1D);

    /// write out the XML file.
    res = mdi.GetXMLDocument();
    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
