#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "HS_WB.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new HS_WB();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_HS_HYDRO, MCLSDESC_HS_HYDRO);
    mdi.SetDescription(MDESC_HS_WB);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_HS_WB);
    mdi.SetName(MID_HS_WB);
    mdi.SetVersion("0.3");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("HS_WB.chm");

    mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILDEPTH, UNIT_DEPTH_MM, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK, UNIT_NON_DIM, DESC_STREAM_LINK, Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(VAR_POROST, UNIT_VOL_FRA_M3M3, DESC_POROST, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_FIELDCAP, UNIT_VOL_FRA_M3M3, DESC_FIELDCAP, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(Tag_RchParam, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Array2D);

    mdi.AddInput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QOVERLAND, UNIT_FLOW_CMS, DESC_QOVERLAND, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QSOIL, UNIT_FLOW_CMS, DESC_QSOIL, Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_SOWB, UNIT_DEPTH_MM, DESC_SOWB, DT_Array2D);
    mdi.AddOutput(VAR_SBOF, UNIT_FLOW_CMS, DESC_SBOF, DT_Array1D);
    mdi.AddOutput(VAR_SBIF, UNIT_FLOW_CMS, DESC_SBIF, DT_Array1D);

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
//mdi.AddInput("D_P","mm","precipitation","Module",DT_Raster);
//
//mdi.AddInput("D_INLO","mm", "Interception Loss Distribution","Module", DT_Raster);			//interception
//mdi.AddInput("D_INET","mm", "Evaporation From Interception Storage","Module", DT_Raster);	//EI
//mdi.AddInput("D_DPST","mm", "depression storage","Module", DT_Raster);						//depression
//mdi.AddInput("D_DEET","mm", "evaporation from depression storage","Module", DT_Raster);	//depression ED
//mdi.AddInput("D_INFIL","mm","Infiltration map of watershed","Module", DT_Raster);		    //infiltration from infiltration module
//mdi.AddInput("D_SOET","mm","soil evapotranpiration","Module",DT_Raster);					//actual evapotranspiration
//mdi.AddInput("D_GRRE","mm","percolation","Module",DT_Raster);								//percolation
//mdi.AddInput("D_Revap","mm","revap","Module",DT_Raster);									//revap
//mdi.AddInput("D_SURU","mm","surface runoff","Module",DT_Raster);							//depression RS
//mdi.AddInput("D_SSRU","mm","subsurface runoff","Module",DT_Raster);						//subsurface runoff RI
//mdi.AddInput("T_RG","mm","groundwater runoff","Module",DT_Array1D);									//groundwater runoff RG  special,not distribution data
//mdi.AddInput("D_SOMO","mm","Distribution of soil moisture","Module",DT_Raster);
//mdi.AddInput("D_SNSB","mm", "snow sublimation","Module",DT_Raster);						//snow sublimation SE
//mdi.AddInput("D_TMIN","oC","min temperature","Module",DT_Raster);							//interpolation
//mdi.AddInput("D_TMAX","oC","max temperature","Module",DT_Raster);							//interpolation
//mdi.AddInput("D_SOTE","oC", "Soil Temperature","Module", DT_Raster);						//soil temperature

