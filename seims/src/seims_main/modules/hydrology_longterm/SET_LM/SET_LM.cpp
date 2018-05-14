#include "SET_LM.h"

#include "text.h"

SET_LM::SET_LM() : m_nCells(-1), m_nSoilLyrs(nullptr),
                   m_soilThk(nullptr), m_soilWtrSto(nullptr), m_soilFC(nullptr), m_soilWP(nullptr),
                   m_pet(nullptr), m_IntcpET(nullptr),
                   m_deprStoET(nullptr), m_maxPltET(nullptr), m_soilTemp(nullptr),
                   m_soilFrozenTemp(NODATA_VALUE),
                   m_soilET(nullptr) {
}

SET_LM::~SET_LM() {
    if (m_soilET != nullptr) Release1DArray(m_soilET);
}

int SET_LM::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_soilET[i] = 0.0f;
        if (m_soilTemp[i] <= m_soilFrozenTemp) { continue; }

        float etDeficiency = m_pet[i] - m_IntcpET[i] - m_deprStoET[i] - m_maxPltET[i];
        if (etDeficiency <= 0.f) continue;
        for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
            float et2d = 0.f;
            if (m_soilWtrSto[i][j] >= m_soilFC[i][j]) { et2d = etDeficiency; } else if (m_soilWtrSto[i][j] >= m_soilWP[i
            ][j]) {
                et2d = etDeficiency * (m_soilWtrSto[i][j] - m_soilWP[i][j]) / (m_soilFC[i][j] - m_soilWP[i][j]);
            } else { et2d = 0.0f; }

            float sm0 = m_soilWtrSto[i][j];
            float availableWater = (m_soilWtrSto[i][j] - m_soilWP[i][j]) * m_soilThk[i][j];
            if (et2d > availableWater) {
                et2d = availableWater;
                m_soilWtrSto[i][j] = m_soilWP[i][j];
            } else {
                m_soilWtrSto[i][j] -= et2d / m_soilThk[i][j];
            }

            if (m_soilWtrSto[i][j] < 0.f) {
                cout << "SET_LM\t" << sm0 << "\t" << m_soilWP[i][j] << "\t" << m_soilWtrSto[i][j] << "\t"
                        << availableWater / m_soilThk[i][j] << "\t" << et2d << endl;
                throw ModelException(MID_SET_LM, "Execute", "moisture is less than zero.");
            }

            etDeficiency -= et2d;
            m_soilET[i] += et2d;
        }
    }
    return 0;
}

void SET_LM::Get1DData(const char* key, int* nRows, float** data) {
    InitialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SOET)) *data = m_soilET;
    else {
        throw ModelException(MID_SET_LM, "Get1DData", "Result " + s + " does not exist.");
    }
    *nRows = m_nCells;
}

void SET_LM::SetValue(const char* key, const float value) {
    string s(key);
    if (StringMatch(s, VAR_T_SOIL)) m_soilFrozenTemp = value;
    else {
        throw ModelException(MID_SET_LM, "SetValue", "Parameter " + s + " does not exist.");
    }
}

void SET_LM::Set1DData(const char* key, const int nRows, float* data) {
    string s(key);
    CheckInputSize(key, nRows);
    if (StringMatch(s, VAR_SOILLAYERS)) m_nSoilLyrs = data;
    else if (StringMatch(s, VAR_INET)) m_IntcpET = data;
    else if (StringMatch(s, VAR_PET)) m_pet = data;
    else if (StringMatch(s, VAR_DEET)) m_deprStoET = data;
    else if (StringMatch(s, VAR_PPT)) m_maxPltET = data;
    else if (StringMatch(s, VAR_SOTE)) m_soilTemp = data;
    else {
        throw ModelException(MID_SET_LM, "Set1DData", "Parameter " + s + " does not exist.");
    }
}

void SET_LM::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    string sk(key);
    CheckInputSize(key, nrows);
    if (StringMatch(sk, VAR_SOL_AWC)) m_soilFC = data;
    else if (StringMatch(sk, VAR_SOL_WPMM)) m_soilWP = data;
    else if (StringMatch(sk, VAR_SOL_ST)) m_soilWtrSto = data;
    else if (StringMatch(sk, VAR_SOILTHICK)) m_soilThk = data;
    else {
        throw ModelException(MID_SET_LM, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

bool SET_LM::CheckInputData() {
    CHECK_POSITIVE(MID_SET_LM, m_nCells);
    CHECK_POINTER(MID_SET_LM, m_soilFC);
    CHECK_POINTER(MID_SET_LM, m_soilWP);
    CHECK_POINTER(MID_SET_LM, m_IntcpET);
    CHECK_POINTER(MID_SET_LM, m_pet);
    CHECK_POINTER(MID_SET_LM, m_deprStoET);
    CHECK_POINTER(MID_SET_LM, m_maxPltET);
    CHECK_POINTER(MID_SET_LM, m_soilWtrSto);
    CHECK_POINTER(MID_SET_LM, m_soilTemp);
    CHECK_NODATA(MID_SET_LM, m_soilFrozenTemp);
    return true;
}

bool SET_LM::CheckInputSize(const char* key, const int n) {
    if (n <= 0) {
        throw ModelException(MID_SET_LM, "CheckInputSize", "Input data for " + string(key) +
                             " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) {
            m_nCells = n;
        } else {
            throw ModelException(MID_SET_LM, "CheckInputSize", "Input data for " + string(key) +
                                 " is invalid. All the input data should have same size.");
        }
    }
    return true;
}

void SET_LM::InitialOutputs() {
    CHECK_POSITIVE(MID_SET_LM, m_nCells);
    if (nullptr == m_soilET) Initialize1DArray(m_nCells, m_soilET, 0.f);
}
