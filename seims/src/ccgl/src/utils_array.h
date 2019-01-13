/*!
 * \file utils_array.h
 * \brief Template functions to initialize and release arrays.
 *        Part of the Common Cross-platform Geographic Library (CCGL)
 *
 * Changelog:
 *   - 1. 2018-05-02 - lj - Make part of CCGL.
 *
 * \author Liangjun Zhu (crazyzlj)
 * \version 1.0
 */
#ifndef CCGL_UTILS_ARRAY_H
#define CCGL_UTILS_ARRAY_H

#include <new> // std::nothrow
#include <cstdarg> // variable arguments
#include <iostream>
#include <vector>

#include "basic.h"

using std::vector;
using std::cout;
using std::endl;
using std::nothrow;

namespace ccgl {
/*!
 * \namespace ccgl::utils_array
 * \brief Array related functions include vector and pointer array.
 */
namespace utils_array {
/*!
 * \brief Initialize DT_Array1D data
 * \param[in] row
 * \param[in] data
 * \param[in] init_value
 * \return True if succeed, else false and the error message will print as well.
 */
template <typename T, typename INI_T>
bool Initialize1DArray(int row, T*& data, INI_T init_value);

/*!
 * \brief Initialize DT_Array1D data based on an existed array
 * \param[in] row
 * \param[in] data
 * \param[in] init_data
 * \return True if succeed, else false and the error message will print as well.
 */
template <typename T, typename INI_T>
bool Initialize1DArray(int row, T*& data, INI_T* init_data);

/*!
 * \brief Initialize DT_Array2D data
 * \param[in] row
 * \param[in] col
 * \param[in] data
 * \param[in] init_value
 * \return True if succeed, else false and the error message will print as well.
 */
template <typename T, typename INI_T>
bool Initialize2DArray(int row, int col, T**& data, INI_T init_value);

/*!
 * \brief Initialize DT_Array2D data based on an existed array
 * The usage of `const T * const *` is refers to http://blog.csdn.net/pmt123456/article/details/50813564
 * \param[in] row
 * \param[in] col
 * \param[in] data
 * \param[in] init_data dimension MUST BE (row, col)
 * \return True if succeed, else false and the error message will print as well.
 */
template <typename T, typename INI_T>
bool Initialize2DArray(int row, int col, T**& data, INI_T** init_data);

/*!
 * \brief Release DT_Array1D data
 * \param[in] data
 */
template <typename T>
void Release1DArray(T*& data);

/*!
 * \brief Release DT_Array2D data
 * \param[in] row row
 * \param[in] data
 */
template <typename T>
void Release2DArray(int row, T**& data);

/*!
 * \brief Batch release of 1D array
 *        Variable arguments with the end of `nullptr`.
 *
 *        The input parameters are listed as `data`, `data2`, ... , `dataN`, and ended with `nullptr`.
 *
 * Example:
 * \code
 *   BatchRelease1DArray(array1, array2, array3, nullptr);
 * \endcode
 *
 * \warning After batch release, the variable will not be set to nullptr.
 *          So, do not use these variables any more.
 *          BTW, this function will not cause memory leak.
 *
 *          USE WITH ALL CAUTIONS CLEARLY AWARED.
 */
template <typename T>
void BatchRelease1DArray(T*& data, ...);

/*!
 * \brief Batch release of 2D array, \sa BatchRelease1DArray
 *        Variable arguments with the end of nullptr.
 *
 * Example:
 * \code
 *   BatchRelease2DArray(rows, array1, array2, array3, nullptr);
 * \endcode
 *
 * \param[in] nrows Rows
 * \param[in] data The input parameters are listed as `data`, `data2`, ... , `dataN`, and ended with `nullptr`.
 * \warning USE WITH ALL CAUTIONS CLEARLY AWARED.
 */
template <typename T>
void BatchRelease2DArray(int nrows, T**& data, ...);

/*!
 * \brief Write 1D array to a file
 * \sa Read1DArrayFromTxtFile(), Read2DArrayFromTxtFile(), Output2DArrayToTxtFile()
 * \param[in] n, data, filename
*/
void Output1DArrayToTxtFile(int n, const float* data, const char* filename);

/*!
 * \brief Write 2D array to a file
 * \sa Read1DArrayFromTxtFile(), Read2DArrayFromTxtFile(), Output1DArrayToTxtFile()
 * \param[in] rows, cols, data, filename
 */
void Output2DArrayToTxtFile(int rows, int cols, const float** data, const char* filename);

/*!
 * \brief Read 1D array from file
 *        The input file should follow the format:
 *           a 1D array sized rows * 1
 *
 *        The size of data is rows
 *
 * \sa Read2DArrayFromTxtFile(), Output1DArrayToTxtFile(), Output2DArrayToTxtFile()
 * \param[in] filename
 * \param[out] rows, data
 */
template <typename T>
void Read1DArrayFromTxtFile(const char* filename, int& rows, T*& data);

/*!
 * \brief Read 2D array from file
 *        The input file should follow the format:
 *            a 2D array sized rows * rows
 *
 *        The size of data is rows * (rows + 1), the first element of each row is the rows
 *
 * \sa Read1DArrayFromTxtFile(), Output1DArrayToTxtFile(), Output2DArrayToTxtFile()
 * \param[in] filename
 * \param[out] rows, data
 */
template <typename T>
void Read2DArrayFromTxtFile(const char* filename, int rows, T**& data);

/*!
 * \brief Read 2D array from string
 *        The input string should follow the format:
 *            float value, total number is rows * rows
 *
 *        The size of data is rows * (rows + 1), the first element of each row is the rows.
 *
 * \param[in] s
 * \param[out] rows, data
 */
template <typename T>
void Read2DArrayFromString(const char* s, int rows, T**& data);

/*!
 * \brief If value in vector container
 * \param[in] val Value, e.g., a int, or float
 * \param[in] vec Vector container, data type is consistent with val
 * \return True if val is in vec, otherwise False
 */
template <typename T>
bool ValueInVector(T val, const vector<T>& vec);

/*!
 * \brief Remove value in vector container
 * \param[in] val Value to be removed, e.g., a int, or float
 * \param[in] vec Vector container, data type is consistent with val
 */
template <typename T>
void RemoveValueInVector(T val, vector<T>& vec);


/************ Implementation of template functions ******************/
template <typename T, typename INI_T>
bool Initialize1DArray(const int row, T*& data, const INI_T init_value) {
    if (nullptr != data) {
        cout << "The input 1D array pointer is not nullptr, without initialized!" << endl;
        return false;
    }
    data = new(nothrow)T[row];
    if (nullptr == data) {
        delete[] data;
        cout << "Bad memory allocated during 1D array initialization!" << endl;
        return false;
    }
    T init = static_cast<T>(init_value);
#pragma omp parallel for
    for (int i = 0; i < row; i++) {
        data[i] = init;
    }
    return true;
}

template <typename T, typename INI_T>
bool Initialize1DArray(const int row, T*& data, INI_T* const init_data) {
    if (nullptr != data) {
        cout << "The input 1D array pointer is not nullptr, without initialized!" << endl;
        return false;
    }
    data = new(nothrow) T[row];
    if (nullptr == data) {
        delete[] data;
        cout << "Bad memory allocated during 1D array initialization!" << endl;
        return false;
    }
    if (nullptr == init_data) {
        cout << "The input parameter init_data MUST NOT be nullptr!" << endl;
        return false;
    }
#pragma omp parallel for
    for (int i = 0; i < row; i++) {
        data[i] = static_cast<T>(init_data[i]);
    }
    return true;
}

template <typename T, typename INI_T>
bool Initialize2DArray(const int row, const int col, T**& data,
                       const INI_T init_value) {
    if (nullptr != data) {
        cout << "The input 2D array pointer is not nullptr, without initialized!" << endl;
        return false;
    }
    data = new(nothrow) T *[row];
    if (nullptr == data) {
        delete[] data;
        cout << "Bad memory allocated during 2D array initialization!" << endl;
        return false;
    }
    T init = static_cast<T>(init_value);
    int bad_alloc = 0;
#pragma omp parallel for reduction(+:bad_alloc)
    for (int i = 0; i < row; i++) {
        data[i] = new(nothrow) T[col];
        if (nullptr == data[i]) {
            delete[] data[i];
            bad_alloc++;
        }
        for (int j = 0; j < col; j++) {
            data[i][j] = init;
        }
    }
    if (bad_alloc > 0) {
        cout << "Bad memory allocated during 2D array initialization!" << endl;
        Release2DArray(row, data);
        return false;
    }
    return true;
}

template <typename T, typename INI_T>
bool Initialize2DArray(const int row, const int col, T**& data,
                       INI_T** const init_data) {
    if (nullptr != data) {
        cout << "The input 2D array pointer is not nullptr, without initialized!" << endl;
        return false;
    }
    data = new(nothrow)T *[row];
    if (nullptr == data) {
        delete[] data;
        cout << "Bad memory allocated during 2D array initialization!" << endl;
        return false;
    }
    int bad_alloc = 0;
    int error_access = 0;
#pragma omp parallel for reduction(+:bad_alloc, error_access)
    for (int i = 0; i < row; i++) {
        data[i] = new(nothrow)T[col];
        if (nullptr == data[i]) {
            delete[] data[i];
            bad_alloc++;
        }
        if (nullptr == init_data[i]) {
            error_access++;
        } else {
            for (int j = 0; j < col; j++) {
                data[i][j] = static_cast<T>(init_data[i][j]);
            }
        }
    }
    if (bad_alloc > 0) {
        cout << "Bad memory allocated during 2D array initialization!" << endl;
        Release2DArray(row, data);
        return false;
    }
    if (error_access > 0) {
        cout << "nullptr pointer existed in init_data during 2D array initialization!" << endl;
        Release2DArray(row, data);
        return false;
    }
    return true;
}

template <typename T>
void Release1DArray(T*& data) {
    if (nullptr != data) {
        delete[] data;
        data = nullptr;
    }
}

template <typename T>
void Release2DArray(const int row, T**& data) {
    if (nullptr == data) {
        return;
    }
#pragma omp parallel for
    for (int i = 0; i < row; i++) {
        if (data[i] != nullptr) {
            delete[] data[i];
            data[i] = nullptr;
        }
    }
    delete[] data;
    data = nullptr;
}

template <typename T>
void BatchRelease1DArray(T*& data, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, data);
    Release1DArray(data);
    T* arg_value = va_arg(arg_ptr, T*);
    while (nullptr != arg_value) {
        Release1DArray(arg_value);
        arg_value = va_arg(arg_ptr, T*);
    }
    va_end(arg_ptr);
}

template <typename T>
void BatchRelease2DArray(const int nrows, T**& data, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, data);
    Release2DArray(nrows, data);
    T** arg_value = va_arg(arg_ptr, T**);
    while (nullptr != arg_value) {
        Release2DArray(nrows, arg_value);
        arg_value = va_arg(arg_ptr, T**);
    }
    va_end(arg_ptr);
}

template <typename T>
bool ValueInVector(const T val, const vector<T>& vec) {
    if (vec.empty()) {
        return false;
    }
    if (find(vec.begin(), vec.end(), val) == vec.end()) {
        return false;
    }
    return true;
}

template <typename T>
void RemoveValueInVector(const T val, vector<T>& vec) {
    for (auto iter = vec.begin(); iter != vec.end();) {
        if (*iter == val) {
            iter = vec.erase(iter);
        } else {
            ++iter;
        }
    }
}

} /* utils_array */
} /* namespace: ccgl */

#endif /* CCGL_UTILS_ARRAY_H */
