#include "../../src/utils_array.h"
#include "gtest/gtest.h"

using namespace ccgl::utils_array;

TEST(TestutilsArray, Handle1DArray) {
    int* int_1d = nullptr;
    EXPECT_EQ(nullptr, int_1d);
    int n = 5;
    int init = 100;
    Initialize1DArray(n, int_1d, init);
    EXPECT_NE(nullptr, int_1d);
    for (int i = 0; i < n; i++) {
        EXPECT_EQ(init, int_1d[i]);
    }
    Release1DArray(int_1d);
    EXPECT_EQ(nullptr, int_1d);
}

TEST(TestutilsArray, Handle2DArray) {
    float** float_2d = nullptr;
    EXPECT_EQ(nullptr, float_2d);
    int r = 5;
    int c = 4;
    float inif = 3.1415926f;
    Initialize2DArray(r, c, float_2d, inif);
    EXPECT_NE(nullptr, float_2d);
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            EXPECT_FLOAT_EQ(inif, float_2d[i][j]);
            float_2d[i][j] += CVT_FLT(i * c + j);
        }
    }
    
    float** float_2d_copy = nullptr;
    Initialize2DArray(r, c, float_2d_copy, float_2d);
    EXPECT_NE(nullptr, float_2d);
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            EXPECT_FLOAT_EQ(float_2d[i][j], float_2d_copy[i][j]);
        }
    }

    Release2DArray(r, float_2d);
    EXPECT_EQ(nullptr, float_2d);
    Release2DArray(r, float_2d_copy);
    EXPECT_EQ(nullptr, float_2d_copy);
}
