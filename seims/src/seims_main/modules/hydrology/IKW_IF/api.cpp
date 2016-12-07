#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "InterFlow_IKW.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new InterFlow_IKW();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    string res = "";
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_INTERFLOW, MCLSDESC_INTERFLOW);
    mdi.SetDescription(MDESC_IKW_IF);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("IKW_IF.chm");
    mdi.SetID(MID_IKW_IF);
    mdi.SetName(MID_IKW_IF);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_HillSlopeTimeStep, UNIT_SECOND, DESC_DT_HS, File_Input, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);

    mdi.AddParameter(VAR_SLOPE, UNIT_PERCENT, DESC_SLOPE, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_CHWIDTH, UNIT_LEN_M, DESC_CHWIDTH, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_STREAM_LINK, UNIT_NON_DIM, DESC_STREAM_LINK, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(Tag_FLOWIN_INDEX_D8, UNIT_NON_DIM, DESC_FLOWIN_INDEX_D8, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(Tag_ROUTING_LAYERS, UNIT_NON_DIM, DESC_ROUTING_LAYERS, Source_ParameterDB, DT_Array2D);

    mdi.AddParameter(VAR_SOILDEPTH, UNIT_LEN_M, DESC_SOILDEPTH, Source_ParameterDB, DT_Raster1D);
    mdi.AddParameter(VAR_SOILDEPTH, UNIT_LEN_M, DESC_SOILDEPTH, Source_ParameterDB,
                     DT_Raster1D); /// Added by LJ, Not tested yet

    mdi.AddParameter(VAR_CONDUCT, UNIT_WTRDLT_MMH, DESC_CONDUCT, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_POROST, UNIT_STRG_M3M, DESC_POROST, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_POREID, UNIT_NON_DIM, DESC_POREID, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_FIELDCAP, UNIT_STRG_M3M, DESC_FIELDCAP, Source_ParameterDB, DT_Array2D);

    mdi.AddParameter(VAR_KI, UNIT_NON_DIM, DESC_KI, Source_ParameterDB, DT_Single);

    mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module, DT_Raster2D);
    mdi.AddInput(VAR_SURU, UNIT_DEPTH_MM, DESC_SURU, Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_QSOIL, UNIT_FLOW_CMS, DESC_QSOIL, DT_Raster1D);
    mdi.AddOutput(VAR_RETURNFLOW, UNIT_DEPTH_MM, DESC_RETURNFLOW, DT_Raster1D);

    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}