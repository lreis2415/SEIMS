#include "text.h"
#include "Interception_SVSC.h"


#include "ClimateParams.h"

clsPI_SVSC::clsPI_SVSC() :
    m_nCells(-1), m_Pi_b(-1.f), m_Init_IS(0.f), m_P(nullptr), m_snowfall(nullptr),
    m_netPrecipitation(nullptr), m_interceptionLoss(nullptr), m_st(nullptr), m_snowAcc(nullptr)
{

#ifndef STORM_MODE
    m_evaporationLoss = nullptr;
#else
    m_hilldt = -1.f;
    m_slope = nullptr;
#endif
    SetModuleName(M_PI_SVSC[0]);
}

clsPI_SVSC::~clsPI_SVSC(void) {
    Release1DArray(this->m_interceptionLoss);
    Release1DArray(this->m_st);
    Release1DArray(this->m_netPrecipitation);
    Release1DArray(this->m_snowAcc);
#ifndef STORM_MODE
    Release1DArray(this->m_evaporationLoss);
#endif
}

void clsPI_SVSC::Set1DData(const char *key, int nRows, FLTPT *data) {
    string s(key);
    if (StringMatch(s, VAR_PCP[0])) {
        m_P = data;
    }else if (StringMatch(s, VAR_SNOWFALL[0])) {
        m_snowfall = data;
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
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + s + " does not exist.");
    }
}

void clsPI_SVSC::SetValue(const char *key, FLTPT data) {
    string s(key);
    if (StringMatch(s, VAR_PI_B[0])) { m_Pi_b = data; }
    else if (StringMatch(s, VAR_INIT_IS[0])) { m_Init_IS = data; }
#ifdef STORM_MODE
    else if (StringMatch(s, Tag_HillSlopeTimeStep[0])) { m_hilldt = data; }
#endif // STORM_MODE
    else {
        throw ModelException(GetModuleName(), "SetValue", "Parameter " + s + " does not exist.");
    }
}

void clsPI_SVSC::Get1DData(const char *key, int *nRows, FLTPT **data) {
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
    } else if (StringMatch(s, VAR_SNAC[0])) {
        *data = m_snowAcc;
    }
    else {
        throw ModelException(GetModuleName(), "Get1DData", "Result " + s + " does not exist.");
    }
    *nRows = this->m_nCells;
}

void clsPI_SVSC:: InitialOutputs() {
    Initialize1DArray(m_nCells, m_st, m_Init_IS);
#ifndef STORM_MODE
    Initialize1DArray(m_nCells, m_evaporationLoss, 0.f);
#endif
    Initialize1DArray(m_nCells, m_netPrecipitation, 0.f);
    Initialize1DArray(m_nCells, m_snowAcc, 0.f);
    Initialize1DArray(m_nCells, m_interceptionLoss, 0.f);
}

int clsPI_SVSC::Execute() {
    //check input data
    CheckInputData();
    /// initialize outputs
    InitialOutputs();

    int julian = JulianDay(m_date);
#pragma omp parallel for
    for (int i = 0; i < this->m_nCells; i++) {
        m_interceptionLoss[i] = 0.f;
        m_netPrecipitation[i] = 0.f;
        FLTPT availableSpace = 0;
        if (m_P[i] > 0.f) {
#ifdef STORM_MODE
            /// correction for slope gradient, water spreads out over larger area
            m_P[i] = m_P[i] * m_hilldt / 3600.f * cos(atan(m_slope[i]));
#endif // STORM_MODE
            //interception storage capacity
            FLTPT degree = 2.f * PI * (julian - 87.f) / 365.f;
            /// For water, min and max are both 0, then no need for specific handling.
            FLTPT min = m_minSt[i];
            FLTPT max = m_maxSt[i];
            FLTPT capacity = min + (max - min) * CalPow(0.5f + 0.5f * sin(degree), m_Pi_b);

            //interception, currently, m_st[i] is storage of (t-1) time step
            availableSpace = NonNeg(capacity - m_st[i]);
            FLTPT loss = Convey(m_P[i], m_interceptionLoss[i], availableSpace, 1, false);
            availableSpace -= loss;
            availableSpace = NonNeg(availableSpace);
            //net precipitation
            m_netPrecipitation[i] = m_P[i] - loss;
            m_st[i] += loss;
        }
        if (m_snowfall[i]>0 && availableSpace > 0) {
            FLTPT loss = Convey(m_snowfall[i], m_interceptionLoss[i], availableSpace, 1, false);
            m_snowAcc[i] += m_snowfall[i] - loss;
            m_st[i] += loss;
        }
#ifndef STORM_MODE
        Convey(m_st[i], m_evaporationLoss[i], m_PET[i], 1);
#endif
    }
    return 0;
}

bool clsPI_SVSC::CheckInputData() {
    CHECK_POSITIVE(GetModuleName(), m_date);
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    CHECK_POINTER(GetModuleName(), m_P);
#ifndef STORM_MODE
    CHECK_POINTER(GetModuleName(), m_PET);
#else
    CHECK_POINTER(GetModuleName(), m_slope);
    CHECK_POSITIVE(GetModuleName(), m_hilldt);
#endif
    CHECK_POINTER(GetModuleName(), m_maxSt);
    CHECK_POINTER(GetModuleName(), m_minSt);

    if (this->m_Pi_b > 1.5f || this->m_Pi_b < 0.5f) {
        throw ModelException(GetModuleName(), "CheckInputData",
                             "The interception storage capacity exponent can not be " + ValueToString(this->m_Pi_b) +
                             ". It should between 0.5 and 1.5.");
    }
    if (this->m_Init_IS > 1.f || this->m_Init_IS < 0.f) {
        throw ModelException(GetModuleName(), "CheckInputData",
                             "The Initial interception storage can not be " + ValueToString(this->m_Init_IS) +
                             ". It should between 0 and 1.");
    }
    return true;
}

bool clsPI_SVSC::CheckInputSize(const char *key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}
