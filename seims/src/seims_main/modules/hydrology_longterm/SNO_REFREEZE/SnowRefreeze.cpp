#include "SnowRefreeze.h"
#include "text.h"

SnowRefreeze::SnowRefreeze():
    m_nCells(-1),
    m_snowLiq(nullptr),
    m_snowAcc(nullptr)
    {
        SetModuleName(M_SNOW_REFREEZE[0]);
    }

void SnowRefreeze::InitialOutputs() {
    Initialize1DArray(m_nCells, m_snowAcc, 0.);
}

SnowRefreeze::~SnowRefreeze() {
    Release1DArray(m_snowAcc);
}

void SnowRefreeze::Set1DData(const char* key, int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_SNOW_LIQUID[0])) { m_snowLiq = data; }
    else if (StringMatch(sk, VAR_SNAC[0])) { m_snowAcc = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

bool SnowRefreeze::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}


bool SnowRefreeze::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException(GetModuleName(), "CheckInputData", "Input data is invalid. The size could not be less than zero.");
    }
    CHECK_POINTER(GetModuleName(), m_snowLiq);
    CHECK_POINTER(GetModuleName(), m_snowAcc);
    return true;
}

void SnowRefreeze::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_SNAC[0])) { *data = m_snowAcc; }
    else {
        throw ModelException(GetModuleName(), "Get1DData", "Output " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

int SnowRefreeze::Execute() {
    CheckInputData();
    InitialOutputs();

 #pragma omp parallel for
     for (int i = 0; i < m_nCells; i++) {
         m_snowAcc[i] = m_snowLiq[i];
         Flush(m_snowLiq[i], m_snowAcc[i]);
     }
    return 0;
}
