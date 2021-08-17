#include "api.h"

#include "KinWavSed_CH.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new KinWavSed_CH();
}

// function to return the XML Metadata document string
//Set up metadata information
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Hui Wu");
    mdi.SetClass(MCLS_CH_EROSION[0], MCLS_CH_EROSION[1]);
    mdi.SetDescription(M_KINWAVSED_CH[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_KINWAVSED_CH[0]);
    mdi.SetName(M_KINWAVSED_CH[0]);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("KinWavSed_CH.chm");

    mdi.AddParameter(Tag_CellSize[0], UNIT_NON_DIM, Tag_CellSize[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_HillSlopeTimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_LayeringMethod[0], UNIT_NON_DIM, Tag_LayeringMethod[1], File_Input, DT_Single);
    mdi.AddParameter(VAR_CH_TCCO[0], UNIT_NON_DIM, VAR_CH_TCCO[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CH_DETCO[0], UNIT_NON_DIM, VAR_CH_DETCO[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_CHWIDTH[0], UNIT_LEN_M, VAR_CHWIDTH[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SLOPE[0], UNIT_PERCENT, VAR_SLOPE[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK[0], UNIT_NON_DIM, VAR_STREAM_LINK[1], Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_USLE_K[0], UNIT_NON_DIM, VAR_USLE_K[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(Tag_FLOWOUT_INDEX_D8[0], UNIT_NON_DIM, Tag_FLOWOUT_INDEX_D8[1], Source_ParameterDB, DT_Array1D);
    mdi.AddParameter(Tag_FLOWIN_INDEX_D8[0], UNIT_NON_DIM, Tag_FLOWIN_INDEX_D8[1], Source_ParameterDB, DT_Array2D);
    // add reach information
    mdi.AddParameter(VAR_REACH_PARAM[0], UNIT_NON_DIM, VAR_REACH_PARAM[1], Source_ParameterDB, DT_Reach);

    // input from other module
    mdi.AddInput(VAR_SED_TO_CH[0], UNIT_KG, VAR_SED_TO_CH[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_HCH[0], UNIT_DEPTH_MM, VAR_HCH[1], Source_Module, DT_Array2D);
    //mdi.AddInput(VAR_QRECH[0], UNIT_FLOW_CMS, VAR_QRECH[1], Source_Module,DT_Array2D);
    // from which module? By LJ
    mdi.AddInput("QRECH", "m3/s", "Flux in the downslope boundary of cells", "Module", DT_Array2D);
    /// set the output variables

    //mdi.AddOutput(VAR_SED_OUTLET, UNIT_KGM3, DESC_SED_OUTLET, DT_Single);
    mdi.AddOutput(VAR_CH_DEP[0], UNIT_KG, VAR_CH_DEP[1], DT_Raster1D);
    mdi.AddOutput(VAR_CH_DET[0], UNIT_KG, VAR_CH_DET[1], DT_Raster1D);
    mdi.AddOutput(VAR_CH_SEDRATE[0], UNIT_KG_S, VAR_CH_SEDRATE[1], DT_Raster1D);
    mdi.AddOutput(VAR_CH_FLOWCAP[0], UNIT_KG, VAR_CH_FLOWCAP[1], DT_Raster1D);
    mdi.AddOutput(VAR_CH_VOL[0], UNIT_VOL_M3, VAR_CH_VOL[1], DT_Raster1D);
    mdi.AddOutput(VAR_CH_V[0], UNIT_SPEED_MS, VAR_CH_V[1], DT_Raster1D);

    mdi.AddDependency(MCLS_OL_EROSION[0], MCLS_OL_EROSION[1]);
    mdi.AddDependency(MCLS_CH_ROUTING[0], MCLS_CH_ROUTING[1]);

    // write out the XML file.
    string res = mdi.GetXMLDocument();

    //return res;

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}

//mdi.AddOutput("SEDSUBBASIN","ton/s", "sediment yield production of each subbasin, sediment flux at subbasin (kg/s)", DT_Raster1D);
//mdi.AddInput("T_HCH","mm", "Water depth in the downslope boundary of cells","Module",DT_Array2D);		//chWH, from channel routing module

//mdi.AddInput("FlowWidth","m", "Flow width of overland plane","Module",DT_Raster);		//FlowWidth, from Overland routing module
//mdi.AddInput("T_QCH", "m3/s", "Flux in the downslope boundary of cells", "Module",DT_Array2D);


/*mdi.AddOutput("CHDETFLOW","kg/m^2", "distribution of channel flow detachment", DT_Raster);
mdi.AddOutput("CHSEDDEP","kg", "distribution of channel sediment deposition", DT_Raster);
mdi.AddOutput("CHSEDCONC","kg/m^3", "distribution of sediment concentration in channel flow", DT_Raster);
mdi.AddOutput("CHSEDINFLOW","kg", "distribution of sediment content in channel flow", DT_Raster);*/

