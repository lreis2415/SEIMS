#include "SUR_GR4J.h"
#include "text.h"

SUR_GR4J::SUR_GR4J() :
    m_nCells(-1), m_pNet(nullptr), m_topSoilThickness(nullptr),
    m_topSoilPorosity(nullptr),
    m_GR4J_X1(nullptr), m_pcpExcess(nullptr), m_infil(nullptr) {
}

SUR_GR4J::~SUR_GR4J() {
    Release1DArray(m_pNet);
    Release1DArray(m_topSoilThickness);
    Release1DArray(m_topSoilPorosity);
    Release1DArray(m_GR4J_X1);
}

void SUR_GR4J::SetValue(const char* key, float value) {
    string skey(key);
    if (StringMatch(skey, "TopSoilThickness")) { m_topSoilThickness[0] = value; }
    else if (StringMatch(skey, "TopSoilPorosity")) { m_topSoilPorosity[0] = value; }
    else {
        throw ModelException("SUR_GR4J", "SetValue", "Parameter " + skey
                             + " does not exist in current module. Please contact the module developer.");
    }
}

void SUR_GR4J::Set1DData(const char* key, int n, float* data) {
    CheckInputSize(key, n);
    string sk(key);

    if (StringMatch(sk, VAR_NEPR[0])) { m_pNet = data; }
    else if (StringMatch(sk, "TopSoilThickness")) { m_topSoilThickness = data; }
    else if (StringMatch(sk, "TopSoilPorosity")) { m_topSoilPorosity = data; }
    else {
        throw ModelException("SUR_GR4J", "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}


bool SUR_GR4J::CheckInputSize(const char* key, int n) {
    if (n <= 0) {
        throw ModelException("SUR_GR4J", "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException("SUR_GR4J", "CheckInputSize", "Input data for " + string(key) +
                                 " is invalid. All the input data should have same size.");
            return false;
        }
    }
    return true;
}

bool SUR_GR4J::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException("SUR_GR4J", "CheckInputData", "Input data is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_pNet == nullptr) {
        throw ModelException("SUR_GR4J", "CheckInputData", "Input data for " + string(VAR_NEPR[0]) + " is invalid.");
        return false;
    }
    if (m_topSoilThickness == nullptr) {
        throw ModelException("SUR_GR4J", "CheckInputData", "Input data for TopSoilThickness is invalid.");
        return false;
    }
    if (m_topSoilPorosity == nullptr) {
        throw ModelException("SUR_GR4J", "CheckInputData", "Input data for TopSoilPorosity is invalid.");
        return false;
    }
    return true;
}

void SUR_GR4J::InitialOutputs() {
    Initialize1DArray(m_nCells, m_GR4J_X1, 0.);
    Initialize1DArray(m_nCells, m_pNet, 0.);
    Initialize1DArray(m_nCells, m_infil, 0.);
}

int SUR_GR4J::Execute() {
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        // Calculate X1
        FLTPT x1 = m_topSoilThickness[i] * m_topSoilPorosity[i];
        m_GR4J_X1[i] = x1;

        FLTPT store = m_infil[i]; // infiltration to soil layer 0 is all stored
        FLTPT sat = store / x1; // saturation
        FLTPT tmp = tanh(m_pNet[i] / x1);
        FLTPT infil = x1 * (1.0 - (sat * sat)) * tmp / (1.0 + sat * tmp);
        FLTPT impF = 0; // m_impermeableFraction[i]
        infil = (1.0 - impF) * infil;

        m_infil[i] = infil;
        m_pcpExcess[i] = m_pNet[i] - infil;
        m_soilWtrSto[i][0] = m_infil[i];
    }


    return 0;
}


void SUR_GR4J::Get1DData(const char* key, int* n, float** data) {
    string sk(key);

    if (StringMatch(sk, VAR_SOL_ST[0])) { *data = m_infil; }
    else if (StringMatch(sk, VAR_EXCP[0])) { *data = m_pcpExcess; }
    else {
        throw ModelException("SUR_GR4J", "Get1DData",
                             "Result " + sk +
                             " does not exist in current module. Please contact the module developer.");
    }
    *n = m_nCells;
}
