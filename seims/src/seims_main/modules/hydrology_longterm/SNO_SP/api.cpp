#include <stdio.h>
#include <string>
#include "api.h"
#include "util.h"
#include "SNO_SP.h"
#include <iostream>
#include "SimulationModule.h"
#include "MetadataInfo.h"
#include "MetadataInfoConst.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new SNO_SP();
}

// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Zhiqiang Yu, LiangJun Zhu");
    mdi.SetClass(MCLS_SNOW, MCLSDESC_SNOW);
    mdi.SetDescription(MDESC_SNO_SP);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetID(MID_SNO_SP);
    mdi.SetName(MID_SNO_SP);
    mdi.SetVersion("1.1");
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("SNO_SP.chm");

    mdi.AddParameter(VAR_T0, UNIT_TEMP_DEG, DESC_T0, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_K_BLOW, UNIT_NON_DIM, DESC_K_BLOW, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_T_SNOW, UNIT_TEMP_DEG, DESC_T_SNOW, Source_ParameterDB, DT_Single);
	// I don't think VAR_SWE0 is useful. By LJ
    // mdi.AddParameter(VAR_SWE0, UNIT_DEPTH_MM, DESC_SWE0, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_LAG_SNOW, UNIT_NON_DIM, DESC_LAG_SNOW, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_C_SNOW6, UNIT_MELT_FACTOR, DESC_C_SNOW6, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_C_SNOW12, UNIT_MELT_FACTOR, DESC_C_SNOW12, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_SNOCOVMX, UNIT_DEPTH_MM, DESC_SNOCOVMX, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_SNO50COV, UNIT_NON_DIM, DESC_SNO50COV, Source_ParameterDB, DT_Single);
	// Net precipitation is updated after snow accumulation.
    mdi.AddInput(VAR_NEPR, UNIT_DEPTH_MM, DESC_NEPR, Source_Module, DT_Raster1D); // from interception module
    //TODO: SNAC is used as unknown variable in the execute() funtion, so why it here as Input, otherwise in Output? By LJ
    //mdi.AddInput(VAR_SNAC, UNIT_DEPTH_MM, DESC_SNAC, Source_Module,DT_Raster1D);	 // from snow water balance module
    // I think VAR_SWE is useless either. By LJ
	// mdi.AddInput(VAR_SWE, UNIT_DEPTH_MM, DESC_SWE, Source_Module, DT_Single);  // from snow water balance module
    // TODO: SNRD currently have not been implemented, therefore initialized as zero. By LJ
    // mdi.AddInput(VAR_SNRD, UNIT_DEPTH_MM, DESC_SNRD, Source_Module, DT_Raster1D); // from snow redistribution module
    // Snow sublimation will be considered in actual evpotranspiration module (AET_PTH), so no need to set as Input. By LJ
    // mdi.AddInput(VAR_SNSB, UNIT_DEPTH_MM, DESC_SNSB, Source_Module, DT_Raster1D); //from snow sublimation module
    mdi.AddInput(VAR_TMAX, UNIT_TEMP_DEG, DESC_TMAX, Source_Module, DT_Raster1D); // from interpolation module
    mdi.AddInput(VAR_TMEAN, UNIT_TEMP_DEG, DESC_TMEAN, Source_Module, DT_Raster1D);

    // set the output variables
    mdi.AddOutput(VAR_SNME, UNIT_DEPTH_MM, DESC_SNME, DT_Raster1D);
    mdi.AddOutput(VAR_SNAC, UNIT_DEPTH_MM, DESC_SNAC, DT_Raster1D);
	
    // write out the XML file.

    string res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}