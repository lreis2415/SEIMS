#include "GREL_HBV.h"
#include "text.h"
#include "Lookup.h"

GREL_HBV::GREL_HBV():
    m_nCells(-1),
    m_glacMelt(nullptr),
    m_snowAcc(nullptr),
    m_snowLiq(nullptr),
    m_landuse(nullptr),
    m_glacStorageCoef(0),
    m_kMin(0),
    m_ag(0),
    m_glacRelease(nullptr)
    {
        SetModuleName(M_GREL_HBV[0]);
    }

GREL_HBV::~GREL_HBV() {
    Release1DArray(m_glacRelease);
}

void GREL_HBV::InitialOutputs() {
    Initialize1DArray(m_nCells, m_glacRelease, 0.);
}

bool GREL_HBV::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException(GetModuleName(), "CheckInputData", "Input data is invalid. The size could not be less than zero.");
    }
    CHECK_POINTER(GetModuleName(), m_glacMelt);
    CHECK_POINTER(GetModuleName(), m_snowAcc);
    CHECK_POINTER(GetModuleName(), m_snowLiq);
    return true;
}

void GREL_HBV::SetValue(const char* key, FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_GLAC_STOR_COEF[0])) m_glacStorageCoef = value;
    else if (StringMatch(sk, VAR_HBV_GREL_KMIN[0])) { m_kMin = value; }
    else if (StringMatch(sk, VAR_HBV_GREL_AG[0])) { m_ag = value; }
    else {
        throw ModelException(GetModuleName(), "SetValue",
                             "Parameter " + sk + " does not exist.");
    }
}

void GREL_HBV::Set1DData(const char* key, int n, FLTPT* data) {
    string sk(key);
    
    if (StringMatch(sk, VAR_GLAC_MELT[0])) { m_glacMelt = data; }
    else if (StringMatch(sk, VAR_SNAC[0])) { m_snowAcc = data; }
    else if (StringMatch(sk, VAR_SNOW_LIQUID[0])) { m_snowLiq = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}
void GREL_HBV::Set1DData(const char* key, int n, int* data) {
    string sk(key);
    if (StringMatch(sk, VAR_LANDUSE[0])) { m_landuse = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
            + " does not exist in current module. Please contact the module developer.");
    }
}
void GREL_HBV::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_GLAC_REL[0])) { *data = m_glacRelease; }
    else {
        throw ModelException(GetModuleName(), "Get1DData", "Output " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

bool GREL_HBV::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

int GREL_HBV::Execute() {
    CheckInputData();
    InitialOutputs();
#ifdef PRINT_DEBUG
    FLTPT s0 = 0;
    for (int i = 0; i < m_nCells; i++) {
        if (!IsGlacier(m_landuse[i])) {
            continue;
        }
        s0 += m_glacMelt[i];
    }
    printf("[GREL_HBV] GMelt=%f\n", s0);
    fflush(stdout);
#endif // PRINT_DEBUG
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (!IsGlacier(m_landuse[i])) {
            continue;
        }
        FLTPT K = m_kMin + (m_glacStorageCoef-m_kMin)*exp(-m_ag*(m_snowAcc[i]+m_snowLiq[i]));
        Convey(m_glacMelt[i],m_glacRelease[i],K*m_glacMelt[i]);
    }
#ifdef PRINT_DEBUG
    FLTPT s1 = 0;
    FLTPT s3 = 0;
    FLTPT s4 = 0;
    int s2 = 0;
    for (int i = 0; i < m_nCells; i++) {
        if (!IsGlacier(m_landuse[i])) {
            continue;
        }
        s2++;
        s1 += m_glacRelease[i];
        s3 += m_snowAcc[i];
        s4 += m_snowLiq[i];
    }
    printf("[GREL_HBV] %d Glacier cells. GRelease=%f. SnowAcc(%f), SnowLiq(%f)\n", s2, s1, s3, s4);
    fflush(stdout);
#endif // PRINT_DEBUG
    return 0;
}
