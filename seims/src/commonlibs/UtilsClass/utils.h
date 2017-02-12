/*!
 * \brief Utilities class to handle string, date time, basic mathematics, and file.
 * classes includes: utilsTime, utilsString, utilsArray, utilsMath, utilsFileIO.
 *
 * \author Junzhi Liu, Liangjun Zhu
 * \version 2.0
 * \date Jul. 2010
 * \revised Dec. 2016
 * 
 */

#ifndef CLS_UTILS
#define CLS_UTILS
/// OpenMP support
#ifdef SUPPORT_OMP
#include <omp.h>
#endif
/// math and STL headers
#include <cmath>
#include <cfloat>
#include <vector>
#include <algorithm>
#include <iterator>
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
#ifdef windows
#include <io.h>
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <windows.h>
#include <winsock2.h>
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
#endif

using namespace std;

/*!
 * Global utility definitions
 */
/**
 * \def NODATA_VALUE
 * \brief NODATA value
 */
#ifndef NODATA_VALUE
#define NODATA_VALUE    -9999.0f
#endif
const float MISSINGFLOAT = -1 * FLT_MAX;
const float MAXIMUMFLOAT  = FLT_MAX;
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
/**
 * \def ZERO
 * \brief zero value used in numeric calculation
 */
#ifndef UTIL_ZERO
#define UTIL_ZERO		1.0e-6f
#endif
/**
 * \def PI
 * \brief PI value used in numeric calculation
 */
#ifndef PI
#define PI				3.14159265358979323846f
#endif
/**
 * \def MINI_SLOPE
 * \brief Minimum slope gradient
 */
#ifndef MINI_SLOPE
#define MINI_SLOPE		0.0001f
#endif

#ifdef windows
#define Tag_ModuleDirectoryName "\\"
#define SEP "\\"
#define Tag_DyLib ".dll"
#else
#define Tag_ModuleDirectoryName "/"
#define SEP "/"
#define Tag_So "lib"
#endif
#ifdef linux
#define Tag_DyLib ".so"
#elif defined macos
#define Tag_DyLib ".dylib"
#elif defined macosold
#define Tag_DyLib ".dylib"
#endif

// define some macro for string related built-in functions
#ifdef MSVC
#define stringcat strcat_s
#define stringcpy strcpy_s
#define strprintf sprintf_s
#define StringTok strtok_s
#define StringScanf sscanf_s
#else
#define stringcat strcat
#define stringcpy strcpy
#define strprintf snprintf
#define StringTok strtok_r
#define StringScanf sscanf
#endif

