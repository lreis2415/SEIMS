#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "ExcessRunoff.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new ExcessRunoff();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass("Surface runoff",
                 "Calculate infiltration and excess precipitation using saturation excess mechanism.");
    mdi.SetDescription("saturation excess method to calculate infiltration and excess precipitation.");
    mdi.SetEmail("liujz@lreis.ac.cn");
    mdi.SetHelpfile("SUR_EXCESS.chm");
    mdi.SetID("SUR_EXCESS");
    mdi.SetName("SUR_EXCESS");
    mdi.SetVersion("0.1");
    mdi.SetWebsite("http://www.website.com");

    mdi.AddParameter("DT_HS", "second", "time step for storm simulation", "ParameterDB_WaterBalance", DT_Single);
    mdi.AddParameter("Conductivity", "mm/h", "Soil hydraulic conductivity", "ParameterDB_WaterBalance", DT_Raster2D);
    mdi.AddParameter("Moist_in", "%", "Initial soil moisture", "ParameterDB_WaterBalance", DT_Raster2D);
    mdi.AddParameter("Porosity", "%", "Soil porosity", "ParameterDB_WaterBalance", DT_Raster2D);
    mdi.AddParameter("FieldCap", "%", "Field capacity", "ParameterDB_WaterBalance", DT_Raster2D);
    mdi.AddParameter("RootDepth", "mm", "Root depth", "ParameterDB_WaterBalance", DT_Raster1D);

    mdi.AddInput("D_NEPR", "mm", "The net precipitation", "Module", DT_Raster1D);
    mdi.AddInput("D_DPST", "mm", "The depression storage", "Module", DT_Raster1D);
    //mdi.AddInput("D_SOMO","%","Soil Moisture","Module", DT_Raster);

    mdi.AddOutput("SOMO", "%", "soil moisture", DT_Raster2D);
    mdi.AddOutput("EXCP", "mm", "The excess precipitation", DT_Raster1D);
    mdi.AddOutput("INFIL", "mm", "Infiltration map of watershed", DT_Raster1D);
    mdi.AddOutput("INFILCAPSURPLUS", "mm", "surplus of infiltration capacity", DT_Raster1D);
    mdi.AddOutput("AccumuInfil", "mm", "accumulative infiltration", DT_Raster1D);

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}