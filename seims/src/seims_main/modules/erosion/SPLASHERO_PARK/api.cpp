#include "api.h"

#include "SplashEro_Park.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new SplashEro_Park();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Hui Wu");
    mdi.SetClass(MCLS_OL_EROSION[0], MCLS_OL_EROSION[1]);
    mdi.SetDescription(M_SplashEro_Park[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_SplashEro_Park[0]);
    mdi.SetName(M_SplashEro_Park[0]);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("SplashEro_Park.chm");

    mdi.AddParameter(Tag_CellSize[0], UNIT_NON_DIM, Tag_CellSize[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_TimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(VAR_OMEGA[0], UNIT_NON_DIM, VAR_OMEGA[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_USLE_K[0], UNIT_NON_DIM, VAR_USLE_K[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_USLE_C[0], UNIT_NON_DIM, VAR_USLE_C[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SLOPE[0], UNIT_PERCENT, VAR_SLOPE[1], Source_ParameterDB, DT_Raster1D);
    //input from other module
    mdi.AddInput(VAR_DPST[0], UNIT_DEPTH_MM, VAR_DPST[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_QOVERLAND[0], UNIT_FLOW_CMS, VAR_QOVERLAND[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], Source_Module, DT_Raster1D);    //Rain, from interception module
    // set the output variables
    mdi.AddOutput(VAR_DETSPLASH[0], UNIT_KG, VAR_DETSPLASH[1], DT_Raster1D);

    mdi.AddDependency(MCLS_INTERC[1], MCLS_INTERC[1]);      //for pNet, Leafdrain
    mdi.AddDependency(MCLS_OL_ROUTING[0], MCLS_OL_ROUTING[1]);

    // write out the XML file.
    string res = mdi.GetXMLDocument();
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
