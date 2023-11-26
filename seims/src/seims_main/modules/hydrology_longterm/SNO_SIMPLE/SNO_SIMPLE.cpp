#include "SNO_SIMPLE.h"
#include "text.h"

SNO_SIMPLE::SNO_SIMPLE():
    m_nCells(-1),
    m_potentialMelt(nullptr),
    m_snowMelt(nullptr)
    {
        SetModuleName(M_SNO_SIMPLE[0]);
    }

void SNO_SIMPLE::InitialOutputs() {
    Initialize1DArray(m_nCells, m_snowMelt, 0.);
}

SNO_SIMPLE::~SNO_SIMPLE() {
    Release1DArray(m_snowMelt);
}

void SNO_SIMPLE::Set1DData(const char* key, int n, FLTPT* data) {
    string sk(key);

    if (StringMatch(sk, VAR_POTENTIAL_MELT[0])) { m_potentialMelt = data; }
    else if (StringMatch(sk, VAR_SNAC[0])) { m_snowAcc = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

bool SNO_SIMPLE::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

bool SNO_SIMPLE::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException(GetModuleName(), "CheckInputData", "Input data is invalid. The size could not be less than zero.");
    }
    CHECK_POINTER(GetModuleName(), m_potentialMelt);
    return true;
}

void SNO_SIMPLE::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_SNME[0])) { *data = m_snowMelt; }
    else {
        throw ModelException(GetModuleName(), "Get1DData", "Output " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

int SNO_SIMPLE::Execute() {
    CheckInputData();
    InitialOutputs();

    #pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        Convey(m_snowAcc[i], m_snowMelt[i], NonNeg(m_potentialMelt[i]));
    }

    return 0; // Return 0 if everything is successful
}
