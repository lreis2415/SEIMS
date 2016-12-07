/*!
 * \brief Define some common used function in Plant growth module, e.g., BIO_EPIC
 * \author Zhiqiang Yu
 * \date 2011-8-10
 * \revised Liang-Jun Zhu
 * \date June, 2016
 */
#pragma once
//using namespace std;
#define MAX_CROP_LAND_USE_ID 97
#define BARE_SOIL_LAND_USE     98
#define WATER_LAND_USE_ID     18

/*!
 * \class PGCommon
 * \ingroup data
 *
 * \brief A basic class for plant growth modules using a simplified version of EPIC model
 *
 */
class PGCommon
{
public:
    /// Constructor
    PGCommon(void);

    /// Destructor
    ~PGCommon(void);

public:
    //get N/P shape parameter, See readplant.f of SWAT
    static void getNPShapeParameter(float fr1, float fr2, float fr3, float *shape1, float *shape2);

    //computes shape parameters shape1 and shape2 for the S curve
    //equation x = y/(y + exp(shape1 + shape2*y)) given 2 (x,y) points along the curve.
    //See ascrv.f of SWAT
    static void getScurveShapeParameter(float xMid, float xEnd, float yMid, float yEnd, float *shape1, float *shape2);

    ///
    static float NPBiomassFraction(float x1, float x2, float x3, float frPHU);

    //plant nitrogen/phosphorus equation, p300 5:2.3.1/p305 5:2.3.19
    //calculate the fraction of nitrogen/phosphorus in the plant biomass
    static float getNPFraction(float fr1, float fr3, float shape1, float shape2, float frPHU);

public:
    float doHeatUnitAccumulation(float potentialHeatUnit, float tMin, float tMax, float tBase);

    //the adjusted radiation-use efficiency by CO2 concentration
    // float RadiationUseEfficiencyAdjustByCO2(float co2) const;

    //the adjusted radiation-use efficiency by vapor pressure deficit
    float RadiationUseEfficiencyAdjustByVPD(float vpd, float radiationUseEfficiencyDeclineRateWithVPD) const;


    static float getNormalization(float distribution);

    static bool IsTree(int m_classification) { return m_classification == 7; }

    static bool IsAnnual(int m_classification)
    {
        return
                m_classification == 1 ||
                m_classification == 2 ||
                m_classification == 4 ||
                m_classification == 5;
    }

    bool IsLegume(int m_classification) const { return m_classification <= 3; }

    bool IsPerennial(int m_classification) const
    {
        return
                m_classification == 3 ||
                m_classification == 6;
    }

    bool IsCoolSeasonAnnual(int m_classification) const
    {
        return
                m_classification == 2 ||
                m_classification == 5;
    }

    bool IsGrain(int m_classification) const { return m_classification == 4; }

    bool IsPlant(int luID) const
    {
        return luID <= MAX_CROP_LAND_USE_ID &&
               luID != WATER_LAND_USE_ID;
    }

public:
    /// added by Liang-Jun Zhu, 2016-6-8

    /// calculates the plant stress factor caused by limited supply of nitrogen or phosphorus
    /// from ntus.f of SWAT, rev 637
    static void calPlantStressByLimitedNP(float u1, float u2, float *uu);
};


