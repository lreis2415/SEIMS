/*!
 * \brief Utilities class to handle string, date time, basic mathematics, and file.
 *        classes includes: utilsTime, utilsString, utilsArray, utilsMath, utilsFileIO.
 * \author Junzhi Liu, Liangjun Zhu
 * \version 2.0
 * \date Jul. 2010
 * \updated Nov. 2017
 */

#ifndef CLS_UTILS
#define CLS_UTILS

/// OpenMP support
#ifdef SUPPORT_OMP
#include <omp.h>
#endif /* SUPPORT_OMP */
/// math and STL headers
#include <cmath>
#include <cfloat>
#include <array>
#include <vector>
#include <map>
#include <algorithm>
#include <iterator>
#include <cstdint>
#include <memory>
/// time
#include <ctime>
/// string
#include <cstring>
/// IO stream
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <fstream>
/// platform
#ifdef WIN32
#include <io.h>
//#define _WINSOCKAPI_    // stops windows.h including winsock.h // _WINSOCKAPI_ is defined by <winsock2.h>
#include <winsock2.h>
#include <windows.h>
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#endif /* WIN32 */

#if (defined macos) || (defined macosold)
#include <libproc.h>
#endif /* macos */
/// assert
#include <cassert>
/// variable arguments
#include <cstdarg>

using namespace std;

/*!
 * Global utility definitions
 */
/**
 * \def NODATA_VALUE
 * \brief NODATA value
 */
#ifndef NODATA_VALUE
#define NODATA_VALUE    (-9999.0f)
#endif /* NODATA_VALUE */
const float MISSINGFLOAT = -1 * FLT_MAX;
const float MAXIMUMFLOAT = FLT_MAX;
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif /* PATH_MAX */
/**
 * \def ZERO
 * \brief zero value used in numeric calculation
 */
#ifndef UTIL_ZERO
#define UTIL_ZERO        1.0e-6f
#endif /* UTIL_ZERO */
/**
 * \def PI
 * \brief PI value used in numeric calculation
 */
#ifndef PI
#define PI                3.14159265358979323846f
#endif /* PI */
/**
 * \def MINI_SLOPE
 * \brief Minimum slope gradient
 */
#ifndef MINI_SLOPE
#define MINI_SLOPE        0.0001f
#endif /* MINI_SLOPE */

#ifdef WIN32
#define Tag_ModuleDirectoryName "\\"
#define SEP "\\"
#define Tag_DyLib ".dll"
#else
#define Tag_ModuleDirectoryName "/"
#define SEP "/"
#define Tag_So "lib"
#endif /* WIN32 */
#ifdef linux
#define Tag_DyLib ".so"
#elif (defined macos) || (defined macosold)
#define Tag_DyLib ".dylib"
#endif /* linux */

// define some macro for string related built-in functions
#ifdef MSVC
#define stringcat strcat_s
#define stringcpy strcpy_s
#define strprintf sprintf_s
#define strtok strtok_s
#define stringscanf sscanf_s
#else
#define stringcat strcat
#define stringcpy strcpy
#define strprintf snprintf
#define strtok strtok_r
#define stringscanf sscanf
#endif /* MSVC */

#if defined(__MINGW32_VERSION) || defined(_MSC_VER)
#define strcasecmp _stricmp
#endif /* defined(__MINGW32_VERSION) || defined(_MSC_VER) */

/*
 * Avoid the compile error on MSVC like this:
 *   warning C4251: 'CLASS_TEST::m_structs':
 *           class 'std::vector<_Ty>' needs to have dll-interface to be used by clients of class
 * refers to http://www.cnblogs.com/duboway/p/3332057.html
 */
#ifdef MSVC
#define DLL_STL_LIST(STL_API, STL_TYPE) \
    template class STL_API std::allocator< STL_TYPE >; \
    template class STL_API std::vector<STL_TYPE, std::allocator< STL_TYPE > >;
#endif /* MSVC */

