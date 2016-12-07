/*!
 * \ingroup util
 * \brief Utility functions to handle numeric, string and file
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date Jul. 2010
 *
 * 
 */
#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include "text.h"
#include <cmath>
#include <sstream>
#include <iostream>
#include <float.h>
#include <cmath>
#include <fstream>
#include <cstring>
#ifndef linux
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <windows.h>
#include <winsock2.h>
//#include <WinSock2.h>
//#include <Windows.h>
#include <direct.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#endif
using namespace std;
/**
 * \def NODATA_VALUE
 * \brief NODATA value
 */
#define NODATA_VALUE    -9999.0f
// TODO, replace NODATA by NODATA_VALUE thoroughly
#define NODATA          -99.0f
const float MISSINGFLOAT = -1 * FLT_MAX;
const float MAXFLOAT  = FLT_MAX;
/**
 * \def ZERO
 * \brief zero value used in numeric calculation
 */
#define UTIL_ZERO                1.0e-6f
/**
 * \def PI
 * \brief PI value used in numeric calculation
 */
#define PI                                3.14159265358979323846f
/**
 * \def MINI_SLOPE
 * \brief Minimum slope gradient
 */
#define MINI_SLOPE            0.0001f

/*!
 * \ingroup util
 * \enum LayeringMethod
 * \brief Grid layering method for parallel computing
 */
enum LayeringMethod
{
    /// layering-from-source method
            UP_DOWN,
    /// layering-from-outlet method
            DOWN_UP
};

/*!
 * \brief Whether d1 is equal to d2 or not
 *
 * \param[in] d1, d2 \a double
 * \return true or false
 * \sa ZERO
 */
extern bool DoubleEqual(double d1, double d2);

/// Check the argument against upper and lower boundary values prior to taking the Exponential
extern float Expo(float xx);

/*!
 * \brief Find files in given paths (Windows version)
 * \param[in] lpPath, expression
 * \param[out] vecFiles
 * \return 0 means success
 */
extern int FindFiles(const char *lpPath, const char *expression, vector<string> &vecFiles);

/*!
 * \brief Whether f1 is equal to f2
 *
 * \param[in]  f1, f2 \a float
 * \return true or false
 */
extern bool FloatEqual(float d1, float d2);

/*!
 * \brief Get the root path of the current executable file
 * \return \a string root path
 */
string GetAppPath();

/*!
 * \brief Return the file name from a given file's path
 *
 * \param[in] fullFileName 
 * \return CoreFileName
 * \sa GetPathFromFullName
 */
extern string GetCoreFileName(const string &fullFileName);

/*!
 * \brief Return the suffix of a given file's path
 *
 * \param[in] fullFileName 
 * \return Suffix
 * \sa GetPathFromFullName
 */
extern string GetSuffix(const string &fullFileName);

/*!
 * \brief Get Path From full file path string
 *
 * \param[in] fullFileName \a string
 * \return filePath string
 * \sa GetCoreFileName
 */
extern string GetPathFromFullName(string &fullFileName);

/*!
 * \brief Get Uppercase of given string
 *
 * \param[in] string
 * \return Uppercase string
 */
extern string GetUpper(string);

/*!
 * \brief Initialize DT_Array1D data
 *
 * \param[in] row
 * \param[in] data
 * \param[in] initialValue
 */
template<typename T>
void Initialize1DArray(int row, T *&data, T initialValue)
{
    data = new T[row];
#pragma omp parallel for
    for (int i = 0; i < row; i++)
        data[i] = initialValue;
}

/*!
 * \brief Initialize DT_Array1D data based on an existed array
 *
 * \param[in] row
 * \param[in] data
 * \param[in] iniData
 */
template<typename T>
void Initialize1DArray(int row, T *&data, T *&iniData)
{
    data = new T[row];
#pragma omp parallel for
    for (int i = 0; i < row; i++)
        data[i] = iniData[i];
}

/*!
 * \brief Initialize DT_Array2D data
 *
 * \param[in] row
 * \param[in] row
 * \param[in] data
 * \param[in] initialValue
 */
template<typename T>
void Initialize2DArray(int row, int col, T **&data, T initialValue)
{
    data = new T *[row];
#pragma omp parallel for
    for (int i = 0; i < row; i++)
    {
        data[i] = new T[col];
        for (int j = 0; j < col; j++)
        {
            data[i][j] = initialValue;
        }
    }
}

/*!
 * \brief Initialize DT_Array2D data based on an existed array
 *
 * \param[in] row
 * \param[in] row
 * \param[in] data
 * \param[in] iniData
 */
template<typename T>
void Initialize2DArray(int row, int col, T **&data, T **&iniData)
{
    data = new T *[row];
#pragma omp parallel for
    for (int i = 0; i < row; i++)
    {
        data[i] = new T[col];
        for (int j = 0; j < col; j++)
        {
            data[i][j] = iniData[i][j];
        }
    }
}

/*!
 * \brief Release DT_Array1D data
 * \param[in] data
 */
template<typename T>
void Release1DArray(T *&data)
{
    delete[] data;
    data = NULL;
}

/*!
 * \brief Release DT_Array2D data
 *
 * \param[in] row Row
 * \param[in] col Column
 * \param[in] data
 */
template<typename T>
void Release2DArray(int row, T **&data)
{
#pragma omp parallel for
    for (int i = 0; i < row; i++)
    {
        if (data[i] != NULL)
            delete[] data[i];
    }
    delete[] data;
    data = NULL;
}

/*!
 * \brief Get local time
 *
 * \param[in] tValue \a time_t date
 * \param[out] tmStruct \a tm struct date
 */
void LocalTime(time_t tValue, struct tm *tmStruct);

