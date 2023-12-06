#include "RainSnowT.h"
#include "text.h"

RainSnowT::RainSnowT() :
m_nCells(-1),
m_t_mean(nullptr),
m_t_rain_snow(0),
m_snowfall(nullptr) {
    SetModuleName(M_RAINSNOW_T[0]);
}

RainSnowT::~RainSnowT() {
    Release1DArray(m_snowfall);
}

void RainSnowT::InitialOutputs() {
    Initialize1DArray(m_nCells, m_snowfall, 0.f);
}

void RainSnowT::SetValue(const char* key, FLTPT value) {
    if (StringMatch(key, VAR_T_SNOW[0])) {m_t_rain_snow = value;}
    else {
        throw ModelException(GetModuleName(), "SetValue", "Parameter " + string(key) + " does not exist.");
    }
}

void RainSnowT::Set1DData(const char* key, int n, FLTPT* data) {
    if (StringMatch(key, VAR_TMEAN[0])) {m_t_mean = data;}
    else if (StringMatch(key, VAR_PCP[0])) {m_pcp = data;}
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + string(key) + " does not exist.");
    }
}

void RainSnowT::Get1DData(const char* key, int* n, FLTPT** data) {
    *n = m_nCells;
    if (StringMatch(key, VAR_SNOWFALL[0])) {
        *data = m_snowfall;
    } else {
        throw ModelException(GetModuleName(), "Get1DData", "Output " + string(key) + " does not exist.");
    }
}

bool RainSnowT::CheckInputData() {
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    return true;
}

bool RainSnowT::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

int RainSnowT::Execute() {
    CheckInputData();
    InitialOutputs();
    memset(m_snowfall, 0, sizeof(FLTPT) * m_nCells);
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_t_mean[i] <= m_t_rain_snow) {
            Flush(m_pcp[i], m_snowfall[i]);
        }
    }
    return 0;
}
