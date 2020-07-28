/*!
 * \file basic.h
 * \brief Basic definitions.
 *        Part of the Common Cross-platform Geographic Library (CCGL)
 *
 * Changelog:
 *   - 1. 2018-05-02 - lj - Initially implementation.
 *   - 2. 2018-06-21 - lj - Test on Intel C++ compiler.
 *   - 3. 2018-08-21 - lj - Doxygen comment style check.
 *
 * \author Liangjun Zhu (crazyzlj)
 * \version 1.1
 */
#ifndef CCGL_BASIC_H
#define CCGL_BASIC_H

/*! `NDEBUG` or `_DEBUG` mean not build on `DEBUG` mode. */
#ifndef NDEBUG
#ifndef _DEBUG
#define _DEBUG
#endif /* _DEBUG */
#endif /* NDEBUG */

/*! A reference to x64 architecture */
#if defined(_WIN64) || defined(__x86_64) || defined(__LP64__)
#define CPP_64
#endif

/*! A reference to MSVC environment */
#if defined _MSC_VER
#define CPP_MSVC
#endif /* _MSC_VER */

/*! A reference to Intel C++ compiler */
#if defined(__INTEL_COMPILER) || defined(__ICL) || defined(__ICC)
#define CPP_ICC
#endif /* __INTEL_COMPILER */

/*! A reference to GCC compiler */
#if defined(__GNUC__)
#define CPP_GCC
/*! A reference to GCC compiler on macOS */
#if defined(__APPLE__)
#define CPP_APPLE
#endif /* __APPLE__ */
#endif /* __GNUC__ */

#include <memory>
#include <stdexcept>
#include <cfloat>
#include <map>
#include <string>
#include <cstring> // strcasecmp in GCC
/// platform
#if defined WINDOWS
// For MSVC and MINGW64 in Windows OS
// #define _WINSOCKAPI_    // In order to stop windows.h including winsock.h
// _WINSOCKAPI_ is defined by <winsock2.h>
#include <winsock2.h>
#include <windows.h>
#endif /* WINDOWS */

#if defined CPP_GCC
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <cerrno>
#endif /* CPP_GCC */

using std::string;

// define some macro for string related built-in functions
#ifdef CPP_MSVC
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
#endif /* CPP_MSVC */

#if defined(__MINGW32_MAJOR_VERSION) || defined(__MINGW64_VERSION_MAJOR)
#define MINGW
#endif

#if defined(MINGW) || defined(_MSC_VER)
#define strcasecmp _stricmp
#endif /* MINGW or MSVC */

#if defined(__clang__) && defined(__apple_build_version__)
// Apple Clang
#if ((__clang_major__ * 100) + __clang_minor__) >= 400
#if __has_feature(cxx_noexcept)
#define HAS_NOEXCEPT
#endif /* NOEXCEPT */
#if __has_feature(cxx_override_control)
#define HAS_OVERRIDE
#endif /* OVERRIDE */
#endif /* Apple Clang */
#elif defined(__clang__)
// Clang
#if ((__clang_major__ * 100) + __clang_minor__) >= 304
#if __has_feature(cxx_noexcept)
#define HAS_NOEXCEPT
#endif /* NOEXCEPT */
#if __has_feature(cxx_override_control)
#define HAS_OVERRIDE
#endif /* OVERRIDE */
#if __has_feature(cxx_variadic_templates)
#define HAS_VARIADIC_TEMPLATES
#endif /* VARIADIC_TEMPLATES */
#endif /* Clang */
#elif defined(CPP_ICC)
// Intel C++
#if ((__INTEL_COMPILER >= 1400) && (__INTEL_COMPILER != 9999)) || (__ICL >= 1400)
#define HAS_NOEXCEPT
#define HAS_OVERRIDE
#define HAS_VARIADIC_TEMPLATES
#endif /* Intel C++ */
#elif defined(CPP_GCC)
// GNU GCC
#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 406 && (__cplusplus >= 201103L || (defined(__GXX_EXPERIMENTAL_CXX0X__) && __GXX_EXPERIMENTAL_CXX0X__))
#define HAS_NOEXCEPT
#define HAS_OVERRIDE
#define HAS_VARIADIC_TEMPLATES
#endif /* GCC */
#elif defined(_MSC_VER)
// MS Visual C++
#if _MSC_VER >= 1900
#define HAS_NOEXCEPT
#endif /* Visual Studio 2015 or later */
#if _MSC_VER >= 1800
#define HAS_VARIADIC_TEMPLATES
#endif /* Visual Studio 2013 or later */
#if _MSC_VER>= 1600
#define HAS_OVERRIDE
#endif /* Visual Studio 2010 or later */
#endif /* Figure out HAS_NOEXCEPT, HAS_VARIADIC_TEMPLATES, and HAS_OVERRIDE or not */

