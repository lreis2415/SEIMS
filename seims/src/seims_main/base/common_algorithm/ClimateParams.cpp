#include "ClimateParams.h"

#include <cmath>

#include "utils_time.h"
#include "utils_math.h"

using namespace ccgl::utils_time;
using namespace ccgl::utils_math;

float MeanBarometricPressure(const float elev) {
    return 101.3f - elev * (0.01152f - 0.544e-6f * elev);
}

float LatentHeatVapor(const float tmean) {
    return 2.501f - 0.002361f * tmean;
}

void MaxSolarRadiation(const int day, const float lat, float& day_l, float& max_sr) {
    float lat_rad = lat * PI / 180.f;
    //Calculate Daylength
    //calculate solar declination: equation 1:1.1.2 in SWAT Theory 2009, p31
    float sd = asin(0.4f * sin((day - 82.0f) / 58.09f)); /// 365/2pi = 58.09

    //calculate the relative distance of the earth from the sun
    //also called the eccentricity correction factor of the orbit, Duffie and Beckman(1980)
    //equation 1:1.1.1 in SWAT Theory 2009, p30
    float dd = 1.0f + 0.033f * cos(day / 58.09f);
    //daylength = 2 * Acos(-Tan(sd) * Tan(lat_rad)) / omega
    //where the angular velocity of the earth's rotation, omega, is equal
    //to 15 deg/hr or 0.2618 rad/hr and 2/0.2618 = 7.6374
    //equation 2.1.6 in SWAT manual

    float h = 0.0f;
    /// equation 1:1.1.4 in SWAT Theory 2009, p32
    float ch = -sin(lat_rad) * tan(sd) / cos(lat_rad);
    if (ch > 1.f) {
        //ch will be >= 1. if latitude exceeds +/- 66.5 deg in winter
        h = 0.0f;
    } else if (ch >= -1.0f) {
        h = acos(ch);
    } else {
        h = PI;
    } //latitude exceeds +/- 66.5 deg in summer
    day_l = 7.6394f * h;
    //Calculate Potential (maximum) Radiation !!
    /// equation 1:1.1.3 in SWAT Theory 2009, p31
    float ys = sin(lat_rad) * sin(sd);
    float yc = cos(lat_rad) * cos(sd);
    /// equation 1:1.1.7 in SWAT Theory 2009, p34
    max_sr = 30.f * dd * (h * ys + yc * sin(h));
}

float PsychrometricConst(const float tmean, const float elev) {
    float pb = MeanBarometricPressure(elev);
    float xl = LatentHeatVapor(tmean);
    return 1.013e-3f * pb / (0.622f * xl);
}

float SaturationVaporPressure(const float mean_tmp) {
    /// Calculate saturation vapor pressure, equation 1:2.3.2 in SWAT Theory 2009, p54
    /// Tetens (1930) and Murray (1967), ee.f in SWAT src.
    if (Abs(mean_tmp + 237.3f) > UTIL_ZERO) {
        float ea = (16.78f * mean_tmp - 116.9f) / (mean_tmp + 237.3f);
        return exp(ea);
    }
    return UTIL_ZERO;
}
