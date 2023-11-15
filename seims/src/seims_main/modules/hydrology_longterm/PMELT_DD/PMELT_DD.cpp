#include "PMELT_DD.h"
#include "text.h"

PMELT_DD::PMELT_DD():
    m_nCells(-1),
    m_t_mean(nullptr), 
    m_potentialMelt(nullptr),
    m_Ma(0),
    m_t_melt(0){
        SetModuleName(M_PMELT_DD[0]);
}

void PMELT_DD::InitialOutputs() {
    Initialize1DArray(m_nCells, m_potentialMelt, 0.);
}

PMELT_DD::~PMELT_DD() {
    Release1DArray(m_potentialMelt);
}

void PMELT_DD::SetValue(const char* key, FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_T0[0])) m_t_melt = value;
    else {
        throw ModelException(GetModuleName(), "SetValue",
                             "Parameter " + sk + " does not exist.");
    }
}


void PMELT_DD::Set1DData(const char* key, int n, FLTPT* data) {
    string sk(key);

    if (StringMatch(sk, VAR_TMEAN[0])) { m_t_mean = data; }
    else if (StringMatch(sk, VAR_MELT_FACTOR[0])) { m_Ma = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

bool PMELT_DD::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

bool PMELT_DD::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException(GetModuleName(), "CheckInputData", "Input data is invalid. The size could not be less than zero.");
        return false;
    }
    CHECK_POINTER(GetModuleName(), m_t_mean);
    CHECK_POINTER(GetModuleName(), m_potentialMelt);
    return true;
}

void PMELT_DD::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_POTENTIAL_MELT[0])) { *data = m_potentialMelt; }
    else {
        throw ModelException(GetModuleName(), "Get1DData", "Output " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

int PMELT_DD::Execute() {
    InitialOutputs();
    CheckInputData();

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_potentialMelt[i] = Max(m_Ma[i] * (m_t_mean[i] - m_t_melt),0);
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