#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "Percolation.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new Percolation_DARCY();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Chunping Ou");
    mdi.SetClass(MCLS_PERCO, MCLSDESC_PERCO);
    mdi.SetDescription(MDESC_PERCO_DARCY);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_PERCO_DARCY);
    mdi.SetName(MID_PERCO_DARCY);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("PERCO_DARCY.chm");

    mdi.AddParameter(Tag_HillSlopeTimeStep, UNIT_SECOND, DESC_TIMESTEP, File_Input, DT_Single);
    mdi.AddParameter(Tag_CellSize, UNIT_NON_DIM, DESC_CellSize, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_CONDUCT, UNIT_WTRDLT_MMH, DESC_CONDUCT, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POROST, UNIT_STRG_M3M, DESC_POROST, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POREID, UNIT_NON_DIM, DESC_POREID, Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_FIELDCAP, UNIT_STRG_M3M, DESC_FIELDCAP, Source_ParameterDB, DT_Raster2D);

    mdi.AddParameter(VAR_SOILDEPTH, UNIT_DEPTH_MM, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster1D);

    mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module, DT_Raster2D);
    // set the output variables
    mdi.AddOutput(VAR_PERCO, UNIT_DEPTH_MM, DESC_PERCO, DT_Raster1D);

    // write out the XML file.
    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
//mdi.AddParameter("t_soil","oC","threshold soil freezing temperature","ParameterDB_WaterBalance", DT_Single);
//mdi.AddInput("D_ES","mm","ES","Module",DT_Raster);											//from actual evapotranspiration module, the output id may not be correct.

//mdi.AddInput("D_SOTE","oC", "Soil Temperature","Module", DT_Raster);						//soil temperature
//mdi.AddParameter("Residual","m3/m3","residual moisture content","ParameterDB_WaterBalance",DT_Raster);