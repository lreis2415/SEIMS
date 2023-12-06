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
    SetModuleName(M_SNO_SP[0]);
}

SNO_SP::~SNO_SP() {
    Release1DArray(m_snowMelt);
    Release1DArray(m_SA);
    Release1DArray(m_packT);
    Release1DArray(m_snowAccum);
    Release1DArray(m_SE);

}

bool SNO_SP::CheckInputData() {
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    CHECK_NODATA(GetModuleName(), m_t0);
    CHECK_NODATA(GetModuleName(), m_kblow);
    CHECK_NODATA(GetModuleName(), m_snowTemp);
    CHECK_NODATA(GetModuleName(), m_lagSnow);
    CHECK_NODATA(GetModuleName(), m_csnow6);
    CHECK_NODATA(GetModuleName(), m_csnow12);
    CHECK_NODATA(GetModuleName(), m_snowCoverMax);
    CHECK_NODATA(GetModuleName(), m_snowCover50);
    CHECK_POINTER(GetModuleName(), m_meanTemp);
    CHECK_POINTER(GetModuleName(), m_maxTemp);
    CHECK_POINTER(GetModuleName(), m_netPcp);
    return true;
}

void SNO_SP::InitialOutputs() {
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    Initialize1DArray(m_nCells, m_snowMelt, 0.);
    Initialize1DArray(m_nCells, m_SA, 0.);
    Initialize1DArray(m_nCells, m_packT, 0.);
    /// the initialization should be removed when snow redistribution module is accomplished. LJ
    Initialize1DArray(m_nCells, m_snowAccum, 0.);
    /// Snow sublimation will be considered in AET_PTH
    Initialize1DArray(m_nCells, m_SE, 0.);
}

int SNO_SP::Execute() {
    CheckInputData();
    InitialOutputs();
    /// determine the shape parameters for the equation which describes area of
    /// snow cover as a function of amount of snow
    if (FloatEqual(m_snowCoverCoef1, NODATA_VALUE) || FloatEqual(m_snowCoverCoef2, NODATA_VALUE)) {
        GetScurveShapeParameter(0.5, 0.95, m_snowCover50, 0.95,
                                &m_snowCoverCoef1, &m_snowCoverCoef2);
    }
    /// adjust melt factor for time of year, i.e., smfac in snom.f
    // which only need to computed once.
    FLTPT sinv = CVT_FLT(sin(2. * PI / 365. * (m_dayOfYear - 81.)));
    FLTPT cmelt = (m_csnow6 + m_csnow12) * 0.5 + (m_csnow6 - m_csnow12) * 0.5 * sinv;
#pragma omp parallel for
    for (int rw = 0; rw < m_nCells; rw++) {
        /// estimate snow pack temperature
        m_packT[rw] = m_packT[rw] * (1. - m_lagSnow) + m_meanTemp[rw] * m_lagSnow;
        /// calculate snow fall
        /// 2023.12.06 WYJ: m_SE is found to be always 0, never set value.
        ///     Don't understand why here m_SA[rw] += m_snowAccum[rw].
        ///     The m_snowAccum is the output, not the redistributed snow as its comment says. But it is the m_SA that actually accumulates and melts.
        ///     It seems m_SA is redundant, but I'm not sure whether to change it, since this module has been used with other modules.
        ///     At least this module cannot be used interchangably with the SNO_HBV. Subsequent modules with VAR_SNAC as input should be paid attention.
        m_SA[rw] += m_snowAccum[rw] - m_SE[rw];
        if (m_meanTemp[rw] < m_snowTemp) {
            /// precipitation will be snow
            m_SA[rw] += m_kblow * m_netPcp[rw];
            m_netPcp[rw] *= (1. - m_kblow);
        }

        if (m_SA[rw] < 0.01) {
            m_snowMelt[rw] = 0.;
        } else {
            FLTPT dt = m_maxTemp[rw] - m_t0;
            if (dt < 0) {
                m_snowMelt[rw] = 0.; //if temperature is lower than t0, the snowmelt is 0.
            } else {
                //calculate using eq. 1:2.5.2 SWAT p58
                m_snowMelt[rw] = cmelt * ((m_packT[rw] + m_maxTemp[rw]) * 0.5 - m_t0);
                // adjust for areal extent of snow cover
                FLTPT snowCoverFrac = 0.; //fraction of HRU area covered with snow
                if (m_SA[rw] < m_snowCoverMax) {
                    FLTPT xx = m_SA[rw] / m_snowCoverMax;
                    snowCoverFrac = xx / (xx + CalExp(m_snowCoverCoef1 = m_snowCoverCoef2 * xx));
                } else {
                    snowCoverFrac = 1.;
                }
                m_snowMelt[rw] *= snowCoverFrac;
                if (m_snowMelt[rw] < 0.) m_snowMelt[rw] = 0.;
                if (m_snowMelt[rw] > m_SA[rw]) m_snowMelt[rw] = m_SA[rw];
                m_SA[rw] -= m_snowMelt[rw];
                m_netPcp[rw] += m_snowMelt[rw];
                if (m_netPcp[rw] < 0.) m_netPcp[rw] = 0.;
            }
        }
    }
    return 0;
}

void SNO_SP::SetValue(const char* key, const FLTPT value) {
    string s(key);
    if (StringMatch(s, VAR_K_BLOW[0])) m_kblow = value;
    else if (StringMatch(s, VAR_T0[0])) m_t0 = value;
    else if (StringMatch(s, VAR_T_SNOW[0])) m_snowTemp = value;
    else if (StringMatch(s, VAR_LAG_SNOW[0])) m_lagSnow = value;
    else if (StringMatch(s, VAR_C_SNOW6[0])) m_csnow6 = value;
    else if (StringMatch(s, VAR_C_SNOW12[0])) m_csnow12 = value;
    else if (StringMatch(s, VAR_SNOCOVMX[0])) m_snowCoverMax = value;
    else if (StringMatch(s, VAR_SNO50COV[0])) m_snowCover50 = value;
    else {
        throw ModelException(GetModuleName(), "SetValue",
                             "Parameter " + s + " does not exist.");
    }
}

void SNO_SP::Set1DData(const char* key, const int n, FLTPT* data) {
    CheckInputSize(key, n, m_nCells);
    string s(key);
    if (StringMatch(s, VAR_TMEAN[0])) m_meanTemp = data;
    else if (StringMatch(s, VAR_TMAX[0])) m_maxTemp = data;
    else if (StringMatch(s, VAR_NEPR[0])) m_netPcp = data;
    else {
        throw ModelException(GetModuleName(), "Set1DData",
                             "Parameter " + s + " does not exist.");
    }
}

void SNO_SP::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SNME[0])) *data = m_snowMelt;
    else if (StringMatch(s, VAR_SNAC[0])) *data = m_snowAccum;
    else {
        throw ModelException(GetModuleName(), "Get1DData",
                             "Result " + s + " does not exist.");
    }
    *n = m_nCells;
}
