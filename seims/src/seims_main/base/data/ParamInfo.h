/*!
 * \file ParamInfo.h
 * \brief Class to store parameter item information
 *
 * Changelog:
 *   - 1. 2018-05-18 - lj - Code review and reformat.
 *   - 2. 2022-08-18 - lj - Change float to FLTPT, use template to support int and float.
 *
 * \author Junzhi Liu, Liangjun Zhu, Shen Shen
 * \version 2.0
 */
#ifndef SEIMS_PARAMETER_INFO_H
#define SEIMS_PARAMETER_INFO_H
#include <vector>
#include <algorithm>

#include "utils_math.h"
#include "utils_string.h"

#include "MetadataInfoConst.h"
#include "basic.h"
#include "text.h"

using namespace ccgl;
using namespace utils_math;
using namespace utils_string;
using std::vector;
using std::map;

/*!
* \ingroup data
 * \class ParamInfo
 *
 * \brief Class to store and manage parameter information from the parameter database
 */
template <typename T>
class ParamInfo {
public:
    //! Construct an empty instance.
    ParamInfo();

    //! Construct for initial parameters from DB
    ParamInfo(string& name, string& desc, string& unit, string& mid,
              T value, string& change, vector<T> impact, vector<vector<int>> impact_subbasins, T maximum, T minimum, bool isint);

    //! Construct for module Parameter
    ParamInfo(string& name, string& basicname, string& desc, string& unit, string& source, string& mid,
              dimensionTypes dim, string& climtype, T value = 0);

    //! Construct for module Input, Output, and InOutput
    ParamInfo(string& name, string& basicname, string& desc, string& unit, string& source, string& mid,
              dimensionTypes dim, transferTypes tftype, string& climtype,
              bool isconst, bool isoutput);

    //! Copy constructor
    ParamInfo(const ParamInfo& another);

    //! Destructor
    ~ParamInfo();

    int SubbasinIndexInImpactSubbasins(int subbasinId);
    T GetImpactBySubbasinId(int subbasinId);
    /*!
     * \brief Return the adjusted value for this parameter
     * \param[in] pre_value Default is NODATA_VALUE which will be treated
     *                      as ParamInfo.Value, otherwise adjust the given value.
     * \return adjusted float value
     */
    //T GetAdjustedValue(T pre_value = NODATA_VALUE);
    T GetAdjustedValue(T pre_value = NODATA_VALUE, int currentSubbasinId=0);

    //! Adjust value with indexed impact
    T GetAdjustedValueWithImpactIndexes(T pre_value, int curImpactIndex, vector<T>* impactSeries);
    T GetAdjustedValueWithSubbasin(T pre_value, int subbasinId, vector<T>* impacts);

    //! Adjust 1D array
    // void Adjust1DArray(int n, T* data);
    void Adjust1DArray(int n, T* data, int currentSubbasinId);

    //! Adjust 1D Raster, \sa Adjust1DArray()
    // void Adjust1DRaster(int n, T* data);
    void Adjust1DRaster(int n, T* data, int currentSubbasinId);

    //! Adjust 1D Raster on selected area
    int Adjust1DRaster(int n, T* data, const int* units, const vector<int>& selunits,
                       const int* lu, const vector<int>& sellu);

    //! Adjust 1D Raster on selected area, using impact index version
    int Adjust1DRasterWithImpactIndexes(int n, T* data, const int* units,
                                        const vector<int>& selunits, const map<int, int>& impactIndexes,
                                        const int* lu, const vector<int>& sellu);

    //! Adjust 2D array
    // void Adjust2DArray(int n, T** data);
    void Adjust2DArray(int n, T** data, int currentSubbasinId);

    //! Adjust 2D Raster
    // void Adjust2DRaster(int n, int lyrs, T** data);
    void Adjust2DRaster(int n, int lyrs, T** data, int currentSubbasinId);

