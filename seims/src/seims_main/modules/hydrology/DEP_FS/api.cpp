#include "api.h"

#include "DepressionFS.h"
#include "text.h"
#include "MetadataInfo.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance() {
    return new DepressionFS();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Junzhi Liu");
    mdi.SetClass(MCLS_DEP[0], MCLS_DEP[1]);
    mdi.SetDescription(M_DEP_FS[1]);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("");
    mdi.SetID(M_DEP_FS[0]);
    mdi.SetName(M_DEP_FS[0]);
    mdi.SetVersion("0.1");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(VAR_DEPREIN[0], UNIT_NON_DIM, VAR_DEPREIN[1], Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_DEPRESSION[0], UNIT_DEPTH_MM, VAR_DEPRESSION[1], Source_ParameterDB, DT_Raster1D);

#ifndef STORM_MODE
    mdi.AddInput(VAR_PET[0], UNIT_DEPTH_MM, VAR_PET[1], Source_Module, DT_Raster1D);    //PET
    mdi.AddInput(VAR_INLO[0], UNIT_DEPTH_MM, VAR_INLO[1], Source_Module, DT_Raster1D);
    mdi.AddOutput(VAR_DEET[0], UNIT_DEPTH_MM, VAR_DEET[1], DT_Raster1D);
#endif /* not STORM_MODE */
    //mdi.AddInput("D_INFIL","mm","Infiltration calculated in the infiltration module", "Module", DT_Raster);							//Infiltration
    mdi.AddOutput(VAR_DPST[0], UNIT_DEPTH_MM, VAR_DPST[1], DT_Raster1D);
    mdi.AddOutput(VAR_SURU[0], UNIT_DEPTH_MM, VAR_SURU[1], DT_Raster1D);
    mdi.AddOutput(VAR_STCAPSURPLUS[0], UNIT_DEPTH_MM, VAR_STCAPSURPLUS[1], DT_Raster1D);

    string res = mdi.GetXMLDocument();
    //return res;

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