static int daysOfMonth[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*!
 * \class utilsTime
 * \brief Time related functions
 */
class utilsTime
{
public:
	utilsTime(void);   //< void constructor
	~utilsTime(void);  //< void destructor
	/*
	 *\brief Precisely and cross-platform time counting function. 
	 */
	static double TimeCounting();
	/*!
	 *\brief Check the given year is a leap year or not.
	 */
	static bool isLeapYear(int yr) { 
		return ((yr % 4) == 0 && ((yr % 100) != 0 || (yr % 400) == 0)); }
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
    static time_t ConvertToTime(string strDate, string const& format, bool includeHour);

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
    static time_t ConvertToTime2(string const& strDate, const char *format, bool includeHour);

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
class utilsString
{
public:
	utilsString(void);   //< void constructor
	~utilsString(void);  //< void destructor
	/*!
	 * \brief Get Uppercase of given string
	 * \param[in] string
	 * \return Uppercase string
	*/
	static string GetUpper(string const& );
	/*!
	 * \brief Match \a char ignore cases
	 *
	 * \param[in] a, b \a char* 
	 * \return true or false
	 * \sa StringMatch()
	 */
	static bool StringMatch(const char *, const char *);

	/*!
	 * \brief Match Strings in UPPERCASE manner
	 * \param[in] text1, text2
	 * \return true or false
	 */
	static bool StringMatch(string const& text1, string const& text2);
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
    static vector<string> SplitString(string const& item);
	/*!
     * \brief Splits the given string based on the given delimiter
     *
     * \param[in] item \a string information
     * \param[in] delimiter \a char
     * \return The split strings vector
     */
    static vector<string> SplitString(string const& item, char delimiter);

    /*
     * \brief Get numeric values by splitting the given string based on the given delimiter
     */
    template<typename T>
    vector<T> SplitStringForValues(string const& item, char delimiter);

    /*
     * \brief Get int values by splitting the given string based on the given delimiter
     */
    static vector<int> SplitStringForInt(string const& item, char delimiter);

    /*
     * \brief Get float values by splitting the given string based on the given delimiter
     */
    static vector<float> SplitStringForFloat(string const& item, char delimiter);

	/*!
	 * \brief Convert value to string
	 * \param[in] val value, e.g., a int, or float
	 * \return converted string
	 */
	template<typename T>
	static string ValueToString(T val);
};

/*!
 * \class utilsArray
 * \brief Array related functions include vector and pointer array.
 */
class utilsArray {
public:
	utilsArray(void);   //< void constructor
	~utilsArray(void);  //< void destructor
	/*!
	 * \brief Initialize DT_Array1D data
	 *
	 * \param[in] row
	 * \param[in] data
	 * \param[in] initialValue
	 */
	template<typename T>
	static void Initialize1DArray(int row, T *&data, T initialValue);
	/*!
	 * \brief Initialize DT_Array1D data based on an existed array
	 *
	 * \param[in] row
	 * \param[in] data
	 * \param[in] iniData
	 */
	template<typename T>
	static void Initialize1DArray(int row, T *&data, T *&iniData);
	/*!
	 * \brief Initialize DT_Array2D data
	 *
	 * \param[in] row
	 * \param[in] col
	 * \param[in] data
	 * \param[in] initialValue
	 */
	template<typename T>
	static void Initialize2DArray(int row, int col, T **&data, T initialValue);
	/*!
	 * \brief Initialize DT_Array2D data based on an existed array
	 *
	 * \param[in] row
	 * \param[in] col
	 * \param[in] data
	 * \param[in] iniData
	 */
	template<typename T>
	static void Initialize2DArray(int row, int col, T **&data, T **&iniData);
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
	 * \brief Write 1D array to a file
	 *
	 * \sa Read1DArrayFromTxtFile(), Read2DArrayFromTxtFile(), Output2DArrayToTxtFile()
	 *
	 * \param[in] n, data, filename
	 */
	static void Output1DArrayToTxtFile(int n, float *data, const char *filename);

	/*!
	 * \brief Write 2D array to a file
	 *
	 * \sa Read1DArrayFromTxtFile(), Read2DArrayFromTxtFile(), Output1DArrayToTxtFile()
	 *
	 * \param[in] nRows, nCols, data, filename
	 */
	static void Output2DArrayToTxtFile(int nRows, int nCols, float **data, const char *filename);

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
	static bool ValueInVector(T &val, vector<T> &vec);
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
class utilsMath
{
public:
	utilsMath(void);   //< void constructor
	~utilsMath(void);  //< void destructor
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
	static T Max(T *a, int n);
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
	static T Sum(int row, T *&data);
	/*!
	 * \brief Sum of a numeric array
	 * Get sum value of a double array with size row and real index idx.
	 * \param[in] row
	 * \param[in] idx
	 * \param[in] data
	 * \return sum
	 */
	template<typename T>
	static T Sum(int row, int *&idx, T *&data);
	/*!
	 * \brief calculate basic statistics at one time
	 * \param[in] values data array
	 * \param[in] num data length
	 * \param[out] derivedvalues \double array, value number, mean, max, min, std, range
	 * \param[in] exclude optional, excluded value, e.g. NoDATA, the default is -9999
	 */
	template<typename T>
	static void basicStatistics(T *values, int num, double **derivedvalues, T exclude = (T) NODATA_VALUE);
	/*!
	 * \brief calculate basic statistics at one time for 2D raster data
	 * \param[in] values data array
	 * \param[in] num data length
	 * \param[in] lyrs layer number
	 * \param[out] derivedvalues \double array, value number, mean, max, min, std, range
	 * \param[in] exclude optional, excluded value, e.g. NoDATA, the default is -9999
	 */
	template<typename T>
	static void basicStatistics(T **values, int num, int lyrs, double ***derivedvalues, T exclude = (T) NODATA_VALUE);
};

/*!
 * \class utilsFileIO
 * \brief File Input and output related functions
 */
class utilsFileIO
{
public:
	utilsFileIO(void);   //< void constructor
	~utilsFileIO(void);  //< void destructor
#ifndef windows
	/*!
	 * \brief Copy file in unix-based platform
	 * \param[in] srcfile \a char source file path
	 * \param[in] dstfile \a char destination file path
	 */
	static int copyfile_unix(const char* srcfile, const char* dstfile);
#endif
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
	static string GetCoreFileName(string const& fullFileName);

	/*!
	 * \brief Return the suffix of a given file's path
	 *
	 * \param[in] fullFileName 
	 * \return Suffix
	 * \sa GetPathFromFullName
	 */
	static string GetSuffix(string const& fullFileName);

	/*!
	 * \brief Replace the suffix by a given suffix
	 *
	 * \param[in] fullFileName 
	 * \param[in] newSuffix
	 * \return new fullFileName
	 */
	static string ReplaceSuffix(string const& fullFileName, string const& newSuffix);

	/*!
	 * \brief Get Path From full file path string
	 *
	 * \param[in] fullFileName \a string
	 * \return filePath string
	 * \sa GetCoreFileName
	 */
	static string GetPathFromFullName(string const& fullFileName);
	/*!
     * \brief Return a flag indicating if the given file exists
     *
     * \param[in] FileName String path of file
     * \return True if Exists, and false if not.
     */
    static bool FileExists(string const& FileName);
	/*!
     * \brief Return a flag indicating if the given path exists
     *
     * \param[in] path String path
     * \return True if Exists, and false if not.
     */
    static bool PathExists(string const& path);
	/*!
	 * \brief Delete the given file if existed.
	 * \param[in] filepath \string
	 * \return 0 if deleted successful, else return nonzero value, e.g. -1.
	 */
	static int DeleteExistedFile(string const& filepath);
	/*!
	 * \brief Find files in given paths
	 * \param[in] lpPath, expression
	 * \param[out] vecFiles
	 * \return 0 means success
	 */
	static int FindFiles(const char *lpPath, const char *expression, vector<string> &vecFiles);
};

/*!
 * \class utils
 * \brief Utility functions excluding time, string, math, and file IO.
 * For example, omp thread setting.
 */
class utils
{
public:
	utils(void);   //< void constructor
	~utils(void);  //< void destructor
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
	 * \brief Print status messages for Debug
	 */
	static void StatusMessage(const char *msg);
};


/***************** Implementation of template functions ***********************/

/************ template functions of utilsString ****************/
template<typename T>
vector<T> utilsString::SplitStringForValues(string const& item, char delimiter)
{
	vector<string> valueStrs = utilsString::SplitString(item, delimiter);
	vector<T> values;
	for (vector<string>::iterator it = valueStrs.begin(); it != valueStrs.end(); it++)
		values.push_back((T)atof((*it).c_str()));
	vector<T>(values).swap(values);
	return values;
}

template<typename T>
string utilsString::ValueToString(T val)
{
	ostringstream oss;
	oss << val;
	return oss.str();
}

/************ template functions of utilsArray *****************/

/************ template functions of utilsMath ******************/
template<typename T>
bool utilsMath::FloatEqual(T v1, T v2){
	if (abs(v1 - v2) < UTIL_ZERO)
		return true;
	else
		return false;
}

template<typename T>
T utilsMath::Max(T *a, int n)
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

template<typename T>
T utilsMath::Sum(int row, T *&data)
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
T utilsMath::Sum(int row, int *&idx, T *&data)
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

template<typename T>
void utilsMath::basicStatistics(T *values, int num, double **derivedvalues, T exclude /* = (T) NODATA_VALUE */){
	double *tmpstats = new double[6];
	double maxv = MISSINGFLOAT;
	double minv = MAXIMUMFLOAT;
	int validnum = 0;
	double sumv = 0.;
	double std = 0.;
	for (int i = 0; i < num; i++)
	{
		if (utilsMath::FloatEqual(values[i], exclude)) continue;
		if (maxv < values[i]) maxv = values[i];
		if (minv > values[i]) minv = values[i];
		validnum += 1;
		sumv += values[i];
	}
	double mean = sumv / (double)validnum;
#pragma omp parallel for reduction(+:std)
	for (int i = 0; i < num; i++)
	{
		if (!utilsMath::FloatEqual(values[i], exclude))
			std += (values[i] - mean) * (values[i] - mean);
	}
	std = sqrt(std / (double)validnum);
	tmpstats[0] = (double)validnum;
	tmpstats[1] = mean;
	tmpstats[2] = maxv;
	tmpstats[3] = minv;
	tmpstats[4] = std;
	tmpstats[5] = maxv - minv;
	*derivedvalues = tmpstats;
}

template<typename T>
void utilsMath::basicStatistics(T **values, int num, int lyrs, double ***derivedvalues, T exclude /* = (T) NODATA_VALUE */){
	double **tmpstats = new double *[6];
	double *maxv = NULL;
	utilsArray::Initialize1DArray(lyrs, maxv, (double)MISSINGFLOAT);
	double *minv = NULL;
	utilsArray::Initialize1DArray(lyrs, minv, (double)MAXIMUMFLOAT);
	double *validnum = NULL;
	utilsArray::Initialize1DArray(lyrs, validnum, 0.);
	double *sumv = NULL;
	utilsArray::Initialize1DArray(lyrs, sumv, 0.);
	double *std = NULL;
	utilsArray::Initialize1DArray(lyrs, std, 0.);
	double *range = NULL;
	utilsArray::Initialize1DArray(lyrs, range, 0.);
	for (int i = 0; i < num; i++)
	{
		for (int j = 0; j < lyrs; j++)
		{
			if (utilsMath::FloatEqual(values[i][j], exclude)) continue;
			if (maxv[j] < values[i][j]) maxv[j] = values[i][j];
			if (minv[j] > values[i][j]) minv[j] = values[i][j];
			validnum[j] += 1;
			sumv[j] += values[i][j];
		}
	}
	double *mean = new double[lyrs];
	for (int j = 0; j < lyrs; j++){
		range[j] = maxv[j] - minv[j];
		mean[j] = sumv[j] / validnum[j];
	}
	for (int j = 0; j < lyrs; j++)
	{
		double tmpstd = 0;
#pragma omp parallel for reduction(+:tmpstd)
		for (int i = 0; i < num; i++){
			if (!utilsMath::FloatEqual(values[i][j], exclude))
				tmpstd += (values[i][j] - mean[j]) * (values[i][j] - mean[j]);
		}
		std[j] = tmpstd;
	}
	for (int j = 0; j < lyrs; j++){
		std[j] = sqrt(std[j] / validnum[j]);
	}
	
	tmpstats[0] = validnum;
	tmpstats[1] = mean;
	tmpstats[2] = maxv;
	tmpstats[3] = minv;
	tmpstats[4] = std;
	tmpstats[5] = range;
	*derivedvalues = tmpstats;
}
/************ template functions of utilsTime ******************/
template<typename T>
void utilsArray::Initialize1DArray(int row, T *&data, T initialValue)
{
	data = new T[row];
#pragma omp parallel for
	for (int i = 0; i < row; i++)
		data[i] = initialValue;
}

template<typename T>
void utilsArray::Initialize1DArray(int row, T *&data, T *&iniData)
{
	data = new T[row];
#pragma omp parallel for
	for (int i = 0; i < row; i++)
		data[i] = iniData[i];
}

template<typename T>
void utilsArray::Initialize2DArray(int row, int col, T **&data, T initialValue)
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

template<typename T>
void utilsArray::Initialize2DArray(int row, int col, T **&data, T **&iniData)
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

template<typename T>
void utilsArray::Release1DArray(T *&data)
{
	delete[] data;
	data = NULL;
}

template<typename T>
void utilsArray::Release2DArray(int row, T **&data)
{
#pragma omp parallel for
	for (int i = 0; i < row; i++)
	{
		if (data[i] != NULL){
			delete[] data[i];
			data[i] = NULL;
		}
	}
	delete[] data;
	data = NULL;
}
template<typename T>
bool utilsArray::ValueInVector(T &val, vector<T> &vec)
{
	typename vector<T>::iterator findIter = find(vec.begin(), vec.end(), val);
	if (findIter == vec.end())
		return false;
	else
		return true;
}

template<typename T>
void utilsArray::RemoveValueInVector(T &val, vector<T> &vec)
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

#endif