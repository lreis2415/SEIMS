#include "utils_math.h"

namespace ccgl {
namespace utils_math {
float Expo(float xx, float upper /* = 20.f */, float lower /* = -20.f */) {
    if (xx < lower) xx = lower;
    if (xx > upper) xx = upper;
    return exp(xx);
}

float Power(const float a, const float n) {
    if (a >= 0.f) {
        return pow(a, n);
    }
    return -pow(-a, n);
}
} /* namespace: utils_math */

} /* namespace: ccgl */
