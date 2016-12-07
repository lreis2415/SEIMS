#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "ReservoirMethod.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new ReservoirMethod();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Wu Hui; Zhiqiang Yu; Liang-Jun Zhu");
    mdi.SetClass(MCLS_GW, MCLSDESC_GW);
    mdi.SetDescription(MDESC_GWA_RE);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("GWA_RE.chm");
    mdi.SetID(MID_GWA_RE);
    mdi.SetName(MID_GWA_RE);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_TimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Config, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);

	/// add DT_Subbasin
	mdi.AddParameter(VAR_SUBBASIN_PARAM, UNIT_NON_DIM, DESC_SUBBASIN_PARAM, Source_ParameterDB, DT_Subbasin);

    mdi.AddParameter(VAR_GW0, UNIT_DEPTH_MM, DESC_GW0, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_GWMAX, UNIT_DEPTH_MM, DESC_GWMAX, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DF_COEF, UNIT_NON_DIM, DESC_DF_COEF, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_KG, UNIT_NON_DIM, DESC_KG, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_Base_ex, UNIT_NON_DIM, DESC_BASE_EX, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SOILDEPTH, UNIT_DEPTH_MM, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_SOILLAYERS, UNIT_NON_DIM, DESC_SOILLAYERS, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_SOILTHICK, UNIT_DEPTH_MM, DESC_SOILTHICK, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_SLOPE, UNIT_PERCENT, DESC_SLOPE, Source_ParameterDB, DT_Raster1D);

    mdi.AddInput(VAR_INET, UNIT_DEPTH_MM, DESC_INET, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DEET, UNIT_DEPTH_MM, DESC_DEET, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SOET, UNIT_DEPTH_MM, DESC_SOET, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_AET_PLT, UNIT_DEPTH_MM, DESC_AET_PLT, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PET, UNIT_DEPTH_MM, DESC_PET, Source_Module, DT_Raster1D);
	// VAR_GWNEW is OPTIONALLY from IUH_CH or other channel routing module
    mdi.AddInput(VAR_GWNEW, UNIT_DEPTH_MM, DESC_GWNEW, Source_Module_Optional, DT_Array1D); 
	// VAR_PERCO is from percolation modules
    mdi.AddInput(VAR_PERCO, UNIT_DEPTH_MM, DESC_PERCO, Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module, DT_Raster2D);

    mdi.AddOutput(VAR_GWWB, UNIT_NON_DIM, DESC_NONE, DT_Array2D);
    mdi.AddOutput(VAR_REVAP, UNIT_DEPTH_MM, DESC_REVAP, DT_Raster1D); //used by soil water balance module
    mdi.AddOutput(VAR_RG, UNIT_DEPTH_MM, DESC_RG, DT_Array1D); //used by soil water balance module
    mdi.AddOutput(VAR_SBQG, UNIT_FLOW_CMS, DESC_SBQG, DT_Array1D); //used by channel flow routing module
    mdi.AddOutput(VAR_SBPET, UNIT_DEPTH_MM, DESC_SBPET, DT_Array1D);
    mdi.AddOutput(VAR_SBGS, UNIT_DEPTH_MM, DESC_SBGS, DT_Array1D);

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
    //mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);