#include "GREL_LINEAR.h"
#include "text.h"
#include "Lookup.h"

GREL_LINEAR::GREL_LINEAR() :
    m_nCells(-1),
    m_glacMelt(nullptr),
    m_landuse(nullptr),
    m_glacStorageCoef(0),
    m_glacRelease(nullptr)
    {
        SetModuleName(VAR_GLAC_MELT[0]);
    }

GREL_LINEAR::~GREL_LINEAR() {
    Release1DArray(m_glacRelease);
}

void GREL_LINEAR::InitialOutputs() {
    Initialize1DArray(m_nCells, m_glacRelease, 0.);
}

bool GREL_LINEAR::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException(M_GREL_LINEAR[0], "CheckInputData", "Input data is invalid. The size could not be less than zero.");
        return false;
    }
    CHECK_POINTER(GetModuleName(), m_glacMelt);
    CHECK_POINTER(VAR_GLAC_REL[0], m_glacRelease);
    return true;
}

void GREL_LINEAR::SetValue(const char* key, FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_GLAC_STOR_COEF[0])) m_glacStorageCoef = value;
    else {
        throw ModelException(M_GREL_LINEAR[0], "SetValue",
                             "Parameter " + sk + " does not exist.");
    }
}

void GREL_LINEAR::Set1DData(const char* key, int n, FLTPT* data) {
    string sk(key);

    if (StringMatch(sk, GetModuleName())) { m_glacMelt = data; }
    else {
        throw ModelException(M_GREL_LINEAR[0], "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

void GREL_LINEAR::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_GLAC_REL[0])) { *data = m_glacRelease; }
    else {
        throw ModelException(M_GREL_LINEAR[0], "Get1DData", "Output " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

bool GREL_LINEAR::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}
// This function is adapted from the Raven project
int GREL_LINEAR::Execute() {
    CheckInputData();
    InitialOutputs();

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (IsGlacier(m_landuse[i])) {
            m_glacRelease[i] += m_glacMelt[i]*m_glacStorageCoef;
        }
        
    }

    return 0;
}
