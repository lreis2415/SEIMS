#include "api.h"

#include "DepressionLinsley.h"
#include "MetadataInfo.h"
#include "text.h"

extern "C" SEIMS_MODULE_API SimulationModule* GetInstance() {
    return new DepressionFSDaily();
}

extern "C" SEIMS_MODULE_API const char* MetadataInformation() {
    MetadataInfo mdi;
    // set the information properties
    mdi.SetAuthor("Junzhi Liu, Liangjun Zhu");
    mdi.SetClass(MCLS_DEP, MCLSDESC_DEP);
    mdi.SetDescription(MDESC_DEP_LINSLEY);
    mdi.SetEmail(SEIMS_EMAIL);
    mdi.SetHelpfile("DEP_LINSLEY.chm");
    mdi.SetID(MID_DEP_LINSLEY);
    mdi.SetName(MID_DEP_LINSLEY);
    mdi.SetVersion("1.2");
    mdi.SetWebsite(SEIMS_SITE);

    mdi.AddParameter(VAR_DEPREIN, UNIT_NON_DIM, DESC_DEPREIN, Source_ParameterDB, DT_Single);// 洼地初始蓄水系数
    mdi.AddParameter(VAR_DEPRESSION, UNIT_DEPTH_MM, DESC_DEPRESSION, Source_ParameterDB, DT_Raster1D);// 洼地深度 mm

    mdi.AddInput(VAR_INET, UNIT_DEPTH_MM, DESC_INET, Source_Module, DT_Raster1D); //Evaporation from intercepted storage
    mdi.AddInput(VAR_PET, UNIT_DEPTH_MM, DESC_PET, Source_Module, DT_Raster1D);   //PET 日潜在蒸散量
    mdi.AddInput(VAR_EXCP, UNIT_DEPTH_MM, DESC_EXCP, Source_Module, DT_Raster1D); //Excess precipitation 过量降水，可形成洼地蓄水或地表径流
    mdi.AddInput(VAR_IMPOUND_TRIG, UNIT_NON_DIM, DESC_IMPOUND_TRIG, Source_Module_Optional, DT_Raster1D);
    mdi.AddInput(VAR_POT_VOL, UNIT_DEPTH_MM, DESC_POT_VOL, Source_Module_Optional, DT_Raster1D);//洼地中的蓄水深度 mm

    mdi.AddOutput(VAR_DPST, UNIT_DEPTH_MM, DESC_DPST, DT_Raster1D);	// 洼地蓄水深度 mm
    mdi.AddOutput(VAR_DEET, UNIT_DEPTH_MM, DESC_DEET, DT_Raster1D); // 洼地蓄水的蒸发深度 mm
    mdi.AddOutput(VAR_SURU, UNIT_DEPTH_MM, DESC_SURU, DT_Raster1D);// 地表径流深度 mm

    // set the dependencies
    mdi.AddDependency(MCLS_CLIMATE, MCLSDESC_CLIMATE);
    mdi.AddDependency(MCLS_INTERC, MCLSDESC_INTERC);
    mdi.AddDependency(MCLS_SUR_RUNOFF, MCLSDESC_SUR_RUNOFF);

    string res = mdi.GetXMLDocument();

    char* tmp = new char[res.size() + 1];
    strprintf(tmp, res.size() + 1, "%s", res.c_str());
    return tmp;
}
