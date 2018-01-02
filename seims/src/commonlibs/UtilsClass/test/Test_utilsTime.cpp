#include "utils.h"
#include "gtest/gtest.h"

TEST(TestutilsTime, isLeapYear) {
    EXPECT_TRUE(utilsTime::isLeapYear(2000));
    EXPECT_FALSE(utilsTime::isLeapYear(1800));
    EXPECT_TRUE(utilsTime::isLeapYear(2004));
}
