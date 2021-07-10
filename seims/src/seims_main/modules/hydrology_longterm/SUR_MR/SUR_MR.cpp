#include "SUR_MR.h"

#include "text.h"

SUR_MR::SUR_MR() :
    m_dt(-1), m_nCells(-1), m_netPcp(nullptr), m_potRfCoef(nullptr),
    m_maxSoilLyrs(-1), m_nSoilLyrs(nullptr),
    m_soilFC(nullptr), m_soilSat(nullptr), m_soilSumSat(nullptr), m_initSoilWtrStoRatio(nullptr),
    m_rfExp(NODATA_VALUE), m_maxPcpRf(NODATA_VALUE), m_deprSto(nullptr), m_meanTemp(nullptr),
    m_soilFrozenTemp(NODATA_VALUE), m_soilFrozenWtrRatio(NODATA_VALUE), m_soilTemp(nullptr),
    m_potVol(nullptr), m_impndTrig(nullptr),
    m_exsPcp(nullptr), m_infil(nullptr), m_soilWtrSto(nullptr), m_soilWtrStoPrfl(nullptr) {
}

SUR_MR::~SUR_MR() {
    if (m_exsPcp != nullptr) Release1DArray(m_exsPcp);
    if (m_infil != nullptr) Release1DArray(m_infil);
    if (m_soilWtrSto != nullptr) Release2DArray(m_nCells, m_soilWtrSto);
    if (m_soilWtrStoPrfl != nullptr) Release1DArray(m_soilWtrStoPrfl);
}

bool SUR_MR::CheckInputData() {
    CHECK_POSITIVE(MID_SUR_MR, m_date);
    CHECK_POSITIVE(MID_SUR_MR, m_dt);
    CHECK_POSITIVE(MID_SUR_MR, m_nCells);
    CHECK_NODATA(MID_SUR_MR, m_soilFrozenTemp);
    CHECK_NODATA(MID_SUR_MR, m_rfExp);
    CHECK_NODATA(MID_SUR_MR, m_maxPcpRf);
    CHECK_NODATA(MID_SUR_MR, m_soilFrozenWtrRatio);
    CHECK_POINTER(MID_SUR_MR, m_initSoilWtrStoRatio);
    CHECK_POINTER(MID_SUR_MR, m_potRfCoef);
    CHECK_POINTER(MID_SUR_MR, m_soilFC);
    CHECK_POINTER(MID_SUR_MR, m_meanTemp);
    CHECK_POINTER(MID_SUR_MR, m_soilTemp);
    CHECK_POINTER(MID_SUR_MR, m_netPcp);
    CHECK_POINTER(MID_SUR_MR, m_deprSto);
    return true;
}

void SUR_MR::InitialOutputs() {
    CHECK_POSITIVE(MID_SUR_MR, m_nCells);
    // allocate the output variables
    if (nullptr == m_exsPcp) {
        Initialize1DArray(m_nCells, m_exsPcp, 0.f);
        Initialize1DArray(m_nCells, m_infil, 0.f);
        Initialize1DArray(m_nCells, m_soilWtrStoPrfl, 0.f);
        Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilWtrSto, NODATA_VALUE);
    }
}

void SUR_MR::InitializeIntermediateVariables(){
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
            if (m_initSoilWtrStoRatio[i] >= 0.f && m_initSoilWtrStoRatio[i] <= 1.f && m_soilFC[i][j] >= 0.f) {
                m_soilWtrSto[i][j] = m_initSoilWtrStoRatio[i] * m_soilFC[i][j];
            }
            else {
                m_soilWtrSto[i][j] = 0.f;
            }
            m_soilWtrStoPrfl[i] += m_soilWtrSto[i][j];
        }
    }

    /// update (sol_sumul) amount of water held in soil profile at saturation
    if (nullptr == m_soilSumSat && m_soilSat != nullptr) {
        m_soilSumSat = new(nothrow) float[m_nCells];
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            m_soilSumSat[i] = 0.f;
            for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
                m_soilSumSat[i] += m_soilSat[i][j];
            }
        }
    }

    m_needReCalIntermediateParams = false;
}

