#include "utils_time.h"

#include <ctime>
#include <iostream>

#include "utils_string.h"

using std::cout;
using std::endl;

namespace ccgl {
namespace utils_time {
double TimeCounting() {
#ifdef windows
    LARGE_INTEGER li;
    if (QueryPerformanceFrequency(&li)) {
        /// CPU supported
        double pc_freq = CVT_DBL(li.QuadPart);
        QueryPerformanceCounter(&li);
        return CVT_DBL(li.QuadPart) / pc_freq; // seconds
    }
    return CVT_DBL(clock()) / CLK_TCK;
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return CVT_DBL(tv.tv_sec) + CVT_DBL(tv.tv_usec) / 1000000.;
#endif /* windows */
}

string ConvertToString(const time_t* date) {
    struct tm* date_info = new tm();
#ifdef windows
    localtime_s(date_info, date);
#else
    localtime_r(date, date_info);
#endif /* windows */
    if (date_info->tm_isdst > 0) {
        date_info->tm_hour -= 1;
    }
    char date_string[11];
    strftime(date_string, 11, "%Y-%m-%d", date_info);
    delete date_info;
    return string(date_string);
}

string ConvertToString2(const time_t* date) {
    // Days number of each month
    static int month_days[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    struct tm* date_info = new tm();
#ifdef windows
    localtime_s(date_info, date);
#else
    localtime_r(date, date_info);
#endif /* windows */
    if (date_info->tm_isdst > 0) {
        if (date_info->tm_hour != 0) {
            date_info->tm_hour -= 1;
        } else {
            date_info->tm_hour = 23;
            date_info->tm_mday -= 1;
            if (date_info->tm_mday == 0) {
                date_info->tm_mon -= 1;

                if (date_info->tm_mon == 0) {
                    date_info->tm_year -= 1;
                    date_info->tm_mon = 12;
                    date_info->tm_mday = 31;
                } else {
                    if (IsLeapYear(date_info->tm_year)) {
                        date_info->tm_mday = month_days[date_info->tm_mon] + 1;
                    } else {
                        date_info->tm_mday = month_days[date_info->tm_mon];
                    }
                }
            }
        }
    }
    char date_string[30];
    strftime(date_string, 30, "%Y-%m-%d %X", date_info);
    string s(date_string);
    delete date_info;
    return s;
}

time_t ConvertToTime(const string& str_date, string const& format, const bool include_hour) {
    time_t t(0);
    if (utils_string::StringMatch(str_date, "")) {
        return t;
    }
    int yr;
    int mn;
    int dy;
    int hr = 0;
    int min = 0;
    int sec = 0;

    try {
        if (include_hour) {
            stringscanf(str_date.c_str(), format.c_str(), &yr, &mn, &dy, &hr, &min, &sec);
        } else {
            stringscanf(str_date.c_str(), format.c_str(), &yr, &mn, &dy);
        }
        struct tm* time_info = new tm();
        time_info->tm_year = yr - 1900;
        time_info->tm_mon = mn - 1;
        time_info->tm_mday = dy;
        time_info->tm_hour = hr;
        time_info->tm_min = min;
        time_info->tm_sec = sec;
        time_info->tm_isdst = false;
        t = mktime(time_info);
        delete time_info;
    } catch (...) {
        cout << "Error occurred when convert " + str_date + " to time_t!" << endl;
        t = 0; // reset to 0 for convenient comparison
    }
    return t;
}

time_t ConvertYMDToTime(int& year, int& month, int& day) {
    time_t t(0);
    try {
        struct tm* time_info = new tm();
        time_info->tm_year = year - 1900;
        time_info->tm_mon = month - 1;
        time_info->tm_mday = day;
        time_info->tm_isdst = false;
        t = mktime(time_info);
        delete time_info;
    } catch (...) {
        cout << "Error in ConvertYMDToTime!" << endl;
        t = 0;
    }
    return t;
}

int GetDateInfoFromTimet(time_t* t, int* year, int* month, int* day) {
    struct tm* date_info = new tm();
#ifdef windows
    localtime_s(date_info, t);
#else
    localtime_r(t, date_info);
#endif /* windows */
    if (date_info->tm_isdst > 0) {
        date_info->tm_hour -= 1;
    }

    char date_string[30];
    strftime(date_string, 30, "%Y-%m-%d %X", date_info);
    int hour, min, sec;
    stringscanf(date_string, "%4d-%2d-%2d %2d:%2d:%2d", year, month, day, &hour, &min, &sec);
    delete date_info;
    return 0;
}

void LocalTime(time_t date, struct tm* t) {
#ifdef windows
    localtime_s(t, &date);
#else
    localtime_r(&date, t);
#endif /* windows */
}


int GetYear(const time_t date) {
    struct tm* date_info = new tm();
    LocalTime(date, date_info);
    int yr = date_info->tm_year + 1900;
    delete date_info;
    return yr;
}

int GetMonth(const time_t date) {
    struct tm* date_info = new tm();
    LocalTime(date, date_info);
    int mon = date_info->tm_mon + 1;
    delete date_info;
    return mon;
}

int GetDay(const time_t date) {
    struct tm* date_info = new tm();
    LocalTime(date, date_info);
    int day = date_info->tm_mday;
    delete date_info;
    return day;
}

int DayOfYear(const time_t date) {
    struct tm* date_info = new tm();
    LocalTime(date, date_info);
    int doy = date_info->tm_yday + 1;
    delete date_info;
    return doy;
}

int DayOfYear(const int year, const int month, const int day) {
    return JulianDay(year, month, day) - JulianDay(year, 1, 1) + 1;
}

int JulianDay(const time_t date) {
    struct tm* date_info = new tm();
    LocalTime(date, date_info);
    int jday = JulianDay(date_info->tm_year, date_info->tm_mon, date_info->tm_mday);
    delete date_info;
    return jday;
}

int JulianDay(const int year, const int month, const int day) {
    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
    int d = day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
    return d;
}

#if defined CPP_MSVC
DateTime SystemTimeToDateTime(const SYSTEMTIME& sys_time) {
    DateTime date_time;
    date_time.year = sys_time.wYear;
    date_time.month = sys_time.wMonth;
    date_time.day = sys_time.wDay;
    date_time.day_of_week = sys_time.wDayOfWeek;
    date_time.day_of_year = DayOfYear(date_time.year, date_time.month, date_time.day);
    date_time.hour = sys_time.wHour;
    date_time.minute = sys_time.wMinute;
    date_time.second = sys_time.wSecond;
    date_time.milliseconds = sys_time.wMilliseconds;

    FILETIME file_time;
    SystemTimeToFileTime(&sys_time, &file_time);
    ULARGE_INTEGER large_int;
    large_int.HighPart = file_time.dwHighDateTime;
    large_int.LowPart = file_time.dwLowDateTime;
    date_time.filetime = large_int.QuadPart;
    date_time.total_milliseconds = date_time.filetime / 10000;

    return date_time;
}

SYSTEMTIME DateTimeToSystemTime(const DateTime& date_time) {
    ULARGE_INTEGER large_int;
    large_int.QuadPart = date_time.filetime;
    FILETIME file_time;
    file_time.dwHighDateTime = large_int.HighPart;
    file_time.dwLowDateTime = large_int.LowPart;

    SYSTEMTIME sys_time;
    FileTimeToSystemTime(&file_time, &sys_time);
    return sys_time;
}
#elif defined CPP_GCC

/*!
 * \brief Convert Turkmenistan Time (Standard Time) to DateTime
 *        TMT is 5 hours ahead of UTC time.
 *        This time zone is in use during standard time in Asia.
 */
DateTime ConvertTMToDateTime(tm* time_info, vint milliseconds) {
    time_t timer = mktime(time_info);
    DateTime dt;
    dt.year = time_info->tm_year + 1900;
    dt.month = time_info->tm_mon + 1;
    dt.day = time_info->tm_mday;
    dt.day_of_week = time_info->tm_wday;
    dt.day_of_year = time_info->tm_yday;
    dt.hour = time_info->tm_hour;
    dt.minute = time_info->tm_min;
    dt.second = time_info->tm_sec;

    dt.milliseconds = milliseconds;
    dt.filetime = CVT_VUINT64(timer * 1000 + milliseconds);
    dt.total_milliseconds = CVT_VUINT64(timer * 1000 + milliseconds);
    return dt;
}

vint GetCurrentMilliseconds() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_usec / 1000;
}
#endif /* CPP_MSVC */

DateTime DateTime::LocalTime() {
#if defined CPP_MSVC
    SYSTEMTIME sys_time;
    GetLocalTime(&sys_time);
    return SystemTimeToDateTime(sys_time);
#elif defined CPP_GCC
    time_t timer = time(nullptr);
    struct tm* time_info = new tm();
    localtime_r(&timer, time_info);
    return ConvertTMToDateTime(time_info, GetCurrentMilliseconds());
#endif /* CPP_MSVC */
}

DateTime DateTime::UTCTime() {
#if defined CPP_MSVC
    SYSTEMTIME utc_time;
    GetSystemTime(&utc_time);
    return SystemTimeToDateTime(utc_time);
#elif defined CPP_GCC
    time_t timer = time(nullptr);
    struct tm* time_info = new tm();
    gmtime_r(&timer, time_info);
    return ConvertTMToDateTime(time_info, GetCurrentMilliseconds());
#endif /* CPP_MSVC */
}

DateTime DateTime::FromDateTime(const int iyear, const int imonth, const int iday, const int ihour,
                                const int iminute, const int isecond, const int imillisecond) {
#if defined CPP_MSVC
    SYSTEMTIME sys_time;
    memset(&sys_time, 0, sizeof(sys_time));
    sys_time.wYear = static_cast<WORD>(iyear);
    sys_time.wMonth = static_cast<WORD>(imonth);
    sys_time.wDay = static_cast<WORD>(iday);
    sys_time.wHour = static_cast<WORD>(ihour);
    sys_time.wMinute = static_cast<WORD>(iminute);
    sys_time.wSecond = static_cast<WORD>(isecond);
    sys_time.wMilliseconds = static_cast<WORD>(imillisecond);

    FILETIME file_time;
    SystemTimeToFileTime(&sys_time, &file_time);
    FileTimeToSystemTime(&file_time, &sys_time);
    return SystemTimeToDateTime(sys_time);
#elif defined CPP_GCC
    tm time_info;
    memset(&time_info, 0, sizeof(time_info));
    time_info.tm_year = iyear - 1900;
    time_info.tm_mon = imonth - 1;
    time_info.tm_mday = iday;
    time_info.tm_hour = ihour;
    time_info.tm_min = iminute;
    time_info.tm_sec = isecond;
    time_info.tm_isdst = -1;

    return ConvertTMToDateTime(&time_info, imillisecond);
#endif
}

DateTime DateTime::FromFileTime(const vuint64_t ifiletime) {
#if defined CPP_MSVC
    ULARGE_INTEGER large_int;
    large_int.QuadPart = ifiletime;
    FILETIME file_time;
    file_time.dwHighDateTime = large_int.HighPart;
    file_time.dwLowDateTime = large_int.LowPart;

    SYSTEMTIME sys_time;
    FileTimeToSystemTime(&file_time, &sys_time);
    return SystemTimeToDateTime(sys_time);
#elif defined CPP_GCC
    time_t timer = static_cast<time_t>(ifiletime / 1000);
    struct tm* time_info = new tm();
    localtime_r(&timer, time_info);
    return ConvertTMToDateTime(time_info, ifiletime % 1000);
#endif
}

DateTime::DateTime() :
    year(0), month(0), day(0), day_of_week(0), day_of_year(0),
    hour(0), minute(0), second(0),
    milliseconds(0), total_milliseconds(0),
    filetime(0) {
}

DateTime DateTime::ToLocalTime() {
#if defined CPP_MSVC
    SYSTEMTIME utc_time = DateTimeToSystemTime(*this);
    SYSTEMTIME local_time;
    SystemTimeToTzSpecificLocalTime(nullptr, &utc_time, &local_time);
    return SystemTimeToDateTime(local_time);
#elif defined CPP_GCC
    time_t local_timer = time(nullptr);
    time_t utc_timer = mktime(gmtime(&local_timer));
    time_t timer = static_cast<time_t>(filetime / 1000) + local_timer - utc_timer;
    struct tm* time_info = new tm();
    localtime_r(&timer, time_info);

    return ConvertTMToDateTime(time_info, milliseconds);
#endif
}

DateTime DateTime::ToUTCTime() {
#if defined CPP_MSVC
    SYSTEMTIME local_time = DateTimeToSystemTime(*this);
    SYSTEMTIME utc_time;
    TzSpecificLocalTimeToSystemTime(nullptr, &local_time, &utc_time);
    return SystemTimeToDateTime(utc_time);
#elif defined CPP_GCC
    time_t timer = static_cast<time_t>(filetime / 1000);
    struct tm* time_info = new tm();
    gmtime_r(&timer, time_info);

    return ConvertTMToDateTime(time_info, milliseconds);
#endif
}

DateTime DateTime::Forward(const int imilliseconds) {
#if defined CPP_MSVC
    return FromFileTime(filetime + milliseconds * 10000);
#elif defined CPP_GCC
    return FromFileTime(filetime + milliseconds);
#endif
}

DateTime DateTime::Backward(const int imilliseconds) {
#if defined CPP_MSVC
    return FromFileTime(filetime - milliseconds * 10000);
#elif defined CPP_GCC
    return FromFileTime(filetime - milliseconds);
#endif
}

} /* namespace: utils_time */
} /* namespace: ccgl */
