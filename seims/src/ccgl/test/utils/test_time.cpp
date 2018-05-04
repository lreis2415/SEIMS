#include "gtest/gtest.h"
#include "../../src/utils_time.h"

using namespace ccgl::utils_time;

TEST(TestutilsTime, isLeapYear) {
    EXPECT_TRUE(IsLeapYear(2000));
    EXPECT_FALSE(IsLeapYear(1800));
    EXPECT_TRUE(IsLeapYear(2004));
}
