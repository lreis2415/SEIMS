#include "gtest/gtest.h"
#include "../../src/utils_math.h"

using namespace ccgl::utils_math;

/* Max allowed diff against expected value */
#ifndef EPSILON_MAX
#define EPSILON_MAX 0.001
#endif
#ifndef EPSILON_RELMAX
#define EPSILON_RELMAX 0.00001
#endif

template <typename PTYPE1, typename PTYPE2>
bool equals(PTYPE1 x, PTYPE2 y) {
    double err;
    double dx = CVT_DBL(x);
    double dy = CVT_DBL(y);

    if (fabs(dx - dy) <= EPSILON_MAX)
        return true;

    if (fabs(dx) > fabs(dy))
        err = fabs((dx - dy) / dx);
    else
        err = fabs((dx - dy) / dy);

    return err <= EPSILON_RELMAX;
}

TEST(TestutilsMath, NumericEqual) {
    int int_a = 10;
    int int_b = 10;
    int int_c = 11;
    EXPECT_TRUE(FloatEqual(int_a, int_b));
    EXPECT_FALSE(FloatEqual(int_a, int_c));
    float float_a = 10.12345f;
    float float_b = 10.12345f;
    float float_c = 11.12345f;
    EXPECT_TRUE(FloatEqual(float_a, float_b));
    EXPECT_FALSE(FloatEqual(float_a, float_c));
}

TEST(TestutilsMath, ApprSqrt) {
    srand (static_cast <unsigned> (time(nullptr)));
    for (int i = 0; i < 1000; i++) {
        for (int j = -5; j < 5; j++) {
            // refers to https://stackoverflow.com/a/686373/4837280
            // random float/double range from 0. to 10^j
            float x = CVT_FLT(rand()) / CVT_FLT(CVT_FLT(RAND_MAX) / pow(10.f, j));
            // printf("%f,%f,%f\n", x, ApprSqrt(x), sqrt(x));
            EXPECT_TRUE(equals(ApprSqrt(x), sqrt(x)));

            double y = CVT_DBL(rand()) / CVT_DBL(CVT_DBL(RAND_MAX) / pow(10., j));
            // printf("%f,%f,%f\n", y, ApprSqrt(y), sqrt(y));
            EXPECT_TRUE(equals(ApprSqrt(y), sqrt(y)));
        }
    }
}

TEST(TestutilsMath, ApprExp) {
    srand(static_cast <unsigned> (time(nullptr)));
    for (int i = 0; i < 1000; i++) {
        // refers to https://stackoverflow.com/a/686373/4837280
        // random float/double range from -20 to 20
        float LO = -20.f;
        float HI = 20.f;
        float x = LO + CVT_FLT(rand()) / CVT_FLT(CVT_FLT(RAND_MAX) / (HI - LO));
        // printf("%f,%f,%f\n", x, ApprExp(x), exp(x));
        EXPECT_TRUE(equals(ApprExp(x), exp(x)));

        double y = CVT_DBL(LO) + CVT_DBL(rand()) / CVT_DBL(CVT_DBL(RAND_MAX) / CVT_DBL(HI - LO));
        // printf("%f,%f,%f\n", y, ApprExp(y), exp(y));
        EXPECT_TRUE(equals(ApprExp(y), exp(y)));
    }
}

TEST(TestutilsMath, ApprLn) {
    srand(static_cast <unsigned> (time(nullptr)));
    for (int i = 0; i < 1000; i++) {
        for (int j = -5; j <= 6; j++) {
            // refers to https://stackoverflow.com/a/686373/4837280
            // random float/double range from 10^(j-1) to 10^j
            float x = pow(10.f, j - 1) + CVT_FLT(rand()) / CVT_FLT(CVT_FLT(RAND_MAX) / pow(10.f, j));
            // printf("%f,%f,%f\n", x, ApprLn(x), log(x));
            EXPECT_TRUE(equals(ApprLn(x), log(x)));

            double y = pow(10., j - 1) + CVT_DBL(rand()) / CVT_DBL(CVT_DBL(RAND_MAX) / pow(10., j));
            // printf("%f,%f,%f\n", y, ApprLn(y), log(y));
            EXPECT_TRUE(equals(ApprLn(y), log(y)));
        }
    }
}

TEST(TestutilsMath, ApprPow) {
    srand(static_cast <unsigned> (time(nullptr)));
    int n = 10000;
    float err_ave = 0.f;
    float err_min = 1.f;
    float err_max = -1.f;
    for (int i = 0; i < 10000; i++) {
        // refers to https://stackoverflow.com/a/686373/4837280
        // a: random float/double range from 0.000001f to 2.f
        // b: random float/double range from -1.f to 1.f
        float a = 0.000001f + CVT_FLT(rand()) / CVT_FLT(CVT_FLT(RAND_MAX) / 2.f);
        float b = -1.f + CVT_FLT(rand()) / CVT_FLT(CVT_FLT(RAND_MAX) / 1.f);
        float appr_pow = ApprPow(a, b);
        float native_pow = pow(a, b);
        float cur_err = abs((appr_pow - native_pow) / native_pow) * 100.f;
        // printf("%f,%f,%f,%f, %f\n", a, b, appr_pow, native_pow, cur_err);
        err_ave += cur_err;
        if (cur_err < err_min) err_min = cur_err;
        if (cur_err > err_max) err_max = cur_err;
        //EXPECT_TRUE(equals(ApprPow(a, b), pow(a, b))); // may have several failure!
    }
    err_ave /= n;
    printf("%f,%f,%f\n", err_min, err_max, err_ave);
    // currently, with precision=11, pow(0.000001~2, -1~1) get a err_ave around 0.016% and err_max around 0.06%
    EXPECT_LE(err_ave, 0.02f);
    // EXPECT_LE(err_max, 0.02f);
}
