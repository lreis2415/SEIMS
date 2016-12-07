#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "KinWavSed_CH.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new KinWavSed_CH();
}

// function to return the XML Metadata document string
//Set up metadata information
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res;

    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Hui Wu");
    mdi.SetClass(MCLS_CH_EROSION, MCLSDESC_CH_EROSION);
    mdi.SetDescription(MDESC_KINWAVSED_CH);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_KINWAVSED_CH);
    mdi.SetName(MID_KINWAVSED_CH);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("KinWavSed_CH.chm");

    mdi.AddParameter(Tag_CellSize, UNIT_NON_DIM, DESC_CellSize, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_HillSlopeTimeStep, UNIT_SECOND, DESC_DT_HS, File_Input, DT_Single);
    mdi.AddParameter(VAR_CH_TCCO, UNIT_NON_DIM, DESC_CH_TCCO, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_CH_DETCO, UNIT_NON_DIM, DESC_CH_DETCO, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_CHWIDTH, UNIT_LEN_M, DESC_CHWIDTH, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SLOPE, UNIT_PERCENT, DESC_SLOPE, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK, UNIT_NON_DIM, DESC_STREAM_LINK, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_USLE_K, UNIT_NON_DIM, DESC_USLE_K, Source_ParameterDB, DT_Raster1D);

    mdi.AddParameter(Tag_FLOWOUT_INDEX_D8, UNIT_NON_DIM, DESC_FLOWOUT_INDEX_D8, Source_ParameterDB, DT_Array1D);
    mdi.AddParameter(Tag_FLOWIN_INDEX_D8, UNIT_NON_DIM, DESC_FLOWIN_INDEX_D8, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(Tag_ReachParameter, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Array2D);

    // input from other module
    mdi.AddInput(VAR_SED_TO_CH, UNIT_KG, DESC_SED_TO_CH, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_HCH, UNIT_DEPTH_MM, DESC_HCH, Source_Module, DT_Array2D);
    //mdi.AddInput(VAR_QRECH, UNIT_FLOW_CMS, DESC_QRECH, Source_Module,DT_Array2D);
    mdi.AddInput("QRECH", "m3/s", "Flux in the downslope boundary of cells", "Module",
                 DT_Array2D);// from which module? By LJ
    /// set the output variables

    mdi.AddOutput(VAR_SED_OUTLET, UNIT_KGM3, DESC_SED_OUTLET, DT_Single);
    mdi.AddOutput(VAR_CH_DEP, UNIT_KG, DESC_CH_DEP, DT_Raster1D);
    mdi.AddOutput(VAR_CH_DET, UNIT_KG, DESC_CH_DET, DT_Raster1D);
    mdi.AddOutput(VAR_CH_SEDRATE, UNIT_KG_S, DESC_CH_SEDRATE, DT_Raster1D);
    mdi.AddOutput(VAR_CH_FLOWCAP, UNIT_KG, DESC_CH_FLOWCAP, DT_Raster1D);
    mdi.AddOutput(VAR_CH_VOL, UNIT_VOL_M3, DESC_CH_VOL, DT_Raster1D);
    mdi.AddOutput(VAR_CH_V, UNIT_SPEED_MS, DESC_CH_V, DT_Raster1D);

    mdi.AddDependency(MCLS_OL_EROSION, MCLSDESC_OL_EROSION);
    mdi.AddDependency(MCLS_CH_ROUTING, MCLSDESC_CH_ROUTING);

    // write out the XML file.
    res = mdi.GetXMLDocument();

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
	
