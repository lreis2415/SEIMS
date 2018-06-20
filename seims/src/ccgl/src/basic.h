/*!
 * \brief Basic definitions.
 *        Part of the Common Cross-platform Geographic Library (CCGL)
 *
 * \author Liangjun Zhu (crazyzlj)
 * \version 1.0
 * \changelog  2018-05-02 - lj - Initially implementation.\n
 */
#ifndef CCGL_BASIC_H
#define CCGL_BASIC_H

#ifndef NDEBUG
#ifndef _DEBUG
#define _DEBUG
#endif /* _DEBUG */
#endif /* NDEBUG */

/// Architecture
#if defined _WIN64 || __x86_64 || __LP64__
#define CPP_64
#endif

#if defined _MSC_VER
#define CPP_MSVC
#else
#define CPP_GCC
#if defined(__APPLE__)
#define CPP_APPLE
#endif
#endif

#include <memory>
#include <stdexcept>
#include <cfloat>
#include <string>
#include <cstring> // strcasecmp in GCC
/// platform
#if defined windows
// For MSVC and MINGW64 in Windows OS
// #define _WINSOCKAPI_    // In order to stop windows.h including winsock.h
// _WINSOCKAPI_ is defined by <winsock2.h>
#include <winsock2.h>
#include <windows.h>
#endif /* windows */
#if defined CPP_GCC
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#endif /* CPP_GCC */

using std::string;

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

#if defined(__MINGW32_MAJOR_VERSION) || defined(__MINGW64_VERSION_MAJOR) || defined(_MSC_VER)
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
#elif defined(__INTEL_COMPILER) || defined(__ICC)
// Intel C++
#if (__INTEL_COMPILER >= 1400) && (__INTEL_COMPILER != 9999)
#define HAS_NOEXCEPT
#define HAS_OVERRIDE
#define HAS_VARIADIC_TEMPLATES
#endif /* Intel C++ */
#elif defined(__GNUC__)
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

#ifdef HAS_NOEXCEPT
#define NOEXCEPT noexcept
#else
#define NOEXCEPT throw()
#endif /* HAS_NOEXCEPT */

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
 * \brief Common Cross-platform Geographic Library
 */
namespace ccgl {
/// x86 and x64 Compatibility
#if defined CPP_MSVC
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

#elif defined CPP_GCC
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
typedef          vint64_t          vint;
typedef          vint64_t          vsint;
typedef          vuint64_t         vuint;
#else
typedef vint32_t vint;
typedef vint32_t vsint;
typedef vuint32_t vuint;
#endif
/// Signed integer representing position.
typedef vint64_t pos_t;

/*!
 * Global utility definitions
 */
#ifndef NODATA_VALUE
#define NODATA_VALUE    (-9999.0f)
#endif /* NODATA_VALUE */

#ifndef MISSINGFLOAT
#define MISSINGFLOAT    (-1 * FLT_MAX)
#endif /* MISSINGFLOAT */

#ifndef MAXIMUMFLOAT
#define MAXIMUMFLOAT    FLT_MAX
#endif /* MAXIMUMFLOAT */

#ifndef PATH_MAX
#define PATH_MAX        1024
#endif /* PATH_MAX */

#ifndef UTIL_ZERO
#define UTIL_ZERO       1.0e-6f
#endif /* UTIL_ZERO */

#ifndef PI
#define PI              3.14159265358979323846f
#endif /* PI */

#ifndef MINI_SLOPE
#define MINI_SLOPE      0.0001f
#endif /* MINI_SLOPE */

#ifdef windows
#define SEP             "\\"
#ifndef MSVC
#define LIBPREFIX       "lib"
#endif
#define LIBSUFFIX       ".dll"
#else
#define SEP             "/"
#define LIBPREFIX       "lib"
#endif /* windows */
#ifdef linux
#define LIBSUFFIX       ".so"
#elif defined macos
#define LIBSUFFIX       ".dylib"
#endif /* linux and macOS */

#ifdef _DEBUG
#define POSTFIX         "d"
#endif
#ifdef RELWITHDEBINFO
#define POSTFIX         "rd"
#endif
#ifdef MINSIZEREL
#define POSTFIX         "s"
#endif
#ifndef POSTFIX
#define POSTFIX         ""
#endif

/*!
* Use static_cast<T>(a) instead (T)a or T(a) to convert datetypes
*/
#define CVT_INT(param)   static_cast<int>((param))
#define CVT_SIZET(param) static_cast<size_t>((param))
#define CVT_FLT(param)   static_cast<float>((param))
#define CVT_DBL(param)   static_cast<double>((param))
#define CVT_TIMET(param) static_cast<time_t>((param))
#define CVT_CHAR(param)  static_cast<char>((param))
#define CVT_STR(param)   static_cast<string>((param))

#define CVT_VINT(param)  static_cast<vint>((param))
#define CVT_VSINT(param) static_cast<vsint>((param))
#define CVT_VUINT(param) static_cast<vuint>((param))
#define CVT_VUINT64(param) static_cast<vuint64_t>((param))

/*!
 * \class NotCopyable
 * \brief Base class for classes that cannot be copied. By inheriting this
 *        class you can disable copying of your classes.
 *        e.g., class myClass: private NotCopyable {}
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
 * reference:
 *    1. http://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
 *    2. https://cmake.org/pipermail/cmake/2007-October/017286.html
 */
int GetAvailableThreadNum();

/*!
 * \brief Set the default omp thread number if necessary
 */
void SetDefaultOpenMPThread();

/*!
 * \brief Set the omp thread number by given thread number
 */
void SetOpenMPThread(int n);

/*!
 * \brief Print status messages for Debug
 */
void StatusMessage(const char* msg);

/*!
 * \brief Sleep milliseconds
 */
inline void SleepMs(const int millisecs) {
#ifdef windows
    Sleep(millisecs);
#else
    usleep(millisecs * 1000);   // usleep takes sleep time_funcs in us (1 millionth of a second)
#endif
}

} /* namespace ccgl */
#endif /* CCGL_BASIC_H */
