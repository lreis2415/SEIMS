#include "api.h"

#include "KinWavSed_OL.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new KinWavSed_OL();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Hui Wu");
    mdi.SetClass(MCLS_OL_EROSION[0], MCLS_OL_EROSION[1]);
    mdi.SetDescription(M_KINWAVSED_OL[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_KINWAVSED_OL[0]);
    mdi.SetName(M_KINWAVSED_OL[0]);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("KinWavSed_OL.chm");

    mdi.AddParameter(Tag_CellSize[0], UNIT_NON_DIM, Tag_CellSize[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_HillSlopeTimeStep[1], File_Input, DT_Single);

    mdi.AddParameter(VAR_OL_SED_ECO1[0], UNIT_NON_DIM, VAR_OL_SED_ECO1[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_OL_SED_ECO2[0], UNIT_NON_DIM, VAR_OL_SED_ECO2[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_OL_SED_CCOE[0], UNIT_NON_DIM, VAR_OL_SED_CCOE[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SLOPE[0], UNIT_PERCENT, VAR_SLOPE[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_USLE_K[0], UNIT_NON_DIM, VAR_USLE_K[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_USLE_C[0], UNIT_NON_DIM, VAR_USLE_C[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK[0], UNIT_NON_DIM, VAR_STREAM_LINK[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_MANNING[0], UNIT_NON_DIM, VAR_MANNING[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHWIDTH[0], UNIT_LEN_M, VAR_CHWIDTH[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_FLOWIN_INDEX_D8[0], UNIT_NON_DIM, Tag_FLOWIN_INDEX_D8[1], Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(Tag_ROUTING_LAYERS[0], UNIT_NON_DIM, Tag_ROUTING_LAYERS[1], Source_ParameterDB, DT_Array2D);

    //input from other module
    mdi.AddInput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], Source_Module, DT_Raster1D);
    mdi.AddInput("D_FlowWidth", "m", "Flow width of overland plane", Source_Module,
                 DT_Raster1D);        //FlowWidth, from Overland routing module
    mdi.AddInput("D_DETSplash", "kg", "the distribution of splash detachment", Source_Module,
                 DT_Raster1D);    //Splash erosion, from splash erosion module
    mdi.AddInput("D_QOverland", "m3/s", "Flux in the downslope boundary of cells", Source_Module, DT_Raster1D);

    /// set the output variables
    mdi.AddOutput(VAR_OL_DET[0], UNIT_KG, VAR_OL_DET[1], DT_Raster1D);
    mdi.AddOutput(VAR_SED_DEP[0], UNIT_KG, VAR_SED_DEP[1], DT_Raster1D);
    mdi.AddOutput(VAR_SED_TO_CH[0], UNIT_KG, VAR_SED_TO_CH[1], DT_Raster1D);
    mdi.AddOutput(VAR_SED_FLOW[0], UNIT_KG, VAR_SED_FLOW[1], DT_Raster1D); // add by Gao, 2016-07-27
    mdi.AddOutput(VAR_SED_FLUX[0], UNIT_KG_S, VAR_SED_FLUX[1], DT_Raster1D); // add by Gao, 2016-07-27

    mdi.AddDependency("Depression", "Depression storage module"); //for WH
    mdi.AddDependency(MCLS_OL_ROUTING[0], MCLS_OL_ROUTING[1]);
    mdi.AddDependency("Soil Detachment", "Rain and overland soil detachment erosion module");          // for DETSplash

    // write out the XML file.
    string res = mdi.GetXMLDocument();

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
