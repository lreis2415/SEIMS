#include "AtmosphericDeposition.h"

#include "text.h"

AtmosphericDeposition::AtmosphericDeposition() :
//input
    m_nCells(-1), m_rcn(-1.f), m_rca(-1.f), m_nMaxSoiLayers(-1),
    m_preci(nullptr), m_drydep_no3(-1.f), m_drydep_nh4(-1.f),
    m_addrno3(-1.f), m_addrnh4(-1.f),
    //output
    m_sol_no3(nullptr), m_sol_nh4(nullptr), m_wshd_rno3(-1.f) {
}

AtmosphericDeposition::~AtmosphericDeposition() {
}

bool AtmosphericDeposition::CheckInputData() {
    CHECK_POSITIVE(MID_ATMDEP, m_nCells);
    CHECK_POSITIVE(MID_ATMDEP, m_nMaxSoiLayers);
    CHECK_POSITIVE(MID_ATMDEP, m_rcn);
    CHECK_POSITIVE(MID_ATMDEP, m_rca);
    CHECK_POSITIVE(MID_ATMDEP, m_drydep_no3);
    CHECK_POSITIVE(MID_ATMDEP, m_drydep_nh4);
    CHECK_POINTER(MID_ATMDEP, m_sol_no3);
    CHECK_POINTER(MID_ATMDEP, m_sol_nh4);
    return true;
}

bool AtmosphericDeposition::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_ATMDEP, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) {
            m_nCells = n;
        } else {
            throw ModelException(MID_ATMDEP, "CheckInputSize",
                                 "Input data for " + string(key) + " is invalid with size: " + ValueToString(n) +
                                     ". The origin size is "
                                     + ValueToString(m_nCells) + ".\n");
        }
    }
    return true;
}

void AtmosphericDeposition::Set1DData(const char *key, int n, float *data) {
    CheckInputSize(key, n);
    string sk(key);
    if (StringMatch(sk, VAR_PCP)) { m_preci = data; }
    else {
        throw ModelException(MID_ATMDEP, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void AtmosphericDeposition::SetValue(const char *key, float value) {
    string sk(key);
    if (StringMatch(sk, VAR_RCN)) { m_rcn = value; }
    else if (StringMatch(sk, VAR_RCA)) { m_rca = value; }
    else if (StringMatch(sk, VAR_DRYDEP_NO3)) { m_drydep_no3 = value; }
    else if (StringMatch(sk, VAR_DRYDEP_NH4)) { m_drydep_nh4 = value; }
    else {
        throw ModelException(MID_ATMDEP, "SetValue",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
    }
}

void AtmosphericDeposition::Set2DData(const char *key, int nRows, int nCols, float **data) {
    CheckInputSize(key, nRows);
    string sk(key);
    m_nMaxSoiLayers = nCols;
    if (StringMatch(sk, VAR_SOL_NO3)) { m_sol_no3 = data; }
    else if (StringMatch(sk, VAR_SOL_NH4)) { m_sol_nh4 = data; }
    else {
        throw ModelException(MID_ATMDEP, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void AtmosphericDeposition:: InitialOutputs() {
    // allocate the output variables
    if (m_addrnh4 < 0.f) {
        m_addrnh4 = 0.f;
        m_addrno3 = 0.f;
    }
    /// initialize m_wshd_rno3 to 0.f at each time step
    if (!FloatEqual(m_wshd_rno3, 0.f)) m_wshd_rno3 = 0.f;
}

int AtmosphericDeposition::Execute() {
    //check the data
    CheckInputData();
     InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        if (m_preci[i] > 0.f) {
            /// Calculate the amount of nitrite and ammonia added to the soil in rainfall
            /// unit conversion: mg/L * mm = 0.01 * kg/ha (CHECKED)
            m_addrno3 = 0.01f * m_rcn * m_preci[i];
            m_addrnh4 = 0.01f * m_rca * m_preci[i];
            m_sol_no3[i][0] += (m_addrno3 + m_drydep_no3 / 365.f);
            m_sol_nh4[i][0] += (m_addrnh4 + m_drydep_nh4 / 365.f);
            m_wshd_rno3 += (m_addrno3 * (1.f / m_nCells));
        }
    }
    return 0;
}

void AtmosphericDeposition::GetValue(const char *key, float *value) {
    string sk(key);
    if (StringMatch(sk, VAR_WSHD_RNO3)) { *value = m_wshd_rno3; }
    else {
        throw ModelException(MID_ATMDEP, "GetValue", "Parameter " + sk + " does not exist.");
    }
}
