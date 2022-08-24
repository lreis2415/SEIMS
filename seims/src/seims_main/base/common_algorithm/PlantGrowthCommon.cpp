#include "PlantGrowthCommon.h"

#include "utils_math.h"
#include <cmath>

using namespace ccgl::utils_math;

void GetNPShapeParameter(const FLTPT fr1, const FLTPT fr2, const FLTPT fr3, FLTPT* shape1, FLTPT* shape2) {
    FLTPT tmpfr2 = fr2;
    FLTPT tmpfr3 = fr3;
    if (fr1 - fr2 < 0.0001) tmpfr2 = fr1 - 0.0001;
    if (fr2 - fr3 < 0.0001) tmpfr3 *= 0.75;

    FLTPT t = fr1 - tmpfr3;
    FLTPT x_mid = 1. - (tmpfr2 - tmpfr3) / t;
    FLTPT x_end = 1. - 0.00001 / t;

    GetScurveShapeParameter(x_mid, x_end, 0.5, 1., shape1, shape2);
}

void GetScurveShapeParameter(const FLTPT x_mid, const FLTPT x_end,
                             const FLTPT y_mid, const FLTPT y_end,
                             FLTPT* shape1, FLTPT* shape2) {
    FLTPT xx = log(y_mid / x_mid - y_mid);
    *shape2 = (xx - log(y_end / x_end - y_end)) / (y_end - y_mid);
    *shape1 = xx + y_mid * (*shape2);
}

FLTPT GetNPFraction(const FLTPT fr1, const FLTPT fr3, const FLTPT shape1,
                    const FLTPT shape2, FLTPT const fr_phu) {
    return (fr1 - fr3) * (1. - fr_phu / (fr_phu + exp(shape1 - shape2 * fr_phu))) + fr3;
}

FLTPT RadiationUseEfficiencyAdjustByVPD(const FLTPT vpd,
                                        const FLTPT rad_use_eff_dec_rate_with_vpd) {
    FLTPT threshold_vpd = 1.;
    if (vpd <= threshold_vpd) return 0.;
    return rad_use_eff_dec_rate_with_vpd * (vpd - threshold_vpd);
}

FLTPT GetNormalization(const FLTPT distribution) {
    return 1. - exp(-distribution);
}

FLTPT DoHeatUnitAccumulation(const FLTPT potential_heat_unit, const FLTPT t_min,
                             const FLTPT t_max, const FLTPT t_base) {
    if (potential_heat_unit <= 0.1) {
        return 0.;
    }
    FLTPT fr_accumulated_heat_unit = 0.;
    FLTPT t_mean = (t_min + t_max) / 2.;
    //tbase = m_param->MinTemperature();
    if (t_mean <= t_base) { return 0.; }
    fr_accumulated_heat_unit += (t_mean - t_base) / potential_heat_unit;
    return fr_accumulated_heat_unit;
}

FLTPT NPBiomassFraction(const FLTPT x1, const FLTPT x2, const FLTPT x3, const FLTPT fr_phu) {
    FLTPT shape_coefficient1 = 0.;
    FLTPT shape_coefficient2 = 0.;
    GetNPShapeParameter(x1, x2, x3, &shape_coefficient1, &shape_coefficient2);
    return GetNPFraction(x1, x3, shape_coefficient1, shape_coefficient2, fr_phu);
}

void CalPlantStressByLimitedNP(const FLTPT u1, const FLTPT u2, FLTPT* uu) {
    FLTPT strsf = 200. * (u1 / (u2 + 0.0001) - 0.5);
    if (strsf <= 0.) {
        strsf = 0.;
    } else {
        if (strsf < 99.) {
            strsf = strsf / (strsf + exp(3.535 - 0.02597 * strsf));
        } else {
            strsf = 1.;
        }
    }
    if (u2 < UTIL_ZERO) { strsf = 1.; }
    *uu = strsf;
}
