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
    struct tm date_info;
#ifdef windows
    localtime_s(&date_info, date);
#else
    localtime_r(date, &date_info);
#endif /* windows */
    if (date_info.tm_isdst > 0) {
        date_info.tm_hour -= 1;
    }
    char date_string[11];
    strftime(date_string, 11, "%Y-%m-%d", &date_info);
    return string(date_string);
}

string ConvertToString2(const time_t* date) {
    // Days number of each month
    static int month_days[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    struct tm date_info;
#ifdef windows
    localtime_s(&date_info, date);
#else
    localtime_r(date, &date_info);
#endif /* windows */
    if (date_info.tm_isdst > 0) {
        if (date_info.tm_hour != 0) {
            date_info.tm_hour -= 1;
        } else {
            date_info.tm_hour = 23;
            date_info.tm_mday -= 1;
            if (date_info.tm_mday == 0) {
                date_info.tm_mon -= 1;

                if (date_info.tm_mon == 0) {
                    date_info.tm_year -= 1;
                    date_info.tm_mon = 12;
                    date_info.tm_mday = 31;
                } else {
                    if (IsLeapYear(date_info.tm_year)) {
                        date_info.tm_mday = month_days[date_info.tm_mon] + 1;
                    } else {
                        date_info.tm_mday = month_days[date_info.tm_mon];
                    }
                }
            }
        }
    }
    char date_string[30];
    strftime(date_string, 30, "%Y-%m-%d %X", &date_info);
    string s(date_string);
    return s;
}

time_t ConvertToTime(const string& str_date, string const& format, bool include_hour) {
    time_t t(0);
    if (utils_string::StringMatch(str_date, "")) {
        return t;
    }
    int yr;
    int mn;
    int dy;
    int hr = 0;

    try {
        if (include_hour) {
            stringscanf(str_date.c_str(), format.c_str(), &yr, &mn, &dy, &hr);
        } else {
            stringscanf(str_date.c_str(), format.c_str(), &yr, &mn, &dy);
        }
        struct tm* timeinfo = new struct tm;
        timeinfo->tm_year = yr - 1900;
        timeinfo->tm_mon = mn - 1;
        timeinfo->tm_mday = dy;
        timeinfo->tm_hour = hr;
        timeinfo->tm_min = 0;
        timeinfo->tm_sec = 0;
        timeinfo->tm_isdst = false;
        t = mktime(timeinfo);
    } catch (...) {
        //throw;
        // do not throw any exceptions in library.
        cout << "Error occurred when convert " + str_date + " to time_t!" << endl;
        t = 0; // reset to 0 for convenient comparison
    }
    return t;
}

time_t ConvertToTime2(string const& str_date, const char* format, const bool include_hour) {
    time_t t(0);
    if (utils_string::StringMatch(str_date, "")) {
        return t;
    }
    int yr;
    int mn;
    int dy;
    int hr = 0;
    int m = 0;
    int s = 0;

    try {
        if (include_hour) {
            stringscanf(str_date.c_str(), format, &yr, &mn, &dy, &hr, &m, &s);
        } else {
            stringscanf(str_date.c_str(), format, &yr, &mn, &dy);
        }

        struct tm* timeinfo = new struct tm;
        timeinfo->tm_year = yr - 1900;
        timeinfo->tm_mon = mn - 1;
        timeinfo->tm_mday = dy;
        timeinfo->tm_hour = hr;
        timeinfo->tm_min = m;
        timeinfo->tm_sec = s;
        timeinfo->tm_isdst = false;
        t = mktime(timeinfo);
    } catch (...) {
        cout << "Error in ConvertToTime2!" << endl;
        t = 0;
    }
    return t;
}

time_t ConvertYMDToTime(int& year, int& month, int& day) {
    time_t t(0);
    try {
        struct tm* timeinfo = new struct tm;
        timeinfo->tm_year = year - 1900;
        timeinfo->tm_mon = month - 1;
        timeinfo->tm_mday = day;
        timeinfo->tm_isdst = false;
        t = mktime(timeinfo);
    } catch (...) {
        cout << "Error in ConvertYMDToTime!" << endl;
        t = 0;
    }
    return t;
}

int GetDateInfoFromTimet(time_t* t, int* year, int* month, int* day) {
    struct tm dateInfo;
#ifdef windows
    localtime_s(&dateInfo, t);
#else
    localtime_r(t, &dateInfo);
#endif /* windows */
    if (dateInfo.tm_isdst > 0) {
        dateInfo.tm_hour -= 1;
    }

    char date_string[30];
    strftime(date_string, 30, "%Y-%m-%d %X", &dateInfo);
    int hour, min, sec;
    stringscanf(date_string, "%4d-%2d-%2d %2d:%2d:%2d", year, month, day, &hour, &min, &sec);
    return 0;
}

void LocalTime(time_t date, struct tm* t) {
#ifdef windows
    localtime_s(t, &date);
#else
    localtime_r(&date, t);
#endif /* windows */
}
} /* namespace: utils_time */
} /* namespace: ccgl */