int SUR_MR::Execute() {
    CheckInputData();
    InitialOutputs();
    if (m_needReCalIntermediateParams) InitializeIntermediateVariables();
    m_maxPcpRf *= m_dt * 1.1574074074074073e-05f; /// 1. / 86400. = 1.1574074074074073e-05;
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        float hWater = 0.f;
        hWater = m_netPcp[i] + m_deprSto[i];
        if (hWater > 0.f) {
            /// update total soil water content
            m_soilWtrStoPrfl[i] = 0.f;
            for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
                m_soilWtrStoPrfl[i] += m_soilWtrSto[i][ly];
            }
            float smFraction = Min(m_soilWtrStoPrfl[i] / m_soilSumSat[i], 1.f);
            // for frozen soil, no infiltration will occur
            if (m_soilTemp[i] <= m_soilFrozenTemp && smFraction >= m_soilFrozenWtrRatio) {
                m_exsPcp[i] = m_netPcp[i];
                m_infil[i] = 0.f;
            } else {
                float alpha = m_rfExp - (m_rfExp - 1.f) * hWater / m_maxPcpRf;
                if (hWater >= m_maxPcpRf) {
                    alpha = 1.f;
                }

                //runoff percentage
                float runoffPercentage;
                if (m_potRfCoef[i] > 0.99f) {
                    runoffPercentage = 1.f;
                } else {
                    runoffPercentage = m_potRfCoef[i] * pow(smFraction, alpha);
                }

                float surfq = hWater * runoffPercentage;
                if (surfq > hWater) surfq = hWater;
                m_infil[i] = hWater - surfq;
                m_exsPcp[i] = surfq;

                /// TODO: Why calculate surfq first, rather than infiltration first?
                ///       I think we should calculate infiltration first, until saturation,
                ///       then surface runoff should be calculated. By LJ.
            }
        } else {
            m_exsPcp[i] = 0.f;
            m_infil[i] = 0.f;
        }
        if (m_infil[i] > 0.f) {
            m_soilWtrSto[i][0] += m_infil[i];
        }
    }
    return 0;
}

void SUR_MR::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, Tag_HillSlopeTimeStep)) m_dt = value;
    else if (StringMatch(sk, VAR_T_SOIL)) m_soilFrozenTemp = value;
    else if (StringMatch(sk, VAR_K_RUN)) m_rfExp = value;
    else if (StringMatch(sk, VAR_P_MAX)) m_maxPcpRf = value;
    else if (StringMatch(sk, VAR_S_FROZEN)) m_soilFrozenWtrRatio = value;
    else {
        throw ModelException(MID_SUR_MR, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void SUR_MR::Set1DData(const char* key, const int n, float* data) {
    CheckInputSize(MID_SUR_MR, key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_RUNOFF_CO)) m_potRfCoef = data;
    else if (StringMatch(sk, VAR_NEPR)) m_netPcp = data;
    else if (StringMatch(sk, VAR_TMEAN)) m_meanTemp = data;
    else if (StringMatch(sk, VAR_MOIST_IN)) m_initSoilWtrStoRatio = data;
    else if (StringMatch(sk, VAR_SOL_SUMSAT)) m_soilSumSat = data;
    else if (StringMatch(sk, VAR_DPST)) m_deprSto = data;
    else if (StringMatch(sk, VAR_SOTE)) m_soilTemp = data;
    else if (StringMatch(sk, VAR_SOILLAYERS)) m_nSoilLyrs = data;
    else if (StringMatch(sk, VAR_POT_VOL)) m_potVol = data;
    else if (StringMatch(sk, VAR_IMPOUND_TRIG)) m_impndTrig = data;
    else {
        throw ModelException(MID_SUR_MR, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void SUR_MR::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    string sk(key);
    CheckInputSize2D(MID_SUR_MR, key, nrows, ncols, m_nCells, m_maxSoilLyrs);
    if (StringMatch(sk, VAR_SOL_AWC)) m_soilFC = data;
    else if (StringMatch(sk, VAR_SOL_UL)) m_soilSat = data;
    else {
        throw ModelException(MID_SUR_MR, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void SUR_MR::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_INFIL)) {
        *data = m_infil; //infiltration
    } else if (StringMatch(sk, VAR_EXCP)) {
        *data = m_exsPcp; // excess precipitation
    } else if (StringMatch(sk, VAR_SOL_SW)) {
        *data = m_soilWtrStoPrfl;
    } else {
        throw ModelException(MID_SUR_MR, "Get1DData", "Result " + sk + " does not exist.");
    }
    *n = m_nCells;
}

void SUR_MR::Get2DData(const char* key, int* nRows, int* nCols, float*** data) {
    InitialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_maxSoilLyrs;
    if (StringMatch(sk, VAR_SOL_ST)) {
        *data = m_soilWtrSto;
    } else {
        throw ModelException(MID_SUR_MR, "Get2DData", "Output " + sk + " does not exist.");
    }
}
