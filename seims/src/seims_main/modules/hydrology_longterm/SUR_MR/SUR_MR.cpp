#include "SUR_MR.h"

#include "text.h"

SUR_MR::SUR_MR() :
    m_nCells(-1), m_netPcp(nullptr), m_potRfCoef(nullptr),
    m_maxSoilLyrs(-1), m_nSoilLyrs(nullptr),
    m_soilFC(nullptr), m_soilSat(nullptr), m_soilSumSat(nullptr), m_initSoilWtrStoRatio(nullptr),
    m_rfExp(NODATA_VALUE), m_maxPcpRf(NODATA_VALUE), m_deprSto(nullptr), m_meanTemp(nullptr),
    m_soilFrozenTemp(NODATA_VALUE), m_soilFrozenWtrRatio(NODATA_VALUE), m_soilTemp(nullptr),
    m_potVol(nullptr), m_impndTrig(nullptr),
    m_exsPcp(nullptr), m_infil(nullptr), m_soilWtrSto(nullptr), m_soilWtrStoPrfl(nullptr) {
    SetModuleName(M_SUR_MR[0]);
}

SUR_MR::~SUR_MR() {
    if (m_exsPcp != nullptr) Release1DArray(m_exsPcp);
    if (m_infil != nullptr) Release1DArray(m_infil);
    if (m_soilWtrSto != nullptr) Release2DArray(m_soilWtrSto);
    if (m_soilWtrStoPrfl != nullptr) Release1DArray(m_soilWtrStoPrfl);
}

bool SUR_MR::CheckInputData() {
    CHECK_POSITIVE(GetModuleName(), m_date);
    CHECK_POSITIVE(GetModuleName(), m_dt);
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    CHECK_NODATA(GetModuleName(), m_soilFrozenTemp);
    CHECK_NODATA(GetModuleName(), m_rfExp);
    CHECK_NODATA(GetModuleName(), m_maxPcpRf);
    CHECK_NODATA(GetModuleName(), m_soilFrozenWtrRatio);
    CHECK_POINTER(GetModuleName(), m_initSoilWtrStoRatio);
    CHECK_POINTER(GetModuleName(), m_potRfCoef);
    CHECK_POINTER(GetModuleName(), m_soilFC);
    CHECK_POINTER(GetModuleName(), m_meanTemp);
    CHECK_POINTER(GetModuleName(), m_soilTemp);
    CHECK_POINTER(GetModuleName(), m_netPcp);
    CHECK_POINTER(GetModuleName(), m_deprSto);
    return true;
}

void SUR_MR::InitialOutputs() {
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    if (nullptr == m_exsPcp) {
        Initialize1DArray(m_nCells, m_exsPcp, 0.);
        Initialize1DArray(m_nCells, m_infil, 0.);
        Initialize1DArray(m_nCells, m_soilWtrStoPrfl, 0.);
        Initialize2DArray(m_nCells, m_maxSoilLyrs, m_soilWtrSto, NODATA_VALUE);

#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
                if (m_initSoilWtrStoRatio[i] >= 0. && m_initSoilWtrStoRatio[i] <= 1. &&
                    m_soilFC[i][j] >= 0.) {
                    m_soilWtrSto[i][j] = m_initSoilWtrStoRatio[i] * m_soilFC[i][j];
                }
                else {
                    m_soilWtrSto[i][j] = 0.;
                }
                m_soilWtrStoPrfl[i] += m_soilWtrSto[i][j];
            }
        }

        /// update (sol_sumul) amount of water held in soil profile at saturation
        if (nullptr == m_soilSumSat && m_soilSat != nullptr) {
            m_soilSumSat = new(nothrow) FLTPT[m_nCells];
#pragma omp parallel for
            for (int i = 0; i < m_nCells; i++) {
                m_soilSumSat[i] = 0.;
                for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
                    m_soilSumSat[i] += m_soilSat[i][j];
                }
            }
        }
    }
}

int SUR_MR::Execute() {
    CheckInputData();
    InitialOutputs();
    m_maxPcpRf *= m_dt * 1.1574074074074073e-05; /// 1. / 86400. = 1.1574074074074073e-05;
#ifdef PRINT_DEBUG
    FLTPT soilWtrBefore = 0;
    FLTPT soilWtrAfter = 0;
    FLTPT rpSum = 0;
#endif

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        FLTPT hWater = 0.;
        hWater = m_netPcp[i] + m_deprSto[i];
        if (hWater > 0.) {
            /// update total soil water content
            m_soilWtrStoPrfl[i] = 0.;
            for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
                m_soilWtrStoPrfl[i] += m_soilWtrSto[i][ly];
            }
            FLTPT smFraction = Min(m_soilWtrStoPrfl[i] / m_soilSumSat[i], 1.);
#ifdef PRINT_DEBUG
            soilWtrBefore += m_soilWtrStoPrfl[i];
#endif
            // for frozen soil, no infiltration will occur
            if (m_soilTemp[i] <= m_soilFrozenTemp && smFraction >= m_soilFrozenWtrRatio) {
                m_exsPcp[i] = m_netPcp[i];
                m_infil[i] = 0.;
            } else {
                FLTPT alpha = m_rfExp - (m_rfExp - 1.) * hWater / m_maxPcpRf;
                if (hWater >= m_maxPcpRf) {
                    alpha = 1.;
                }

                //runoff percentage
                FLTPT runoffPercentage;
                if (m_potRfCoef[i] > 0.99) {
                    runoffPercentage = 1.;
                } else {
                    runoffPercentage = m_potRfCoef[i] * CalPow(smFraction, alpha);
                }
#ifdef PRINT_DEBUG
                rpSum += runoffPercentage;
#endif
                FLTPT surfq = hWater * runoffPercentage;
                if (surfq > hWater) surfq = hWater;
                m_infil[i] = hWater - surfq;
                m_exsPcp[i] = surfq;

                /// TODO: Why calculate surfq first, rather than infiltration first?
                ///       I think we should calculate infiltration first, until saturation,
                ///       then surface runoff should be calculated. By LJ.
            }
        } else {
            m_exsPcp[i] = 0.;
            m_infil[i] = 0.;
        }
        if (m_infil[i] > 0.) {
            m_soilWtrSto[i][0] += m_infil[i];
        }
    }
