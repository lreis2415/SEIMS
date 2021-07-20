#include "gtest/gtest.h"
#include "../../src/utils_time.h"

using namespace ccgl::utils_time;

TEST(TestutilsTime, isLeapYear) {
    EXPECT_TRUE(IsLeapYear(2000));
    EXPECT_FALSE(IsLeapYear(1800));
    EXPECT_TRUE(IsLeapYear(2004));
}

// Date time for tests of TestutilsTime
// UTC+08:00, 2018-06-09 07:04:58
// UTC+00:00, 2018-06-08 23:04:58
//    tm_year=2018, tm_mon=6, tm_mday=9, tm_hour=7, tm_min=4, tm_sec=58, tm_wday=5, tm_yday=160
double get_utc_offset() {
    time_t t = 1528499098;
    struct tm *utc = new tm();
    struct tm *local = new tm();
    GetDateTime(t, utc);
    GetDateTime(t, local, false);
    double diff = difftime(mktime(utc), mktime(local));
    delete utc;
    delete local;
    return diff;
}

double tzone_offset = get_utc_offset();

TEST(TestutilsTime, ConvertToString) {
    time_t t = 1528499098;
    string utc_t = ConvertToString(t); // By default, the input and output is UTC time
    EXPECT_STREQ("2018-06-08", utc_t.c_str());
    if (tzone_offset + 8 * 3600 < UTIL_ZERO) {
        string local_t = ConvertToString(t, false); // The input and output is local time
        EXPECT_STREQ("2018-06-09", local_t.c_str());
    }
}

TEST(TestutilsTime, ConvertToString2) {
    time_t t = 1528499098;
    string utc_t = ConvertToString2(t);
    EXPECT_STREQ("2018-06-08 23:04:58", utc_t.c_str());
    if (tzone_offset + 8 * 3600 < UTIL_ZERO) {
        string local_t = ConvertToString2(t, false);
        EXPECT_STREQ("2018-06-09 07:04:58", local_t.c_str());
    }
}

TEST(TestutilsTime, ConvertToTime) {
    string time_str_blank = "";
    EXPECT_EQ(0, ConvertToTime(time_str_blank, "%d-%d-%d", true));
    string time_str = "2018-06-09"; // UTC: 1528502400, UTC+8: 1528473600
    EXPECT_EQ(1528502400, ConvertToTime(time_str, "%d-%d-%d", false));
    EXPECT_EQ(1528502400, ConvertToTime(time_str, "%4d-%2d-%2d", false));
    string time_str_hour = "2018-06-09 07:04:58"; // UTC+8: 1528499098, UTC+0:  1528527898
    EXPECT_EQ(1528527898, ConvertToTime(time_str_hour, "%d-%d-%d %d:%d:%d", true));
    EXPECT_EQ(1528527898, ConvertToTime(time_str_hour, "%4d-%2d-%2d %2d:%2d:%2d", true));
    if (tzone_offset + 8 * 3600 < UTIL_ZERO) {
        EXPECT_EQ(1528473600, ConvertToTime(time_str, "%d-%d-%d", false, false));
        EXPECT_EQ(1528473600, ConvertToTime(time_str, "%4d-%2d-%2d", false, false));
        EXPECT_EQ(1528499098, ConvertToTime(time_str_hour, "%d-%d-%d %d:%d:%d", true, false));
        EXPECT_EQ(1528499098, ConvertToTime(time_str_hour, "%4d-%2d-%2d %2d:%2d:%2d", true, false));
    }
}

TEST(TestutilsTime, ConvertYMDToTime) {
    int yr = 2018;
    int mon = 6;
    int day = 9;
    EXPECT_EQ(1528502400, ConvertYMDToTime(yr, mon, day));
    if (tzone_offset + 8 * 3600 < UTIL_ZERO) {
        EXPECT_EQ(1528473600, ConvertYMDToTime(yr, mon, day,false));
    }
}

TEST(TestutilsTime, GetDateInfoFromTimet) {
    time_t t = 1528499098;
    int yr, mon, day;
    EXPECT_EQ(0, GetDateInfoFromTimet(t, &yr, &mon, &day));
    EXPECT_EQ(2018, yr);
    EXPECT_EQ(6, mon);
    EXPECT_EQ(8, day);
    if (tzone_offset + 8 * 3600 < UTIL_ZERO) {
        EXPECT_EQ(0, GetDateInfoFromTimet(t, &yr, &mon, &day, false));
        EXPECT_EQ(2018, yr);
        EXPECT_EQ(6, mon);
        EXPECT_EQ(9, day);
    }
}

TEST(TestutilsTime, GetYearMonthDay) {
    EXPECT_EQ(2018, GetYear(1528499098));
    EXPECT_EQ(6, GetMonth(1528499098));
    EXPECT_EQ(8, GetDay(1528499098));
    EXPECT_EQ(159, DayOfYear(1528499098));
    EXPECT_EQ(159, DayOfYear(2018, 6, 8));
    EXPECT_EQ(JulianDay(2018, 6, 8), JulianDay(1528499098));
    if (tzone_offset + 8 * 3600 < UTIL_ZERO) {
        EXPECT_EQ(2018, GetYear(1528499098,false));
        EXPECT_EQ(6, GetMonth(1528499098,false));
        EXPECT_EQ(9, GetDay(1528499098,false));
        EXPECT_EQ(160, DayOfYear(1528499098,false));
        EXPECT_EQ(160, DayOfYear(2018, 6, 9));
        EXPECT_EQ(JulianDay(2018, 6, 9), JulianDay(1528499098, false));
    }
}