/*!
 * \brief Write 1D array to a file
 *
 * \sa Read1DArrayFromTxtFile(), Read2DArrayFromTxtFile(), Output2DArrayToTxtFile()
 *
 * \param[in] n, data, filename
 */
extern void Output1DArrayToTxtFile(int n, float *data, const char *filename);

/*!
 * \brief Write 2D array to a file
 *
 * \sa Read1DArrayFromTxtFile(), Read2DArrayFromTxtFile(), Output1DArrayToTxtFile()
 *
 * \param[in] nRows, nCols, data, filename
 */
extern void Output2DArrayToTxtFile(int nRows, int nCols, float **data, const char *filename);

/// deal with positive and negative float numbers
float Power(float a, float n);

/*!
 * \brief Read 1D array from file
 *
 * The input file should follow the format:
 *     a 1D array sized nRows * 1
 * The size of data is nRows
 * \sa Read2DArrayFromTxtFile(), Output1DArrayToTxtFile(), Output2DArrayToTxtFile()
 * \param[in] filename, nRows
 * \param[out] data
 */
extern void Read1DArrayFromTxtFile(const char *filename, int &nRows, float *&data);

/*!
 * \brief Read 2D array from file
 *
 * The input file should follow the format:
 *     a 2D array sized nRows * nRows
 * The size of data is nRows * (nRows + 1), the first element of each row is the nRows
 * \sa Read1DArrayFromTxtFile(), Output1DArrayToTxtFile(), Output2DArrayToTxtFile()
 * \param[in] filename, nRows
 * \param[out] data
 */
extern void Read2DArrayFromTxtFile(const char *filename, int &nRows, float **&data);

/*!
 * \brief Read 2D array from string
 *
 * The input string should follow the format:
 *     float value, total number is nRows * nRows
 * The size of data is nRows * (nRows + 1), the first element of each row is the nRows.
 *
 * \param[in] s, nRows
 * \param[out] data
 */
extern void Read2DArrayFromString(const char *s, int &nRows, float **&data);

/*!
 * \brief Print status messages for Debug
 */
extern void StatusMessage(const char *msg);

/*!
 * \brief Match \a char ignore cases
 *
 * \param[in] a, b \a char* 
 * \return true or false
 * \sa StringMatch()
 */
extern bool StrEqualIgnoreCase(const char *, const char *);

/*!
 * \brief Match Strings in UPPERCASE manner
 *
 * \param[in] text1, text2
 * \return true or false
 */
extern bool StringMatch(string text1, string text2);
/*!
 * \brief Max value of a numeric array
 *
 * Get maximum value in a numeric array with size n.
 *
 * \param[in] a, n
 * \return max value
 */
template<typename T>
T Max(T *a, int n)
{
	T m = a[0];
	for (int i = 1; i < n; i++)
	{
		if (a[i] > m)
		{
			m = a[i];
		}
	}
	return m;
}
/*!
 * \brief Sum of a numeric array
 *
 * Get sum value of a double array with size row.
 *
 * \param[in] row
 * \param[in] data
 * \return sum
 */
template<typename T>
T Sum(int row, T *&data)
{
	T tmp = 0;
#pragma omp parallel for reduction(+:tmp)
	for (int i = 0; i < row; i++)
	{
		tmp += data[i];
	}
	return tmp;
}

template<typename T>
T Sum(int row, int *&idx, T *&data)
{
	T tmp = 0;
#pragma omp parallel for reduction(+:tmp)
	for (int i = 0; i < row; i++)
	{
		int j = idx[i];
		tmp += data[j];
	}
	return tmp;
}
/*!
 * \brief Trim given string's heading and tailing by "<space>,\n,\t,\r"
 *
 * \param[in] s \a string information
 * \return Trimmed string
 */
extern string &trim(string &s);

// define some macro for string related built-in functions, by Liangjun
#ifdef MSVC
#define stringcat strcat_s
#define stringcpy strcpy_s
#else
#define stringcat strcat
#define stringcpy strcpy
#endif

#ifndef linux
#define strprintf sprintf_s
#define StringTok strtok_s
#ifdef MSVC
#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif
#endif
#else
#define strprintf snprintf
#define StringTok strtok_r
/*!
 * \brief Copy file in linux
 *
 * \param[in] srcfile \a char source file path
 * \param[in] dstfile \a char destination file path
 */
extern int copyfile_linux(const char* srcfile, const char* dstfile);
#endif

/*!
* \brief Convert value to string
*
* \param[in] val value, e.g., a int, or float
* \return converted string
*/
template<typename T>
string ValueToString(T val)
{
    ostringstream oss;
    oss << val;
    return oss.str();
}

/*!
* \brief If value in vector container
*
* \param[in] val Value, e.g., a int, or float
* \param[in] vec Vector container, data type is consistent with val
* \return True if val is in vec, otherwise False
*/
template<typename T>
bool ValueInVector(T &val, vector<T> &vec)
{
    typename vector<T>::iterator findIter = find(vec.begin(), vec.end(), val);
    if (findIter == vec.end())
        return false;
    else
        return true;
}

/*!
* \brief Remove value in vector container
*
* \param[in] val Value to be removed, e.g., a int, or float
* \param[in] vec Vector container, data type is consistent with val
*/
template<typename T>
void RemoveValueInVector(T &val, vector<T> &vec)
{
    typename vector<T>::iterator Iter = vec.begin();
    for (; Iter != vec.end(); Iter++)
    {
        if (*Iter == val)
            Iter = vec.erase(Iter);
        if (Iter == vec.end())
            break;
    }
}
/*
 *\brief Counting time for program
 * Cross-platform
*/
double TimeCounting();