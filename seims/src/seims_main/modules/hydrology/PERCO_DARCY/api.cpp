#include "api.h"

#include "Percolation.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new Percolation_DARCY();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Chunping Ou");
    mdi.SetClass(MCLS_PERCO[0], MCLS_PERCO[1]);
    mdi.SetDescription(M_PERCO_DARCY[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_PERCO_DARCY[0]);
    mdi.SetName(M_PERCO_DARCY[0]);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("PERCO_DARCY.chm");

    mdi.AddParameter(Tag_HillSlopeTimeStep[0], UNIT_SECOND, Tag_TimeStep[1], File_Input, DT_Single);
    mdi.AddParameter(Tag_CellSize[0], UNIT_NON_DIM, Tag_CellSize[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_CellWidth[0], UNIT_LEN_M, Tag_CellWidth[1], Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_CONDUCT[0], UNIT_WTRDLT_MMH, VAR_CONDUCT[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POROST[0], UNIT_STRG_M3M, VAR_POROST[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_POREIDX[0], UNIT_NON_DIM, VAR_POREIDX[1], Source_ParameterDB, DT_Raster2D);
    mdi.AddParameter(VAR_FIELDCAP[0], UNIT_STRG_M3M, VAR_FIELDCAP[1], Source_ParameterDB, DT_Raster2D);

    mdi.AddParameter(VAR_SOILDEPTH[0], UNIT_DEPTH_MM, VAR_SOILDEPTH[1], Source_ParameterDB, DT_Raster1D);

    mdi.AddInput(VAR_SOL_ST[0], UNIT_DEPTH_MM, VAR_SOL_ST[1], Source_Module, DT_Raster2D);
    // set the output variables
    mdi.AddOutput(VAR_PERCO[0], UNIT_DEPTH_MM, VAR_PERCO[1], DT_Raster1D);

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
