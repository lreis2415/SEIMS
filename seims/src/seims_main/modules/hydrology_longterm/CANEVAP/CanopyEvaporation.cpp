#include "CanopyEvaporation.h"
#include "text.h"

CanopyEvaporation::CanopyEvaporation():
m_nCells(-1),
m_canopyStorage(nullptr),
m_canopyEvaporation(nullptr)
{
    SetModuleName(M_CAN_EVAP[0]);
}

void CanopyEvaporation::InitialOutputs() {
    Initialize1DArray(m_nCells, m_canopyEvaporation, 0.);
}

CanopyEvaporation::~CanopyEvaporation() {
    Release1DArray(m_canopyEvaporation);
}

void CanopyEvaporation::Set1DData(const char* key, int n, FLTPT* data) {
    string sk(key);

    if (StringMatch(sk, GetModuleName())) { m_canopyStorage = data; }
    else {
        throw ModelException(M_CAN_EVAP[0], "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

bool CanopyEvaporation::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

bool CanopyEvaporation::CheckInputData(void) {
    CHECK_POINTER(M_CAN_EVAP[0], m_canopyStorage);
    return true;
}

void CanopyEvaporation::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_CANEVAP[0])) { *data = m_canopyEvaporation; }
    else {
        throw ModelException(M_CAN_EVAP[0], "Get1DData", "Output " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

int CanopyEvaporation::Execute() {
    CheckInputData();
    InitialOutputs();
    
    memcpy(m_canopyEvaporation, m_canopyStorage, m_nCells * sizeof(FLTPT));
    memset(m_canopyStorage, 0, m_nCells * sizeof(FLTPT));

    return 0;
}
