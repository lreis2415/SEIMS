#include "ChannelRoutingCommon.h"

#include "basic.h"
#include <cmath>

FLTPT manningQ(const FLTPT x1, const FLTPT x2, const FLTPT x3, const FLTPT x4) {
    return x1 * CalPow(x2, (FLTPT)0.6666) * CalSqrt(x4) / x3;
}

FLTPT ChannleBottomWidth(const FLTPT ch_wth, FLTPT& ch_sideslp, FLTPT& ch_depth) {
    FLTPT btm_wth = ch_wth - 2. * ch_sideslp * ch_depth;
    if (btm_wth < UTIL_ZERO) {
        btm_wth = 0.5 * ch_wth;
        ch_sideslp = (ch_wth - btm_wth) * 0.5 / ch_depth;
    }
    return btm_wth;
}

FLTPT ChannelWettingPerimeter(const FLTPT ch_btmwth, const FLTPT ch_depth, const FLTPT wtr_depth,
                              const FLTPT ch_sideslp, const FLTPT ch_wth, const FLTPT fps /* = 4. */) {
    if (wtr_depth <= ch_depth) {
        return ch_btmwth + 2. * wtr_depth * CalSqrt(ch_sideslp * ch_sideslp + 1.);
    }
    // wtr_depth greater than ch_depth means floodplanin. Eq. 7:1.1.10 in SWAT theory 2009.
    FLTPT p = ch_btmwth + 2. * ch_depth * CalSqrt(ch_sideslp * ch_sideslp + 1.);
    p += 4. * ch_wth + 2. * (wtr_depth - ch_depth) * CalSqrt(fps * fps + 1.);
    return p;
}

FLTPT ChannelWettingPerimeter(const FLTPT ch_btmwth, const FLTPT wtr_depth, const FLTPT ch_sideslp) {
    return ChannelWettingPerimeter(ch_btmwth, wtr_depth, wtr_depth, ch_sideslp, ch_btmwth);
}

FLTPT ChannelCrossSectionalArea(const FLTPT ch_btmwth, const FLTPT ch_depth, const FLTPT wtr_depth,
                                const FLTPT ch_sideslp, const FLTPT ch_wth, const FLTPT fps /* = 4. */) {
    if (wtr_depth <= ch_depth) {
        return (ch_btmwth + ch_sideslp * wtr_depth) * wtr_depth;
    }
    // wtr_depth greater than ch_depth means floodplanin. Eq. 7:1.1.9 in SWAT theory 2009.
    // The source code in line 133 in ttcoef.f of SWAT is problematic.
    FLTPT cross_area = (ch_btmwth + ch_sideslp * ch_depth) * ch_depth;
    cross_area += ((fps + 1.) * ch_wth + fps * (wtr_depth - ch_depth)) * (wtr_depth - ch_depth);
    return cross_area;
}

FLTPT ChannelCrossSectionalArea(const FLTPT ch_btmwth, const FLTPT wtr_depth, const FLTPT ch_sideslp) {
    // Note: ch_wth and fps will not be used, thus can be any values.
    return ChannelCrossSectionalArea(ch_btmwth, wtr_depth, wtr_depth, ch_sideslp, ch_btmwth);
}

FLTPT StorageTimeConstant(const FLTPT ch_manning, const FLTPT ch_slope, const FLTPT ch_len,
                          const FLTPT radius) {
    // Average flow velocity (m/s) by Manning equation
    FLTPT velocity = manningQ(1., radius, ch_manning, ch_slope);
    // Wave celerity (m/s)
    FLTPT celerity = velocity * 5. / 3.;
    return ch_len / celerity / 3600.;
}
