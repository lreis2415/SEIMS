#include "GMELT_SIMPLE.h"
#include "text.h"
#include "Lookup.h"

GMELT_SIMPLE::GMELT_SIMPLE() :
    m_nCells(-1),
    m_potentialMelt(nullptr),
    m_excessPcp(nullptr),
    m_landuse(nullptr),
    m_glacMelt(nullptr)
    {
        SetModuleName(M_GMELT_SIMPLE[0]);
    }

GMELT_SIMPLE::~GMELT_SIMPLE() {
    Release1DArray(m_glacMelt);
}

void GMELT_SIMPLE::InitialOutputs() {
    Initialize1DArray(m_nCells, m_glacMelt, 0.);
}

bool GMELT_SIMPLE::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException(M_PMELT_DD[0], "CheckInputData", "Input data is invalid. The size could not be less than zero.");
        return false;
    }
    CHECK_POINTER(GetModuleName(), m_potentialMelt);
    CHECK_POINTER(GetModuleName(), m_excessPcp);
    CHECK_POINTER(GetModuleName(), m_landuse);
    return true;
}

void GMELT_SIMPLE::Set1DData(const char* key, int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_POTENTIAL_MELT[0])) { m_potentialMelt = data; }
    else if (StringMatch(sk, VAR_EXCP[0])) { m_excessPcp = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}
void GMELT_SIMPLE::Set1DData(const char* key, int n, int* data) {
    string sk(key);
    if (StringMatch(sk, VAR_LANDUSE[0])) { m_landuse = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
            + " does not exist in current module. Please contact the module developer.");
    }
}

void GMELT_SIMPLE::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_GLAC_MELT[0])) { *data = m_glacMelt; }
    else {
        throw ModelException(GetModuleName(), "Get1DData", "Output " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

bool GMELT_SIMPLE::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}
// This function is adapted from the Raven project
int GMELT_SIMPLE::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (IsGlacier(m_landuse[i])) {
            Flush(m_excessPcp[i],m_glacMelt[i]);
            Supply(m_glacMelt[i],m_potentialMelt[i]);
        }
    }
    return 0;
}
