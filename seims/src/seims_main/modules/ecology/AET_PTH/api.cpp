#include <stdio.h>
#include <string>
#include "util.h"
#include "api.h"
#include "AET_PriestleyTaylorHargreaves.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new AET_PT_H();
}

/// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;
    string res;

    mdi.SetAuthor("LiangJun Zhu");
    mdi.SetClass(MCLS_AET, MCLSDESC_AET);
    mdi.SetDescription(MDESC_AET_PTH);
    mdi.SetID(MID_AET_PTH);
    mdi.SetName(MID_AET_PTH);
    mdi.SetVersion("1.1");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    /// set parameters from database
    mdi.AddParameter(VAR_ESCO, UNIT_NON_DIM, DESC_ESCO, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILLAYERS, UNIT_NON_DIM, DESC_SOILLAYERS, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_SOILDEPTH, UNIT_DEPTH_MM, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_SOILTHICK, UNIT_DEPTH_MM, DESC_SOILTHICK, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SOL_AWC, UNIT_DEPTH_MM, DESC_SOL_AWC, Source_ParameterDB, DT_Raster2D);

    mdi.AddParameter(VAR_SOL_NO3, UNIT_CONT_KGHA, DESC_SOL_NO3, Source_Module, DT_Raster2D);
    /// set input from other modules
    mdi.AddInput(DataType_MeanTemperature, UNIT_TEMP_DEG, DESC_TMEAN, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_LAIDAY, UNIT_AREA_RATIO, DESC_LAIDAY, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PET, UNIT_WTRDLT_MMD, DESC_PET, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_INET, UNIT_DEPTH_MM, DESC_INET, Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_SNAC, UNIT_DEPTH_MM, DESC_SNAC, Source_Module, DT_Raster1D); /// in swat, sno_hru
    mdi.AddInput(VAR_SNSB, UNIT_DEPTH_MM, DESC_SNSB, Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_SOL_COV, UNIT_CONT_KGHA, DESC_SOL_COV, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module, DT_Raster2D);/// sol_st in SWAT
	mdi.AddInput(VAR_SOL_SW, UNIT_DEPTH_MM, DESC_SOL_SW, Source_Module, DT_Raster1D); /// sol_sw in SWAT
    /// set the output variables
	mdi.AddOutput(VAR_SNSB, UNIT_DEPTH_MM, DESC_SNSB, DT_Raster1D); /// in swat, snoev
    mdi.AddOutput(VAR_PPT, UNIT_DEPTH_MM, DESC_PPT, DT_Raster1D);
    mdi.AddOutput(VAR_SOET, UNIT_DEPTH_MM, DESC_SOET, DT_Raster1D);
    mdi.AddOutput(VAR_SNAC, UNIT_DEPTH_MM, DESC_SNAC, DT_Raster1D);
    mdi.AddOutput(VAR_SNO3UP, UNIT_CONT_KGHA, DESC_SNO3UP, DT_Single);
    //mdi.AddOutput(VAR_SOL_SW, UNIT_DEPTH_MM, DESC_SOL_SW, DT_Raster1D); /// mm, different from VAR_SOMO
    //mdi.AddOutput(VAR_SOMO, UNIT_DEPTH_MM, DESC_SOL_ST, DT_Raster2D);
    /// write out the XML file.
    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}