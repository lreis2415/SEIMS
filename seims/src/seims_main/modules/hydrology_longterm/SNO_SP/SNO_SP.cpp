#include "SNO_SP.h"

#include "text.h"
#include "PlantGrowthCommon.h"
#include "utils_time.h"


SNO_SP::SNO_SP() :
    m_nCells(-1), m_t0(NODATA_VALUE), m_kblow(NODATA_VALUE), m_snowTemp(NODATA_VALUE),
    m_lagSnow(NODATA_VALUE), m_csnow6(NODATA_VALUE), m_csnow12(NODATA_VALUE),
    m_snowCoverMax(NODATA_VALUE), m_snowCover50(NODATA_VALUE),
    m_snowCoverCoef1(NODATA_VALUE), m_snowCoverCoef2(NODATA_VALUE),
    m_meanTemp(nullptr), m_maxTemp(nullptr), m_netPcp(nullptr),
    m_snowAccum(nullptr), m_SE(nullptr), m_packT(nullptr), m_snowMelt(nullptr), m_SA(nullptr) {
}

SNO_SP::~SNO_SP() {
    if (m_snowMelt != nullptr) Release1DArray(m_snowMelt);
    if (m_SA != nullptr) Release1DArray(m_SA);
    if (m_packT != nullptr) Release1DArray(m_packT);
}

bool SNO_SP::CheckInputData() {
    CHECK_POSITIVE(MID_SNO_SP, m_nCells);
    CHECK_NODATA(MID_SNO_SP, m_t0);
    CHECK_NODATA(MID_SNO_SP, m_kblow);
    CHECK_NODATA(MID_SNO_SP, m_snowTemp);
    CHECK_NODATA(MID_SNO_SP, m_lagSnow);
    CHECK_NODATA(MID_SNO_SP, m_csnow6);
    CHECK_NODATA(MID_SNO_SP, m_csnow12);
    CHECK_NODATA(MID_SNO_SP, m_snowCoverMax);
    CHECK_NODATA(MID_SNO_SP, m_snowCover50);
    CHECK_POINTER(MID_SNO_SP, m_meanTemp);
    CHECK_POINTER(MID_SNO_SP, m_maxTemp);
    CHECK_POINTER(MID_SNO_SP, m_netPcp);
    return true;
}

void SNO_SP::InitialOutputs() {
    CHECK_POSITIVE(MID_SNO_SP, m_nCells);
    if (nullptr == m_snowMelt) Initialize1DArray(m_nCells, m_snowMelt, 0.f);
    if (nullptr == m_SA) Initialize1DArray(m_nCells, m_SA, 0.f);
    if (nullptr == m_packT) Initialize1DArray(m_nCells, m_packT, 0.f);
    if (nullptr == m_snowAccum) {
        /// the initialization should be removed when snow redistribution module is accomplished. LJ
        Initialize1DArray(m_nCells, m_snowAccum, 0.f);
    }
    if (nullptr == m_SE) {
        /// Snow sublimation will be considered in AET_PTH
        Initialize1DArray(m_nCells, m_SE, 0.f);
    }
}

