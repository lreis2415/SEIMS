#include "SET_LM.h"

#include "text.h"

SET_LM::SET_LM() : m_nCells(-1), m_frozenT(NODATA_VALUE),
                   m_fc(nullptr), m_wp(nullptr), m_EI(nullptr), m_PET(nullptr), m_ED(nullptr), m_plantET(nullptr),
                   m_sm(nullptr), m_soilT(nullptr), m_soilLayers(nullptr), m_soilThick(nullptr),
                   m_soilET(nullptr){
}

SET_LM::~SET_LM() {
    Release1DArray(m_soilET);
}

int SET_LM::Execute() {
    CheckInputData();
     InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_soilET[i] = 0.0f;
        if (m_soilT[i] <= m_frozenT) { continue; }

        float etDeficiency = m_PET[i] - m_EI[i] - m_ED[i] - m_plantET[i];
        if (etDeficiency <= 0.f) continue;
        for (int j = 0; j < (int) m_soilLayers[i]; j++) {
            float et2d = 0.f;
            if (m_sm[i][j] >= m_fc[i][j]) { et2d = etDeficiency; }
            else if (m_sm[i][j] >= m_wp[i][j]) {
                et2d = etDeficiency * (m_sm[i][j] - m_wp[i][j]) / (m_fc[i][j] - m_wp[i][j]);
            } else { et2d = 0.0f; }

            float sm0 = m_sm[i][j];
            float availableWater = (m_sm[i][j] - m_wp[i][j]) * m_soilThick[i][j];
            if (et2d > availableWater) {
                et2d = availableWater;
                m_sm[i][j] = m_wp[i][j];
            } else {
                m_sm[i][j] -= et2d / m_soilThick[i][j];
            }

            if (m_sm[i][j] < 0.f) {
                cout << "SET_LM\t" << sm0 << "\t" << m_wp[i][j] << "\t" << m_sm[i][j] << "\t"
                     << availableWater / m_soilThick[i][j] << "\t" << et2d << endl;
                throw ModelException(MID_SET_LM, "Execute", "moisture is less than zero.");
            }

            etDeficiency -= et2d;
            m_soilET[i] += et2d;
        }
    }
    return 0;
}

void SET_LM::Get1DData(const char *key, int *nRows, float **data) {
     InitialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SOET)) { *data = m_soilET; }
    else {
        throw ModelException(MID_SET_LM, "Get1DData", "Result " + s + " does not exist.");
    }
    *nRows = m_nCells;
}

void SET_LM::SetValue(const char *key, float data) {
    string s(key);
    if (StringMatch(s, VAR_T_SOIL)) { m_frozenT = data; }
    else {
        throw ModelException(MID_SET_LM, "SetValue", "Parameter " + s + " does not exist.");
    }
}

void SET_LM::Set1DData(const char *key, int nRows, float *data) {
    string s(key);
    CheckInputSize(key, nRows);
    if (StringMatch(s, VAR_SOILLAYERS)) { m_soilLayers = data; }
    else if (StringMatch(s, VAR_INET)) { m_EI = data; }
    else if (StringMatch(s, VAR_PET)) { m_PET = data; }
    else if (StringMatch(s, VAR_DEET)) { m_ED = data; }
    else if (StringMatch(s, VAR_PPT)) { m_plantET = data; }
    else if (StringMatch(s, VAR_SOTE)) { m_soilT = data; }
    else {
        throw ModelException(MID_SET_LM, "SetValue", "Parameter " + s + " does not exist.");
    }
}

void SET_LM::Set2DData(const char *key, int nrows, int ncols, float **data) {
    string sk(key);
    CheckInputSize(key, nrows);
    if (StringMatch(sk, VAR_FIELDCAP)) { m_fc = data; }
    else if (StringMatch(sk, VAR_WILTPOINT)) { m_wp = data; }
    else if (StringMatch(sk, VAR_SOL_ST)) { m_sm = data; }
    else if (StringMatch(sk, VAR_SOILTHICK)) { m_soilThick = data; }
    else {
        throw ModelException(MID_SET_LM, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

bool SET_LM::CheckInputData() {
    CHECK_POSITIVE(MID_SET_LM, m_nCells);
    CHECK_POINTER(MID_SET_LM, m_fc);
    CHECK_POINTER(MID_SET_LM, m_wp);
    CHECK_POINTER(MID_SET_LM, m_EI);
    CHECK_POINTER(MID_SET_LM, m_PET);
    CHECK_POINTER(MID_SET_LM, m_ED);
    CHECK_POINTER(MID_SET_LM, m_plantET);
    CHECK_POINTER(MID_SET_LM, m_sm);
    CHECK_POINTER(MID_SET_LM, m_soilT);
    CHECK_DATA(MID_SET_LM, FloatEqual(m_frozenT, NODATA_VALUE), "The threshold soil freezing temperature has not been set.");
    return true;
}

bool SET_LM::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_SET_LM, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(MID_SET_LM, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}

void SET_LM:: InitialOutputs() {
    CHECK_POSITIVE(MID_SET_LM, m_nCells);
    if (nullptr == m_soilET) { Initialize1DArray(m_nCells, m_soilET, 0.f); }
}