    //! Adjust 1D Raster on selected area
    int Adjust2DRaster(int n, int lyrs, T** data, const int* units, const vector<int>& selunits,
                       const int* lu, const vector<int>& sellu);

    //! Adjust 2D Raster on selected area, using impact index version
    int Adjust2DRasterWithImpactIndexes(int n, int lyrs, T** data, const int* units,
                                        const vector<int>& selunits, const map<int, int>& impactIndexes,
                                        const int* lu, const vector<int>& sellu);

    string GetImpactsString() {
        string res = "";
        for (int i = 0; i < Impacts.size(); i++) {
            // res += CVT_STR(Impacts[i]);
            // “static_cast”: 无法从“_Ty”转换为“std::string”	bmps
            res += std::to_string(Impacts[i]);

            if (i < Impacts.size() - 1) {
                res += "|";
            }
        }
        return res;
    }

    //! Name
    string Name;
    //! Basic name
    string BasicName;
    //! Description
    string Description;
    //! Units
    string Units;
    //! Source, to identify which the parameters can be derived
    string Source;
    //! Used by Module Ids
    string ModuleID;

    //! Data dimension type
    dimensionTypes Dimension;
    //! Data transfer type
    transferTypes Transfer;
    //! Value
    T Value;
    //! Change type
    string Change;
    //! Impact values
    vector<T> Impacts;
    vector<vector<int>> ImpactSubbasins;
    //! Absolute maximum value
    T Maximum;
    //! Absolute minimum value
    T Minimum;
    //! is integer?
    bool IsInteger;
    //! Dependence parameters
    ParamInfo* DependPara;
    //! Climate type
    string ClimateType;
    //! Is constant or not
    bool IsConstant;
    //! Is output or not
    bool IsOutput;
    //! Is output to other modules or not
    bool OutputToOthers;
    //! whether is initialized
    bool initialized;
    //! If the BMP effectiveness is variable, set the values of impacts.
    //! This is reserved for BMP. --wyj 2024.1.24
    vector<T> ImpactSeries;
};


/*******************************************************/
/************* Implementation Code Begin ***************/
/*******************************************************/

template <typename T>
ParamInfo<T>::ParamInfo() :
    Name(""), BasicName(""), Description(""), Units(""), Source(""), ModuleID(""),
    Dimension(DT_Unknown), Transfer(TF_None),
    Value(0), Change(""), Maximum(0), Minimum(0), IsInteger(false),
    DependPara(nullptr), ClimateType(""),
    IsConstant(false), IsOutput(false), OutputToOthers(false),
    initialized(false), ImpactSeries() {
}

template <typename T>
ParamInfo<T>::ParamInfo(string& name, string& desc, string& unit, string& mid, T value, string& change,
                        vector<T> impact, vector<vector<int>> impact_subbasins, T maximum, T minimum, bool isint):
    Name(name), BasicName(""), Description(desc), Units(unit), Source(""), ModuleID(mid),
    Dimension(DT_Unknown), Transfer(TF_None),
    Value(value), Change(change), Impacts(impact), ImpactSubbasins(impact_subbasins), Maximum(maximum), Minimum(minimum), IsInteger(isint),
    DependPara(nullptr), ClimateType(""),
    IsConstant(false), IsOutput(false), OutputToOthers(false),
    initialized(false), ImpactSeries() {
}

template <typename T>
ParamInfo<T>::ParamInfo(string& name, string& basicname, string& desc, string& unit, string& source, string& mid,
                        dimensionTypes dim, string& climtype, T value /* = 0 */):
    Name(name), BasicName(basicname), Description(desc), Units(unit), Source(source), ModuleID(mid),
    Dimension(dim), Transfer(TF_None),
    Value(value), Change(""), Maximum(0), Minimum(0), IsInteger(false),
    DependPara(nullptr), ClimateType(climtype),
    IsConstant(false), IsOutput(false), OutputToOthers(false),
    initialized(false), ImpactSeries() {
}

