#include "text.h"
#include "Interception_MCS.h"

#include "utils_time.h"

clsPI_MCS::clsPI_MCS() :
    m_embnkFr(0.15), m_pcp2CanalFr(0.5), m_landUse(nullptr),
    m_intcpStoCapExp(-1.), m_initIntcpSto(0.), m_maxIntcpStoCap(nullptr),
    m_minIntcpStoCap(nullptr),
    m_pcp(nullptr),m_snowfall(nullptr), m_pet(nullptr), m_canSto(nullptr),
    m_intcpLoss(nullptr), m_netPcp(nullptr),m_snowAcc(nullptr), m_nCells(-1) {

#ifndef STORM_MODE
    m_IntcpET = nullptr;
    m_pet = nullptr;
#else
    m_hilldt = -1;
    m_slope = nullptr;
#endif
    SetModuleName(M_PI_MCS[0]);
}

clsPI_MCS::~clsPI_MCS() {
    Release1DArray(m_intcpLoss);
    Release1DArray(m_canSto);
    Release1DArray(m_netPcp);
    Release1DArray(m_snowAcc);
#ifndef STORM_MODE
    Release1DArray(m_IntcpET);
#endif
}

void clsPI_MCS::Set1DData(const char* key, int nrows, FLTPT* data) {
    CheckInputSize(key, nrows, m_nCells);
    string s(key);
    if (StringMatch(s, VAR_PCP[0])) m_pcp = data;
    else if (StringMatch(s, VAR_SNOWFALL[0])) m_snowfall = data;
    else if (StringMatch(s, VAR_PET[0])) {
#ifndef STORM_MODE
        m_pet = data;
#endif
    } else if (StringMatch(s, VAR_INTERC_MAX[0])) m_maxIntcpStoCap = data;
    else if (StringMatch(s, VAR_INTERC_MIN[0])) m_minIntcpStoCap = data;
    else {
        throw ModelException(GetModuleName(), "Set1DData",
                             "Parameter " + s + " does not exist.");
    }
}

void clsPI_MCS::Set1DData(const char* key, int nrows, int* data) {
    CheckInputSize(key, nrows, m_nCells);
    string s(key);
    if (StringMatch(s, VAR_LANDUSE[0])) m_landUse = data;
    else {
        throw ModelException(GetModuleName(), "Set1DData",
                             "Integer Parameter " + s + " does not exist.");
    }
}

void clsPI_MCS::SetValue(const char* key, const FLTPT value) {
    string s(key);
    if (StringMatch(s, VAR_PI_B[0])) m_intcpStoCapExp = value;
    else if (StringMatch(s, VAR_INIT_IS[0])) m_initIntcpSto = value;
    else if (StringMatch(s, VAR_PCP2CANFR_PR[0])) m_pcp2CanalFr = value;
    else if (StringMatch(s, VAR_EMBNKFR_PR[0])) m_embnkFr = value;
#ifdef STORM_MODE
    else if (StringMatch(s, Tag_HillSlopeTimeStep[0])) m_hilldt = data;
#endif // STORM_MODE
    else {
        throw ModelException(GetModuleName(), "SetValue",
                             "Parameter " + s + " does not exist.");
    }
}

void clsPI_MCS::SetValue(const char* key, const int value) {
    string s(key);
#ifdef STORM_MODE
    if (StringMatch(s, Tag_HillSlopeTimeStep[0])) m_hilldt = data;
    else {
        throw ModelException(GetModuleName(), "SetValue",
                             "Integer Parameter " + s + " does not exist.");
    }
#endif // STORM_MODE
}

void clsPI_MCS::Get1DData(const char* key, int* nRows, FLTPT** data) {
    InitialOutputs();
    string s = key;
    if (StringMatch(s, VAR_INLO[0])) {
        *data = m_intcpLoss;
    } else if (StringMatch(s, VAR_INET[0])) {
#ifndef STORM_MODE
        *data = m_IntcpET;
#endif
    } else if (StringMatch(s, VAR_CANSTOR[0])) {
        *data = m_canSto;
    } else if (StringMatch(s, VAR_NEPR[0])) {
        *data = m_netPcp;
    } else if (StringMatch(s, VAR_SNAC[0])) {
        *data = m_snowAcc;
    }
    else {
        throw ModelException(GetModuleName(), "Get1DData",
                             "Result " + s + " does not exist.");
    }
    *nRows = m_nCells;
}

void clsPI_MCS::InitialOutputs() {
    Initialize1DArray(m_nCells, m_canSto, m_initIntcpSto);
#ifndef STORM_MODE
    Initialize1DArray(m_nCells, m_IntcpET, 0.);
#endif
    Initialize1DArray(m_nCells, m_netPcp, 0.);
    Initialize1DArray(m_nCells, m_snowAcc, 0.);
    Initialize1DArray(m_nCells, m_intcpLoss, 0.);
}

