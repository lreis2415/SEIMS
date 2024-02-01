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
        FLTPT t = m_meanTemp[i];
        FLTPT t1 = m_meanTempPre1[i];
        FLTPT t2 = m_meanTempPre2[i];
        if ((t > 60. || t < -90.) || (t1 > 60. || t1 < -90.) || (t2 > 60. || t2 < -90.)) {
            cout << "cell index: " << i << ", t1: " << t1 << ", t2: " << t2 << endl;
            errCount++;
        } else {
            if (m_landUse[i] == LANDUSE_ID_WATR) { /// if current landuse is water
                m_soilTemp[i] = t;
            } else {
                FLTPT t10 = m_a0 + m_a1 * t2 + m_a2 * t1 + m_a3 * t
                        + m_b1 * sin(radWt * m_dayOfYear) + m_d1 * cos(radWt * m_dayOfYear)
                        + m_b2 * sin(2. * radWt * m_dayOfYear) + m_d2 * cos(2. * radWt * m_dayOfYear);
                m_soilTemp[i] = t10 * m_kSoil10 + m_soilTempRelFactor10[i];
                if (m_soilTemp[i] > 60. || m_soilTemp[i] < -90.) {
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
        throw ModelException(M_STP_FP[0], "Execute", "The calculation of soil temperature failed!");
    }
    return 0;
}

bool SoilTemperatureFINPL::CheckInputData() {
    CHECK_POSITIVE(M_STP_FP[0], m_nCells);
    CHECK_NODATA(M_STP_FP[0], m_a0);
    CHECK_NODATA(M_STP_FP[0], m_a1);
    CHECK_NODATA(M_STP_FP[0], m_a2);
    CHECK_NODATA(M_STP_FP[0], m_a3);
    CHECK_NODATA(M_STP_FP[0], m_b1);
    CHECK_NODATA(M_STP_FP[0], m_b2);
    CHECK_NODATA(M_STP_FP[0], m_d1);
    CHECK_NODATA(M_STP_FP[0], m_d2);
    CHECK_NODATA(M_STP_FP[0], m_kSoil10);
    CHECK_POINTER(M_STP_FP[0], m_soilTempRelFactor10);
    CHECK_POINTER(M_STP_FP[0], m_meanTemp);
    CHECK_POINTER(M_STP_FP[0], m_landUse);
    return true;
}

void SoilTemperatureFINPL::SetValue(const char* key, const FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_SOL_TA0[0])) m_a0 = value;
    else if (StringMatch(sk, VAR_SOL_TA1[0])) m_a1 = value;
    else if (StringMatch(sk, VAR_SOL_TA2[0])) m_a2 = value;
    else if (StringMatch(sk, VAR_SOL_TA3[0])) m_a3 = value;
    else if (StringMatch(sk, VAR_SOL_TB1[0])) m_b1 = value;
    else if (StringMatch(sk, VAR_SOL_TB2[0])) m_b2 = value;
    else if (StringMatch(sk, VAR_SOL_TD1[0])) m_d1 = value;
    else if (StringMatch(sk, VAR_SOL_TD2[0])) m_d2 = value;
    else if (StringMatch(sk, VAR_K_SOIL10[0])) m_kSoil10 = value;
}

void SoilTemperatureFINPL::Set1DData(const char* key, const int n, int* data) {
    CheckInputSize(key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_LANDUSE[0])) m_landUse = data;
    else {
        throw ModelException(M_STP_FP[0], "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void SoilTemperatureFINPL::Set1DData(const char* key, const int n, FLTPT* data) {
    CheckInputSize(key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_SOIL_T10[0])) m_soilTempRelFactor10 = data;
    else if (StringMatch(sk, VAR_TMEAN[0])) m_meanTemp = data;
    else {
        throw ModelException(M_STP_FP[0], "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void SoilTemperatureFINPL::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_SOTE[0])) *data = m_soilTemp;
    else if (StringMatch(sk, VAR_TMEAN1[0])) *data = m_meanTempPre1;
    else if (StringMatch(sk, VAR_TMEAN2[0])) *data = m_meanTempPre2;
    else {
        throw ModelException(M_STP_FP[0], "Get1DData", "Parameter " + sk + " does not exist.");
    }
}

void SoilTemperatureFINPL::InitialOutputs() {
    CHECK_POSITIVE(M_STP_FP[0], m_nCells);
    // initialize m_t1 and m_t2 as m_tMean
    if (nullptr == m_meanTempPre1 && m_meanTemp != nullptr) {
        Initialize1DArray(m_nCells, m_meanTempPre1, m_meanTemp);
    }
    if (nullptr == m_meanTempPre2 && m_meanTemp != nullptr) {
        Initialize1DArray(m_nCells, m_meanTempPre2, m_meanTemp);
    }
    if (nullptr == m_soilTemp) {
        Initialize1DArray(m_nCells, m_soilTemp, 0.);
    }
}