/*! A compatible reference to `noexcept` or `throw()` if not supported by the compiler. */
#ifdef HAS_NOEXCEPT
#define NOEXCEPT noexcept
#else
#define NOEXCEPT throw()
#endif /* HAS_NOEXCEPT */

/*! A compatible reference to `override` or blank if not supported by the compiler. */
#ifdef HAS_OVERRIDE
#define OVERRIDE override
#else
#define OVERRIDE
#endif /* HAS_OVERRIDE */

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

#ifdef USE_GDAL
/* Ignore warning on Windows MSVC compiler caused by GDAL.
* refers to http://blog.csdn.net/liminlu0314/article/details/8227518
*/
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(disable: 4100 4190 4251 4275 4305 4309 4819 4996)
#endif /* Ignore warnings of GDAL */
#endif /* USE_GDAL */

/*!
 * \namespace ccgl
 * \brief Common Cross-platform Geographic Library (CCGL)
 */
namespace ccgl {
#if defined CPP_MSVC
/// x86 and x64 Compatibility
/// 1-byte (8-bit) signed integer
typedef signed __int8 vint8_t;
/// 1-byte (8-bit) unsigned integer
typedef unsigned __int8 vuint8_t;
/// 2-byte (16-bit) signed integer
typedef signed __int16 vint16_t;
/// 2-byte (16-bit) unsigned integer
typedef unsigned __int16 vuint16_t;
/// 4-byte (32-bit) signed integer
typedef signed __int32 vint32_t;
/// 4-byte (32-bit) unsigned integer
typedef unsigned __int32 vuint32_t;
/// 8-byte (64-bit) signed integer
typedef signed __int64 vint64_t;
/// 8-byte (64-bit) unsigned integer
typedef unsigned __int64 vuint64_t;

#else
typedef          int8_t            vint8_t;
typedef          uint8_t           vuint8_t;
typedef          int16_t           vint16_t;
typedef          uint16_t          vuint16_t;
typedef          int32_t           vint32_t;
typedef          uint32_t          vuint32_t;
typedef          int64_t           vint64_t;
typedef          uint64_t          vuint64_t;
#endif

#ifdef CPP_64
typedef vint64_t vint;
typedef vint64_t vsint;
typedef vuint64_t vuint;
#else
typedef vint32_t vint;
typedef vint32_t vsint;
typedef vuint32_t vuint;
#endif
/// Signed integer representing position.
typedef vint64_t pos_t;

///
/// Global utility definitions
///

/*! Default NoData value for raster data etc. */
#ifndef NODATA_VALUE
#define NODATA_VALUE    (-9999.0f)
#endif /* NODATA_VALUE */

/*! Missing float value */
#ifndef MISSINGFLOAT
#define MISSINGFLOAT    (-1 * FLT_MAX)
#endif /* MISSINGFLOAT */

/*! Maximum float value */
#ifndef MAXIMUMFLOAT
#define MAXIMUMFLOAT    FLT_MAX
#endif /* MAXIMUMFLOAT */

/*! Maximum length of full file path */
#ifndef PATH_MAX
#define PATH_MAX        1024
#endif /* PATH_MAX */

/*! A approximation of Zero */
#ifndef UTIL_ZERO
#define UTIL_ZERO       1.0e-6f
#endif /* UTIL_ZERO */

/*! A approximation of PI */
#ifndef PI
#define PI              3.14159265358979323846f
#endif /* PI */

/*! Minimum slope(radian) value */
#ifndef MINI_SLOPE
#define MINI_SLOPE      0.0001f
#endif /* MINI_SLOPE */

#ifdef WINDOWS
#define SEP             "\\"
#ifndef MSVC
#define LIBPREFIX       "lib"
#endif
#define LIBSUFFIX       ".dll"
#else
#define SEP             "/"
#define LIBPREFIX       "lib"
#endif /* Windows */
#ifdef LINUX
#define LIBSUFFIX       ".so"
#elif defined MACOSX
#define LIBSUFFIX       ".dylib"
#endif /* Linux and macOS */

/*! A reference to the postfix of executable file for DEBUG mode */
#ifdef _DEBUG
#define POSTFIX         "d"
#endif
/*! A reference to the postfix of executable file for RELWITHDEBINFO mode */
#ifdef RELWITHDEBINFO
#define POSTFIX         "rd"
#endif
/*! A reference to the postfix of executable file for MINSIZEREL mode */
#ifdef MINSIZEREL
#define POSTFIX         "s"
#endif
/*! A reference to the postfix of executable file for RELEASE mode */
#ifndef POSTFIX
#define POSTFIX         ""
#endif

///
/// Use static_cast<T>(a) instead (T)a or T(a) to convert datetypes
///

/*! Convert to integer `int` */
#define CVT_INT(param)   static_cast<int>((param))
/*! Convert to size_t `size_t` */
#define CVT_SIZET(param) static_cast<size_t>((param))
/*! Convert to float `float` */
#define CVT_FLT(param)   static_cast<float>((param))
/*! Convert to double `double` */
#define CVT_DBL(param)   static_cast<double>((param))
/*! Convert to time_t `time_t` */
#define CVT_TIMET(param) static_cast<time_t>((param))
/*! Convert to char `char` */
#define CVT_CHAR(param)  static_cast<char>((param))
/*! Convert to string `string` */
#define CVT_STR(param)   static_cast<string>((param))

/*! Convert to 8-byte (64-bit) signed integer `vint` */
#define CVT_VINT(param)  static_cast<vint>((param))
/*! Convert to 8-byte (64-bit) signed integer `vsint` */
#define CVT_VSINT(param) static_cast<vsint>((param))
/*! Convert to 8-byte (64-bit) unsigned integer `vuint` */
#define CVT_VUINT(param) static_cast<vuint>((param))
/*! Convert to 8-byte (64-bit) unsigned integer `vuint64_t` */
#define CVT_VUINT64(param) static_cast<vuint64_t>((param))


typedef std::map<string, string> STRING_MAP;

#ifdef CPP_64
#define ITOA_S		_i64toa_s
#define ITOW_S		_i64tow_s
#define I64TOA_S	_i64toa_s
#define I64TOW_S	_i64tow_s
#define UITOA_S		_ui64toa_s
#define UITOW_S		_ui64tow_s
#define UI64TOA_S	_ui64toa_s
#define UI64TOW_S	_ui64tow_s
#else
#define ITOA_S		_itoa_s
#define ITOW_S		_itow_s
#define I64TOA_S	_i64toa_s
#define I64TOW_S	_i64tow_s
#define UITOA_S		_ui64toa_s
#define UITOW_S		_ui64tow_s
#define UI64TOA_S	_ui64toa_s
#define UI64TOW_S	_ui64tow_s
#endif

/*!
 * \class NotCopyable
 * \brief Base class for classes that cannot be copied. By inheriting this
 *        class you can disable copying of your classes.
 *
 * \code
 *   class myClass: private NotCopyable {}
 *   // or
 *   class myClass: NotCopyable {}
 * \endcode
 */
class NotCopyable {
private:
    NotCopyable(const NotCopyable&);

