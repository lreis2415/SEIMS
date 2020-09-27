#include "SoilTemperatureFINPL.h"

#include "text.h"
#include "utils_time.h"

SoilTemperatureFINPL::SoilTemperatureFINPL() :
    m_a0(NODATA_VALUE), m_a1(NODATA_VALUE), m_a2(NODATA_VALUE),
    m_a3(NODATA_VALUE),
    m_b1(NODATA_VALUE), m_b2(NODATA_VALUE), m_d1(NODATA_VALUE),
    m_d2(NODATA_VALUE),
    m_kSoil10(NODATA_VALUE), m_nCells(-1),
    m_soilTempRelFactor10(nullptr),
    m_landUse(nullptr), m_meanTemp(nullptr), m_meanTempPre1(nullptr),
    m_meanTempPre2(nullptr),
    m_soilTemp(nullptr) {
}

SoilTemperatureFINPL::~SoilTemperatureFINPL() {
    if (m_soilTemp != nullptr) Release1DArray(m_soilTemp);
    if (m_meanTempPre1 != nullptr) Release1DArray(m_meanTempPre1);
    if (m_meanTempPre2 != nullptr) Release1DArray(m_meanTempPre2);
}

int SoilTemperatureFINPL::Execute() {
    CheckInputData();
    InitialOutputs();
    size_t errCount = 0;
#pragma omp parallel for reduction(+: errCount)
    for (int i = 0; i < m_nCells; i++) {
        float t = m_meanTemp[i];
        float t1 = m_meanTempPre1[i];
        float t2 = m_meanTempPre2[i];
        if ((t > 60.f || t < -90.f) || (t1 > 60.f || t1 < -90.f) || (t2 > 60.f || t2 < -90.f)) {
            cout << "cell index: " << i << ", t1: " << t1 << ", t2: " << t2 << endl;
            errCount++;
        } else {
            if (FloatEqual(CVT_INT(m_landUse[i]), LANDUSE_ID_WATR)) {
                /// if current landuse is water
                m_soilTemp[i] = t;
            } else {
                float t10 = m_a0 + m_a1 * t2 + m_a2 * t1 + m_a3 * t
                        + m_b1 * sin(radWt * m_dayOfYear) + m_d1 * cos(radWt * m_dayOfYear)
                        + m_b2 * sin(2.f * radWt * m_dayOfYear) + m_d2 * cos(2.f * radWt * m_dayOfYear);
                m_soilTemp[i] = t10 * m_kSoil10 + m_soilTempRelFactor10[i];
                if (m_soilTemp[i] > 60.f || m_soilTemp[i] < -90.f) {
                    cout << "The calculated soil temperature at cell (" << i
                            << ") is out of reasonable range: " << m_soilTemp[i]
                            << ". JulianDay: " << m_dayOfYear << ",t: " << t << ", t1: "
                            << t1 << ", t2: " << t2 << ", relativeFactor: " << m_soilTempRelFactor10[i] << endl;
                    errCount++;
                }
            }
            //save the temperature
            m_meanTempPre2[i] = m_meanTempPre1[i];
            m_meanTempPre1[i] = t;
        }
    }
    if (errCount > 0) {
        throw ModelException(MID_STP_FP, "Execute", "The calculation of soil temperature failed!");
    }
    return 0;
}

bool SoilTemperatureFINPL::CheckInputData() {
    CHECK_POSITIVE(MID_STP_FP, m_nCells);
    CHECK_NODATA(MID_STP_FP, m_a0);
    CHECK_NODATA(MID_STP_FP, m_a1);
    CHECK_NODATA(MID_STP_FP, m_a2);
    CHECK_NODATA(MID_STP_FP, m_a3);
    CHECK_NODATA(MID_STP_FP, m_b1);
    CHECK_NODATA(MID_STP_FP, m_b2);
    CHECK_NODATA(MID_STP_FP, m_d1);
    CHECK_NODATA(MID_STP_FP, m_d2);
    CHECK_NODATA(MID_STP_FP, m_kSoil10);
    CHECK_POINTER(MID_STP_FP, m_soilTempRelFactor10);
    CHECK_POINTER(MID_STP_FP, m_meanTemp);
    CHECK_POINTER(MID_STP_FP, m_landUse);
    return true;
}

void SoilTemperatureFINPL::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, VAR_SOL_TA0)) m_a0 = value;
    else if (StringMatch(sk, VAR_SOL_TA1)) m_a1 = value;
    else if (StringMatch(sk, VAR_SOL_TA2)) m_a2 = value;
    else if (StringMatch(sk, VAR_SOL_TA3)) m_a3 = value;
    else if (StringMatch(sk, VAR_SOL_TB1)) m_b1 = value;
    else if (StringMatch(sk, VAR_SOL_TB2)) m_b2 = value;
    else if (StringMatch(sk, VAR_SOL_TD1)) m_d1 = value;
    else if (StringMatch(sk, VAR_SOL_TD2)) m_d2 = value;
    else if (StringMatch(sk, VAR_K_SOIL10)) m_kSoil10 = value;
}

void SoilTemperatureFINPL::Set1DData(const char* key, const int n, float* data) {
    CheckInputSize(MID_STP_FP, key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_SOIL_T10)) m_soilTempRelFactor10 = data;
    else if (StringMatch(sk, VAR_TMEAN)) m_meanTemp = data;
    else if (StringMatch(sk, VAR_LANDUSE)) m_landUse = data;
    else {
        throw ModelException(MID_STP_FP, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void SoilTemperatureFINPL::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_SOTE)) *data = m_soilTemp;
    else if (StringMatch(sk, VAR_TMEAN1)) *data = m_meanTempPre1;
    else if (StringMatch(sk, VAR_TMEAN2)) *data = m_meanTempPre2;
    else {
        throw ModelException(MID_STP_FP, "Get1DData", "Parameter " + sk + " does not exist in current module.");
    }
}

void SoilTemperatureFINPL::InitialOutputs() {
    CHECK_POSITIVE(MID_STP_FP, m_nCells);
    // initialize m_t1 and m_t2 as m_tMean
    if (nullptr == m_meanTempPre1 && m_meanTemp != nullptr) {
        Initialize1DArray(m_nCells, m_meanTempPre1, m_meanTemp);
    }
    if (nullptr == m_meanTempPre2 && m_meanTemp != nullptr) {
        Initialize1DArray(m_nCells, m_meanTempPre2, m_meanTemp);
    }
    if (nullptr == m_soilTemp) {
        Initialize1DArray(m_nCells, m_soilTemp, 0.f);
    }
}
