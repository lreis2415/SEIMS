#include "ParamInfo.h"

#include <algorithm>

#include "utils_math.h"
#include "utils_string.h"
#include "text.h"

using namespace utils_math;
using namespace utils_string;

ParamInfo::ParamInfo() : Name(""), Units(""), Description(""), ModuleID(""), Dimension(DT_Unknown),
                         Transfer(TF_None), Source(""), Value(0.f), Impact(0.f), Change(""),
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

float ParamInfo::GetAdjustedValue(const float pre_value /* = NODATA_VALUE */) {
    float res = pre_value;
    if (FloatEqual(pre_value, NODATA_VALUE)) {
        res = Value;
    }
    if (FloatEqual(res, NODATA_VALUE)) {
        /// Do not change NoData value
        return res;
    }

    if (StringMatch(Change, PARAM_CHANGE_RC) && !FloatEqual(Impact, 1.f)) {
        res *= Impact;
    } else if (StringMatch(Change, PARAM_CHANGE_AC) && !FloatEqual(Impact, 0.f)) {
        res += Impact;
    } else if (StringMatch(Change, PARAM_CHANGE_VC) && !FloatEqual(Impact, NODATA_VALUE)) {
        res = Impact;
    } else if (StringMatch(Change, PARAM_CHANGE_NC)) {
        //don't change
        return res;
    }
    if (!FloatEqual(Maximum, NODATA_VALUE) && res > Maximum) res = Maximum;
    if (!FloatEqual(Minimun, NODATA_VALUE) && res < Minimun) res = Minimun;
    return res;
}

void ParamInfo::Adjust1DArray(const int n, float* data) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        if (!FloatEqual(data[i], NODATA_VALUE)) {
            /// Do not change NoData value
            data[i] = GetAdjustedValue(data[i]);
        }
    }
}

void ParamInfo::Adjust1DRaster(const int n, float* data) {
    Adjust1DArray(n, data);
}

int ParamInfo::Adjust1DRaster(const int n, float* data, const float* units,
                              const vector<int>& selunits,
                              const float* lu, const vector<int>& sellu) {
    int count = 0;
#pragma omp parallel for reduction(+:count)
    for (int i = 0; i < n; i++) {
        if (FloatEqual(data[i], NODATA_VALUE)) {
            /// Do not change NoData value
            continue;
        }
        int curunit = CVT_INT(units[i]);
        int curlu = CVT_INT(lu[i]);
        if (find(selunits.begin(), selunits.end(), curunit) == selunits.end()) {
            continue;
        }
        if (find(sellu.begin(), sellu.end(), curlu) == sellu.end()) {
            continue;
        }
        data[i] = GetAdjustedValue(data[i]);
        count += 1;
    }
    return count;
}

void ParamInfo::Adjust2DArray(const int n, float** data) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        int cur_cols = CVT_INT(data[i][0]);
        Adjust1DArray(cur_cols, data[i] + 1);
    }
}

void ParamInfo::Adjust2DRaster(const int n, const int lyrs, float** data) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        Adjust1DArray(lyrs, data[i]);
    }
}

int ParamInfo::Adjust2DRaster(const int n, const int lyrs, float** data, float* units,
                               const vector<int>& selunits, float* lu, const vector<int>& sellu) {
    int count = 0;
#pragma omp parallel for reduction(+:count)
    for (int i = 0; i < n; i++) {
        int curunit = CVT_INT(units[i]);
        int curlu = CVT_INT(lu[i]);
        if (find(selunits.begin(), selunits.end(), curunit) == selunits.end()) {
            continue;
        }
        if (find(sellu.begin(), sellu.end(), curlu) == sellu.end()) {
            continue;
        }
        for (int j = 0; j < lyrs; j++) {
            data[i][j] = GetAdjustedValue(data[i][j]);
        }
        count += 1;
    }
    return count;
}
