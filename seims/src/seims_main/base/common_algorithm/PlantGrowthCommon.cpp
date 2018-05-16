#include "PlantGrowthCommon.h"

#include <cmath>

void GetNPShapeParameter(const float fr1, const float fr2, const float fr3, float* shape1, float* shape2) {
    float tmpfr2 = fr2;
    float tmpfr3 = fr3;
    if (fr1 - fr2 < 0.0001f) tmpfr2 = fr1 - 0.0001f;
    if (fr2 - fr3 < 0.0001f) tmpfr3 *= 0.75f;

    float t = fr1 - tmpfr3;
    float x_mid = 1.0f - (tmpfr2 - tmpfr3) / t;
    float x_end = 1.0f - 0.00001f / t;

    GetScurveShapeParameter(x_mid, x_end, 0.5f, 1.0f, shape1, shape2);
}

void GetScurveShapeParameter(const float x_mid, const float x_end,
                             const float y_mid, const float y_end,
                             float* shape1, float* shape2) {
    float xx = log(y_mid / x_mid - y_mid);
    *shape2 = (xx - log(y_end / x_end - y_end)) / (y_end - y_mid);
    *shape1 = xx + y_mid * (*shape2);
}

float GetNPFraction(const float fr1, const float fr3, const float shape1,
                    const float shape2, float const fr_phu) {
    return (fr1 - fr3) * (1.0f - fr_phu / (fr_phu + exp(shape1 - shape2 * fr_phu))) + fr3;
}

float RadiationUseEfficiencyAdjustByVPD(const float vpd,
                                        const float rad_use_eff_dec_rate_with_vpd) {
    float threshold_vpd = 1.0f;
    if (vpd <= threshold_vpd) return 0.0f;
    return rad_use_eff_dec_rate_with_vpd * (vpd - threshold_vpd);
}

float GetNormalization(const float distribution) {
    return 1.f - exp(-distribution);
}

float DoHeatUnitAccumulation(const float potential_heat_unit, const float t_min,
                             const float t_max, const float t_base) {
    if (potential_heat_unit <= 0.1) {
        return 0.f;
    }
    float fr_accumulated_heat_unit = 0.f;
    float t_mean = (t_min + t_max) / 2.f;
    //tbase = m_param->MinTemperature();
    if (t_mean <= t_base) {
        return 0.f;
    }
    fr_accumulated_heat_unit += (t_mean - t_base) / potential_heat_unit;
    return fr_accumulated_heat_unit;
}

float NPBiomassFraction(const float x1, const float x2, const float x3, const float fr_phu) {
    float shape_coefficient1 = 0.f;
    float shape_coefficient2 = 0.f;
    GetNPShapeParameter(x1, x2, x3, &shape_coefficient1, &shape_coefficient2);
    return GetNPFraction(x1, x3, shape_coefficient1, shape_coefficient2, fr_phu);
}

void CalPlantStressByLimitedNP(const float u1, const float u2, float* uu) {
    float strsf = 200.f * (u1 / (u2 + 0.0001f) - 0.5f);
    if (strsf <= 0.f) {
        strsf = 0.f;
    } else {
        if (strsf < 99.f) {
            strsf = strsf / (strsf + exp(3.535f - 0.02597f * strsf));
        } else {
            strsf = 1.f;
        }
    }
    if (u2 < UTIL_ZERO) strsf = 1.f;
    *uu = strsf;
}
