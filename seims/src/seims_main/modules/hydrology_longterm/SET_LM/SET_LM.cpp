#include "SET_LM.h"

#include "text.h"

SET_LM::SET_LM() :
    m_nCells(-1), m_maxSoilLyrs(-1), m_nSoilLyrs(nullptr),
    m_soilThk(nullptr), m_soilWtrSto(nullptr), m_soilFC(nullptr),
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
    int errCount = 0;
#pragma omp parallel for reduction(+: errCount)
    for (int i = 0; i < m_nCells; i++) {
        m_soilET[i] = 0.;
        if (m_soilTemp[i] <= m_soilFrozenTemp) { continue; }

        FLTPT etDeficiency = m_pet[i] - m_IntcpET[i] - m_deprStoET[i] - m_maxPltET[i];
        for (int j = 0; j < CVT_INT(m_nSoilLyrs[i]); j++) {
            if (etDeficiency <= 0.) break;
            FLTPT et2d = 0.;
            if (m_soilWtrSto[i][j] >= m_soilFC[i][j]) {
                et2d = etDeficiency;
            } else if (m_soilWtrSto[i][j] >= 0.) {
                et2d = etDeficiency * m_soilWtrSto[i][j] / m_soilFC[i][j];
            } else {
                et2d = 0.;
            }
            if (et2d > m_soilWtrSto[i][j]) {
                et2d = m_soilWtrSto[i][j];
                m_soilWtrSto[i][j] = 0.;
            } else {
                m_soilWtrSto[i][j] -= et2d;
            }
            if (isinf(m_soilWtrSto[i][j]) || isnan(m_soilWtrSto[i][j]) || m_soilWtrSto[i][j] < 0.) {
                cout << "SET_LM: moisture is less than zero" << m_soilWtrSto[i][j] << "\t" << et2d << endl;
                errCount++;
            }
            etDeficiency -= et2d;
            m_soilET[i] += et2d;
        }
    }
    if (errCount > 0) {
        throw ModelException(M_SET_LM[0], "Execute",
                             "Soil moisture can not less than zero!");
    }
    return 0;
}

void SET_LM::Get1DData(const char* key, int* nRows, FLTPT** data) {
    InitialOutputs();
    string s(key);
    if (StringMatch(s, VAR_SOET[0])) *data = m_soilET;
    else {
        throw ModelException(M_SET_LM[0], "Get1DData",
                             "Result " + s + " does not exist.");
    }
    *nRows = m_nCells;
}

void SET_LM::SetValue(const char* key, const FLTPT value) {
    string s(key);
    if (StringMatch(s, VAR_T_SOIL[0])) m_soilFrozenTemp = value;
    else {
        throw ModelException(M_SET_LM[0], "SetValue",
                             "Parameter " + s + " does not exist.");
    }
}

void SET_LM::Set1DData(const char* key, const int nrows, FLTPT* data) {
    string s(key);
    CheckInputSize(M_SET_LM[0], key, nrows, m_nCells);
    if (StringMatch(s, VAR_INET[0])) m_IntcpET = data;
    else if (StringMatch(s, VAR_PET[0])) m_pet = data;
    else if (StringMatch(s, VAR_DEET[0])) m_deprStoET = data;
    else if (StringMatch(s, VAR_PPT[0])) m_maxPltET = data;
    else if (StringMatch(s, VAR_SOTE[0])) m_soilTemp = data;
    else {
        throw ModelException(M_SET_LM[0], "Set1DData",
                             "Parameter " + s + " does not exist.");
    }
}

void SET_LM::Set1DData(const char* key, const int nrows, int* data) {
    string s(key);
    CheckInputSize(M_SET_LM[0], key, nrows, m_nCells);
    if (StringMatch(s, VAR_SOILLAYERS[0])) m_nSoilLyrs = data;
    else {
        throw ModelException(M_SET_LM[0], "Set1DData",
                             "Integer Parameter " + s + " does not exist.");
    }
}
void SET_LM::Set2DData(const char* key, const int nrows, const int ncols, FLTPT** data) {
    string sk(key);
    CheckInputSize2D(M_SET_LM[0], key, nrows, ncols, m_nCells, m_maxSoilLyrs);
    if (StringMatch(sk, VAR_SOL_AWC_AMOUNT[0])) m_soilFC = data;
    else if (StringMatch(sk, VAR_SOL_ST[0])) m_soilWtrSto = data;
    else if (StringMatch(sk, VAR_SOILTHICK[0])) m_soilThk = data;
    else {
        throw ModelException(M_SET_LM[0], "Set2DData",
                             "Parameter " + sk + " does not exist.");
    }
}

bool SET_LM::CheckInputData() {
    CHECK_POSITIVE(M_SET_LM[0], m_nCells);
    CHECK_POINTER(M_SET_LM[0], m_soilFC);
    CHECK_POINTER(M_SET_LM[0], m_IntcpET);
    CHECK_POINTER(M_SET_LM[0], m_pet);
    CHECK_POINTER(M_SET_LM[0], m_deprStoET);
    CHECK_POINTER(M_SET_LM[0], m_maxPltET);
    CHECK_POINTER(M_SET_LM[0], m_soilWtrSto);
    CHECK_POINTER(M_SET_LM[0], m_soilTemp);
    CHECK_NODATA(M_SET_LM[0], m_soilFrozenTemp);
    return true;
}

void SET_LM::InitialOutputs() {
    CHECK_POSITIVE(M_SET_LM[0], m_nCells);
    if (nullptr == m_soilET) Initialize1DArray(m_nCells, m_soilET, 0.);
}
