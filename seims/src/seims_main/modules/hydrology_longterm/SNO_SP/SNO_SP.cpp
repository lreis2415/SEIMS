#include "seims.h"
#include "ClimateParams.h"
#include "PlantGrowthCommon.h"

#include "SNO_SP.h"

SNO_SP::SNO_SP() : m_nCells(-1), m_t0(NODATA_VALUE), m_kblow(NODATA_VALUE), m_tsnow(NODATA_VALUE),
                   m_lagSnow(NODATA_VALUE), m_csnow6(NODATA_VALUE), m_csnow12(NODATA_VALUE),
                   m_snowCoverMax(NODATA_VALUE), m_snowCover50(NODATA_VALUE),
                   m_snowCoverCoef1(NODATA_VALUE), m_snowCoverCoef2(NODATA_VALUE),
                   m_Pnet(nullptr), m_tMean(nullptr), m_tMax(nullptr),
                   m_SE(nullptr), m_SR(nullptr), m_SM(nullptr), m_SA(nullptr), m_packT(nullptr) {
}

SNO_SP::~SNO_SP(void) {
    if (m_SM != nullptr) Release1DArray(m_SM);
    if (m_SA != nullptr) Release1DArray(m_SA);
    if (m_packT != nullptr) Release1DArray(m_packT);
}

bool SNO_SP::CheckInputData() {
    CHECK_POSITIVE(MID_SNO_SP, m_nCells);
    CHECK_NODATA(MID_SNO_SP, m_t0);
    CHECK_NODATA(MID_SNO_SP, m_kblow);
    CHECK_NODATA(MID_SNO_SP, m_tsnow);
    CHECK_NODATA(MID_SNO_SP, m_lagSnow);
    CHECK_NODATA(MID_SNO_SP, m_csnow6);
    CHECK_NODATA(MID_SNO_SP, m_csnow12);
    CHECK_NODATA(MID_SNO_SP, m_snowCoverMax);
    CHECK_NODATA(MID_SNO_SP, m_snowCover50);
    CHECK_POINTER(MID_SNO_SP, m_tMean);
    CHECK_POINTER(MID_SNO_SP, m_tMax);
    CHECK_POINTER(MID_SNO_SP, m_Pnet);
    return true;
}

void SNO_SP::initialOutputs() {
    CHECK_POSITIVE(MID_SNO_SP, m_nCells);
    if (nullptr == m_SM) Initialize1DArray(m_nCells, m_SM, 0.f);
    if (nullptr == m_SA) Initialize1DArray(m_nCells, m_SA, 0.f);
    if (nullptr == m_packT) Initialize1DArray(m_nCells, m_packT, 0.f);
    if (nullptr == m_SR) {  /// the initialization should be removed when snow redistribution module is accomplished. LJ
        Initialize1DArray(m_nCells, m_SR, 0.f);
    }
    if (nullptr == m_SE) { /// Snow sublimation will be considered in AET_PTH
        Initialize1DArray(m_nCells, m_SE, 0.f);
    }
}

int SNO_SP::Execute() {
    CheckInputData();
    initialOutputs();
    /// determine the shape parameters for the equation which describes area of
    /// snow cover as a function of amount of snow
    if (m_snowCoverCoef1 == NODATA_VALUE || m_snowCoverCoef2 == NODATA_VALUE) {
        PGCommon::getScurveShapeParameter(0.5f, 0.95f, m_snowCover50, 0.95f, &m_snowCoverCoef1, &m_snowCoverCoef2);
    }
    /// adjust melt factor for time of year, i.e., smfac in snom.f
    // which only need to computed once.
    int d = JulianDay(m_date);
    float sinv = float(sin(2.f * PI / 365.f * (d - 81.f)));
    float cmelt = (m_csnow6 + m_csnow12) / 2.f + (m_csnow6 - m_csnow12) / 2.f * sinv;
#pragma omp parallel for
    for (int rw = 0; rw < m_nCells; rw++) {
        /// estimate snow pack temperature
        m_packT[rw] = m_packT[rw] * (1 - m_lagSnow) + m_tMean[rw] * m_lagSnow;
        /// calculate snow fall
        m_SA[rw] = m_SA[rw] + m_SR[rw] - m_SE[rw];
        if (m_tMean[rw] < m_tsnow) /// precipitation will be snow
        {
            m_SA[rw] += m_kblow * m_Pnet[rw];
            m_Pnet[rw] *= (1.f - m_kblow);
        }

        if (m_SA[rw] < 0.01) {
            m_SM[rw] = 0.f;
        } else {
            float dt = m_tMax[rw] - m_t0;
            if (dt < 0) {
                m_SM[rw] = 0.f;  //if temperature is lower than t0, the snowmelt is 0.
            } else {
                //calculate using eq. 1:2.5.2 SWAT p58
                m_SM[rw] = cmelt * ((m_packT[rw] + m_tMax[rw]) / 2.f - m_t0);
                // adjust for areal extent of snow cover
                float snowCoverFrac = 0.f; //fraction of HRU area covered with snow
                if (m_SA[rw] < m_snowCoverMax) {
                    float xx = m_SA[rw] / m_snowCoverMax;
                    snowCoverFrac = xx / (xx + exp(m_snowCoverCoef1 = m_snowCoverCoef2 * xx));
                } else {
                    snowCoverFrac = 1.f;
                }
                m_SM[rw] *= snowCoverFrac;
                if (m_SM[rw] < 0.f) m_SM[rw] = 0.f;
                if (m_SM[rw] > m_SA[rw]) m_SM[rw] = m_SA[rw];
                m_SA[rw] -= m_SM[rw];
                m_Pnet[rw] += m_SM[rw];
                if (m_Pnet[rw] < 0.f) m_Pnet[rw] = 0.f;
            }
        }
    }
    return 0;
}

bool SNO_SP::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_SNO_SP, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(MID_SNO_SP, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}

void SNO_SP::SetValue(const char *key, float data) {
    string s(key);
    if (StringMatch(s, VAR_K_BLOW)) { m_kblow = data; }
    else if (StringMatch(s, VAR_T0)) { m_t0 = data; }
    else if (StringMatch(s, VAR_T_SNOW)) { m_tsnow = data; }
    else if (StringMatch(s, VAR_LAG_SNOW)) { m_lagSnow = data; }
    else if (StringMatch(s, VAR_C_SNOW6)) { m_csnow6 = data; }
    else if (StringMatch(s, VAR_C_SNOW12)) { m_csnow12 = data; }
    else if (StringMatch(s, VAR_SNOCOVMX)) { m_snowCoverMax = data; }
    else if (StringMatch(s, VAR_SNO50COV)) { m_snowCover50 = data; }
    else {
        throw ModelException(MID_SNO_SP, "SetValue", "Parameter " + s + " does not exist.");
    }
}

void SNO_SP::Set1DData(const char *key, int n, float *data) {
    CheckInputSize(key, n);
    string s(key);
    if (StringMatch(s, VAR_TMEAN)) { m_tMean = data; }
    else if (StringMatch(s, VAR_TMAX)) { m_tMax = data; }
    else if (StringMatch(s, VAR_NEPR)) { m_Pnet = data; }
    else {
        throw ModelException(MID_SNO_SP, "Set1DData", "Parameter " + s + " does not exist.");
    }
}

void SNO_SP::Get1DData(const char *key, int *n, float **data) {
    initialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SNME)) { *data = m_SM; }
    else if (StringMatch(s, VAR_SNAC)) { *data = m_SR; }
    else {
        throw ModelException(MID_SNO_SP, "Get1DData", "Result " + s + " does not exist.");
    }
    *n = m_nCells;
}
