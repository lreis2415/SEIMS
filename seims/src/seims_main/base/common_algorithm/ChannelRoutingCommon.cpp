#include "ChannelRoutingCommon.h"

#include <cmath>

float manningQ(const float x1, const float x2, const float x3, const float x4) {
    return x1 * pow(x2, 0.6666f) * sqrt(x4) / x3;
}
