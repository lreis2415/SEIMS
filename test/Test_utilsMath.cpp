#include "utils.h"
#include "gtest/gtest.h"

TEST(TestutilsMath, NumericEqual) {
    int int_a = 10;
    int int_b = 10;
    int int_c = 11;
    EXPECT_TRUE(utilsMath::FloatEqual(int_a, int_b));
    EXPECT_FALSE(utilsMath::FloatEqual(int_a, int_c));
    float float_a = 10.12345f;
    float float_b = 10.12345f;
    float float_c = 11.12345f;
    EXPECT_TRUE(utilsMath::FloatEqual(float_a, float_b));
    EXPECT_FALSE(utilsMath::FloatEqual(float_a, float_c));
}
