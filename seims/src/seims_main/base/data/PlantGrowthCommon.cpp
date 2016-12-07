#include "PlantGrowthCommon.h"
#include "util.h"

PGCommon::PGCommon()
{
}

PGCommon::~PGCommon(void)
{
}

void PGCommon::getNPShapeParameter(float fr1, float fr2, float fr3, float *shape1, float *shape2)
{
    if (fr1 - fr2 < 0.0001f) fr2 = fr1 - 0.0001f;
    if (fr2 - fr3 < 0.0001f) fr3 *= 0.75f;

    float t = fr1 - fr3;
    float xMid = 1.0f - (fr2 - fr3) / t;
    float xEnd = 1.0f - 0.00001f / t;

    getScurveShapeParameter(xMid, xEnd, 0.5f, 1.0f, shape1, shape2);
}

void PGCommon::getScurveShapeParameter(float xMid, float xEnd, float yMid, float yEnd, float *shape1, float *shape2)
{
    float xx = log(yMid / xMid - yMid);
    *shape2 = (xx - log(yEnd / xEnd - yEnd)) / (yEnd - yMid);
    *shape1 = xx + (yMid * (*shape2));
}

float PGCommon::getNPFraction(float fr1, float fr3, float shape1, float shape2, float frPHU)
{
    return (fr1 - fr3) * (1.0f - frPHU / (frPHU + exp(shape1 - shape2 * frPHU))) + fr3;
}


float PGCommon::RadiationUseEfficiencyAdjustByVPD(float vpd, float radiationUseEfficiencyDeclineRateWithVPD) const
{
    float thresholdVPD = 1.0f;
    if (vpd <= thresholdVPD) return 0.0f;
    return radiationUseEfficiencyDeclineRateWithVPD * (vpd - thresholdVPD);
}

float PGCommon::getNormalization(float distribution)
{
    return 1.f - exp(-distribution);
}

float PGCommon::doHeatUnitAccumulation(float potentialHeatUnit, float tMin, float tMax, float tBase)
{
    if (potentialHeatUnit <= 0.1)
        return 0.f;
    float frAccumulatedHeatUnit = 0.f;
    float tMean = (tMin + tMax) / 2.f;
    //tbase = m_param->MinTemperature();
    if (tMean <= tBase)
        return 0.f;
    frAccumulatedHeatUnit += (tMean - tBase) / potentialHeatUnit;
    return frAccumulatedHeatUnit;
}

float PGCommon::NPBiomassFraction(float x1, float x2, float x3, float frPHU)
{
    float ShapeCoefficient1 = 0.f;
    float ShapeCoefficient2 = 0.f;
    getNPShapeParameter(x1, x2, x3, &(ShapeCoefficient1), &(ShapeCoefficient2));
    return getNPFraction(x1, x3, ShapeCoefficient1, ShapeCoefficient2, frPHU);
}

void PGCommon::calPlantStressByLimitedNP(float u1, float u2, float *uu)
{
    float strsf = 200.f * (u1 / (u2 + 0.0001f) - 0.5f);
    if (strsf <= 0.f)
        strsf = 0.f;
    else
    {
        if (strsf < 99.f)
            strsf = strsf / (strsf + exp(3.535f - 0.02597f * strsf));
        else
            strsf = 1.f;
    }
    if (u2 < UTIL_ZERO) strsf = 1.f;
    *uu = strsf;
}
