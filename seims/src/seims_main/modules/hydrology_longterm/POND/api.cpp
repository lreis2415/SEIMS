#include "api.h"

#include "pond.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule *GetInstance()
{
    return new POND();
}

/// function to return the XML Metadata document string
extern "C" SEIMS_MODULE_API const char *MetadataInformation()
{
    MetadataInfo mdi;
    string res;

    mdi.SetAuthor("ShenFang");
    mdi.SetClass(MCLS_PADDY, MCLSDESC_PADDY);
    mdi.SetDescription(MDESC_POND);
    mdi.SetID(MID_POND);
    mdi.SetName(MID_POND);
    mdi.SetVersion("1.2");
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetWebsite(SEIMS_SITE);
    mdi.SetHelpfile("");
    /// set parameters from database
	// add reach information
	//mdi.AddParameter(VAR_REACH_PARAM, UNIT_NON_DIM, DESC_REACH_PARAM, Source_ParameterDB, DT_Reach);
	//mdi.AddParameter(VAR_POND_PARAM, UNIT_NON_DIM, DESC_POND_PARAM, Source_ParameterDB, DT_Pond);
	mdi.AddParameter(Tag_TimeStep, UNIT_DAY, DESC_TIMESTEP, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_KV_PADDY, UNIT_PER_DAY, DESC_KV_PADDY, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_KN_PADDY, UNIT_PER_DAY, DESC_KN_PADDY, Source_ParameterDB, DT_Single);
	mdi.AddParameter(VAR_LANDUSE, UNIT_NON_DIM, DESC_LANDUSE, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(VAR_CONDUCT, UNIT_WTRDLT_MMH, DESC_CONDUCT, Source_ParameterDB, DT_Raster2D);
	mdi.AddParameter(VAR_POND, UNIT_NON_DIM, DESC_POND, Source_ParameterDB, DT_Raster1D);
	mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
	mdi.AddParameter(Tag_FLOWIN_INDEX_D8, UNIT_NON_DIM, DESC_FLOWIN_INDEX_D8, Source_ParameterDB, DT_Array2D);
	mdi.AddParameter(Tag_FLOWOUT_INDEX_D8, UNIT_NON_DIM, DESC_FLOWOUT_INDEX_D8, Source_ParameterDB, DT_Array1D);
	//mdi.AddInput(VAR_IRRDEPTH, UNIT_DEPTH_MM, DESC_IRRDEPTH, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_EXCP, UNIT_DEPTH_MM, DESC_EXCP, Source_Module, DT_Raster1D);    //Excess precipitation
	mdi.AddInput(VAR_OLFLOW, UNIT_DEPTH_MM, DESC_OLFLOW, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_SOL_ST, UNIT_DEPTH_MM, DESC_SOL_ST, Source_Module_Optional, DT_Raster2D);
	mdi.AddInput(VAR_PET, UNIT_WTRDLT_MMD, DESC_PET, Source_Module, DT_Raster1D);

	mdi.AddInput(VAR_SEDYLD, UNIT_KG, DESC_SEDYLD, Source_Module, DT_Raster1D); /// m_sedYield
	mdi.AddInput(VAR_SANDYLD, UNIT_KG, DESC_SANDYLD, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_SILTYLD, UNIT_KG, DESC_SILTYLD, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_CLAYYLD, UNIT_KG, DESC_CLAYYLD, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_SAGYLD, UNIT_KG, DESC_SAGYLD, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_LAGYLD, UNIT_KG, DESC_LAGYLD, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_SUR_NO3, UNIT_CONT_KGHA, DESC_SUR_NO3, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_SUR_NH4, UNIT_CONT_KGHA, DESC_SUR_NH4, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_SUR_SOLP, UNIT_CONT_KGHA, DESC_SUR_SOLP, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_SUR_COD, UNIT_CONT_KGHA, DESC_SUR_COD, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_SEDORGN, UNIT_CONT_KGHA, DESC_SEDORGN, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_SEDORGP, UNIT_CONT_KGHA, DESC_SEDORGP, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_SEDMINPA, UNIT_CONT_KGHA, DESC_SEDMINPA, Source_Module, DT_Raster1D);
	mdi.AddInput(VAR_SEDMINPS, UNIT_CONT_KGHA, DESC_SEDMINPS, Source_Module, DT_Raster1D);
	//mdi.AddInput(VAR_CHST, UNIT_VOL_M3, DESC_CHST, Source_Module, DT_Array1D);
	/// set the output variables
	mdi.AddOutput(VAR_POND_VOL, UNIT_DEPTH_MM, DESC_POND_VOL, DT_Raster1D);
	mdi.AddOutput(VAR_POND_SA, UNIT_DEPTH_M2, DESC_POND_SA, DT_Raster1D);

    /// write out the XML file.
    res = mdi.GetXMLDocument();

    char *tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}