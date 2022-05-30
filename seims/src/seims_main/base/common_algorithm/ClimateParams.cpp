#include "ClimateParams.h"

#include <cmath>

#include "utils_time.h"
#include "utils_math.h"

using namespace ccgl::utils_time;
using namespace ccgl::utils_math;

FLTPT MeanBarometricPressure(const FLTPT elev) {
    return 101.3 - elev * (0.01152 - 0.544e-6 * elev);
}

FLTPT LatentHeatVapor(const FLTPT tmean) {
    return 2.501 - 0.002361 * tmean;
}

void MaxSolarRadiation(const int day, const FLTPT lat, FLTPT& day_l, FLTPT& max_sr) {
    FLTPT lat_rad = lat * PI / 180.;
    //Calculate Daylength
    //calculate solar declination: equation 1:1.1.2 in SWAT Theory 2009, p31
    FLTPT sd = asin(0.4 * sin((day - 82.0) / 58.09)); /// 365/2pi = 58.09

    //calculate the relative distance of the earth from the sun
    //also called the eccentricity correction factor of the orbit, Duffie and Beckman(1980)
    //equation 1:1.1.1 in SWAT Theory 2009, p30
    FLTPT dd = 1.0 + 0.033 * cos(day / 58.09);
    //daylength = 2 * Acos(-Tan(sd) * Tan(lat_rad)) / omega
    //where the angular velocity of the earth's rotation, omega, is equal
    //to 15 deg/hr or 0.2618 rad/hr and 2/0.2618 = 7.6374
    //equation 2.1.6 in SWAT manual

    FLTPT h = 0.0;
    /// equation 1:1.1.4 in SWAT Theory 2009, p32
    FLTPT ch = -sin(lat_rad) * tan(sd) / cos(lat_rad);
    if (ch > 1.) {
        //ch will be >= 1. if latitude exceeds +/- 66.5 deg in winter
        h = 0.0;
    } else if (ch >= -1.0) {
        h = acos(ch);
    } else {
        h = PI;
    } //latitude exceeds +/- 66.5 deg in summer
    day_l = 7.6394 * h;
    //Calculate Potential (maximum) Radiation !!
    /// equation 1:1.1.3 in SWAT Theory 2009, p31
    FLTPT ys = sin(lat_rad) * sin(sd);
    FLTPT yc = cos(lat_rad) * cos(sd);
    /// equation 1:1.1.7 in SWAT Theory 2009, p34
    max_sr = 30. * dd * (h * ys + yc * sin(h));
}

FLTPT PsychrometricConst(const FLTPT tmean, const FLTPT elev) {
    FLTPT pb = MeanBarometricPressure(elev);
    FLTPT xl = LatentHeatVapor(tmean);
    return 1.013e-3 * pb / (0.622 * xl);
}

FLTPT SaturationVaporPressure(const FLTPT mean_tmp) {
    /// Calculate saturation vapor pressure, equation 1:2.3.2 in SWAT Theory 2009, p54
    /// Tetens (1930) and Murray (1967), ee.f in SWAT src.
    if (Abs(mean_tmp + 237.3) > UTIL_ZERO) {
        FLTPT ea = (16.78 * mean_tmp - 116.9) / (mean_tmp + 237.3);
        return exp(ea);
    }
    return UTIL_ZERO;
}
