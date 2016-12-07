#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "SplashEro_Park.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new SplashEro_Park();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res;

    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Hui Wu");
    mdi.SetClass(MCLS_OL_EROSION, MCLSDESC_OL_EROSION);
    mdi.SetDescription(MDESC_SplashEro_Park);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_SplashEro_Park);
    mdi.SetName(MID_SplashEro_Park);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("SplashEro_Park.chm");

    mdi.AddParameter(Tag_CellSize, UNIT_NON_DIM, DESC_CellSize, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_HillSlopeTimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single);
    mdi.AddParameter(VAR_OMEGA, UNIT_NON_DIM, DESC_OMEGA, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_USLE_K, UNIT_NON_DIM, DESC_USLE_K, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_USLE_C, UNIT_NON_DIM, DESC_USLE_C, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SLOPE, UNIT_PERCENT, DESC_SLOPE, Source_ParameterDB, DT_Raster1D);
    //input from other module
    mdi.AddInput(VAR_DPST, UNIT_DEPTH_MM, DESC_DPST, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SURU, UNIT_DEPTH_MM, DESC_SURU, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QOVERLAND, UNIT_FLOW_CMS, DESC_QOVERLAND, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, Source_Module, DT_Raster1D);    //Rain, from interception module
    // set the output variables
    mdi.AddOutput(VAR_DETSPLASH, UNIT_KG, DESC_DETSPLASH, DT_Raster1D);

    mdi.AddDependency(MCLSDESC_INTERC, MCLSDESC_INTERC);      //for pNet, Leafdrain
    mdi.AddDependency(MCLS_OL_ROUTING, MCLSDESC_OL_ROUTING);

    // write out the XML file.
    res = mdi.GetXMLDocument();
    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
//mdi.AddParameter("Hplant","m","the height of the plants","ParameterDB_Sediment",DT_Raster);
//mdi.AddParameter("GRAD","","slope gradient (sine of slope angle)","ParameterDB_WaterBalance",DT_Raster);
//mdi.AddParameter("StoneFrac","","the fraction of stones on the surface, affects splash","ParameterDB_Sediment",DT_Raster);
//mdi.AddParameter("GrassFrac","","the fraction of grasstrip in a cell","ParameterDB_Sediment",DT_Raster);
//mdi.AddParameter("CoverFrac","","the fraction of vegetation cover","ParameterDB_Interception",DT_Raster);
//mdi.AddParameter("RandRough","cm","The random roughness","ParameterDB_Sediment",DT_Raster);
//mdi.AddParameter("CHWIDTH", "m", "channel width", "ParameterDB_Discharge", DT_Raster);

//WaterDepth = Depression + SurfaceRunoffDepth
//mdi.AddInput("D_SNAC","mm","snow accumulation","Module",DT_Raster);	//SnowCover, from snow water balance module
//mdi.AddInput("D_LeafDrain","m","leaf drainage from canopy","Module",DT_Raster);			//from Interception module
