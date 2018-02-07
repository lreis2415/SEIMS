/*!
 * \brief ModelException class
 * \author Liangjun Zhu, Junzhi Liu
 * \version 1.2
 * \note 2018-02-07  lj - Using noexcept if the compiler support
 * \date Feb. 2018
 */
#ifndef MODEL_EXCEPTION
#define MODEL_EXCEPTION

#include <stdexcept>
#include <type_traits>
#include <string>

using namespace std;

#if defined(__clang__) && defined(__apple_build_version__)
// Apple Clang
#if ((__clang_major__ * 100) + __clang_minor__) >= 400 && __has_feature(cxx_noexcept)
#define HAS_NOEXCEPT
#endif /* Apple Clang */
#elif defined(__clang__)
// Clang
#if ((__clang_major__ * 100) + __clang_minor__) >= 304 && __has_feature(cxx_noexcept)
#define HAS_NOEXCEPT
#endif /* Clang */
#elif defined(__INTEL_COMPILER) || defined(__ICC)
// Intel C++
#if (__INTEL_COMPILER >= 1400) && (__INTEL_COMPILER != 9999)
#define HAS_NOEXCEPT
#endif /* Intel C++ */
#elif defined(__GNUC__)
// GNU GCC
#if (__GNUC__ * 100 + __GNUC_MINOR__) >= 406 && (__cplusplus >= 201103L || (defined(__GXX_EXPERIMENTAL_CXX0X__) && __GXX_EXPERIMENTAL_CXX0X__))
#define HAS_NOEXCEPT
#endif /* GCC */
#elif defined(_MSC_VER)
// MS Visual C++
#if _MSC_VER >= 1900
#define HAS_NOEXCEPT
#endif /* Visual C++ */
#endif /* Figure out HAS_NOEXCEPT or not */
#ifdef HAS_NOEXCEPT
#define NOEXCEPT noexcept
#else
#define NOEXCEPT throw()
#endif /* HAS_NOEXCEPT */

/*!
* \ingroup util
* \class ModelException
* \brief Print the exception message
*/
class ModelException : public exception {
public:
    /*!
    * \brief Constructor
    * \param[in] className, functionName, msg
    */
    ModelException(string className, string functionName, string msg) :
        m_runtime_error("Class:" + className + "\n" + "Function:" +
        functionName + "\n" + "Message:" + msg) {};

    /*!
    * \brief Construct error information (string version)
    * \return error information
    */
    string toString() { return m_runtime_error.what(); };

    /*!
    * \brief Overload function to construct error information
    * \return \a char* error information
    */
    const char *what() NOEXCEPT { return m_runtime_error.what(); };

private:
    runtime_error m_runtime_error;
};
#endif /* MODEL_EXCEPTION */