static int daysOfMonth[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*
 * Constant value type pointer
 */
typedef const int*     CINTPTR;
typedef const float*   CFLOATPTR;
typedef const double*  CDOUBLEPTR;

/*!
 * \class utilsTime
 * \brief Time related functions
 */
class utilsTime {
public:
    utilsTime() = default;   //< void constructor
    ~utilsTime() = default;  //< void destructor
    /*
     *\brief Precisely and cross-platform time counting function.
     */
    static double TimeCounting();

    /*!
     *\brief Check the given year is a leap year or not.
     */
    static bool isLeapYear(int yr) {        return ((yr % 4) == 0 && ((yr % 100) != 0 || (yr % 400) == 0));    }

    /*!
     * \brief Convert date time to string as the format of "YYYY-MM-DD"
     *
     *
     * \param[in] date \a time_t data type
     * \return Date time \a string
     */
    static string ConvertToString(const time_t *date);

    /*!
     * \brief  Convert date time to string as the format of "YYYY-MM-DD HH"
     *
     *
     * \param[in] date \a time_t data type
     * \return Date time \a string
     */
    static string ConvertToString2(const time_t *date);

    /*!
     * \brief Convert string to date time, string format could be %4d%2d%2d or %d-%d-%d
     *
     * e.g.: strDate => 20000323, format=> %4d%2d%2d
     *       strDate => 2000-03-23, format => %d-%d-%d
     *
     *
     * \param[in] strDate \a string date
     * \param[in] format \a string format
     * \param[in] includeHour \a bool Include Hour?
     * \return Date time \a time_t
     */
    static time_t ConvertToTime(const string& strDate, string const &format, bool includeHour);

    /*!
     * \brief Convert string to date time, string format could be "%4d-%2d-%2d %2d:%2d:%2d"
     *
     * e.g.: strDate => 2000-03-23 10:30:00, format=> %4d-%2d-%2d %2d:%2d:%2d
     *
     *
     * \param[in] strDate \a string date
     * \param[in] format \a string format
     * \param[in] includeHour \a bool Include Hour?
     * \return Date time \a time_t
     */
    static time_t ConvertToTime2(string const &strDate, const char *format, bool includeHour);

    /*!
     * \brief Convert integer year, month, and day to date time
     *
     * \param[in] strDate \a string date
     * \param[in] format \a string format
     * \param[in] includeHour \a bool Include Hour?
     * \return Date time \a time_t
     */
    static time_t ConvertYMDToTime(int &year, int &month, int &day);

    /*!
     * \brief Get date information from \a time_t variable
     *
     * \param[in] t \a time_t date
     * \param[out] year, month, day \a int value
     */
    static int GetDateInfoFromTimet(time_t *t, int *year, int *month, int *day);

    /*!
     * \brief Get local time
     *
     * \param[in] tValue \a time_t date
     * \param[out] tmStruct \a tm struct date
     */
    static void LocalTime(time_t tValue, struct tm *tmStruct);
};

/*!
 * \class utilsString
 * \brief String related functions
 */
class utilsString {
public:
    utilsString() = default;   //< void constructor
    ~utilsString() = default;  //< void destructor
    /*!
     * \brief Get Uppercase of given string
     * \param[in] string
     * \return Uppercase string
    */
    static string GetUpper(const string& s);

    /*!
     * \brief Match \a char ignore cases
     *
     * \param[in] a, b \a char*
     * \return true or false
     * \sa StringMatch()
     */
    static bool StringMatch(const char *a, const char *b);

    /*!
     * \brief Match Strings in UPPERCASE manner
     * \param[in] text1, text2
     * \return true or false
     */
    static bool StringMatch(const string &text1, const string &text2);

    /*!
     * \brief Trim Both leading and trailing spaces
     * \sa trim
     * \param[in] str \a string
     */
    static void TrimSpaces(string &str);

    /*!
     * \brief Trim given string's heading and tailing by "<space>,\n,\t,\r"
     * \sa TrimSpaces
     * \param[in] s \a string information
     * \return Trimmed string
     */
    static string &trim(string &s);

    /*!
     * \brief Splits the given string by spaces
     *
     * \param[in] item \a string information
     * \return The split strings vector
     */
    static vector<string> SplitString(const string &item);

    /*!
     * \brief Splits the given string based on the given delimiter
     *
     * \param[in] item \a string information
     * \param[in] delimiter \a char
     * \return The split strings vector
     */
    static vector<string> SplitString(const string &item, char delimiter);

    /*
     * \brief Get numeric values by splitting the given string based on the given delimiter
     */
    template<typename T>
    vector<T> SplitStringForValues(string const &item, char delimiter);

    /*
     * \brief Get int values by splitting the given string based on the given delimiter
     */
    static vector<int> SplitStringForInt(const string &item, char delimiter);

    /*
     * \brief Get float values by splitting the given string based on the given delimiter
     */
    static vector<float> SplitStringForFloat(const string &item, char delimiter);

    /*!
     * \brief Convert value to string
     * \param[in] val value, e.g., a int, or float
     * \return converted string
     */
    template<typename T>
    static string ValueToString(const T& val);
};

/*!
 * \class utilsArray
 * \brief Array related functions include vector and pointer array.
 */
class utilsArray {
public:
    utilsArray() = default;   //< void constructor
    ~utilsArray() = default;  //< void destructor
    /*!
     * \brief Initialize DT_Array1D data
     *
     * \param[in] row
     * \param[in] data
     * \param[in] initialValue
     * \return True if succeed, else false and the error message will print as well.
     */
    template<typename T>
    static bool Initialize1DArray(int row, T *&data, T initialValue);

    /*!
     * \brief Initialize DT_Array1D data based on an existed array
     *
     * \param[in] row
     * \param[in] data
     * \param[in] iniData
     * \return True if succeed, else false and the error message will print as well.
     */
    template<typename T>
    static bool Initialize1DArray(int row, T *&data, const T *iniData);

    /*!
     * \brief Initialize DT_Array2D data
     *
     * \param[in] row
     * \param[in] col
     * \param[in] data
     * \param[in] initialValue
     * \return True if succeed, else false and the error message will print as well.
     */
    template<typename T>
    static bool Initialize2DArray(int row, int col, T **&data, T initialValue);

    /*!
     * \brief Initialize DT_Array2D data based on an existed array
     * The usage of `const T * const *` is refers to http://blog.csdn.net/pmt123456/article/details/50813564
     * \param[in] row
     * \param[in] col
     * \param[in] data
     * \param[in] iniData, the dimension MUST BE (row, col)
     * \return True if succeed, else false and the error message will print as well.
     */
    template<typename T>
    static bool Initialize2DArray(int row, int col, T **&data, const T * const *iniData);

    /*!
     * \brief Release DT_Array1D data
     * \param[in] data
     */
    template<typename T>
    static void Release1DArray(T *&data);

    /*!
     * \brief Release DT_Array2D data
     *
     * \param[in] row Row
     * \param[in] col Column
     * \param[in] data
     */
    template<typename T>
    static void Release2DArray(int row, T **&data);
    /*!
     * \brief Batch release of 1D array
     *        Variable arguments with the end of nullptr.
     * \param[in] data, data2, ... , dataN, nullptr
     * \usage BatchRelease1DArray(array1, array2, array3, nullptr);
     * \caution After batch release, the variable will not be set to nullptr.
     *          So, do not use these variables any more.
     *          BTW, this function will not cause memory leak.
     *          USE WITH ALL CAUTIONS CLEARLY AWARED.
     */
    template<typename T>
    static void BatchRelease1DArray(T*& data, ...);
    /*!
    * \brief Batch release of 2D array, \sa BatchRelease1DArray
    *        Variable arguments with the end of nullptr.
    * \param[in] nrows Rows
    * \param[in] data, data2, ... , dataN, nullptr
    * \usage BatchRelease2DArray(rows, array1, array2, array3, nullptr);
    * \caution USE WITH ALL CAUTIONS CLEARLY AWARED.
    */
    template<typename T>
    static void BatchRelease2DArray(int nrows, T**& data, ...);
    /*!
     * \brief Write 1D array to a file
     *
     * \sa Read1DArrayFromTxtFile(), Read2DArrayFromTxtFile(), Output2DArrayToTxtFile()
     *
     * \param[in] n, data, filename
     */
    static void Output1DArrayToTxtFile(int n, CFLOATPTR data, const char *filename);

    /*!
     * \brief Write 2D array to a file
     *
     * \sa Read1DArrayFromTxtFile(), Read2DArrayFromTxtFile(), Output1DArrayToTxtFile()
     *
     * \param[in] nRows, nCols, data, filename
     */
    static void Output2DArrayToTxtFile(int nRows, int nCols, const CFLOATPTR *data, const char *filename);

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
    static void Read1DArrayFromTxtFile(const char *filename, int &nRows, float *&data);

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
    static void Read2DArrayFromTxtFile(const char *filename, int &nRows, float **&data);

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
    static void Read2DArrayFromString(const char *s, int &nRows, float **&data);

    /*!
    * \brief If value in vector container
    *
    * \param[in] val Value, e.g., a int, or float
    * \param[in] vec Vector container, data type is consistent with val
    * \return True if val is in vec, otherwise False
    */
    template<typename T>
    static bool ValueInVector(const T &val, const vector<T> &vec);

    /*!
    * \brief Remove value in vector container
    *
    * \param[in] val Value to be removed, e.g., a int, or float
    * \param[in] vec Vector container, data type is consistent with val
    */
    template<typename T>
    static void RemoveValueInVector(T &val, vector<T> &vec);
};

/*!
 * \class utilsMath
 * \brief Basic mathematics related functions
 */
class utilsMath {
public:
    utilsMath() = default;   //< void constructor
    ~utilsMath() = default;  //< void destructor
    /*!
     * \brief Whether v1 is equal to v2
     * \param[in]  v1, v2 numeric value
     * \return true or false
     */
    template<typename T>
    static bool FloatEqual(T v1, T v2);

    /*!
     *\brief Check the argument against upper and lower boundary values prior to doing Exponential function
     */
    static float Expo(float xx, float upper = 20.f, float lower = -20.f);

    /*!
     *\brief deal with positive and negative float numbers
     */
    static float Power(float a, float n);

    /*!
     * \brief Max value of a numeric array
     * Get maximum value in a numeric array with size n.
     * \param[in] a, n
     * \return max value
     */
    template<typename T>
    static T Max(const T *a, int n);

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
    static T Sum(int row, const T *data);

    /*!
     * \brief Sum of a numeric array
     * Get sum value of a double array with size row and real index idx.
     * \param[in] row
     * \param[in] idx
     * \param[in] data
     * \return sum
     */
    template<typename T>
    static T Sum(int row, int *&idx, const T *data);

    /*!
     * \brief calculate basic statistics at one time
     * \param[in] values data array
     * \param[in] num data length
     * \param[out] derivedvalues \double array, value number, mean, max, min, std, range
     * \param[in] exclude optional, excluded value, e.g. NoDATA, the default is -9999
     */
    template<typename T>
    static void basicStatistics(const T *values, int num, double **derivedvalues, T exclude = (T)NODATA_VALUE);

    /*!
     * \brief calculate basic statistics at one time for 2D raster data
     * \param[in] values data array
     * \param[in] num data length
     * \param[in] lyrs layer number
     * \param[out] derivedvalues \double array, value number, mean, max, min, std, range
     * \param[in] exclude optional, excluded value, e.g. NoDATA, the default is -9999
     */
    template<typename T>
    static void basicStatistics(const T * const *values, int num, int lyrs, double ***derivedvalues, T exclude = (T)NODATA_VALUE);
};

/*!
 * \class utilsFileIO
 * \brief File Input and output related functions
 */
class utilsFileIO {
public:
    utilsFileIO() = default;   //< void constructor
    ~utilsFileIO() = default;  //< void destructor
#ifndef WIN32

    /*!
     * \brief Copy file in unix-based platform
     * \param[in] srcfile \a char source file path
     * \param[in] dstfile \a char destination file path
     */
    static int copyfile_unix(const char *srcfile, const char *dstfile);

#endif /* WIN32 */
    /*!
    * \brief Check the given directory path is exists or not.
    */
    static bool DirectoryExists(const string& dirpath);
    /*!
     * \brief Clean a directory if exists, otherwise create it.
     */
    static bool CleanDirectory(const string& dirpath);
    /*!
     * \brief Get the root path of the current executable file
     * \return \a string root path
     */
    static string GetAppPath();

    /*!
     * \brief Return the file name from a given file's path
     *
     * \param[in] fullFileName
     * \return CoreFileName
     * \sa GetPathFromFullName
     */
    static string GetCoreFileName(string const &fullFileName);

    /*!
     * \brief Return the suffix of a given file's path
     *
     * \param[in] fullFileName
     * \return Suffix
     * \sa GetPathFromFullName
     */
    static string GetSuffix(string const &fullFileName);

    /*!
     * \brief Replace the suffix by a given suffix
     *
     * \param[in] fullFileName
     * \param[in] newSuffix
     * \return new fullFileName
     */
    static string ReplaceSuffix(string const &fullFileName, string const &newSuffix);

    /*!
     * \brief Get Path From full file path string
     *
     * \param[in] fullFileName \a string
     * \return filePath string
     * \sa GetCoreFileName
     */
    static string GetPathFromFullName(string const &fullFileName);

    /*!
     * \brief Return a flag indicating if the given file exists
     *
     * \param[in] FileName String path of file
     * \return True if Exists, and false if not.
     */
    static bool FileExists(string const &FileName);

    /*!
     * \brief Return a flag indicating if the given path exists
     *
     * \param[in] path String path
     * \return True if Exists, and false if not.
     */
    static bool PathExists(string const &path);

    /*!
     * \brief Delete the given file if existed.
     * \param[in] filepath \string
     * \return 0 if deleted successful, else return nonzero value, e.g. -1.
     */
    static int DeleteExistedFile(const string &filepath);

    /*!
     * \brief Find files in given paths
     * \param[in] lpPath, expression
     * \param[out] vecFiles
     * \return 0 means success
     */
    static int FindFiles(const char *lpPath, const char *expression, vector<string> &vecFiles);
    /*!
     * \brief Load short plain text file as string vector, ignore comments begin with '#' and empty lines
     * \param[in] filepath Plain text file path
     * \param[out] contentStrs Each line without CRLF or LF stored in vector
     * \return True when read successfully, and false with empty contentStrs when failed
     */
    static bool LoadPlainTextFile(const string& filepath, vector<string>& contentStrs);
};

/*!
 * \class utils
 * \brief Utility functions excluding time, string, math, and file IO.
 * For example, omp thread setting.
 */
class utils {
public:
    utils() = default;   //< void constructor
    ~utils() = default;  //< void destructor
    /*!
     * \brief Check if the IP address is valid.
     */
    static bool isIPAddress(const char *ip);

    /*!
     * \brief Writes an entry to the log file. Normally only used for debug
     *
     * \param[in] msg \a string log message
     * \param[in] logpath \a string Optional
     */
    static void Log(string msg, string logpath = "debugInfo.log");

    /*!
     * \brief Detect the available threads number
     * reference:
     *    1. http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
     *    2. https://cmake.org/pipermail/cmake/2007-October/017286.html
     */
    static int GetAvailableThreadNum();

    /*!
     * \brief Set the default omp thread number if necessary
     */
    static void SetDefaultOpenMPThread();

    /*!
     * \brief Set the omp thread number by given thread number
     */
    static void SetOpenMPThread(int n);

    /*!
     * \brief Print status messages for Debug
     */
    static void StatusMessage(const char *msg);
};


/***************** Implementation of template functions ***********************/

/************ template functions of utilsString ****************/
template<typename T>
vector <T> utilsString::SplitStringForValues(string const &item, char delimiter) {
    vector <string> valueStrs = utilsString::SplitString(item, delimiter);
    vector <T> values;
    for (vector<string>::iterator it = valueStrs.begin(); it != valueStrs.end(); it++) {
        values.push_back((T) atof((*it).c_str()));
    }
    vector<T>(values).swap(values);
    return values;
}

template<typename T>
string utilsString::ValueToString(const T& val) {
    ostringstream oss;
    oss << val;
    return oss.str();
}

/************ template functions of utilsArray *****************/

/************ template functions of utilsMath ******************/
template<typename T>
bool utilsMath::FloatEqual(T v1, T v2) {
    if (abs(v1 - v2) < UTIL_ZERO) {
        return true;
    } else {
        return false;
    }
}

template<typename T>
T utilsMath::Max(const T *a, int n) {
    T m = a[0];
    for (int i = 1; i < n; i++) {
        if (a[i] > m) {
            m = a[i];
        }
    }
    return m;
}

template<typename T>
T utilsMath::Sum(int row, const T *data) {
    T tmp = 0;
#pragma omp parallel for reduction(+:tmp)
    for (int i = 0; i < row; i++) {
        tmp += data[i];
    }
    return tmp;
}

template<typename T>
T utilsMath::Sum(int row, int *&idx, const T *data) {
    T tmp = 0;
#pragma omp parallel for reduction(+:tmp)
    for (int i = 0; i < row; i++) {
        int j = idx[i];
        tmp += data[j];
    }
    return tmp;
}

template<typename T>
void utilsMath::basicStatistics(const T *values, int num, double **derivedvalues, T exclude /* = (T) NODATA_VALUE */) {
    double *tmpstats = new double[6];
    double maxv = MISSINGFLOAT;
    double minv = MAXIMUMFLOAT;
    int validnum = 0;
    double sumv = 0.;
    double std = 0.;
    for (int i = 0; i < num; i++) {
        if (utilsMath::FloatEqual(values[i], exclude)) continue;
        if (maxv < values[i]) maxv = values[i];
        if (minv > values[i]) minv = values[i];
        validnum += 1;
        sumv += values[i];
    }
    double mean = sumv / (double) validnum;
#pragma omp parallel for reduction(+:std)
    for (int i = 0; i < num; i++) {
        if (!utilsMath::FloatEqual(values[i], exclude)) {
            std += (values[i] - mean) * (values[i] - mean);
        }
    }
    std = sqrt(std / (double) validnum);
    tmpstats[0] = (double) validnum;
    tmpstats[1] = mean;
    tmpstats[2] = maxv;
    tmpstats[3] = minv;
    tmpstats[4] = std;
    tmpstats[5] = maxv - minv;
    *derivedvalues = tmpstats;
}

template<typename T>
void
utilsMath::basicStatistics(const T * const *values, int num, int lyrs, double ***derivedvalues, T exclude /* = (T) NODATA_VALUE */) {
    double **tmpstats = new double *[6];
    for (int i = 0; i < 6 ; i++) {
        tmpstats[i] = new double[lyrs];
    }
    for (int j = 0; j < lyrs ; j++) {
        tmpstats[0][j] = 0.;                   /// valid number
        tmpstats[1][j] = 0.;                   /// mean
        tmpstats[2][j] = (double)MISSINGFLOAT; /// maximum
        tmpstats[3][j] = (double)MAXIMUMFLOAT; /// minimum
        tmpstats[4][j] = 0.;                   /// std
        tmpstats[5][j] = 0.;                   /// range
    }
    double *sumv = nullptr;
    utilsArray::Initialize1DArray(lyrs, sumv, 0.);
    for (int i = 0; i < num; i++) {
        for (int j = 0; j < lyrs; j++) {
            if (utilsMath::FloatEqual(values[i][j], exclude)) continue;
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
            if (!utilsMath::FloatEqual(values[i][j], exclude)) {
                tmpstd += (values[i][j] - tmpstats[1][j]) * (values[i][j] - tmpstats[1][j]);
            }
        }
        tmpstats[4][j] = tmpstd;
    }
    for (int j = 0; j < lyrs; j++) {
        tmpstats[4][j] = sqrt(tmpstats[4][j] / tmpstats[0][j]);
    }
    utilsArray::Release1DArray(sumv);
    *derivedvalues = tmpstats;
}

/************ template functions of utilsTime ******************/
template<typename T>
bool utilsArray::Initialize1DArray(int row, T *&data, T initialValue) {
    if (nullptr != data) {
        cout << "The input 1D array pointer is not nullptr, without initialized!" << endl;
        return false;
    }
    data = new(nothrow)T[row];
    if (nullptr == data) {
        cout << "Bad memory allocated during 1D array initialization!" << endl;
        return false;
    }
#pragma omp parallel for
    for (int i = 0; i < row; i++) {
        data[i] = initialValue;
    }
    return true;
}

template<typename T>
bool utilsArray::Initialize1DArray(int row, T *&data, const T *iniData) {
    if (nullptr != data) {
        cout << "The input 1D array pointer is not nullptr, without initialized!" << endl;
        return false;
    }
    data = new(nothrow) T[row];
    if (nullptr == data) {
        cout << "Bad memory allocated during 1D array initialization!" << endl;
        return false;
    }
    if (nullptr == iniData) {
        cout << "The input parameter iniData MUST NOT be nullptr!" << endl;
        return false;
    }
#pragma omp parallel for
    for (int i = 0; i < row; i++) {
        data[i] = iniData[i];
    }
    return true;
}

template<typename T>
bool utilsArray::Initialize2DArray(int row, int col, T **&data, T initialValue) {
    if (nullptr != data) {
        cout << "The input 2D array pointer is not nullptr, without initialized!" << endl;
        return false;
    }
    data = new(nothrow) T *[row];
    if (nullptr == data) {
        cout << "Bad memory allocated during 2D array initialization!" << endl;
        return false;
    }
    int badAlloc = 0;
#pragma omp parallel for reduction(+:badAlloc)
    for (int i = 0; i < row; i++) {
        data[i] = new(nothrow) T[col];
        if (nullptr == data[i]) {
            badAlloc++;
        }
        for (int j = 0; j < col; j++) {
            data[i][j] = initialValue;
        }
    }
    if (badAlloc > 0) {
        cout << "Bad memory allocated during 2D array initialization!" << endl;
        utilsArray::Release2DArray(row, data);
        return false;
    }
    return true;
}

template<typename T>
bool utilsArray::Initialize2DArray(int row, int col, T **&data, const T * const *iniData) {
    if (nullptr != data) {
        cout << "The input 2D array pointer is not nullptr, without initialized!" << endl;
        return false;
    }
    data = new(nothrow)T *[row];
    if (nullptr == data) {
        cout << "Bad memory allocated during 2D array initialization!" << endl;
        return false;
    }
    int badAlloc = 0;
    int errorAccess = 0;
#pragma omp parallel for reduction(+:badAlloc, errorAccess)
    for (int i = 0; i < row; i++) {
        data[i] = new(nothrow)T[col];
        if (nullptr == data[i]) {
            badAlloc++;
        }
        if (nullptr == iniData[i]) {
            errorAccess++;
        }
        else {
            for (int j = 0; j < col; j++) {
                data[i][j] = iniData[i][j];
            }
        }
    }
    if (badAlloc > 0) {
        cout << "Bad memory allocated during 2D array initialization!" << endl;
        utilsArray::Release2DArray(row, data);
        return false;
    }
    if (errorAccess > 0) {
        cout << "nullptr pointer existed in iniData during 2D array initialization!" << endl;
        utilsArray::Release2DArray(row, data);
        return false;
    }
    return true;
}

template<typename T>
void utilsArray::Release1DArray(T *&data) {
    if (nullptr != data) {
        delete[] data;
        data = nullptr;
    }
}

template<typename T>
void utilsArray::Release2DArray(int row, T **&data) {
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

template<typename T>
void utilsArray::BatchRelease1DArray(T*& data, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, data);
    utilsArray::Release1DArray(data);
    T* argValue = va_arg(arg_ptr, T*);
    while (nullptr != argValue) {
        utilsArray::Release1DArray(argValue);
        argValue = va_arg(arg_ptr, T*);
    }
    va_end(arg_ptr);
}

template<typename T>
void utilsArray::BatchRelease2DArray(int nrows, T**& data, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, data);
    utilsArray::Release2DArray(nrows, data);
    T** argValue = va_arg(arg_ptr, T**);
    while (nullptr != argValue) {
        utilsArray::Release2DArray(nrows, argValue);
        argValue = va_arg(arg_ptr, T**);
    }
    va_end(arg_ptr);
}

template<typename T>
bool utilsArray::ValueInVector(const T &val, const vector<T> &vec) {
    if (vec.empty()) {
        return false;
    }
    typename vector<T>::iterator findIter = find(vec.begin(), vec.end(), val);
    if (findIter == vec.end()) {
        return false;
    } else {
        return true;
    }
}

template<typename T>
void utilsArray::RemoveValueInVector(T &val, vector<T> &vec) {
    typename vector<T>::iterator Iter = vec.begin();
    for (; Iter != vec.end(); Iter++) {
        if (*Iter == val) {
            Iter = vec.erase(Iter);
        }
        if (Iter == vec.end()) {
            break;
        }
    }
}

#endif /* CLS_UTILS */
