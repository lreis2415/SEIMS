#include "text.h"
#include "ParamInfo.h"

using namespace std;

ParamInfo::ParamInfo() : Name(""), Units(""), Description(""), ModuleID(""), Dimension(DT_Unknown),
                         Transfer(TF_Whole), Source(""), Value(0.f), Impact(0.f), Change(""),
                         Maximum(0.f), Minimun(0.f), DependPara(nullptr), ClimateType(""),
                         IsConstant(false), IsOutput(false), OutputToOthers(false),
                         BasicName(""), initialized(false) {
}

ParamInfo::ParamInfo(const ParamInfo& another) {
    Name = another.Name;
    Units = another.Units;
    Description = another.Description;
    Dimension = another.Dimension;
    Transfer = another.Transfer;
    Value = another.Value;
    Change = another.Change;
    Impact = another.Impact;
    Maximum = another.Maximum;
    Minimun = another.Minimun;
    ModuleID = another.ModuleID;
    Source = another.Source;
    DependPara = another.DependPara;
    ClimateType = another.ClimateType;
    IsConstant = another.IsConstant;
    IsOutput = another.IsOutput;
    OutputToOthers = another.OutputToOthers;
    BasicName = another.BasicName;
    initialized = another.initialized;
}

ParamInfo::~ParamInfo() {
    if (DependPara != nullptr) {
        DependPara = nullptr;
    }
}

float ParamInfo::GetAdjustedValue(float pre_value /* = NODATA_VALUE */) {
    if (FloatEqual(pre_value, NODATA_VALUE)) {
        pre_value = Value;
    }
    float res = pre_value;
    if (FloatEqual(res, NODATA_VALUE)) {
        /// Do not change NoData value
        return res;
    }

    if (StringMatch(Change, PARAM_CHANGE_RC) && !FloatEqual(Impact, 1.f)) {
        res = pre_value * Impact;
    } else if (StringMatch(Change, PARAM_CHANGE_AC) && !FloatEqual(Impact, 0.f)) {
        res = pre_value + Impact;
    } else if (StringMatch(Change, PARAM_CHANGE_VC) && !FloatEqual(Impact, NODATA_VALUE)) {
        res = Impact;
    } else if (StringMatch(Change, PARAM_CHANGE_NC)) {
        //don't change
        return res;
    }
    if (res > Maximum) res = Maximum;
    if (res < Minimun) res = Minimun;
    return res;
}

void ParamInfo::Adjust1DArray(int n, float* data) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        if (!FloatEqual(data[i], NODATA_VALUE)) {
            /// Do not change NoData value
            data[i] = GetAdjustedValue(data[i]);
        }
    }
}

void ParamInfo::Adjust1DRaster(int n, float* data) {
    Adjust1DArray(n, data);
}

void ParamInfo::Adjust1DRaster(int n, float* data, const float* units, vector<int> selunits,
                               const float* lu, vector<int> sellu) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        if (FloatEqual(data[i], NODATA_VALUE)) {
            /// Do not change NoData value
            continue;
        }
        int curunit = int(units[i]);
        int curlu = int(lu[i]);
        if (find(selunits.begin(), selunits.end(), curunit) == selunits.end()) {
            continue;
        }
        if (find(sellu.begin(), sellu.end(), curlu) == sellu.end()) {
            continue;
        }
        data[i] = GetAdjustedValue(data[i]);
    }
}

void ParamInfo::Adjust2DArray(int n, float** data) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        int curCols = int(data[i][0]);
        Adjust1DArray(curCols, data[i] + 1);
    }
}

void ParamInfo::Adjust2DRaster(int n, int lyrs, float** data) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        Adjust1DArray(lyrs, data[i]);
    }
}

void ParamInfo::Adjust2DRaster(int n, int lyr, float** data, float* units,
                               vector<int> selunits, float* lu, vector<int> sellu) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        Adjust1DRaster(lyr, data[i], units, selunits, lu, sellu);
    }
}