int clsPI_MCS::Execute() {
    CheckInputData();
    InitialOutputs();
#ifdef PRINT_DEBUG
    FLTPT s1 = 0;
    FLTPT s2 = 0;
    for (int i = 0; i < m_nCells; i++)
    {
        s1 += m_pcp[i];
        s2 += m_canSto[i];
    }
    printf("[PI_MCS] Before: PCP=%f, canStor=%f\n", s1, s2);
#endif
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_intcpLoss[i] = 0.;
        m_netPcp[i] = 0.;
        FLTPT availableSpace = 0;
        if (m_pcp[i] > 0.) {
#ifdef STORM_MODE
            /// correction for slope gradient, water spreads out over larger area
            /// 1. / 3600. = 0.0002777777777777778
            m_P[i] = m_P[i] * m_hilldt * 0.0002777777777777778 * cos(atan(m_slope[i]));
#endif // STORM_MODE
            //interception storage capacity, 1. / 365. = 0.0027397260273972603
            FLTPT degree = 2. * PI * (m_dayOfYear - 87.) * 0.0027397260273972603;
            /// For water, min and max are both 0, then no need for specific handling.
            FLTPT min = m_minIntcpStoCap[i];
            FLTPT max = m_maxIntcpStoCap[i];
            FLTPT capacity = min + (max - min) * CalPow(0.5 + 0.5 * sin(degree), m_intcpStoCapExp);

            //interception, currently, m_st[i] is storage of (t-1) time step
            availableSpace = NonNeg(capacity - m_canSto[i]);
            availableSpace -= Convey(m_pcp[i], m_intcpLoss[i], availableSpace, 1, false);
            m_canSto[i] += m_intcpLoss[i];
            // if the cell is paddy, by default 15% part of pcp will be allocated to embankment area
            FLTPT pcp2canal=0;
            if (CVT_INT(m_landUse[i]) == LANDUSE_ID_PADDY) {
                //water added into ditches from low embankment, should be added to somewhere else.
                pcp2canal = m_pcp[i] * m_pcp2CanalFr * m_embnkFr;
            }
            m_netPcp[i] = m_pcp[i] - m_intcpLoss[i] - pcp2canal;
        }
        if (m_snowfall != nullptr && m_snowfall[i] > 0 && availableSpace > 0) {
            FLTPT snowIntercept = Convey(m_snowfall[i], m_intcpLoss[i], availableSpace, 1, false);
            m_canSto[i] += snowIntercept;
            m_snowAcc[i] += m_snowfall[i] - snowIntercept;
        }
#ifndef STORM_MODE
        //evaporation
        m_IntcpET[i]=0;
        Convey(m_canSto[i], m_IntcpET[i], m_pet[i], 1);
#endif
    }
#ifdef PRINT_DEBUG
    FLTPT s3 = 0;
    FLTPT s4 = 0;
    FLTPT s5 = 0;
    FLTPT s6 = 0;
    for (int i = 0; i < m_nCells; i++)
    {
        s3 += m_netPcp[i];
        s4 += m_canSto[i];
        s5 += m_intcpLoss[i]; 
        s6 += m_IntcpET[i];
    }
    printf("[PI_MCS] After: NEPR=%f, canStor=%f, intcpLoss=%f, intcpET=%f\n", s3, s4,s5,s6);
#endif
    return 0;
}

bool clsPI_MCS::CheckInputData() {
    CHECK_POSITIVE(GetModuleName(), m_date);
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    CHECK_POINTER(GetModuleName(), m_pcp);
#ifndef STORM_MODE
    CHECK_POINTER(GetModuleName(), m_pet);
#else
    CHECK_POINTER(GetModuleName(), m_slope);
    CHECK_POINTER(GetModuleName(), m_hilldt);
#endif
    CHECK_POINTER(GetModuleName(), m_maxIntcpStoCap);
    CHECK_POINTER(GetModuleName(), m_minIntcpStoCap);
    CHECK_DATA(GetModuleName(), m_intcpStoCapExp > 1.5 || m_intcpStoCapExp < 0.5,
               "The interception storage capacity exponent "
               "can not be " + ValueToString(m_intcpStoCapExp) + ". It should between 0.5 and 1.5.");
    CHECK_DATA(GetModuleName(), m_initIntcpSto > 1. || m_initIntcpSto < 0.,
               "The Initial interception storage cannot "
               "be " + ValueToString(m_initIntcpSto) + ". It should between 0 and 1.");
    return true;
}
