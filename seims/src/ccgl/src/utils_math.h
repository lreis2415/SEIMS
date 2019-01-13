/*!
 * \file utils_math.h
 * \brief Useful equations.
 *        Part of the Common Cross-platform Geographic Library (CCGL)
 *
 * Changelog:
 *   - 1. 2018-05-02 - lj - Make part of CCGL.
 *
 * \author Liangjun Zhu (crazyzlj)
 * \version 1.0
 */
#ifndef CCGL_UTILS_MATH_H
#define CCGL_UTILS_MATH_H

#include <cmath>

#include "basic.h"
#include "utils_array.h"

namespace ccgl {
/*!
 * \namespace ccgl::utils_math
 * \brief Basic mathematics related functions
 */
namespace utils_math {
/*! Return maximum value */
#ifndef Max
#define Max(a, b) ((a) >= (b) ? (a) : (b))
#endif
/*! Return minimum value */
#ifndef Min
#define Min(a, b) ((a) >= (b) ? (b) : (a))
#endif
/*! Return absoulte value */
#ifndef Abs
#define Abs(x) ((x) >= 0 ? (x) : -(x))
#endif

/*!
 * \brief Whether v1 is equal to v2
 * \param[in]  v1 Numeric value of data type 1
 * \param[in]  v2 Numeric value of data type 2
 * \return true or false
 */
template <typename T1, typename T2>
bool FloatEqual(T1 v1, T2 v2) {
    return Abs(CVT_DBL(v1) - CVT_DBL(v2)) < 1.e-32;
}

/*!
 * \brief Check the argument against upper and lower boundary values prior to doing Exponential function
 */
float Expo(float xx, float upper = 20.f, float lower = -20.f);

/*!
 *\brief deal with positive and negative float numbers
 */
float Power(float a, float n);

/*!
 * \brief Get maximum value in a numeric array with size n.
 * \param[in] a, n
 * \return max value
 */
template <typename T>
T MaxInArray(const T* a, int n);

/*!
* \brief Get minimum value in a numeric array with size n.
* \param[in] a, n
* \return min value
*/
template <typename T>
T MinInArray(const T* a, int n);

/*!
 * \brief Sum of a numeric array
 * Get sum value of a double array with size row.
 * \param[in] row
 * \param[in] data
 * \return sum
 */
template <typename T>
T Sum(int row, const T* data);

/*!
 * \brief Sum of a numeric array
 * Get sum value of a double array with size row and real index idx.
 * \param[in] row
 * \param[in] idx
 * \param[in] data
 * \return sum
 */
template <typename T>
T Sum(int row, int*& idx, const T* data);

/*!
 * \brief calculate basic statistics at one time_funcs
 * \param[in] values data array
 * \param[in] num data length
 * \param[out] derivedvalues \a double array, value number, mean, max, min, std, range
 * \param[in] exclude optional, excluded value, e.g. NoDATA, the default is -9999
 */
template <typename T>
void BasicStatistics(const T* values, int num, double** derivedvalues,
                     T exclude = static_cast<T>(NODATA_VALUE));

/*!
 * \brief calculate basic statistics at one time_funcs for 2D raster data
 * \param[in] values data array
 * \param[in] num data length
 * \param[in] lyrs layer number
 * \param[out] derivedvalues \a double array, value number, mean, max, min, std, range
 * \param[in] exclude optional, excluded value, e.g. NoDATA, the default is -9999
 */
template <typename T>
void BasicStatistics(const T*const * values, int num, int lyrs,
                     double*** derivedvalues, T exclude = static_cast<T>(NODATA_VALUE));

/************ Implementation of template functions ******************/
template <typename T>
T MaxInArray(const T* a, const int n) {
    T m = a[0];
    for (int i = 1; i < n; i++) {
        if (a[i] > m) {
            m = a[i];
        }
    }
    return m;
}

template <typename T>
T MinInArray(const T* a, const int n) {
    T m = a[0];
    for (int i = 1; i < n; i++) {
        if (a[i] < m) {
            m = a[i];
        }
    }
    return m;
}

template <typename T>
T Sum(const int row, const T* data) {
    T tmp = 0;
#pragma omp parallel for reduction(+:tmp)
    for (int i = 0; i < row; i++) {
        tmp += data[i];
    }
    return tmp;
}

template <typename T>
T Sum(const int row, int*& idx, const T* data) {
    T tmp = 0;
#pragma omp parallel for reduction(+:tmp)
    for (int i = 0; i < row; i++) {
        int j = idx[i];
        tmp += data[j];
    }
    return tmp;
}

template <typename T>
void BasicStatistics(const T* values, const int num, double** derivedvalues,
                     T exclude /* = CVT_TYP(NODATA_VALUE) */) {
    double* tmpstats = new double[6];
    double maxv = MISSINGFLOAT;
    double minv = MAXIMUMFLOAT;
    int validnum = 0;
    double sumv = 0.;
    double std = 0.;
    for (int i = 0; i < num; i++) {
        if (FloatEqual(values[i], exclude)) continue;
        if (maxv < values[i]) maxv = values[i];
        if (minv > values[i]) minv = values[i];
        validnum += 1;
        sumv += values[i];
    }
    tmpstats[0] = CVT_DBL(validnum);
    double mean = sumv / tmpstats[0];
#pragma omp parallel for reduction(+:std)
    for (int i = 0; i < num; i++) {
        if (!FloatEqual(values[i], exclude)) {
            std += (values[i] - mean) * (values[i] - mean);
        }
    }
    std = sqrt(std / tmpstats[0]);
    tmpstats[1] = mean;
    tmpstats[2] = maxv;
    tmpstats[3] = minv;
    tmpstats[4] = std;
    tmpstats[5] = maxv - minv;
    *derivedvalues = tmpstats;
}

template <typename T>
void BasicStatistics(const T*const * values, const int num, const int lyrs,
                     double*** derivedvalues, T exclude /* = CVT_TYP(NODATA_VALUE) */) {
    double** tmpstats = new double *[6];
    for (int i = 0; i < 6; i++) {
        tmpstats[i] = new double[lyrs];
    }
    for (int j = 0; j < lyrs; j++) {
        tmpstats[0][j] = 0.;                    /// valid number
        tmpstats[1][j] = 0.;                    /// mean
        tmpstats[2][j] = CVT_DBL(MISSINGFLOAT); /// maximum
        tmpstats[3][j] = CVT_DBL(MAXIMUMFLOAT); /// minimum
        tmpstats[4][j] = 0.;                    /// std
        tmpstats[5][j] = 0.;                    /// range
    }
    double* sumv = nullptr;
    utils_array::Initialize1DArray(lyrs, sumv, 0.);
    for (int i = 0; i < num; i++) {
        for (int j = 0; j < lyrs; j++) {
            if (FloatEqual(values[i][j], exclude)) continue;
            if (tmpstats[2][j] < values[i][j]) tmpstats[2][j] = values[i][j];
            if (tmpstats[3][j] > values[i][j]) tmpstats[3][j] = values[i][j];
            tmpstats[0][j] += 1;
            sumv[j] += values[i][j];
        }
    }

    for (int j = 0; j < lyrs; j++) {
        tmpstats[5][j] = tmpstats[2][j] - tmpstats[3][j];
        tmpstats[1][j] = sumv[j] / tmpstats[0][j];
    }
    for (int j = 0; j < lyrs; j++) {
        double tmpstd = 0;
#pragma omp parallel for reduction(+:tmpstd)
        for (int i = 0; i < num; i++) {
            if (!FloatEqual(values[i][j], exclude)) {
                tmpstd += (values[i][j] - tmpstats[1][j]) * (values[i][j] - tmpstats[1][j]);
            }
        }
        tmpstats[4][j] = tmpstd;
    }
    for (int j = 0; j < lyrs; j++) {
        tmpstats[4][j] = sqrt(tmpstats[4][j] / tmpstats[0][j]);
    }
    utils_array::Release1DArray(sumv);
    *derivedvalues = tmpstats;
}
} /* namespace: utils_math */
} /* namespace: ccgl */

#endif /* CCGL_UTILS_MATH_H */
