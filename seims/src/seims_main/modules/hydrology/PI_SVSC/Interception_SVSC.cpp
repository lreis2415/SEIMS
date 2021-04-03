#include "Interception_SVSC.h"

#include "text.h"
#include "ClimateParams.h"

clsPI_SVSC::clsPI_SVSC(void) : m_nCells(-1), m_Pi_b(-1.f), m_Init_IS(0.f),
                             m_netPrecipitation(NULL), m_interceptionLoss(NULL), m_st(NULL) {
    
#ifndef STORM_MODE
    m_evaporationLoss = NULL;
#else
    m_hilldt = -1.f;
    m_slope = NULL;
#endif
}

clsPI_SVSC::~clsPI_SVSC(void) {
    if (this->m_interceptionLoss != NULL) Release1DArray(this->m_interceptionLoss);
    if (this->m_st != NULL) Release1DArray(this->m_st);
    if (this->m_netPrecipitation != NULL) Release1DArray(this->m_netPrecipitation);
#ifndef STORM_MODE
    if (this->m_evaporationLoss != NULL) Release1DArray(this->m_evaporationLoss);
#endif
}

void clsPI_SVSC::Set1DData(const char *key, int nRows, float *data) {
    this->CheckInputSize(key, nRows);

    string s(key);
    if (StringMatch(s, VAR_PCP[0])) {
        m_P = data;
    }
    else if (StringMatch(s, VAR_PET[0])) {
#ifndef STORM_MODE
        m_PET = data;
#endif
    } else if (StringMatch(s, VAR_INTERC_MAX[0])) {
        m_maxSt = data;
    } else if (StringMatch(s, VAR_INTERC_MIN[0])) {
        m_minSt = data;
    } else {
        throw ModelException(M_PI_SVSC[0], "Set1DData", "Parameter " + s + " does not exist.");
    }
}

void clsPI_SVSC::SetValue(const char *key, float data) {
    string s(key);
    if (StringMatch(s, VAR_PI_B[0])) { m_Pi_b = data; }
    else if (StringMatch(s, VAR_INIT_IS[0])) { m_Init_IS = data; }
#ifdef STORM_MODE
    else if (StringMatch(s, Tag_HillSlopeTimeStep[0])) { m_hilldt = data; }
#endif // STORM_MODE
    else {
        throw ModelException(M_PI_SVSC[0], "SetValue", "Parameter " + s + " does not exist.");
    }
}

void clsPI_SVSC::Get1DData(const char *key, int *nRows, float **data) {
    InitialOutputs();
    string s = key;
    if (StringMatch(s, VAR_INLO[0])) {
        *data = m_interceptionLoss;
    } else if (StringMatch(s, VAR_INET[0])) {
#ifndef STORM_MODE
        *data = m_evaporationLoss;
#endif
    } else if (StringMatch(s, VAR_CANSTOR[0])) {
        *data = m_st;
    } else if (StringMatch(s, VAR_NEPR[0])) {
        *data = m_netPrecipitation;
    } else {
        throw ModelException(M_PI_SVSC[0], "Get1DData", "Result " + s + " does not exist.");
    }
    *nRows = this->m_nCells;
}

void clsPI_SVSC:: InitialOutputs() {
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

int clsPI_SVSC::Execute() {
    //check input data
    CheckInputData();
    /// initialize outputs
    InitialOutputs();

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
            } else {
                m_interceptionLoss[i] = m_P[i];
            }

            //net precipitation
            m_netPrecipitation[i] = m_P[i] - m_interceptionLoss[i];
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

bool clsPI_SVSC::CheckInputData() {
    if (this->m_date < 0) {
        throw ModelException(M_PI_SVSC[0], "CheckInputData", "You have not set the time.");
    }

    if (m_nCells <= 0) {
        throw ModelException(M_PI_SVSC[0], "CheckInputData",
                             "The dimension of the input data can not be less than zero.");
    }

    if (this->m_P == NULL) {
        throw ModelException(M_PI_SVSC[0], "CheckInputData", "The precipitation data can not be NULL.");
    }
#ifndef STORM_MODE
    if (this->m_PET == NULL) {
        throw ModelException(M_PI_SVSC[0], "CheckInputData", "The PET data can not be NULL.");
    }
#else
    if (this->m_slope == NULL) {
        throw ModelException(M_PI_SVSC[0], "CheckInputData", "The slope gradient can not be NULL.");
    }
    if (this->m_hilldt < 0) {
        throw ModelException(M_PI_SVSC[0], "CheckInputData", "The Hillslope scale time step must greater than 0.");
    }
#endif
    if (this->m_maxSt == NULL) {
        throw ModelException(M_PI_SVSC[0], "CheckInputData",
                             "The maximum interception storage capacity can not be NULL.");
    }

    if (this->m_minSt == NULL) {
        throw ModelException(M_PI_SVSC[0], "CheckInputData",
                             "The minimum interception storage capacity can not be NULL.");
    }

    if (this->m_Pi_b > 1.5f || this->m_Pi_b < 0.5f) {
        throw ModelException(M_PI_SVSC[0], "CheckInputData",
                             "The interception storage capacity exponent can not be " + ValueToString(this->m_Pi_b) +
                                 ". It should between 0.5 and 1.5.");
    }
    if (this->m_Init_IS > 1.f || this->m_Init_IS < 0.f) {
        throw ModelException(M_PI_SVSC[0], "CheckInputData",
                             "The Initial interception storage can not be " + ValueToString(this->m_Init_IS) +
                                 ". It should between 0 and 1.");
    }
    return true;
}

bool clsPI_SVSC::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(M_PI_SVSC[0], "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (this->m_nCells != n) {
        if (this->m_nCells <= 0) { this->m_nCells = n; }
        else {
            throw ModelException(M_PI_SVSC[0], "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}
