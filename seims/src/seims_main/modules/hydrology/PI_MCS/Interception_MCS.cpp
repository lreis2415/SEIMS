#include "Interception_MCS.h"

#include "text.h"
#include "utils_time.h"

clsPI_MCS::clsPI_MCS() : m_nCells(-1), m_Pi_b(-1.f), m_Init_IS(0.f),
                         m_netPrecipitation(nullptr), m_interceptionLoss(nullptr), m_st(nullptr),
                         m_landuse(nullptr), m_pcp2canfr_pr(0.5f), m_embnkfr_pr(0.15f){
#ifndef STORM_MODE
    m_evaporationLoss = nullptr;
#else
    m_hilldt = -1.f;
    m_slope = nullptr;
#endif
}

clsPI_MCS::~clsPI_MCS() {
    if (m_interceptionLoss != nullptr) Release1DArray(m_interceptionLoss);
    if (m_st != nullptr) Release1DArray(m_st);
    if (m_netPrecipitation != nullptr) Release1DArray(m_netPrecipitation);
#ifndef STORM_MODE
    if (m_evaporationLoss != nullptr) Release1DArray(m_evaporationLoss);
#endif
}

void clsPI_MCS::Set1DData(const char *key, int nRows, float *data) {
    CheckInputSize(key, nRows);

    string s(key);
    if (StringMatch(s, VAR_PCP)) { m_P = data; }
    else if (StringMatch(s, VAR_PET)) {
#ifndef STORM_MODE
        m_PET = data;
#endif
    }
    else if (StringMatch(s, VAR_INTERC_MAX)) { m_maxSt = data; }
    else if (StringMatch(s, VAR_INTERC_MIN)) { m_minSt = data; }
    else if (StringMatch(s, VAR_LANDUSE)) { m_landuse = data; }
	else {
        throw ModelException(MID_PI_SVSC, "Set1DData", "Parameter " + s + " does not exist.");
    }
}

void clsPI_MCS::SetValue(const char *key, float data) {
    string s(key);
    if (StringMatch(s, VAR_PI_B)) { m_Pi_b = data; }
    else if (StringMatch(s, VAR_INIT_IS)) { m_Init_IS = data; }
    else if (StringMatch(s, VAR_PCP2CANFR_PR)) {m_pcp2canfr_pr = data; }
    else if (StringMatch(s, VAR_EMBNKFR_PR)) {m_embnkfr_pr = data; }
#ifdef STORM_MODE
    else if (StringMatch(s, Tag_HillSlopeTimeStep)) { m_hilldt = data; }
#endif // STORM_MODE
    else {
        throw ModelException(MID_PI_SVSC, "SetValue", "Parameter " + s + " does not exist.");
    }
}

void clsPI_MCS::Get1DData(const char *key, int *nRows, float **data) {
    InitialOutputs();
    string s = key;
    if (StringMatch(s, VAR_INLO)) {
        *data = m_interceptionLoss;
    } else if (StringMatch(s, VAR_INET)) {
#ifndef STORM_MODE
        *data = m_evaporationLoss;
#endif
    } else if (StringMatch(s, VAR_CANSTOR)) {
        *data = m_st;
    } else if (StringMatch(s, VAR_NEPR)) {
        *data = m_netPrecipitation;
    } else {
        throw ModelException(MID_PI_SVSC, "Get1DData", "Result " + s + " does not exist.");
    }
    *nRows = m_nCells;
}

void clsPI_MCS::InitialOutputs() {
    if (m_st == nullptr) {
        Initialize1DArray(m_nCells, m_st, m_Init_IS);
    }
#ifndef STORM_MODE
    if (m_evaporationLoss == nullptr) {
        Initialize1DArray(m_nCells, m_evaporationLoss, 0.f);
    }
#endif
    if (m_netPrecipitation == nullptr) {
        Initialize1DArray(m_nCells, m_netPrecipitation, 0.f);
    }
    if (m_interceptionLoss == nullptr) {
        Initialize1DArray(m_nCells, m_interceptionLoss, 0.f);
    }
}

int clsPI_MCS::Execute() {
    //check input data
    CheckInputData();
    /// initialize outputs
    InitialOutputs();

    int julian = utils_time::JulianDay(m_date);
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_P[i] > 0.f) {
#ifdef STORM_MODE
            /// correction for slope gradient, water spreads out over larger area
            m_P[i] = m_P[i] * m_hilldt / 3600.f * cos(atan(m_slope[i]));
#endif // STORM_MODE
            //interception storage capacity
            float degree = 2.f * PI * (julian - 87.f) / 365.f;
            /// For water, min and max are both 0, then no need for specific handling.
            float min = m_minSt[i];
            float max = m_maxSt[i];
            float capacity = min + (max - min) * pow(0.5f + 0.5f * sin(degree), m_Pi_b);

            //interception, currently, m_st[i] is storage of (t-1) time step
            float availableSpace = capacity - m_st[i];
            if (availableSpace < 0) {
                availableSpace = 0.f;
            }

            if (availableSpace < m_P[i]) {
                m_interceptionLoss[i] = availableSpace;
                //if the cell is paddy, by default 15% part of pcp will be allocated to embankment area
                if ((int)m_landuse[i] == LANDUSE_ID_PADDY){
                    //water added into ditches from low embankment, should be added to somewhere else.
                    float pcp2canal = m_P[i] * m_pcp2canfr_pr * m_embnkfr_pr;

                    m_netPrecipitation[i] = m_P[i] - m_interceptionLoss[i] - pcp2canal;
                }
                else{
                    //net precipitation
                    m_netPrecipitation[i] = m_P[i] - m_interceptionLoss[i];
                }
            } else {
                m_interceptionLoss[i] = m_P[i];
                m_netPrecipitation[i] = 0.f;
            }

            m_st[i] += m_interceptionLoss[i];
        } else {
            m_interceptionLoss[i] = 0.f;
            m_netPrecipitation[i] = 0.f;
        }
#ifndef STORM_MODE
        //evaporation
        if (m_st[i] > m_PET[i]) {
            m_evaporationLoss[i] = m_PET[i];
        } else {
            m_evaporationLoss[i] = m_st[i];
        }
        m_st[i] -= m_evaporationLoss[i];
#endif
    }
    return 0;
}

bool clsPI_MCS::CheckInputData() {
    CHECK_POSITIVE(MID_PI_MSC, m_date);
    CHECK_POSITIVE(MID_PI_MSC, m_nCells);
    CHECK_POINTER(MID_PI_MSC, m_P);
#ifndef STORM_MODE
    CHECK_POINTER(MID_PI_MSC, m_PET);
#else
    CHECK_POINTER(MID_PI_MSC, m_slope);
    CHECK_POINTER(MID_PI_MSC, m_hilldt);
#endif
    CHECK_POINTER(MID_PI_MSC, m_maxSt);
    CHECK_POINTER(MID_PI_MSC, m_minSt);
    CHECK_DATA(MID_PI_MSC, m_Pi_b > 1.5f || m_Pi_b < 0.5f, "The interception storage capacity exponent "
               "can not be " + ValueToString(m_Pi_b) + ". It should between 0.5 and 1.5.");
    CHECK_DATA(MID_PI_MSC, m_Init_IS > 1.f || m_Init_IS < 0.f, "The Initial interception storage cannot "
               "be " + ValueToString(m_Init_IS) + ". It should between 0 and 1.");
    return true;
}

bool clsPI_MCS::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_PI_SVSC, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(MID_PI_SVSC, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}
