#include "utils.h"
#include "gtest/gtest.h"

TEST(TestutilsArray, Handle1DArray) {
    int *int1d = nullptr;
    EXPECT_EQ(nullptr, int1d);
    int n = 5;
    int init = 100;
    utilsArray::Initialize1DArray(n, int1d, init);
    EXPECT_NE(nullptr, int1d);
    for (int i = 0; i < n; i++) {
        EXPECT_EQ(init, int1d[i]);
    }
    utilsArray::Release1DArray(int1d);
    EXPECT_EQ(nullptr, int1d);
}

TEST(TestutilsArray, Handle2DArray) {
    float **float2d = nullptr;
    EXPECT_EQ(nullptr, float2d);
    int r = 5;
    int c = 4;
    float inif = 3.1415926f;
    utilsArray::Initialize2DArray(r, c, float2d, inif);
    EXPECT_NE(nullptr, float2d);
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            EXPECT_FLOAT_EQ(inif, float2d[i][j]);
        }
    }
    utilsArray::Release2DArray(r, float2d);
    EXPECT_EQ(nullptr, float2d);
}
