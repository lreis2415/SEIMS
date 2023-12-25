#include "RainSnowHBV.h"
#include "text.h"

RainSnowHBV::RainSnowHBV() :
m_nCells(-1),
m_t_mean(nullptr),
m_t_rain_snow(0),
m_t_rain_snow_delta(0),
m_snowfall(nullptr) {
    SetModuleName(M_RAINSNOW_HBV[0]);
}

RainSnowHBV::~RainSnowHBV() {
    Release1DArray(m_snowfall);
}

void RainSnowHBV::InitialOutputs() {
    Initialize1DArray(m_nCells, m_snowfall, 0.f);
}

void RainSnowHBV::SetValue(const char* key, FLTPT value) {
    if (StringMatch(key, VAR_T_SNOW[0])) {m_t_rain_snow = value;}
    else if (StringMatch(key, VAR_T_RAIN_SNOW_DELTA[0])) {m_t_rain_snow_delta = value;}
    else {
        throw ModelException(GetModuleName(), "SetValue", "Parameter " + string(key) + " does not exist.");
    }
}

void RainSnowHBV::Set1DData(const char* key, int n, FLTPT* data) {
    if (StringMatch(key, VAR_TMEAN[0])) {m_t_mean = data;}
    else if (StringMatch(key, VAR_PCP[0])) {m_pcp = data;}
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + string(key) + " does not exist.");
    }
}

void RainSnowHBV::Get1DData(const char* key, int* n, FLTPT** data) {
    *n = m_nCells;
    if (StringMatch(key, VAR_SNOWFALL[0])) {
        *data = m_snowfall;
    } else {
        throw ModelException(GetModuleName(), "Get1DData", "Output " + string(key) + " does not exist.");
    }
}

bool RainSnowHBV::CheckInputData() {
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    return true;
}

bool RainSnowHBV::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

int RainSnowHBV::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        FLTPT frac = 0;
        if (m_t_mean[i] <= m_t_rain_snow - m_t_rain_snow_delta / 2) {
            frac = 1;
        } else if (m_t_mean[i] >= m_t_rain_snow + m_t_rain_snow_delta / 2){
            frac = 0;
        } else{
            frac = 0.5 + (m_t_rain_snow - m_t_rain_snow_delta) / m_t_rain_snow_delta;
        }
        Convey(m_pcp[i], m_snowfall[i], frac*m_pcp[i]);
    }
#ifdef PRINT_DEBUG
    FLTPT s1 = 0;
    FLTPT s2 = 0;
    FLTPT s3 = 0;
    for (int i = 0; i < m_nCells; i++) {
        s1 += m_pcp[i];
        s2 += m_snowfall[i];
        s3 += m_t_mean[i];
    }
    s3/=m_nCells;
    printf("[RAINSNOW_HBV] T_mean=%f, pcp=%f, snowfall=%f\n", s3, s1, s2);
    fflush(stdout);
#endif // PRINT_DEBUG
    return 0;
}
