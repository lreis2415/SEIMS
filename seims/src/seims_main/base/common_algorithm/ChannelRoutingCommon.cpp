#include "ChannelRoutingCommon.h"

#include "basic.h"
#include <cmath>

float manningQ(const float x1, const float x2, const float x3, const float x4) {
    return x1 * pow(x2, 0.6666f) * sqrt(x4) / x3;
}

float ChannleBottomWidth(const float ch_wth, float& ch_sideslp, float& ch_depth) {
    float btm_wth = ch_wth - 2.f * ch_sideslp * ch_depth;
    if (btm_wth < UTIL_ZERO) {
        btm_wth = 0.5f * ch_wth;
        ch_sideslp = (ch_wth - btm_wth) * 0.5f / ch_depth;
    }
    return btm_wth;
}

float ChannelWettingPerimeter(const float ch_btmwth, const float ch_depth, const float wtr_depth,
                              const float ch_sideslp, const float ch_wth, const float fps /* = 4.f */) {
    if (wtr_depth <= ch_depth) {
        return ch_btmwth + 2.f * wtr_depth * sqrt(ch_sideslp * ch_sideslp + 1.f);
    }
    // wtr_depth greater than ch_depth means floodplanin. Eq. 7:1.1.10 in SWAT theory 2009.
    float p = ch_btmwth + 2.f * ch_depth * sqrt(ch_sideslp * ch_sideslp + 1.f);
    p += 4.f * ch_wth + 2.f * (wtr_depth - ch_depth) * sqrt(fps * fps + 1.f);
    return p;
}

float ChannelWettingPerimeter(const float ch_btmwth, const float wtr_depth, const float ch_sideslp) {
    return ChannelWettingPerimeter(ch_btmwth, wtr_depth, wtr_depth, ch_sideslp, ch_btmwth);
}

float ChannelCrossSectionalArea(const float ch_btmwth, const float ch_depth, const float wtr_depth,
                                const float ch_sideslp, const float ch_wth, const float fps /* = 4.f */) {
    if (wtr_depth <= ch_depth) {
        return (ch_btmwth + ch_sideslp * wtr_depth) * wtr_depth;
    }
    // wtr_depth greater than ch_depth means floodplanin. Eq. 7:1.1.9 in SWAT theory 2009.
    // The source code in line 133 in ttcoef.f of SWAT is problematic.
    float cross_area = (ch_btmwth + ch_sideslp * ch_depth) * ch_depth;
    cross_area += ((fps + 1.f) * ch_wth + fps * (wtr_depth - ch_depth)) * (wtr_depth - ch_depth);
    return cross_area;
}

float ChannelCrossSectionalArea(const float ch_btmwth, const float wtr_depth, const float ch_sideslp) {
    // Note: ch_wth and fps will not be used, thus can be any values.
    return ChannelCrossSectionalArea(ch_btmwth, wtr_depth, wtr_depth, ch_sideslp, ch_btmwth);
}

float StorageTimeConstant(const float ch_manning, const float ch_slope, const float ch_len,
                          const float radius) {
    // Average flow velocity (m/s) by Manning equation
    float velocity = manningQ(1.f, radius, ch_manning, ch_slope);
    // Wave celerity (m/s)
    float celerity = velocity * 5.f / 3.f;
    return ch_len / celerity / 3600.f;
}
