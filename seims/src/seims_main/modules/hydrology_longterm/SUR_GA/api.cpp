#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "SUR_GA.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new SUR_GA();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Wu Hui");
    mdi.SetClass("Surface runoff", "Calculate infiltration and excess precipitation.");
    mdi.SetDescription("Green and Ampt method to calculate infiltration and excess precipitation.");
    mdi.SetEmail("");
    mdi.SetHelpfile("SUR_GA.chm");
    mdi.SetID("SUR_GA");
    mdi.SetName("SUR_GA");
    mdi.SetVersion("0.1");
    mdi.SetWebsite("http://www.website.com");

    mdi.AddParameter("TimeStep", "hour", "time step of the simulation", "file.in", DT_Single);
    mdi.AddParameter("T_snow", "degree Celsius", "snowfall temperature", "ParameterDB_Snow", DT_Single);
    mdi.AddParameter("t_soil", "degree Celsius", "threshold soil freezing temperature", "ParameterDB_WaterBalance",
                     DT_Single);
    mdi.AddParameter("T0", "degree Celsius", "snowmelt threshold temperature", "ParameterDB_Snow", DT_Single);
    mdi.AddParameter("Conductivity", "mm/h", "Soil hydraulic conductivity", "ParameterDB_WaterBalance",
                     DT_Raster2D);  //soil_k
    mdi.AddParameter("Clay", "%", "Percent of clay content", "ParameterDB_WaterBalance", DT_Raster2D);
    mdi.AddParameter("Sand", "%", "Percent of sand content", "ParameterDB_WaterBalance", DT_Raster2D);
    mdi.AddParameter("porosity", "-", "Soil porosity", "ParameterDB_WaterBalance", DT_Raster2D);
    mdi.AddParameter("Rootdepth", "m", "Root depth", "ParameterDB_WaterBalance", DT_Raster1D);
    mdi.AddParameter("CN2", "-", "CN under moisture condition II", "ParameterDB_WaterBalance", DT_Raster1D);
    mdi.AddParameter("Fieldcap", "m3/m3", "Soil field capacity", "ParameterDB_WaterBalance", DT_Raster2D);
    mdi.AddParameter("Wiltingpoint", "m3/m3", "Plant wilting point moisture", "ParameterDB_WaterBalance", DT_Raster2D);
    mdi.AddParameter("s_frozen", "m3/m3", "frozen soil moisture", "ParameterDB_WaterBalance", DT_Single);
    mdi.AddParameter("Mask", "", "Array containing the row and column numbers for valid cells", "ParameterDB_Snow",
                     DT_Array2D);

    mdi.AddInput("D_NEPR", "mm", "The net precipitation", "Module", DT_Raster1D);
    mdi.AddInput("D_TMin", "degree Celsius", "The minimum air temperature", "Module", DT_Raster1D);
    mdi.AddInput("D_TMax", "degree Celsius", "The maximum air temperature", "Module", DT_Raster1D);
    mdi.AddInput("D_SOMO", "m3/m3", "The soil moisture", "Module", DT_Raster2D);
    mdi.AddInput("D_DPST", "mm", "The depression storage", "Module", DT_Raster1D);
    mdi.AddInput("D_SOTE", "degree Celsius", "The soil temperature", "Module", DT_Raster1D);
    mdi.AddInput("D_SNAC", "mm", "The snow accumulation", "Module", DT_Raster1D);
    mdi.AddInput("D_SNME", "mm", "The snowmelt", "Module", DT_Raster1D);

    mdi.AddOutput("EXCP", "mm", "The excess precipitation", DT_Raster1D);
    mdi.AddOutput("INFIL", "mm", "Infiltration map of watershed", DT_Raster1D);

    // set the dependencies
    mdi.AddDependency("Interpolation", "Interpolation module");      //for pNet,T,
    mdi.AddDependency("Soil water", "Soil Water Balance module");    // soilMoisture at given day
    mdi.AddDependency("Depression", "Depression Storage module");     // SD
    mdi.AddDependency("soil temperature", "Soil temperature module");  // TS
    mdi.AddDependency("Snow balance", "Snow balance module");          // SA
    mdi.AddDependency("Snowmelt", "Snowmelt module");                  // SM

    // write out the XML file.

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}