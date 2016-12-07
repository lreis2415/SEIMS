#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "MUSLE_AS.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new MUSLE_AS();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Zhiqiang Yu");
    mdi.SetClass(MCLS_OL_EROSION, MCLSDESC_OL_EROSION);
    mdi.SetDescription(MDESC_MUSLE_AS);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_MUSLE_AS);
    mdi.SetName(MID_MUSLE_AS);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("MUSLE_AS.chm");

    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_DEPRATIO, UNIT_NON_DIM, DESC_DEPRATIO, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_USLE_K, UNIT_NON_DIM, DESC_USLE_K, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_USLE_C, UNIT_NON_DIM, DESC_USLE_C, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_USLE_P, UNIT_NON_DIM, DESC_USLE_P, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_ACC, UNIT_NON_DIM, DESC_ACC, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SLOPE, UNIT_PERCENT, DESC_SLOPE, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_SUBBASIN_PARAM, UNIT_NON_DIM, DESC_SUBBASIN_PARAM, Source_ParameterDB, DT_Subbasin);
    mdi.AddParameter(VAR_STREAM_LINK, UNIT_NON_DIM, DESC_STREAM_LINK, Source_ParameterDB, DT_Raster1D);

	mdi.AddParameter(VAR_DETACH_SAND, UNIT_NON_DIM, DESC_DETACH_SAND, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_DETACH_SILT, UNIT_NON_DIM, DESC_DETACH_SILT, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_DETACH_CLAY, UNIT_NON_DIM, DESC_DETACH_CLAY, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_DETACH_SAG, UNIT_NON_DIM, DESC_DETACH_SAG, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_DETACH_LAG, UNIT_NON_DIM, DESC_DETACH_LAG, Source_ParameterDB, DT_Raster1D);

    //input from other module
	// mdi.AddInput(VAR_SURU, UNIT_DEPTH_MM, DESC_SURU, Source_Module, DT_Raster1D); // use SURU or OLFLOW???
	mdi.AddInput(VAR_OLFLOW, UNIT_DEPTH_MM, DESC_OLFLOW, Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SNAC, UNIT_DEPTH_MM, DESC_SNAC, Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_SOER, UNIT_KG, DESC_SOER, DT_Raster1D);
    mdi.AddOutput(VAR_USLE_LS, UNIT_NON_DIM, DESC_USLE_LS, DT_Raster1D);
	mdi.AddOutput(VAR_SANDYLD, UNIT_KG, DESC_SANDYLD, DT_Raster1D);
	mdi.AddOutput(VAR_SILTYLD, UNIT_KG, DESC_SILTYLD, DT_Raster1D);
	mdi.AddOutput(VAR_CLAYYLD, UNIT_KG, DESC_CLAYYLD, DT_Raster1D);
	mdi.AddOutput(VAR_SAGYLD, UNIT_KG, DESC_SAGYLD, DT_Raster1D);
	mdi.AddOutput(VAR_LAGYLD, UNIT_KG, DESC_LAGYLD, DT_Raster1D);

    // write out the XML file
    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
    //mdi.AddParameter(Tag_CellSize, UNIT_NON_DIM, DESC_CellSize, Source_ParameterDB, DT_Single);
    //mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);