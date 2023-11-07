#include "PMELT_DD.h"
#include "text.h"

PMELT_DD::PMELT_DD():
    m_nCells(-1),
    m_temp_daily_ave(nullptr), 
    m_potentialMelt(nullptr),
    m_Ma(0),
    m_melt_temp(0),{
}

void PMELT_DD::InitialOutputs() {
    Initialize1DArray(m_nCells, m_potentialMelt, 0.);
}

PMELT_DD::~PMELT_DD() {
    Release1DArray(m_potentialMelt);
}

void PMELT_DD::SetValue(const char* key, FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_MA[0])) m_Ma = value;
    else if (StringMatch(sk, VAR_MELT_TEMP[0])) m_melt_temp = value;
    else {
        throw ModelException(M_PMELT_DD[0], "SetValue",
                             "Parameter " + sk + " does not exist.");
    }
}

void PMELT_DD::Set1DData(const char* key, int n, FLTPT* data) {
    CheckInputSize(key, n);
    string sk(key);

    if (StringMatch(sk, VAR_TMEAN[0])) { m_temp_daily_ave = data; }
    else {
        throw ModelException(M_PMELT_DD[0], "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

bool PMELT_DD::CheckInputSize(const char* key, int n) {
    if (n <= 0) {
        throw ModelException(M_PMELT_DD[0], "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(M_PMELT_DD[0], "CheckInputSize", "Input data for " + string(key)
                                 + " is invalid. All the input data should have same size.");
            return false;
        }
    }
    return true;
}

void PMELT_DD::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_POTENTIAL_MELT[0])) { *data = m_potentialMelt; }
    else {
        throw ModelException(M_PMELT_DD[0], "Get1DData", "Output " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

int PMELT_DD::Execute() {
    InitialOutputs();
    CheckInputData();

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_potentialMelt[i] = m_Ma * (m_temp_daily_ave[i] - m_melt_temp);
    }

#ifdef PRINT_DEBUG
    FLTPT sum = 0;
    for (int i = 0; i < m_nCells; i++) {
        sum += m_potentialMelt[i];
    }
    printf("\n[PMELT_DD] m_potentialMelt=%f\n", sum);
    fflush(stdout);
#endif
    return 0;
}