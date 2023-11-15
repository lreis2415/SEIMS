#include "CanopySublimation.h"
#include "text.h"

CanopySublimation::CanopySublimation():
    m_nCells(-1),
    m_canopySnowStorage(nullptr),
    m_canopySublimation(nullptr)
    {
        SetModuleName(M_CAN_SUBLIMATION[0]);
    }

void CanopySublimation::InitialOutputs() {
    Initialize1DArray(m_nCells, m_canopySublimation, 0.);
}

CanopySublimation::~CanopySublimation() {
    Release1DArray(m_canopySublimation);
}

void CanopySublimation::Set1DData(const char* key, int n, FLTPT* data) {
    string sk(key);

    if (StringMatch(sk, VAR_CANSTOR_SNOW[0])) { m_canopySnowStorage = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

bool CanopySublimation::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

bool CanopySublimation::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException(GetModuleName(), "CheckInputData", "Input data is invalid. The size could not be less than zero.");
        return false;
    }
    CHECK_POINTER(GetModuleName(), m_canopySnowStorage);
    CHECK_POINTER(GetModuleName(), m_canopySublimation);
    return true;
}

void CanopySublimation::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_CANSLM[0])) { *data = m_canopySublimation; }
    else {
        throw ModelException(GetModuleName(), "Get1DData", "Output " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

int CanopySublimation::Execute() {
    CheckInputData();
    InitialOutputs();

    memcpy(m_canopySublimation, m_canopySnowStorage, m_nCells * sizeof(FLTPT));
    memset(m_canopySnowStorage, 0, m_nCells * sizeof(FLTPT));

    return 0;
}
