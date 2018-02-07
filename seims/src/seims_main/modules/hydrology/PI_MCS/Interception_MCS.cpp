#include "seims.h"
#include "ClimateParams.h"

#include "Interception_MCS.h"

clsPI_MCS::clsPI_MCS(void) : m_nCells(-1), m_Pi_b(-1.f), m_Init_IS(0.f),
                             m_netPrecipitation(NULL), m_interceptionLoss(NULL), m_st(NULL),
                             m_landuse(NULL), m_pcp2canfr_pr(0.5f), m_embnkfr_pr(0.15f){
#ifndef STORM_MODE
    m_evaporationLoss = NULL;
#else
    m_hilldt = -1.f;
    m_slope = NULL;
#endif
}

clsPI_MCS::~clsPI_MCS(void) {
    if (this->m_interceptionLoss != NULL) Release1DArray(this->m_interceptionLoss);
    if (this->m_st != NULL) Release1DArray(this->m_st);
    if (this->m_netPrecipitation != NULL) Release1DArray(this->m_netPrecipitation);
#ifndef STORM_MODE
    if (this->m_evaporationLoss != NULL) Release1DArray(this->m_evaporationLoss);
#endif
}

void clsPI_MCS::Set1DData(const char *key, int nRows, float *data) {
    this->CheckInputSize(key, nRows);

    string s(key);
    if (StringMatch(s, VAR_PCP)) {
        m_P = data;
    }
    else if (StringMatch(s, VAR_PET)) {
#ifndef STORM_MODE
        m_PET = data;
#endif
    } else if (StringMatch(s, VAR_INTERC_MAX)) {
        m_maxSt = data;
    } else if (StringMatch(s, VAR_INTERC_MIN)) {
        m_minSt = data;
	}
    else if (StringMatch(s, VAR_LANDUSE))
        m_landuse = data;
	else {
        throw ModelException(MID_PI_SVSC, "Set1DData", "Parameter " + s + " does not exist.");
    }
}

void clsPI_MCS::SetValue(const char *key, float data) {
    string s(key);
    if (StringMatch(s, VAR_PI_B)) { this->m_Pi_b = data; }
    else if (StringMatch(s, VAR_INIT_IS)) { this->m_Init_IS = data; }
    else if (StringMatch(s, VAR_OMP_THREADNUM)) { SetOpenMPThread((int) data); }
    else if (StringMatch(s, VAR_PCP2CANFR_PR)) {this->m_pcp2canfr_pr = data; }
    else if (StringMatch(s, VAR_EMBNKFR_PR)) {this->m_embnkfr_pr = data; }
#ifdef STORM_MODE
    else if (StringMatch(s, Tag_HillSlopeTimeStep)) { m_hilldt = data; }
#endif // STORM_MODE
    else {
        throw ModelException(MID_PI_SVSC, "SetValue", "Parameter " + s + " does not exist.");
    }
}

void clsPI_MCS::Get1DData(const char *key, int *nRows, float **data) {
    initialOutputs();
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
    *nRows = this->m_nCells;
}

void clsPI_MCS::initialOutputs() {
    if (this->m_st == NULL) {
        Initialize1DArray(m_nCells, m_st, m_Init_IS);
    }
#ifndef STORM_MODE
    if (this->m_evaporationLoss == NULL) {
        Initialize1DArray(m_nCells, m_evaporationLoss, 0.f);
    }
#endif
    if (this->m_netPrecipitation == NULL) {
        Initialize1DArray(m_nCells, m_netPrecipitation, 0.f);
    }
    if (this->m_interceptionLoss == NULL) {
        Initialize1DArray(m_nCells, m_interceptionLoss, 0.f);
    }
}

int clsPI_MCS::Execute() {
    //check input data
    CheckInputData();
    /// initialize outputs
    initialOutputs();

    int julian = JulianDay(m_date);
#pragma omp parallel for
    for (int i = 0; i < this->m_nCells; i++) {
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
    if (this->m_date < 0) {
        throw ModelException(MID_PI_SVSC, "CheckInputData", "You have not set the time.");
    }

    if (m_nCells <= 0) {
        throw ModelException(MID_PI_SVSC, "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    }

    if (this->m_P == NULL) {
        throw ModelException(MID_PI_SVSC, "CheckInputData", "The precipitation data can not be NULL.");
    }
#ifndef STORM_MODE
    if (this->m_PET == NULL) {
        throw ModelException(MID_PI_SVSC, "CheckInputData", "The PET data can not be NULL.");
    }
#else
    if (this->m_slope == NULL) {
        throw ModelException(MID_PI_SVSC, "CheckInputData", "The slope gradient can not be NULL.");
    }
    if (this->m_hilldt < 0) {
        throw ModelException(MID_PI_SVSC, "CheckInputData", "The Hillslope scale time step must greater than 0.");
    }
#endif
    if (this->m_maxSt == NULL) {
        throw ModelException(MID_PI_SVSC, "CheckInputData",
                             "The maximum interception storage capacity can not be NULL.");
    }

    if (this->m_minSt == NULL) {
        throw ModelException(MID_PI_SVSC, "CheckInputData",
                             "The minimum interception storage capacity can not be NULL.");
    }

    if (this->m_Pi_b > 1.5f || this->m_Pi_b < 0.5f) {
        throw ModelException(MID_PI_SVSC, "CheckInputData",
                             "The interception storage capacity exponent can not be " + ValueToString(this->m_Pi_b) +
                                 ". It should between 0.5 and 1.5.");
    }
    if (this->m_Init_IS > 1.f || this->m_Init_IS < 0.f) {
        throw ModelException(MID_PI_SVSC, "CheckInputData",
                             "The Initial interception storage can not be " + ValueToString(this->m_Init_IS) +
                                 ". It should between 0 and 1.");
    }
    return true;
}

bool clsPI_MCS::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_PI_SVSC, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (this->m_nCells != n) {
        if (this->m_nCells <= 0) { this->m_nCells = n; }
        else {
            throw ModelException(MID_PI_SVSC, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}
