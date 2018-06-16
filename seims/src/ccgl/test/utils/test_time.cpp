#include "gtest/gtest.h"
#include "../../src/utils_time.h"

using namespace ccgl::utils_time;

TEST(TestutilsTime, isLeapYear) {
    EXPECT_TRUE(IsLeapYear(2000));
    EXPECT_FALSE(IsLeapYear(1800));
    EXPECT_TRUE(IsLeapYear(2004));
}

TEST(TestutilsTime, ConvertToString) {
    time_t t = 1528499098; // tm_year=2018, tm_mon=6, tm_mday=9, tm_hour=7, tm_min=4, tm_sec=58, tm_wday=5, tm_yday=160
    string str_t = ConvertToString(&t);
    EXPECT_STREQ("2018-06-09", str_t.c_str());
}

TEST(TestutilsTime, ConvertToString2) {
    time_t t = 1528499098;
    string str_t = ConvertToString2(&t);
    EXPECT_STREQ("2018-06-09 07:04:58", str_t.c_str());
}

TEST(TestutilsTime, ConvertToTime) {
    string time_str_blank = "";
    EXPECT_EQ(0, ConvertToTime(time_str_blank, "%d-%d-%d", true));
    string time_str = "2018-06-09";
    EXPECT_EQ(1528473600, ConvertToTime(time_str, "%d-%d-%d", false));
    EXPECT_EQ(1528473600, ConvertToTime(time_str, "%4d-%2d-%2d", false));
    string time_str_hour = "2018-06-09 07:04:58";
    EXPECT_EQ(1528499098, ConvertToTime(time_str_hour, "%d-%d-%d %d:%d:%d", true));
    EXPECT_EQ(1528499098, ConvertToTime(time_str_hour, "%4d-%2d-%2d %2d:%2d:%2d", true));
}

TEST(TestutilsTime, ConvertYMDToTime) {
    int yr = 2018;
    int mon = 6;
    int day = 9;
    EXPECT_EQ(1528473600, ConvertYMDToTime(yr, mon, day));
}

TEST(TestutilsTime, GetDateInfoFromTimet) {
    time_t t = 1528499098;
    int yr, mon, day;
    EXPECT_EQ(0, GetDateInfoFromTimet(&t, &yr, &mon, &day));
    EXPECT_EQ(2018, yr);
    EXPECT_EQ(6, mon);
    EXPECT_EQ(9, day);
}

TEST(TestutilsTime, GetYearMonthDay) {
    EXPECT_EQ(2018, GetYear(1528499098));
    EXPECT_EQ(6, GetMonth(1528499098));
    EXPECT_EQ(9, GetDay(1528499098));
    EXPECT_EQ(160, DayOfYear(1528499098));
    EXPECT_EQ(160, DayOfYear(2018,6,9));
}
