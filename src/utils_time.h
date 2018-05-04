/*!
 * \brief Time and datetime related functions.
 *        Part of the Common Cross-platform Geographic Library (CCGL)
 *
 * \author Liangjun Zhu (crazyzlj)
 * \version 1.0
 * \changelog  2018-05-01 - lj - Make part of CCGL.\n
 */
#ifndef CCGL_UTILS_TIME_H
#define CCGL_UTILS_TIME_H

#include "basic.h"

namespace ccgl {
/*!
 * \namespace utils_time
 * \brief Time related functions
 */
namespace utils_time {
/*!
 * \brief Precisely and cross-platform time counting function.
 */
double TimeCounting();

/*!
 * \brief Check the given year is a leap year or not.
 */
inline bool IsLeapYear(const int yr) { return yr % 4 == 0 && (yr % 100 != 0 || yr % 400 == 0); }

/*!
 * \brief Convert date time to string as the format of "YYYY-MM-DD"
 * \param[in] date \a time_t data type
 * \return Date time \a string
 */
string ConvertToString(const time_t* date);

/*!
 * \brief  Convert date time to string as the format of "YYYY-MM-DD HH"
 * \param[in] date \a time_t data type
 * \return Date time \a string
 */
string ConvertToString2(const time_t* date);

/*!
 * \brief Convert string to date time, string format could be %4d%2d%2d or %d-%d-%d
 *        e.g., str_date => 20000323, format=> %4d%2d%2d
 *              str_date => 2000-03-23, format => %d-%d-%d
 * \param[in] str_date \a string date
 * \param[in] format \a string format
 * \param[in] include_hour \a bool Include Hour?
 * \return Date time \a time_t
 */
time_t ConvertToTime(const string& str_date, string const& format, bool include_hour);

/*!
 * \brief Convert string to date time, string format could be "%4d-%2d-%2d %2d:%2d:%2d"
 *        e.g., str_date => 2000-03-23 10:30:00, format=> %4d-%2d-%2d %2d:%2d:%2d
 * \param[in] str_date \a string date
 * \param[in] format \a string format
 * \param[in] include_hour \a bool Include Hour?
 * \return Date time \a time_t
 */
time_t ConvertToTime2(string const& str_date, const char* format, bool include_hour);

/*!
 * \brief Convert integer year, month, and day to date time
 * \param[in] year year number from 1970
 * \param[in] month month range from 1 to 12
 * \param[in] day day range from 1 to 31
 * \return Date time \a time_t
 */
time_t ConvertYMDToTime(int& year, int& month, int& day);

/*!
 * \brief Get date information from \a time_t variable
 * \param[in] t \a time_t date
 * \param[out] year, month, day \a int value
 */
int GetDateInfoFromTimet(time_t* t, int* year, int* month, int* day);

/*!
 * \brief Get local time
 * \param[in] date \a time_t date
 * \param[out] t \a tm struct date
 */
void LocalTime(time_t date, struct tm* t);

} /* namespace: utils_time */
} /* namespace: ccgl */

#endif /* CCGL_UTILS_TIME_H */
