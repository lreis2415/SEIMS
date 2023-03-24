#include "api.h"

#include "SUR_CN.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new SUR_CN();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Wu Hui");
    mdi.SetClass(MCLS_SUR_RUNOFF[0], MCLS_SUR_RUNOFF[1]);
    mdi.SetDescription(M_SUR_CN[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("SUR_CN.chm");
    mdi.SetID(M_SUR_CN[0]);
    mdi.SetName(M_SUR_CN[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(VAR_T_SNOW[0], UNIT_TEMP_DEG, VAR_T_SNOW[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T_SOIL[0], UNIT_TEMP_DEG, VAR_T_SOIL[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T0[0], UNIT_TEMP_DEG, VAR_T0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_S_FROZEN[0], UNIT_NON_DIM, VAR_S_FROZEN[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_CN2[0], UNIT_NON_DIM, VAR_CN2[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MOIST_IN[0], UNIT_VOL_FRA_M3M3, VAR_MOIST_IN[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_ROOTDEPTH[0], UNIT_LEN_M, VAR_ROOTDEPTH[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(VAR_SOILDEPTH[0], UNIT_LEN_M, VAR_SOILDEPTH[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_FIELDCAP[0], UNIT_VOL_FRA_M3M3, VAR_FIELDCAP[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POROST[0], UNIT_NON_DIM, VAR_POROST[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_WILTPOINT[0], UNIT_VOL_FRA_M3M3, VAR_WILTPOINT[1], Source_ParameterDB, DT_Raster2D);

    mdi.AddInput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], Source_Module, DT_Raster1D);  //from interception module
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_DPST[0], UNIT_DEPTH_MM, VAR_DPST[1], Source_Module, DT_Raster1D);  //from depression module
    mdi.AddInput(VAR_SOTE[0], UNIT_TEMP_DEG, VAR_SOTE[1], Source_Module, DT_Raster1D);  //from soil temperature module
    mdi.AddInput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], Source_Module, DT_Raster1D);  //from snow accumulation module
    mdi.AddInput(VAR_SNME[0], UNIT_DEPTH_MM, VAR_SNME[1], Source_Module, DT_Raster1D);  //from snowmelt module

    mdi.AddOutput(VAR_EXCP[0], UNIT_DEPTH_MM, VAR_EXCP[1], DT_Raster1D);  // just for depression module.
    mdi.AddOutput(VAR_INFIL[0], UNIT_DEPTH_MM, VAR_INFIL[1], DT_Raster1D);
    mdi.AddOutput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], DT_Raster2D);

    // write out the XML file.

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
//mdi.AddParameter(VAR_DEPREIN[0], UNIT_NON_DIM, VAR_DEPREIN[1], Source_ParameterDB, DT_Single);

//
//mdi.AddInput(VAR_TMIN[0], UNIT_TEMP_DEG, VAR_TMIN[1], Source_Module,DT_Raster1D);	//from interpolation module
//mdi.AddInput(VAR_TMAX[0], UNIT_TEMP_DEG, VAR_TMAX[1], Source_Module,DT_Raster1D);	//from interpolation module

//// set the dependencies
//mdi.AddDependency("Interpolation","Interpolation module");      //for pNet,T,
//mdi.AddDependency("Soil water","Soil Water Balance module");    // soilMoisture at given day
//mdi.AddDependency("Depression","Depression Storage module");     // SD
//mdi.AddDependency("soil temperature","Soil temperature module");  // TS
//mdi.AddDependency("Snow balance","Snow balance module");          // SA
//mdi.AddDependency("Snowmelt","Snowmelt module");                  // SM
