#include "AtmosphericDeposition.h"

#include "text.h"

AtmosphericDeposition::AtmosphericDeposition() :
    m_nCells(-1), m_maxSoilLyrs(-1), m_rainNO3Conc(-1.f), m_rainNH4Conc(-1.f),
    m_dryDepNO3(-1.f), m_dryDepNH4(-1.f), m_pcp(nullptr),
    m_soilNH4(nullptr), m_soilNO3(nullptr) {
}

AtmosphericDeposition::~AtmosphericDeposition() {
}

bool AtmosphericDeposition::CheckInputData() {
    CHECK_POSITIVE(MID_ATMDEP, m_nCells);
    CHECK_POSITIVE(MID_ATMDEP, m_maxSoilLyrs);
    CHECK_POSITIVE(MID_ATMDEP, m_rainNO3Conc);
    CHECK_POSITIVE(MID_ATMDEP, m_rainNH4Conc);
    CHECK_POSITIVE(MID_ATMDEP, m_dryDepNO3);
    CHECK_POSITIVE(MID_ATMDEP, m_dryDepNH4);
    CHECK_POINTER(MID_ATMDEP, m_soilNO3);
    CHECK_POINTER(MID_ATMDEP, m_soilNH4);
    return true;
}

void AtmosphericDeposition::Set1DData(const char* key, const int n, float* data) {
    CheckInputSize(MID_ATMDEP, key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_PCP)) m_pcp = data;
    else {
        throw ModelException(MID_ATMDEP, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void AtmosphericDeposition::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, VAR_RCN)) m_rainNO3Conc = value;
    else if (StringMatch(sk, VAR_RCA)) m_rainNH4Conc = value;
    else if (StringMatch(sk, VAR_DRYDEP_NO3)) m_dryDepNO3 = value;
    else if (StringMatch(sk, VAR_DRYDEP_NH4)) m_dryDepNH4 = value;
    else {
        throw ModelException(MID_ATMDEP, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void AtmosphericDeposition::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    CheckInputSize2D(MID_ATMDEP, key, nrows, ncols, m_nCells, m_maxSoilLyrs);
    string sk(key);
    m_maxSoilLyrs = ncols;
    if (StringMatch(sk, VAR_SOL_NO3)) m_soilNO3 = data;
    else if (StringMatch(sk, VAR_SOL_NH4)) m_soilNH4 = data;
    else {
        throw ModelException(MID_ATMDEP, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

int AtmosphericDeposition::Execute() {
    CheckInputData();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_pcp[i] <= 0.f) { continue; }
        /// Calculate the amount of nitrite and ammonia added to the soil in rainfall
        /// unit conversion: mg/L * mm = 0.01 * kg/ha (CHECKED)
        float addrno3 = 0.01f * m_rainNO3Conc * m_pcp[i];
        float addrnh4 = 0.01f * m_rainNH4Conc * m_pcp[i];
        m_soilNO3[i][0] += addrno3 + m_dryDepNO3 * 0.0027397260273972603f; // 1. / 365.f;
        m_soilNH4[i][0] += addrnh4 + m_dryDepNH4 * 0.0027397260273972603f;
    }
    return 0;
}
