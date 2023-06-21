#include "api.h"

#include "SNO_WB.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new SNO_WB();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Chunping Ou");
    mdi.SetClass(MCLS_SNOW[0], MCLS_SNOW[1]);
    mdi.SetDescription(M_SNO_WB[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(M_SNO_WB[0]);
    mdi.SetName(M_SNO_WB[0]);
    mdi.SetVersion("0.5");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("SNO_WB.chm");
    mdi.SetAbstractionTypeToConceptual();

    mdi.AddParameter(Tag_CellSize[0], UNIT_NON_DIM, Tag_CellSize[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T0[0], UNIT_TEMP_DEG, VAR_T0[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_BLOW[0], UNIT_NON_DIM, VAR_K_BLOW[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T_SNOW[0], UNIT_TEMP_DEG, VAR_T_SNOW[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SWE0[0], UNIT_DEPTH_MM, VAR_SWE0[1], Source_ParameterDB, DT_Single);

    // In my view, this is irrelevant, so should be deprecated. By LJ.
    //mdi.AddParameter(Tag_SubbasinSelected,  UNIT_NON_DIM, Tag_SubbasinSelected[1]ected, File_Output, DT_Array1D); //this list is used to constrain the output size. Its name must be subbasinSelected.
    //mdi.AddParameter(VAR_SUBBSN[0],  UNIT_NON_DIM, VAR_SUBBSN[1],  Source_ParameterDB, DT_Raster1D); //this list is used to constrain the output size

    mdi.AddInput(VAR_NEPR[0], UNIT_DEPTH_MM, VAR_NEPR[1], Source_Module, DT_Raster1D);
    // TODO: SNRD currently have not incoming modules, therefore initialized as zero.By LJ
    //mdi.AddInput(VAR_SNRD[0], UNIT_DEPTH_MM, VAR_SNRD[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SNSB[0], UNIT_DEPTH_MM, VAR_SNSB[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_SNME[0], UNIT_DEPTH_MM, VAR_SNME[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMEAN[0], UNIT_TEMP_DEG, VAR_TMEAN[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_TMAX[0], UNIT_TEMP_DEG, VAR_TMAX[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_PCP[0], UNIT_DEPTH_MM, VAR_PCP[1], Source_Module, DT_Raster1D);
    mdi.AddInput(VAR_WS[0], UNIT_SPEED_MS, VAR_WS[1], Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_SNAC[0], UNIT_DEPTH_MM, VAR_SNAC[1], DT_Raster1D);
    mdi.AddOutput(VAR_SWE[0], UNIT_DEPTH_MM, VAR_SWE[1], DT_Single);

    // set the dependencies
    //mdi.AddDependency("T", "average temperature obtained from the interpolation module");
    //mdi.AddDependency("P", "average precipitation obtained from the interpolation module");
    //mdi.AddDependency("Pnet", "net precipitation obtained from interception model used to calculate P in equation ");
    //mdi.AddDependency("SA", "snow accumulation");
    //mdi.AddDependency("SE", "snow sublimation");
    //mdi.AddDependency("SR", "snow redistribution");
    //mdi.AddDependency("SM", "snow melt");

    // write out the XML file.

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
//mdi.AddOutput(VAR_SNWB[0], UNIT_DEPTH_MM, VAR_SNWB[1], DT_Array2D);