    NotCopyable& operator=(const NotCopyable&);
public:
    NotCopyable();
};

/*!
 * \class Object
 * \brief Base of all classes.
 */
class Object {
public:
    virtual ~Object();
};

/*!
 * \class Interface
 * \brief Base type of all interfaces. All interface types are encouraged to be virtual inherited.
 */
class Interface: NotCopyable {
public:
    virtual ~Interface();
};

/*!
 * \class ModelException
 * \brief Print the exception message
 */
class ModelException: public std::exception {
public:
    /*!
     * \brief Constructor
     * \param[in] class_name
     * \param[in] function_name
     * \param[in] msg
     */
    ModelException(const string& class_name, const string& function_name, const string& msg);

    /*!
     * \brief Construct error information (string version)
     * \return error information
     */
    string ToString();

    /*!
     * \brief Overload function to construct error information
     * \return \a char* error information
     */
    const char* what() const NOEXCEPT OVERRIDE;

private:
    std::runtime_error runtime_error_;
};

/*!
 * \brief Check if the IP address is valid.
 * \param[in] ip \a char* IP address.
 */
bool IsIpAddress(const char* ip);

/*!
 * \brief Writes an entry to the log file. Normally only used for debug
 * \param[in] msg \a string log message
 * \param[in] logpath \a string Optional
 */
void Log(const string& msg, const string& logpath = "debugInfo.log");

/*!
 * \brief Detect the available threads number
 *
 * Reference:
 *   - 1. http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
 *   - 2. https://cmake.org/pipermail/cmake/2007-October/017286.html
 */
int GetAvailableThreadNum();

/*!
 * \brief Set the default omp thread number if necessary
 */
void SetDefaultOpenMPThread();

/*!
 * \brief Set the omp thread number by given thread number
 * \param[in] n Thread number greater than 1.
 */
void SetOpenMPThread(int n);

/*!
 * \brief Print status messages for Debug
 * \param[in] msg \a char* Message
 */
void StatusMessage(const char* msg);

/*!
 * \brief Sleep milliseconds
 * \param[in] millisecs Sleep timespan.
 */
inline void SleepMs(const int millisecs) {
#ifdef WINDOWS
    Sleep(millisecs);
#else
    usleep(millisecs * 1000);   // usleep takes sleep time_funcs in us (1 millionth of a second)
#endif
}

} /* namespace ccgl */
#endif /* CCGL_BASIC_H */
