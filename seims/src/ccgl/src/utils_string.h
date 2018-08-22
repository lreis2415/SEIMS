/*!
 * \file utils_string.h
 * \brief Handling string related issues.
 *        Part of the Common Cross-platform Geographic Library (CCGL)
 *
 * Changelog:
 *   - 1. 2018-05-02 - lj - Make part of CCGL.
 *
 * \author Liangjun Zhu (crazyzlj)
 * \version 1.0
 */
#ifndef CCGL_UTILS_STRING_H
#define CCGL_UTILS_STRING_H

#include <sstream>
#include <vector>

#include "basic.h"

using std::string;
using std::vector;

namespace ccgl {
/*!
 * \namespace ccgl::utils_string
 * \brief String related functions
 */
namespace utils_string {
/*!
 * \brief Get Uppercase of given string
 * \param[in] str
 * \return Uppercase string
 */
string GetUpper(const string& str);

/*!
 * \brief Match \a char ignore cases
 * \param[in] a, b \a char*
 * \return true or false
 * \sa StringMatch()
 */
bool StringMatch(const char* a, const char* b);

/*!
 * \brief Match Strings in UPPERCASE manner
 * \param[in] text1, text2
 * \return true or false
 */
bool StringMatch(const string& text1, const string& text2);

/*!
 * \brief Trim Both leading and trailing spaces
 * \sa Trim
 * \param[in] str \a string
 */
void TrimSpaces(string& str);

/*!
 * \brief Trim given string's heading and tailing by "<space>,\n,\t,\r"
 * \sa TrimSpaces
 * \param[in] s \a string information
 * \return Trimmed string
 */
string& Trim(string& s);

/*!
 * \brief Splits the given string by spaces
 * \param[in] item \a string information
 * \return The split strings vector
 */
vector<string> SplitString(const string& item);

/*!
 * \brief Splits the given string based on the given delimiter
 * \param[in] item \a string information
 * \param[in] delimiter \a char
 * \return The split strings vector
 */
vector<string> SplitString(const string& item, char delimiter);

/*!
 * \brief Convert value to string
 * \param[in] val value, e.g., a int, or float
 * \return converted string
 */
template <typename T>
string ValueToString(const T& val) {
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

/*!
 * \brief Get numeric values by splitting the given string based on the given delimiter
 */
template <typename T>
bool SplitStringForValues(const string& items, const char delimiter, vector<T>& values);


/************ Implementation of template functions ******************/
template <typename T>
bool SplitStringForValues(const string& items, const char delimiter, vector<T>& values) {
    vector<string> value_strs = SplitString(items, delimiter);
    char* end = nullptr;
    for (vector<string>::iterator it = value_strs.begin(); it != value_strs.end(); ++it) {
        if ((*it).find_first_of("0123456789") == string::npos) {
            continue;
        }
        values.emplace_back(static_cast<T>(strtod((*it).c_str(), &end)));
    }
    vector<T>(values).swap(values);
    return true;
}

} /* namespace: utils_string */
} /* namespace: ccgl */

#endif /* CCGL_UTILS_STRING_H */