template <typename T>
ParamInfo<T>::ParamInfo(string& name, string& basicname, string& desc, string& unit, string& source, string& mid,
                        dimensionTypes dim, transferTypes tftype, string& climtype,
                        bool isconst, bool isoutput) :
    Name(name), BasicName(basicname), Description(desc), Units(unit), Source(source), ModuleID(mid),
    Dimension(dim), Transfer(tftype),
    Value(0), Change(""), Maximum(0), Minimum(0), IsInteger(false),
    DependPara(nullptr), ClimateType(climtype),
    IsConstant(isconst), IsOutput(isoutput), OutputToOthers(false),
    initialized(false), ImpactSeries() {
}

template <typename T>
ParamInfo<T>::ParamInfo(const ParamInfo<T>& another) {
    Name = another.Name;
    BasicName = another.BasicName;
    Description = another.Description;
    Units = another.Units;
    Source = another.Source;
    ModuleID = another.ModuleID;
    Dimension = another.Dimension;
    Transfer = another.Transfer;
    Value = another.Value;
    Change = another.Change;
    Impacts = another.Impacts;
    Maximum = another.Maximum;
    Minimum = another.Minimum;
    IsInteger = another.IsInteger;
    DependPara = another.DependPara;
    ClimateType = another.ClimateType;
    IsConstant = another.IsConstant;
    IsOutput = another.IsOutput;
    OutputToOthers = another.OutputToOthers;
    initialized = another.initialized;
    ImpactSeries = another.ImpactSeries;
}

template <typename T>
ParamInfo<T>::~ParamInfo() {
    if (DependPara != nullptr) {
        DependPara = nullptr;
    }
}

template <typename T>
int ParamInfo<T>::SubbasinIndexInImpactSubbasins(int subbasinId){
        if (ImpactSubbasins.empty()) {
            return -1;
        }
        // first find if exactly match
        for (int i = 0; i < ImpactSubbasins.size(); i++) {
            if (find(ImpactSubbasins[i].begin(), ImpactSubbasins[i].end(), subbasinId) != ImpactSubbasins[i].end()) {
                return i;
            }
        }
        // if not exactly match, find if there is a subbasin with id 0 as default
        for (int i = 0; i < ImpactSubbasins.size(); i++) {
            if (find(ImpactSubbasins[i].begin(), ImpactSubbasins[i].end(), 0) != ImpactSubbasins[i].end()) {
                return i;
            }
        }
        return -1;
}

