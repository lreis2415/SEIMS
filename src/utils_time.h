/*!
 * \brief Time and datetime related functions.
 *        Part of the Common Cross-platform Geographic Library (CCGL)
 *
 * \author Liangjun Zhu (crazyzlj)
 * \changelog  2018-05-01 - lj - Make part of CCGL.\n
 *             2018-05-23 - lj - Add DateTime struct which combines date and time.\n
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
 *        divisible by 4, not if divisible by 100, but true if divisible by 400
 */
inline bool IsLeapYear(const int yr) { return !(yr % 4) && (yr % 100 || !(yr % 400)); }

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

/*!
 * \brief Get the year
 * \return int year
 */
int GetYear(time_t date);

/*!
 * \brief Get the month
 * \return int month
 */
int GetMonth(time_t date);

/*!
 * \brief Get the day
 * \return int day
 */
int GetDay(time_t date);

/*!
 * \brief Get the day of one day, [1, 366]
 */
int DayOfYear(time_t date);

/*!
* \brief Get the day of one day, [1, 366]
*/
int DayOfYear(int year, int month, int day);

/*!
* \brief Get the Julian day from time_t date
*/
int JulianDay(time_t date);

/*!
 * \brief Get the Julian day of one day from year, month, and day.
 *        Algorithm adopted from boost::date_time::gregorian_calendar_base::day_number.
 * \return int Julian day
 */
int JulianDay(int year, int month, int day);

/*!
 * \brief A type representing the combination of date and time.
 *        Refers to the DateTime struct implemented in Vlpp by vczh.
 */
struct DateTime {
    int year;                     /// Year
    int month;                    /// Month since January - [1, 12]
    int day;                      /// Day of the month - [1, 31]
    int day_of_week;              /// Day of the week since Sunday - [0, 6]
    int day_of_year;              /// Day of the year - [0, 365]
    int hour;                     /// Hour of the day since midnight - [0, 23]
    int minute;                   /// Minutes after the hour - [0, 59]
    int second;                   /// Seconds after the minute - [0, 59]
    int milliseconds;             /// Milliseconds after the second - [0, 999]
    vuint64_t total_milliseconds; /// Total milliseconds of the time
    vuint64_t filetime;           /// The number of 100-nanosecond intervals since January 1, 1601 (UTC).

    /*!
     * \brief Get the current local time.
     */
    static DateTime LocalTime();

    /*!
     * \brief Get the current UTC time.
     */
    static DateTime UTCTime();

    /*!
     * \brief Create a date time value from each time element value.
     */
    static DateTime FromDateTime(int iyear, int imonth, int iday, int ihour = 0,
                                 int iminute = 0, int isecond = 0, int imillisecond = 0);

    /*!
     * \brief Create a date time value from FILETIME.
     */
    static DateTime FromFileTime(vuint64_t ifiletime);

    DateTime(); ///< Create an empty date time value.

    DateTime ToLocalTime(); ///< Convert the UTC time to the local time.

    DateTime ToUTCTime(); ///< Convert the local time to the UTC time.

    DateTime Forward(int imilliseconds); ///< Move forward by the delta in milliseconds.

    DateTime Backward(int imilliseconds); ///< Move backward by the delta in milliseconds.

    bool operator==(const DateTime& value) const { return filetime == value.filetime; }
    bool operator!=(const DateTime& value) const { return filetime != value.filetime; }
    bool operator<(const DateTime& value) const { return filetime < value.filetime; }
    bool operator<=(const DateTime& value) const { return filetime <= value.filetime; }
    bool operator>(const DateTime& value) const { return filetime > value.filetime; }
    bool operator>=(const DateTime& value) const { return filetime >= value.filetime; }
};

} /* namespace: utils_time */
} /* namespace: ccgl */

#endif /* CCGL_UTILS_TIME_H */
