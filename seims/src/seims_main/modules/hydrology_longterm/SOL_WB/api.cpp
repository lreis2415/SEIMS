#include <stdio.h>
#include <string>
#include <iostream>
#include "api.h"
#include "util.h"
#include "SOL_WB.h"
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new SOL_WB();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    mdi.SetAuthor("Chunping Ou");
    mdi.SetClass(MCLS_WTRBALANCE, MCLSDESC_WTRBALANCE);
    mdi.SetDescription(MDESC_SOL_WB);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_SOL_WB);
    mdi.SetName(MID_SOL_WB);
    mdi.SetVersion("1.0");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("SOL_WB.chm");

	mdi.AddParameter(VAR_SOL_ZMX, UNIT_DEPTH_MM, DESC_SOL_ZMX, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_SOILLAYERS, UNIT_NON_DIM, DESC_SOILLAYERS, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_SOILTHICK,UNIT_DEPTH_MM, DESC_SOILTHICK, Source_ParameterDB, DT_Raster1D);
	/// add DT_Subbasin
	mdi.AddParameter(VAR_SUBBASIN_PARAM, UNIT_NON_DIM, DESC_SUBBASIN_PARAM, Source_ParameterDB, DT_Subbasin);

    mdi.AddInput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, Source_Module, DT_Raster1D);// m_pNet
    mdi.AddInput(VAR_INFIL, UNIT_DEPTH_MM, DESC_INFIL, Source_Module, DT_Raster1D);// m_Infil
    mdi.AddInput(VAR_SOET, UNIT_DEPTH_MM, DESC_SOET, Source_Module, DT_Raster1D);//m_ES
    mdi.AddInput(VAR_REVAP, UNIT_DEPTH_MM, DESC_REVAP, Source_Module, DT_Raster1D);//m_Revap
    mdi.AddInput(VAR_SSRU, UNIT_DEPTH_MM, DESC_SSRU, Source_Module, DT_Raster2D);//m_RI
    mdi.AddInput(VAR_PERCO, UNIT_DEPTH_MM, DESC_PERCO, Source_Module, DT_Raster2D);//m_Perco
    mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module, DT_Raster2D);//m_somo
	//variables used to output
	mdi.AddInput(VAR_PCP, UNIT_DEPTH_MM, DESC_PCP, Source_Module, DT_Raster1D); //m_PCP
	mdi.AddInput(VAR_INLO, UNIT_DEPTH_MM, DESC_INLO, Source_Module, DT_Raster1D);//m_Interc
	mdi.AddInput(VAR_DPST, UNIT_DEPTH_MM, DESC_DPST, Source_Module, DT_Raster1D);//m_Dep
	mdi.AddInput(VAR_INET, UNIT_DEPTH_MM, DESC_INET, Source_Module, DT_Raster1D);//m_EI
	mdi.AddInput(VAR_DEET, UNIT_DEPTH_MM, DESC_DEET, Source_Module, DT_Raster1D);//m_ED
    mdi.AddInput(VAR_SURU, UNIT_DEPTH_MM, DESC_SURU, Source_Module, DT_Raster1D);//m_RS
    mdi.AddInput(VAR_RG, UNIT_DEPTH_MM, DESC_RG, Source_Module, DT_Array1D);//m_RG
	mdi.AddInput(VAR_SNSB, UNIT_DEPTH_MM, DESC_SNSB, Source_Module_Optional,DT_Raster1D);//m_SE
	mdi.AddInput(VAR_TMEAN, UNIT_TEMP_DEG, DESC_TMEAN, Source_Module, DT_Raster1D);//m_tMean
    mdi.AddInput(VAR_SOTE, UNIT_TEMP_DEG, DESC_SOTE, Source_Module, DT_Raster1D);//m_SoilT

    // set the output variables
    mdi.AddOutput(VAR_SOWB, UNIT_DEPTH_MM, DESC_SOWB, DT_Array2D);

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
	//mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D); //m_subbasin
    //mdi.AddInput(VAR_TMIN, UNIT_TEMP_DEG, DESC_TMIN, Source_Module, DT_Raster1D);//m_tMin
    //mdi.AddInput(VAR_TMAX, UNIT_TEMP_DEG, DESC_TMAX, Source_Module, DT_Raster1D);//m_tMax
    //mdi.AddParameter(VAR_SOILDEPTH, UNIT_DEPTH_MM, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster1D);