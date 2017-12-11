#include "ParamInfo.h"

using namespace std;

ParamInfo::ParamInfo() : Change(""),
                         Description(""),
                         Dimension(DT_Unknown),
                         Impact(0.f),
                         Maximum(0.f),
                         Minimun(0.f),
                         ModuleID(""),
                         Name(""),
                         Source(""),
                         Units(""),
                         Value(0.f),
                         DependPara(nullptr),
                         IsConstant(false),
                         ClimateType(""),
                         IsOutput(false),
                         OutputToOthers(false),
                         initialized(false) {
}

float ParamInfo::GetAdjustedValue(float pre_value /* = NODATA_VALUE */) {
    if (FloatEqual(pre_value, NODATA_VALUE)) {
        pre_value = Value;
    }
    float res = pre_value;
    if (FloatEqual(res, NODATA_VALUE)) {  /// Do not change NoData value
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
    }
    return res;
}

void ParamInfo::Adjust1DArray(int n, float *data) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        if (!FloatEqual(data[i], NODATA_VALUE)) {  /// Do not change NoData value
            data[i] = GetAdjustedValue(data[i]);
        }
    }
}

void ParamInfo::Adjust1DRaster(int n, float *data) {
    Adjust1DArray(n, data);
}

void ParamInfo::Adjust1DRaster(int n, float *data, const float *units, vector<int> selunits,
                               const float *lu, vector<int> sellu) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        if (FloatEqual(data[i], NODATA_VALUE)) {  /// Do not change NoData value
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

void ParamInfo::Adjust2DArray(int n, float **data) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        int curCols = (int) data[i][0];
        Adjust1DArray(curCols, data[i] + 1);
    }
}

void ParamInfo::Adjust2DRaster(int n, int lyrs, float **data) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        Adjust1DArray(lyrs, data[i]);
    }
}

void ParamInfo::Adjust2DRaster(int n, int lyr, float **data, float *units,
                               vector<int> selunits, float *lu, vector<int> sellu) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        Adjust1DRaster(lyr, data[i], units, selunits, lu, sellu);
    }
}
