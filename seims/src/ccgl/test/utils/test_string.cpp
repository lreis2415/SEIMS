#include "gtest/gtest.h"
#include "../../src/utils_string.h"

using namespace ccgl;
using namespace ccgl::utils_string;

TEST(TestutilsString, GetUpper) {
    string low_str = "ccgl";
    EXPECT_STREQ("CCGL", GetUpper(low_str).c_str());
}

TEST(TestutilsString, TrimSpaces) {
    string str = " CCGL\t";
    TrimSpaces(str);
    EXPECT_STREQ("CCGL", str.c_str());
}

TEST(TestutilsString, Trim) {
    string str = "\n CCGL\t ";
    Trim(str);
    EXPECT_STREQ("CCGL", str.c_str());
}

TEST(TestutilsString, StringMatch) {
    string str1 = "ccgl";
    string str2 = "ccgL";
    EXPECT_TRUE(StringMatch(str1, str2));
}

TEST(TestutilsString, SplitString) {
    string str = "1-2-3-4";
    vector<string> strs = SplitString(str, '-');
    EXPECT_STREQ("3", strs[2].c_str());
}

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

TEST(TestutilsString, ValueToString) {
    int a = 12;
    EXPECT_STREQ("12", ValueToString(a).c_str());
    float b = 1.2f;
    EXPECT_STREQ("1.2", ValueToString(b).c_str());
    float c = 1.2e2;
    EXPECT_STREQ("120", ValueToString(c).c_str());
}

TEST(TestutilsString, IsInt) {
    string int_str1 = "123";
    bool flag1 = false;
    vint int1 = IsInt(int_str1, flag1);
    EXPECT_TRUE(flag1);
    EXPECT_EQ(123, int1);
    string int_str2 = "12.3";
    bool flag2 = false;
    vint int2 = IsInt(int_str2, flag1);
    EXPECT_FALSE(flag2);
    EXPECT_NE(123, int2);
    string int_str3 = "123.0";
    bool flag3 = false;
    vint int3 = IsInt(int_str3, flag3);
    EXPECT_FALSE(flag3);
    EXPECT_EQ(123, int3);
}

TEST(TestutilsString, IsDouble) {
    string dbl_str1 = "1.23";
    bool flag1 = false;
    double dbl1 = IsDouble(dbl_str1, flag1);
    EXPECT_TRUE(flag1);
    EXPECT_DOUBLE_EQ(1.23, dbl1);
}

TEST(TestutilsString, IsNumber) {
    string str = "1.23";
    EXPECT_TRUE(IsNumber(str));
    str = "123int";
    EXPECT_FALSE(IsNumber(str));
    str = "123 ";
    EXPECT_FALSE(IsNumber(str));
    str = "12.3f";
    EXPECT_FALSE(IsNumber(str));
    str = "12.3e2";
    EXPECT_TRUE(IsNumber(str));
}

TEST(TestutilsString, ToInt) {
    string str = "123";
    vint int1 = ToInt(str);
    EXPECT_EQ(123, int1);
}