#ifdef PRINT_DEBUG
    FLTPT s1=0;
    FLTPT s2=0;
    FLTPT s3=0;
    FLTPT s4=0;
    for (int i = 0; i < m_nCells; i++) {
        for (int ly = 0; ly < CVT_INT(m_nSoilLyrs[i]); ly++) {
            soilWtrAfter += m_soilWtrSto[i][ly];
        }
        s1 += m_netPcp[i];
        s2 += m_exsPcp[i];
        s3 += m_infil[i];
        s4 += m_deprSto[i];
    }
    printf("[SUR_MR]soil water: %f->%f\n", soilWtrBefore,soilWtrAfter);
    printf("[SUR_MR]m_netPcp+dep->m_exsPcp+m_infil: %f+%f->%f+%f\n", s1,s4, s2,s3);
    printf("[SUR_MR]runoffPercentage avg: %f\n", rpSum/m_nCells);
    fflush(stdout);
#endif
    return 0;
}

void SUR_MR::SetValue(const char* key, const FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_T_SOIL[0])) m_soilFrozenTemp = value;
    else if (StringMatch(sk, VAR_K_RUN[0])) m_rfExp = value;
    else if (StringMatch(sk, VAR_P_MAX[0])) m_maxPcpRf = value;
    else if (StringMatch(sk, VAR_S_FROZEN[0])) m_soilFrozenWtrRatio = value;
    else {
        throw ModelException(GetModuleName(), "SetValue",
                             "Parameter " + sk + " does not exist.");
    }
}

void SUR_MR::Set1DData(const char* key, const int n, FLTPT* data) {
    CheckInputSize(key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_RUNOFF_CO[0])) m_potRfCoef = data;
    else if (StringMatch(sk, VAR_NEPR[0])) m_netPcp = data;
    else if (StringMatch(sk, VAR_TMEAN[0])) m_meanTemp = data;
    else if (StringMatch(sk, VAR_MOIST_IN[0])) m_initSoilWtrStoRatio = data;
    else if (StringMatch(sk, VAR_SOL_SUMSAT[0])) m_soilSumSat = data;
    else if (StringMatch(sk, VAR_DPST[0])) m_deprSto = data;
    else if (StringMatch(sk, VAR_SOTE[0])) m_soilTemp = data;
    else if (StringMatch(sk, VAR_POT_VOL[0])) m_potVol = data;
    else {
        throw ModelException(GetModuleName(), "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void SUR_MR::Set1DData(const char* key, const int n, int* data) {
    CheckInputSize(key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_SOILLAYERS[0])) m_nSoilLyrs = data;
    else if (StringMatch(sk, VAR_IMPOUND_TRIG[0])) m_impndTrig = data;
    else {
        throw ModelException(GetModuleName(), "Set1DData",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void SUR_MR::Set2DData(const char* key, const int nrows, const int ncols, FLTPT** data) {
    string sk(key);
    CheckInputSize2D(key, nrows, ncols, m_nCells, m_maxSoilLyrs);
    if (StringMatch(sk, VAR_SOL_AWC_AMOUNT[0])) m_soilFC = data;
    else if (StringMatch(sk, VAR_SOL_UL[0])) m_soilSat = data;
    else {
        throw ModelException(GetModuleName(), "Set2DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void SUR_MR::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_INFIL[0])) {
        *data = m_infil; //infiltration
    } else if (StringMatch(sk, VAR_EXCP[0])) {
        *data = m_exsPcp; // excess precipitation
    } else if (StringMatch(sk, VAR_SOL_SW[0])) {
        *data = m_soilWtrStoPrfl;
    } else {
        throw ModelException(GetModuleName(), "Get1DData",
                             "Result " + sk + " does not exist.");
    }
    *n = m_nCells;
}

void SUR_MR::Get2DData(const char* key, int* nRows, int* nCols, FLTPT*** data) {
    InitialOutputs();
    string sk(key);
    *nRows = m_nCells;
    *nCols = m_maxSoilLyrs;
    if (StringMatch(sk, VAR_SOL_ST[0])) {
        *data = m_soilWtrSto;
    } else {
        throw ModelException(GetModuleName(), "Get2DData",
                             "Output " + sk + " does not exist.");
    }
}