//template <typename T>
//T ParamInfo<T>::GetAdjustedValue(const T pre_value /* = NODATA_VALUE */) {
//    T res = pre_value;
//    if (FloatEqual(pre_value, NODATA_VALUE)) {
//        res = Value;
//    }
//    if (FloatEqual(res, NODATA_VALUE)) {
//        /// Do not change NoData value
//        return res;
//    }
//    
//    if (StringMatch(Change, PARAM_CHANGE_RC) && !FloatEqual(Impact, 1.)) {
//        res *= Impact;
//    }
//    else if (StringMatch(Change, PARAM_CHANGE_AC) && !FloatEqual(Impact, 0.)) {
//        res += Impact;
//    }
//    else if (StringMatch(Change, PARAM_CHANGE_VC) && !FloatEqual(Impact, NODATA_VALUE)) {
//        res = Impact;
//    }
//    else if (StringMatch(Change, PARAM_CHANGE_NC)) {
//        //don't change
//        return res;
//    }
//
//    if (!FloatEqual(Maximum, NODATA_VALUE) && res > Maximum) res = Maximum;
//    if (!FloatEqual(Minimum, NODATA_VALUE) && res < Minimum) res = Minimum;
//    return res;
//}
template <typename T>
T ParamInfo<T>::GetAdjustedValue(const T pre_value /* = NODATA_VALUE */, int currentSubbasinId ) {
    T res = pre_value;
    if (FloatEqual(pre_value, NODATA_VALUE)) {
        res = Value;
    }
    if (FloatEqual(res, NODATA_VALUE)) {
        /// Do not change NoData value
        return res;
    }
    T impact = GetImpactBySubbasinId(currentSubbasinId);
    if (FloatEqual(impact, NODATA_VALUE)) {
        return Value;
    }
    if (StringMatch(Change, PARAM_CHANGE_RC) && !FloatEqual(impact, 1.)) {
        res *= impact;
    }
    else if (StringMatch(Change, PARAM_CHANGE_AC) && !FloatEqual(impact, 0.)) {
        res += impact;
    }
    else if (StringMatch(Change, PARAM_CHANGE_VC) && !FloatEqual(impact, NODATA_VALUE)) {
        res = impact;
    }
    else if (StringMatch(Change, PARAM_CHANGE_NC)) {
        //don't change
        return res;
    }

    if (!FloatEqual(Maximum, NODATA_VALUE) && res > Maximum) res = Maximum;
    if (!FloatEqual(Minimum, NODATA_VALUE) && res < Minimum) res = Minimum;
    return res;
}
template <typename T>
T ParamInfo<T>::GetAdjustedValueWithImpactIndexes(const T pre_value, const int curImpactIndex, vector<T>* impactSeries) {
    T res = pre_value;
    if (FloatEqual(pre_value, NODATA_VALUE)) {
        res = Value;
    }
    if (FloatEqual(res, NODATA_VALUE)) {
        /// Do not change NoData value
        return res;
    }

    T tmpImpact = impactSeries->at(curImpactIndex);
    if (StringMatch(Change, PARAM_CHANGE_RC) && !FloatEqual(tmpImpact, 1)) {
        res *= tmpImpact;
    }
    else if (StringMatch(Change, PARAM_CHANGE_AC) && !FloatEqual(tmpImpact, 0)) {
        res += tmpImpact;
    }
    else if (StringMatch(Change, PARAM_CHANGE_VC) && !FloatEqual(tmpImpact, NODATA_VALUE)) {
        res = tmpImpact;
    }
    else if (StringMatch(Change, PARAM_CHANGE_NC)) {
        //don't change
        return res;
    }

    if (!FloatEqual(Maximum, NODATA_VALUE) && res > Maximum) res = Maximum;
    if (!FloatEqual(Minimum, NODATA_VALUE) && res < Minimum) res = Minimum;
    return res;
}

template <typename T>
T ParamInfo<T>::GetAdjustedValueWithSubbasin(T pre_value, int subbasinId, vector<T>* impacts){
    int impactIndex = SubbasinIndexInImpactSubbasins(subbasinId);
    T value = GetAdjustedValueWithImpactIndexes(pre_value, impactIndex, &impacts);
}


template <typename T>
T ParamInfo<T>::GetImpactBySubbasinId(int subbasinId) {
    int impactIndex = SubbasinIndexInImpactSubbasins(subbasinId);
    if (impactIndex < 0) {
        return NODATA_VALUE;
    }
    return Impacts[impactIndex];
}
// template <typename T>
// void ParamInfo<T>::Adjust1DArray(const int n, T* data) {
// #pragma omp parallel for
//     for (int i = 0; i < n; i++) {
//         if (!FloatEqual(data[i], NODATA_VALUE)) {
//             /// Do not change NoData value
//             data[i] = GetAdjustedValue(data[i]);
//         }
//     }
// }

template <typename T>
void ParamInfo<T>::Adjust1DArray(const int n, T* data, int currentSubbasinId) {
    int impactIndex = SubbasinIndexInImpactSubbasins(currentSubbasinId);
    if (impactIndex < 0) {
        return;
    }
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        if (!FloatEqual(data[i], NODATA_VALUE)) {
            /// Do not change NoData value
            data[i] = GetAdjustedValueWithImpactIndexes(data[i], impactIndex, &Impacts);
        }
    }
}

