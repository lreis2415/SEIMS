#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "KinWavSed_OL.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new KinWavSed_OL();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res;

    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Hui Wu");
    mdi.SetClass(MCLS_OL_EROSION, MCLSDESC_OL_EROSION);
    mdi.SetDescription(MDESC_KINWAVSED_OL);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_KINWAVSED_OL);
    mdi.SetName(MID_KINWAVSED_OL);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("KinWavSed_OL.chm");

    mdi.AddParameter(Tag_CellSize, UNIT_NON_DIM, DESC_CellSize, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_HillSlopeTimeStep, UNIT_SECOND, DESC_DT_HS, File_Input, DT_Single);

    mdi.AddParameter(VAR_OL_SED_ECO1, UNIT_NON_DIM, DESC_OL_SED_ECO1, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_OL_SED_ECO2, UNIT_NON_DIM, DESC_OL_SED_ECO2, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_OL_SED_CCOE, UNIT_NON_DIM, DESC_OL_SED_CCOE, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SLOPE, UNIT_PERCENT, DESC_SLOPE, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_USLE_K, UNIT_NON_DIM, DESC_USLE_K, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_USLE_C, UNIT_NON_DIM, DESC_USLE_C, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK, UNIT_NON_DIM, DESC_STREAM_LINK, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MANNING, UNIT_NON_DIM, DESC_MANNING, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHWIDTH, UNIT_LEN_M, DESC_CHWIDTH, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_FLOWIN_INDEX_D8, UNIT_NON_DIM, DESC_FLOWIN_INDEX_D8, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(Tag_ROUTING_LAYERS, UNIT_NON_DIM, DESC_ROUTING_LAYERS, Source_ParameterDB, DT_Array2D);

    //input from other module
    mdi.AddInput(VAR_SURU, UNIT_DEPTH_MM, DESC_SURU, Source_Module, DT_Raster1D);
    mdi.AddInput("D_FlowWidth", "m", "Flow width of overland plane", Source_Module,
                 DT_Raster1D);        //FlowWidth, from Overland routing module
    mdi.AddInput("D_DETSplash", "kg", "the distribution of splash detachment", Source_Module,
                 DT_Raster1D);    //Splash erosion, from splash erosion module
    mdi.AddInput("D_QOverland", "m3/s", "Flux in the downslope boundary of cells", Source_Module, DT_Raster1D);

    /// set the output variables
    mdi.AddOutput(VAR_OL_DET, UNIT_KG, DESC_OL_DET, DT_Raster1D);
	mdi.AddOutput(VAR_SED_DEP, UNIT_KG, DESC_SED_DEP, DT_Raster1D);
	mdi.AddOutput(VAR_SED_TO_CH, UNIT_KG, DESC_SED_TO_CH, DT_Raster1D);
	mdi.AddOutput(VAR_SED_FLOW, UNIT_KG, DESC_SED_FLOW, DT_Raster1D); // add by Gao, 2016-07-27
	mdi.AddOutput(VAR_SED_FLUX, UNIT_KG_S, DESC_SED_FLUX, DT_Raster1D); // add by Gao, 2016-07-27

    mdi.AddDependency("Depression", "Depression storage module"); //for WH
    mdi.AddDependency(MCLS_OL_ROUTING, MCLSDESC_OL_ROUTING);
    mdi.AddDependency("Soil Detachment", "Rain and overland soil detachment erosion module");          // for DETSplash

    // write out the XML file.
    res = mdi.GetXMLDocument();

    //return res;

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}


//mdi.AddInput("D_CELLH","mm","Water depth in the downslope boundary of cells","Module",DT_Raster);		//WH, from Overland routing module
//mdi.AddInput("D_HTOCH", "mm", "Water depth added to channel water depth", "Module",DT_Raster);
//mdi.AddParameter("StoneFrac","","the fraction of stones on the surface, affects splash","ParameterDB_Sediment",DT_Raster);
//mdi.AddParameter("GrassFrac","","the fraction of grasstrip in a cell","ParameterDB_Sediment",DT_Raster);
//mdi.AddOutput("TestV","m/s", "distribution of sediment content in flow", DT_Raster1D);
//mdi.AddOutput("TestQV","m/s", "distribution of sediment content in flow", DT_Raster1D);
//mdi.AddOutput("DETFLOW","kg/m^2", "distribution of flow detachment", DT_Raster);
//mdi.AddOutput("SEDCONC","kg/m^3", "distribution of sediment concentration in flow", DT_Raster);
//mdi.AddOutput("SedKG","kg", "distribution of sediment content in flow", DT_Raster);