#include "GMELT_HBV.h"
#include "text.h"
#include "Lookup.h"

GMELT_HBV::GMELT_HBV() :
    m_nCells(-1),
    m_potentialMelt(nullptr),
    m_snowLiq(nullptr),
    m_landuse(nullptr),
    m_hbv_glacier_melt_correction(0),
    m_glacierMelt(nullptr)
    {
        SetModuleName(M_GMELT_HBV[0]);
    }

GMELT_HBV::~GMELT_HBV() {
    Release1DArray(m_glacierMelt);
}

void GMELT_HBV::InitialOutputs() {
    Initialize1DArray(m_nCells, m_glacierMelt, 0.);
}

bool GMELT_HBV::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException(GetModuleName(), "CheckInputData", "Input data is invalid. The size could not be less than zero.");
        return false;
    }
    CHECK_POINTER(GetModuleName(), m_landuse);
    CHECK_POINTER(GetModuleName(), m_potentialMelt);
    return true;
}

void GMELT_HBV::SetValue(const char* key, FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_HBV_GLAC_MELT_CORR[0])) m_hbv_glacier_melt_correction = value;
    else {
        throw ModelException(GetModuleName(), "SetValue",
                             "Parameter " + sk + " does not exist.");
    }
}

void GMELT_HBV::Set1DData(const char* key, int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_POTENTIAL_MELT[0])) { m_potentialMelt = data; }
    else if (StringMatch(sk, VAR_SNOW_LIQUID[0])) { m_snowLiq = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}
void GMELT_HBV::Set1DData(const char* key, int n, int* data) {
    string sk(key);
    if (StringMatch(sk, VAR_LANDUSE[0])) { m_landuse = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
            + " does not exist in current module. Please contact the module developer.");
    }
}
void GMELT_HBV::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    *n = m_nCells;
    if (StringMatch(sk, VAR_GLAC_MELT[0])) { *data = m_glacierMelt; }
    else {
        throw ModelException(GetModuleName(), "Get1DData", "Output " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

bool GMELT_HBV::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

int GMELT_HBV::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_snowLiq[i] > 0){
            continue;
        }
        if (IsGlacier(m_landuse[i])) {
            Supply(m_glacierMelt[i], m_hbv_glacier_melt_correction * NonNeg(m_potentialMelt[i]));
        }
    }
#ifdef PRINT_DEBUG
    FLTPT s1 = 0;
    FLTPT s2 = 0;
    for (int i = 0; i < m_nCells; i++) {
        if (IsGlacier(m_landuse[i])) {
            s1 += m_glacierMelt[i];
        }else {
            s2 += m_glacierMelt[i];
        }
    }
    printf("[GMELT_HBV] m_glacierMelt=%f. (%f on non-glacier cells)\n", s1, s2);
    fflush(stdout);
#endif // PRINT_DEBUG

    return 0;
}