// template <typename T>
// void ParamInfo<T>::Adjust1DRaster(const int n, T* data) {
//     Adjust1DArray(n, data);
// }
template <typename T>
void ParamInfo<T>::Adjust1DRaster(const int n, T* data, int currentSubbasinId) {
    Adjust1DArray(n, data, currentSubbasinId);
}
template <typename T>
int ParamInfo<T>::Adjust1DRaster(const int n, T* data, const int* units,
                                 const vector<int>& selunits,
                                 const int* lu, const vector<int>& sellu) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (FloatEqual(data[i], NODATA_VALUE)) {
            /// Do not change NoData value
            continue;
        }
        int curunit = units[i];
        int curlu = lu[i];
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

template <typename T>
int ParamInfo<T>::Adjust1DRasterWithImpactIndexes(const int n, T* data, const int* units,
                                                  const vector<int>& selunits, const map<int, int>& impactIndexes,
                                                  const int* lu, const vector<int>& sellu) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (FloatEqual(data[i], NODATA_VALUE)) {
            /// Do not change NoData value
            continue;
        }
        int curunit = units[i];
        int curlu = lu[i];
        //cannot find, continue
        if (find(selunits.begin(), selunits.end(), curunit) == selunits.end()) {
            continue;
        }
        if (find(sellu.begin(), sellu.end(), curlu) == sellu.end()) {
            continue;
        }
        map<int, int>::const_iterator it = impactIndexes.find(curunit);
        if (it == impactIndexes.end()) {
            continue;
        }
        data[i] = GetAdjustedValueWithImpactIndexes(data[i], it->second, &ImpactSeries);
        count += 1;
    }
    return count;
}

// template <typename T>
// void ParamInfo<T>::Adjust2DArray(const int n, T** data) {
// #pragma omp parallel for
//     for (int i = 0; i < n; i++) {
//         Adjust1DArray(CVT_INT(data[i][0]), data[i] + 1);
//     }
// }

template <typename T>
void ParamInfo<T>::Adjust2DArray(const int n, T** data, int currentSubbasinId) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        Adjust1DArray(CVT_INT(data[i][0]), data[i] + 1, currentSubbasinId);
    }
}

// template <typename T>
// void ParamInfo<T>::Adjust2DRaster(const int n, const int lyrs, T** data) {
// #pragma omp parallel for
//     for (int i = 0; i < n; i++) {
//         Adjust1DArray(lyrs, data[i]);
//     }
// }

template <typename T>
void ParamInfo<T>::Adjust2DRaster(const int n, const int lyrs, T** data, int currentSubbasinId) {
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        Adjust1DRaster(lyrs, data[i], currentSubbasinId);
    }
}

template <typename T>
int ParamInfo<T>::Adjust2DRaster(const int n, const int lyrs, T** data,
                                 const int* units, const vector<int>& selunits,
                                 const int* lu, const vector<int>& sellu) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        int curunit = units[i];
        int curlu = lu[i];
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

template <typename T>
int ParamInfo<T>::Adjust2DRasterWithImpactIndexes(const int n, const int lyrs, T** data,
                                                  const int* units, const vector<int>& selunits,
                                                  const map<int, int>& impactIndexes,
                                                  const int* lu, const vector<int>& sellu) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        int curunit = units[i];
        int curlu = lu[i];
        //cannot find, continue
        if (find(selunits.begin(), selunits.end(), curunit) == selunits.end()) {
            continue;
        }
        if (find(sellu.begin(), sellu.end(), curlu) == sellu.end()) {
            continue;
        }
        map<int, int>::const_iterator it = impactIndexes.find(curunit);
        if (it == impactIndexes.end()) {
            continue;
        }
        for (int j = 0; j < lyrs; j++) {
            data[i][j] = GetAdjustedValueWithImpactIndexes(data[i][j], it->second, &ImpactSeries);
        }
        count += 1;
    }
    return count;
}

#endif /* SEIMS_PARAMETER_INFO_H */
