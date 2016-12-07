#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "UnsaturatedFlow.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new UnsaturatedFlow();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res;
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass("Unsaturated flow", "Calculate the flow of soil water within multi-layers.");
    mdi.SetDescription("UnsaturatedFlow");
    mdi.SetEmail("");
    mdi.SetID("UnsaturatedFlow");
    mdi.SetName("UnsaturatedFlow");
    mdi.SetVersion("0.5");
    mdi.SetWebsite("");
    mdi.SetHelpfile("UnsaturatedFlow.chm");

    mdi.AddParameter("Rootdepth", "m", "Root depth", "ParameterDB_WaterBalance", DT_Raster2D);
    mdi.AddParameter("Fieldcap", "m3/m3", "Soil field capacity", "ParameterDB_WaterBalance",
                     DT_Raster2D);                                //0f
    mdi.AddParameter("Wiltingpoint", "m3/m3", "Plant wilting point moisture", "ParameterDB_WaterBalance",
                     DT_Raster2D);                    //0w
    mdi.AddParameter("T_Soil", "oC", "threshold soil freezing temperature", "ParameterDB_WaterBalance",
                     DT_Single);                    //

    // set the parameters (non-time series)
    mdi.AddInput("D_PET", "mm", "pet", "Module",
                 DT_Raster1D);                                        //from interpolation module			PET
    mdi.AddInput("D_INET", "mm", "Evaporation From Interception Storage", "Module",
                 DT_Raster1D);    //from interception module			EI
    mdi.AddInput("D_DEET", "mm", "Distribution of depression ET", "Module",
                 DT_Raster1D);            //from depression module			ED
    mdi.AddInput("D_SOMO", "mm", "Distribution of soil moisture", "Module",
                 DT_Raster2D);            //from soil water balance module	0
    mdi.AddInput("D_SOTE", "oC", "Soil Temperature", "Module", DT_Raster1D);                        //soil temperature
    // set the parameters (non-time series)
    mdi.AddInput("D_GRRE", "mm", "percolation", "Module",
                 DT_Raster1D);                                //from percolation module
    mdi.AddInput("D_INFIL", "mm", "infiltration", "Module",
                 DT_Raster1D);                            //from infiltration module
    mdi.AddInput("D_SSRU", "mm", "Distribution of subsurface runoff.", "Module",
                 DT_Raster1D);            //form subsurface runoff module

    // set the output variables
    mdi.AddOutput("SOET", "mm", "Distribution of soil evapotranspiration for a user defined period.", DT_Raster1D);

    // write out the XML file.
    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}