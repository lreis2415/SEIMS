#include "Interception_MCS.h"

#include "text.h"
#include "utils_time.h"

clsPI_MCS::clsPI_MCS() :
    m_embnkFr(0.15f), m_pcp2CanalFr(0.5f), m_landUse(nullptr),
    m_intcpStoCapExp(-1.f), m_initIntcpSto(0.f), m_maxIntcpStoCap(nullptr),
    m_minIntcpStoCap(nullptr),
    m_pcp(nullptr), m_pet(nullptr), m_canSto(nullptr),
    m_intcpLoss(nullptr), m_netPcp(nullptr), m_nCells(-1) {
    
#ifndef STORM_MODE
    m_IntcpET = nullptr;
#else
    m_hilldt = -1.f;
    m_slope = nullptr;
#endif
}

clsPI_MCS::~clsPI_MCS() {
    if (m_intcpLoss != nullptr) Release1DArray(m_intcpLoss);
    if (m_canSto != nullptr) Release1DArray(m_canSto);
    if (m_netPcp != nullptr) Release1DArray(m_netPcp);
#ifndef STORM_MODE
    if (m_IntcpET != nullptr) Release1DArray(m_IntcpET);
#endif
}

void clsPI_MCS::Set1DData(const char* key, int nrows, float* data) {
    CheckInputSize(M_PI_MCS[0], key, nrows, m_nCells);
    string s(key);
    if (StringMatch(s, VAR_PCP[0])) m_pcp = data;
    else if (StringMatch(s, VAR_PET[0])) {
#ifndef STORM_MODE
        m_pet = data;
#endif
    } else if (StringMatch(s, VAR_INTERC_MAX[0])) m_maxIntcpStoCap = data;
    else if (StringMatch(s, VAR_INTERC_MIN[0])) m_minIntcpStoCap = data;
    else if (StringMatch(s, VAR_LANDUSE[0])) m_landUse = data;
    else {
        throw ModelException(M_PI_MCS[0], "Set1DData", "Parameter " + s + " does not exist.");
    }
}

void clsPI_MCS::SetValue(const char* key, const float value) {
    string s(key);
    if (StringMatch(s, VAR_PI_B[0])) m_intcpStoCapExp = value;
    else if (StringMatch(s, VAR_INIT_IS[0])) m_initIntcpSto = value;
    else if (StringMatch(s, VAR_PCP2CANFR_PR[0])) m_pcp2CanalFr = value;
    else if (StringMatch(s, VAR_EMBNKFR_PR[0])) m_embnkFr = value;
#ifdef STORM_MODE
    else if (StringMatch(s, Tag_HillSlopeTimeStep[0])) m_hilldt = data;
#endif // STORM_MODE
    else {
        throw ModelException(M_PI_MCS[0], "SetValue", "Parameter " + s + " does not exist.");
    }
}

void clsPI_MCS::Get1DData(const char* key, int* nRows, float** data) {
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
    } else {
        throw ModelException(M_PI_MCS[0], "Get1DData", "Result " + s + " does not exist.");
    }
    *nRows = m_nCells;
}

void clsPI_MCS::InitialOutputs() {
    if (m_canSto == nullptr) {
        Initialize1DArray(m_nCells, m_canSto, m_initIntcpSto);
    }
#ifndef STORM_MODE
    if (m_IntcpET == nullptr) {
        Initialize1DArray(m_nCells, m_IntcpET, 0.f);
    }
#endif
    if (m_netPcp == nullptr) {
        Initialize1DArray(m_nCells, m_netPcp, 0.f);
    }
    if (m_intcpLoss == nullptr) {
        Initialize1DArray(m_nCells, m_intcpLoss, 0.f);
    }
}

int clsPI_MCS::Execute() {
    //check input data
    CheckInputData();
    /// initialize outputs
    InitialOutputs();

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_pcp[i] > 0.f) {
#ifdef STORM_MODE
            /// correction for slope gradient, water spreads out over larger area
            /// 1. / 3600. = 0.0002777777777777778
            m_P[i] = m_P[i] * m_hilldt * 0.0002777777777777778f * cos(atan(m_slope[i]));
#endif // STORM_MODE
            //interception storage capacity, 1. / 365. = 0.0027397260273972603
            float degree = 2.f * PI * (m_dayOfYear - 87.f) * 0.0027397260273972603f;
            /// For water, min and max are both 0, then no need for specific handling.
            float min = m_minIntcpStoCap[i];
            float max = m_maxIntcpStoCap[i];
            float capacity = min + (max - min) * pow(0.5f + 0.5f * sin(degree), m_intcpStoCapExp);

            //interception, currently, m_st[i] is storage of (t-1) time step
            float availableSpace = capacity - m_canSto[i];
            if (availableSpace < 0) {
                availableSpace = 0.f;
            }

            if (availableSpace < m_pcp[i]) {
                m_intcpLoss[i] = availableSpace;
                //if the cell is paddy, by default 15% part of pcp will be allocated to embankment area
                if (CVT_INT(m_landUse[i]) == LANDUSE_ID_PADDY) {
                    //water added into ditches from low embankment, should be added to somewhere else.
                    float pcp2canal = m_pcp[i] * m_pcp2CanalFr * m_embnkFr;
                    m_netPcp[i] = m_pcp[i] - m_intcpLoss[i] - pcp2canal;
                } else {
                    //net precipitation
                    m_netPcp[i] = m_pcp[i] - m_intcpLoss[i];
                }
            } else {
                m_intcpLoss[i] = m_pcp[i];
                m_netPcp[i] = 0.f;
            }

            m_canSto[i] += m_intcpLoss[i];
        } else {
            m_intcpLoss[i] = 0.f;
            m_netPcp[i] = 0.f;
        }
#ifndef STORM_MODE
        //evaporation
        if (m_canSto[i] > m_pet[i]) {
            m_IntcpET[i] = m_pet[i];
        } else {
            m_IntcpET[i] = m_canSto[i];
        }
        m_canSto[i] -= m_IntcpET[i];
#endif
    }
    return 0;
}

bool clsPI_MCS::CheckInputData() {
    CHECK_POSITIVE(M_PI_MCS[0], m_date);
    CHECK_POSITIVE(M_PI_MCS[0], m_nCells);
    CHECK_POINTER(M_PI_MCS[0], m_pcp);
#ifndef STORM_MODE
    CHECK_POINTER(M_PI_MCS[0], m_pet);
#else
    CHECK_POINTER(M_PI_MCS[0], m_slope);
    CHECK_POINTER(M_PI_MCS[0], m_hilldt);
#endif
    CHECK_POINTER(M_PI_MCS[0], m_maxIntcpStoCap);
    CHECK_POINTER(M_PI_MCS[0], m_minIntcpStoCap);
    CHECK_DATA(M_PI_MCS[0], m_intcpStoCapExp > 1.5f || m_intcpStoCapExp < 0.5f,
        "The interception storage capacity exponent "
        "can not be " + ValueToString(m_intcpStoCapExp) + ". It should between 0.5 and 1.5.");
    CHECK_DATA(M_PI_MCS[0], m_initIntcpSto > 1.f || m_initIntcpSto < 0.f, "The Initial interception storage cannot "
        "be " + ValueToString(m_initIntcpSto) + ". It should between 0 and 1.");
    return true;
}
