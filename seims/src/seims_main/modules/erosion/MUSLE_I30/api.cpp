#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "MUSLE_I30.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance()
{
	return new MUSLE_I30();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char* MetadataInformation()
{
	MetadataInfo mdi;

	// set the information properties
	mdi.SetAuthor("Zhiqiang Yu");
	mdi.SetClass("erosion", "Calculate the amount of sediment yield.");
	mdi.SetDescription("use MUSLE method to calcualte sediment yield of each cell");
	mdi.SetEmail("");
	mdi.SetID("MUSLE_I30");
	mdi.SetName("MUSLE_I30");
	mdi.SetVersion("0.5");
	mdi.SetWebsite("");
	mdi.SetHelpfile("MUSLE_I30.chm");

	mdi.AddParameter("cellwidth","m","the width (length) of cell","mask.asc",DT_Single);
	mdi.AddParameter("USLE_C","","the cover management factor","ParameterDB_Sediment",DT_Raster1D);
	mdi.AddParameter("USLE_P","","the erosion control practice factor ","ParameterDB_Sediment",DT_Raster1D);
	mdi.AddParameter("USLE_K","","the soil erodibility factor","ParameterDB_Sediment",DT_Raster1D);
	mdi.AddParameter("flow_acc","","the number of flow accumulation cells of each cell","ParameterDB_Sediment",DT_Raster1D);
	mdi.AddParameter("slope","%","slope of the cell","ParameterDB_WaterBalance",DT_Raster1D);

	mdi.AddParameter("T0_s","hr","time of concentration","ParameterDB_Sediment",DT_Raster1D);
	mdi.AddParameter("adj_pkr","","peak rate adjustment factor","ParameterDB_Sediment",DT_Single);
	mdi.AddParameter("rain_yrs","","number of yeares of data used to obtain values for RAINHHMX","ParameterDB_Sediment",DT_Single);

	mdi.AddParameter("p_stat","","static information of precipitation","HydroclimateDB",DT_Array2D);

	//input from other module	
	mdi.AddInput("D_SURU","mm","surface runoff","Module",DT_Raster1D);		//from depression module
	mdi.AddInput("D_SNAC","mm","snow accumulation","Module",DT_Raster1D);	//from snow water balance module
	mdi.AddInput("D_P","mm","precipitation","Module",DT_Raster1D);			//from interpolation module
	mdi.AddInput("D_SNME","mm","snow melt","Module",DT_Raster1D);			//from snow melt module

	// set the output variables
	mdi.AddOutput("SOER","metric tons", "distribution of soil erosion", DT_Raster1D);

	// write out the XML file.

	string res = mdi.GetXMLDocument();

	char* tmp = new char[res.size()+1];
	strprintf(tmp, res.size()+1, "%s", res.c_str());
	return tmp;
}