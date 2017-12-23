#include "utils.h"
#include "gtest/gtest.h"

TEST(TestutilsString, SplitStringForFloat) {
    string float_strs = "-1.0,1.0e3,-0.001,2.5e-1";
    vector<float> floats = utilsString::SplitStringForFloat(float_strs, ',');
    EXPECT_EQ(4, floats.size());
    EXPECT_FLOAT_EQ(-1.f, floats[0]);
    EXPECT_FLOAT_EQ(1000.f, floats[1]);
    EXPECT_FLOAT_EQ(-0.001f, floats[2]);
    EXPECT_FLOAT_EQ(0.25f, floats[3]);
}