int SNO_SP::Execute() {
    CheckInputData();
    InitialOutputs();
    /// determine the shape parameters for the equation which describes area of
    /// snow cover as a function of amount of snow
    if (m_snowCoverCoef1 == NODATA_VALUE || m_snowCoverCoef2 == NODATA_VALUE) {
        GetScurveShapeParameter(0.5f, 0.95f, m_snowCover50, 0.95f, &m_snowCoverCoef1, &m_snowCoverCoef2);
    }
    /// adjust melt factor for time of year, i.e., smfac in snom.f
    // which only need to computed once.
    float sinv = CVT_FLT(sin(2.f * PI / 365.f * (m_dayOfYear - 81.f)));
    float cmelt = (m_csnow6 + m_csnow12) * 0.5f + (m_csnow6 - m_csnow12) * 0.5f * sinv;
#pragma omp parallel for
    for (int rw = 0; rw < m_nCells; rw++) {
        /// estimate snow pack temperature
        m_packT[rw] = m_packT[rw] * (1 - m_lagSnow) + m_meanTemp[rw] * m_lagSnow;
        /// calculate snow fall
        m_SA[rw] += m_snowAccum[rw] - m_SE[rw];
        if (m_meanTemp[rw] < m_snowTemp) {
            /// precipitation will be snow
            m_SA[rw] += m_kblow * m_netPcp[rw];
            m_netPcp[rw] *= (1.f - m_kblow);
        }

        if (m_SA[rw] < 0.01) {
            m_snowMelt[rw] = 0.f;
        } else {
            float dt = m_maxTemp[rw] - m_t0;
            if (dt < 0) {
                m_snowMelt[rw] = 0.f; //if temperature is lower than t0, the snowmelt is 0.
            } else {
                //calculate using eq. 1:2.5.2 SWAT p58
                m_snowMelt[rw] = cmelt * ((m_packT[rw] + m_maxTemp[rw]) * 0.5f - m_t0);
                // adjust for areal extent of snow cover
                float snowCoverFrac = 0.f; //fraction of HRU area covered with snow
                if (m_SA[rw] < m_snowCoverMax) {
                    float xx = m_SA[rw] / m_snowCoverMax;
                    snowCoverFrac = xx / (xx + exp(m_snowCoverCoef1 = m_snowCoverCoef2 * xx));
                } else {
                    snowCoverFrac = 1.f;
                }
                m_snowMelt[rw] *= snowCoverFrac;
                if (m_snowMelt[rw] < 0.f) m_snowMelt[rw] = 0.f;
                if (m_snowMelt[rw] > m_SA[rw]) m_snowMelt[rw] = m_SA[rw];
                m_SA[rw] -= m_snowMelt[rw];
                m_netPcp[rw] += m_snowMelt[rw];
                if (m_netPcp[rw] < 0.f) m_netPcp[rw] = 0.f;
            }
        }
    }
    return 0;
}

void SNO_SP::SetValue(const char* key, const float value) {
    string s(key);
    if (StringMatch(s, VAR_K_BLOW)) m_kblow = value;
    else if (StringMatch(s, VAR_T0)) m_t0 = value;
    else if (StringMatch(s, VAR_T_SNOW)) m_snowTemp = value;
    else if (StringMatch(s, VAR_LAG_SNOW)) m_lagSnow = value;
    else if (StringMatch(s, VAR_C_SNOW6)) m_csnow6 = value;
    else if (StringMatch(s, VAR_C_SNOW12)) m_csnow12 = value;
    else if (StringMatch(s, VAR_SNOCOVMX)) m_snowCoverMax = value;
    else if (StringMatch(s, VAR_SNO50COV)) m_snowCover50 = value;
    else {
        throw ModelException(MID_SNO_SP, "SetValue", "Parameter " + s + " does not exist.");
    }
}

void SNO_SP::Set1DData(const char* key, const int n, float* data) {
    CheckInputSize(MID_SNO_SP, key, n, m_nCells);
    string s(key);
    if (StringMatch(s, VAR_TMEAN)) m_meanTemp = data;
    else if (StringMatch(s, VAR_TMAX)) m_maxTemp = data;
    else if (StringMatch(s, VAR_NEPR)) m_netPcp = data;
    else {
        throw ModelException(MID_SNO_SP, "Set1DData", "Parameter " + s + " does not exist.");
    }
}

void SNO_SP::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SNME)) *data = m_snowMelt;
    else if (StringMatch(s, VAR_SNAC)) *data = m_snowAccum;
    else {
        throw ModelException(MID_SNO_SP, "Get1DData", "Result " + s + " does not exist.");
    }
    *n = m_nCells;
}
