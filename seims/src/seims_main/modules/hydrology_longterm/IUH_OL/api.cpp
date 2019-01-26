#include "api.h"

#include "IUH_OL.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new IUH_OL();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;

    // set the information properties
    mdi.SetAuthor("Wu Hui, Zhiqiang Yu, Liangjun Zhu");
    mdi.SetClass(MCLS_OL_ROUTING, MCLSDESC_OL_ROUTING);
    mdi.SetDescription(MDESC_IUH_OL);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("");
    mdi.SetID(MID_IUH_OL);
    mdi.SetName(MID_IUH_OL);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(Tag_TimeStep, UNIT_HOUR, DESC_TIMESTEP, File_Input, DT_Single);
    mdi.AddParameter(Tag_CellWidth, UNIT_LEN_M, DESC_CellWidth, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_SUBBSNID_NUM, UNIT_NON_DIM, DESC_SUBBSNID_NUM, Source_ParameterDB, DT_Single);
    mdi.AddParameter(Tag_SubbasinId, UNIT_NON_DIM, Tag_SubbasinId, Source_ParameterDB, DT_Single);
    mdi.AddParameter(VAR_OL_IUH, UNIT_NON_DIM, DESC_OL_IUH, Source_ParameterDB, DT_Array2D);
    mdi.AddParameter(VAR_SUBBSN, UNIT_NON_DIM, DESC_SUBBSN, Source_ParameterDB, DT_Raster1D);
    // field version add
    /// add cell area
    mdi.AddParameter(VAR_FIELDAREA, UNIT_AREA_M2, DESC_UNITAREA, Source_ParameterDB_Optional, DT_Raster1D);
    // add landuse to settle pond
    mdi.AddParameter(VAR_LANDUSE, UNIT_NON_DIM, DESC_LANDUSE, Source_ParameterDB_Optional, DT_Raster1D);
    // add field flow to pond or river parameter
    mdi.AddParameter(VAR_FLOWPOND, UNIT_NON_DIM, DESC_FLOWPOND, Source_ParameterDB_Optional, DT_Raster1D);
    /// add pond topological sort
    mdi.AddParameter(VAR_PONDSORT, UNIT_NON_DIM, DESC_POND_SORT, Source_ParameterDB_Optional, DT_Raster1D);

    mdi.AddInput(VAR_SURU, UNIT_DEPTH_MM, DESC_SURU, Source_Module, DT_Raster1D);

    mdi.AddOutput(VAR_OLFLOW, UNIT_DEPTH_MM, DESC_OLFLOW, DT_Raster1D);
    mdi.AddOutput(VAR_SBOF, UNIT_FLOW_CMS, DESC_SBOF, DT_Array1D, TF_SingleValue);

    // write out the XML file.
    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
