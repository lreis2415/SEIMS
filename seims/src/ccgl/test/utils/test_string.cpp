#include "gtest/gtest.h"
#include "../../src/utils_string.h"

using namespace ccgl::utils_string;

TEST(TestutilsString, SplitStringForValuesFlt) {
    string float_strs = "-1.0,1.0e3,-0.001,2.5e-1";
    vector<float> floats;
    SplitStringForValues(float_strs, ',', floats);
    EXPECT_EQ(4, floats.size());
    EXPECT_FLOAT_EQ(-1.f, floats[0]);
    EXPECT_FLOAT_EQ(1000.f, floats[1]);
    EXPECT_FLOAT_EQ(-0.001f, floats[2]);
    EXPECT_FLOAT_EQ(0.25f, floats[3]);
}

TEST(TestutilsString, SplitStringForValuesInt) {
    string int_strs = "-1,0,1,4";
    vector<int> ints;
    SplitStringForValues(int_strs, ',', ints);
    EXPECT_EQ(4, ints.size());
    EXPECT_EQ(-1, ints[0]);
    EXPECT_EQ(0, ints[1]);
    EXPECT_EQ(1, ints[2]);
    EXPECT_EQ(4, ints[3]);
}
