#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "SUR_CN.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new SUR_CN();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Wu Hui");
    mdi.SetClass(MCLS_SUR_RUNOFF, MCLSDESC_SUR_RUNOFF);
    mdi.SetDescription(MDESC_SUR_CN);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("SUR_CN.chm");
    mdi.SetID(MID_SUR_CN);
    mdi.SetName(MID_SUR_CN);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(VAR_T_SNOW, UNIT_TEMP_DEG, DESC_T_SNOW, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T_SOIL, UNIT_TEMP_DEG, DESC_T_SOIL, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T0, UNIT_TEMP_DEG, DESC_T0, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_S_FROZEN, UNIT_NON_DIM, DESC_S_FROZEN, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_CN2, UNIT_NON_DIM, DESC_CN2, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MOIST_IN, UNIT_VOL_FRA_M3M3, DESC_MOIST_IN, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_ROOTDEPTH, UNIT_LEN_M, DESC_ROOTDEPTH, Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(VAR_SOILDEPTH, UNIT_LEN_M, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_FIELDCAP, UNIT_VOL_FRA_M3M3, DESC_FIELDCAP, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POROST, UNIT_NON_DIM, DESC_POROST, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_WILTPOINT, UNIT_VOL_FRA_M3M3, DESC_WILTPOINT, Source_ParameterDB, DT_Raster2D);

    mdi.AddInput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, Source_Module,
                 DT_Raster1D);                    //from interception module
    mdi.AddInput(VAR_TMEAN, UNIT_TEMP_DEG, DESC_TMEAN, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DPST, UNIT_DEPTH_MM, DESC_DPST, Source_Module,
                 DT_Raster1D);                    //from depression module
    mdi.AddInput(VAR_SOTE, UNIT_TEMP_DEG, DESC_SOTE, Source_Module,
                 DT_Raster1D);            //from soil temperature module
    mdi.AddInput(VAR_SNAC, UNIT_DEPTH_MM, DESC_SNAC, Source_Module,
                 DT_Raster1D);                    //from snow accumulation module
    mdi.AddInput(VAR_SNME, UNIT_DEPTH_MM, DESC_SNME, Source_Module,
                 DT_Raster1D);                                //from snowmelt module

    mdi.AddOutput(VAR_EXCP, UNIT_DEPTH_MM, DESC_EXCP, DT_Raster1D);// just for depression module.
    mdi.AddOutput(VAR_INFIL, UNIT_DEPTH_MM, DESC_INFIL, DT_Raster1D);
    mdi.AddOutput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, DT_Raster2D);

    // write out the XML file.

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
//mdi.AddParameter(VAR_DEPREIN, UNIT_NON_DIM, DESC_DEPREIN, Source_ParameterDB, DT_Single);

//
//mdi.AddInput(VAR_TMIN, UNIT_TEMP_DEG, DESC_TMIN, Source_Module,DT_Raster1D);	//from interpolation module
//mdi.AddInput(VAR_TMAX, UNIT_TEMP_DEG, DESC_TMAX, Source_Module,DT_Raster1D);	//from interpolation module

//// set the dependencies
//mdi.AddDependency("Interpolation","Interpolation module");      //for pNet,T,
//mdi.AddDependency("Soil water","Soil Water Balance module");    // soilMoisture at given day
//mdi.AddDependency("Depression","Depression Storage module");     // SD
//mdi.AddDependency("soil temperature","Soil temperature module");  // TS
//mdi.AddDependency("Snow balance","Snow balance module");          // SA
//mdi.AddDependency("Snowmelt","Snowmelt module");                  // SM