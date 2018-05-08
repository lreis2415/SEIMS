/*!
 * \brief Define some common used function in Plant growth module, e.g., BIO_EPIC
 * \author Zhiqiang Yu
 * \date 2011-8-10
 * \revised Liang-Jun Zhu
 * \date June, 2016
 */
#ifndef SEIMS_PLANTGROWTH_COMMON_H
#define SEIMS_PLANTGROWTH_COMMON_H

#include "basic.h"
#include "BMPText.h"

using namespace ccgl;

#define MAX_CROP_LAND_USE_ID   97
#define BARE_SOIL_LAND_USE     98
#define WATER_LAND_USE_ID      18


//get N/P shape parameter, See readplant.f of SWAT
void GetNPShapeParameter(float fr1, float fr2, float fr3, float* shape1, float* shape2);

//computes shape parameters shape1 and shape2 for the S curve
//equation x = y/(y + exp(shape1 + shape2*y)) given 2 (x,y) points along the curve.
//See ascrv.f of SWAT
void GetScurveShapeParameter(float x_mid, float x_end, float y_mid, float y_end,
                             float* shape1, float* shape2);

///
float NPBiomassFraction(float x1, float x2, float x3, float fr_phu);

//plant nitrogen/phosphorus equation, p300 5:2.3.1/p305 5:2.3.19
//calculate the fraction of nitrogen/phosphorus in the plant biomass
float GetNPFraction(float fr1, float fr3, float shape1, float shape2, float fr_phu);

float DoHeatUnitAccumulation(float potential_heat_unit, float t_min, float t_max, float t_base);

//the adjusted radiation-use efficiency by CO2 concentration
// float RadiationUseEfficiencyAdjustByCO2(float co2) const;

//the adjusted radiation-use efficiency by vapor pressure deficit
float RadiationUseEfficiencyAdjustByVPD(float vpd, float rad_use_eff_dec_rate_with_vpd);

float GetNormalization(float distribution);

inline bool IsTree(const int classification) { return classification == 7; }

inline bool IsAnnual(const int classification) {
    return classification == CROP_IDC_WARM_SEASON_ANNUAL_LEGUME ||
            classification == CROP_IDC_CODE_SEASON_ANNUAL_LEGUME ||
            classification == CROP_IDC_WARM_SEASON_ANNUAL ||
            classification == CROP_IDC_COLD_SEASON_ANNUAL;
}

inline bool IsLegume(const int classification) {
    return classification <= CROP_IDC_PERENNIAL_LEGUME;
}

inline bool IsPerennial(const int classification) {
    return classification == CROP_IDC_PERENNIAL_LEGUME ||
            classification == CROP_IDC_PERENNIAL;
}

inline bool IsCoolSeasonAnnual(const int classification) {
    return classification == CROP_IDC_CODE_SEASON_ANNUAL_LEGUME ||
            classification == CROP_IDC_COLD_SEASON_ANNUAL;
}

inline bool IsGrain(const int classification) {
    return classification == CROP_IDC_WARM_SEASON_ANNUAL;
}

inline bool IsPlant(const int lu_id) {
    return lu_id <= MAX_CROP_LAND_USE_ID && lu_id != WATER_LAND_USE_ID;
}

/// added by Liang-Jun Zhu, 2016-6-8

/// calculates the plant stress factor caused by limited supply of nitrogen or phosphorus
/// from ntus.f of SWAT, rev 637
void CalPlantStressByLimitedNP(float u1, float u2, float* uu);

#endif /* SEIMS_PLANTGROWTH_COMMON_H */
