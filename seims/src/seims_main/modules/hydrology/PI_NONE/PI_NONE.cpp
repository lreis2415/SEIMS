#include "text.h"
#include "PI_NONE.h"

#include "utils_time.h"

PI_NONE::PI_NONE() :
m_nCells(-1),m_pcp(nullptr),m_snowfall(nullptr),
m_netPcp(nullptr), m_snowAcc(nullptr)
{
    SetModuleName(M_PI_NONE[0]);
}

PI_NONE::~PI_NONE() {
    Release1DArray(m_pcp);
    Release1DArray(m_snowfall);
}

void PI_NONE::Set1DData(const char* key, int nrows, FLTPT* data) {
    CheckInputSize(key, nrows, m_nCells);
    string s(key);
    if (StringMatch(s, VAR_PCP[0])) m_pcp = data;
    else if (StringMatch(s, VAR_SNOWFALL[0])) m_snowfall = data;
    else {
        throw ModelException(GetModuleName(), "Set1DData",
                             "Parameter " + s + " does not exist.");
    }
}


void PI_NONE::Get1DData(const char* key, int* nRows, FLTPT** data) {
    InitialOutputs();
    string s = key;
    if (StringMatch(s, VAR_NEPR[0])) {
        *data = m_netPcp;
    } else if (StringMatch(s, VAR_SNAC[0])) {
        *data = m_snowAcc;
    }
    else {
        throw ModelException(GetModuleName(), "Get1DData",
                             "Result " + s + " does not exist.");
    }
    *nRows = m_nCells;
}

void PI_NONE::InitialOutputs() {
    Initialize1DArray(m_nCells, m_netPcp, 0.);
    Initialize1DArray(m_nCells, m_snowAcc, 0.);
}

int PI_NONE::Execute() {
    CheckInputData();
    InitialOutputs();
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_netPcp[i] += m_pcp[i];
        m_snowAcc[i] += m_snowfall[i];
    }
    return 0;
}

bool PI_NONE::CheckInputData() {
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    CHECK_POINTER(GetModuleName(), m_pcp);
    CHECK_POINTER(GetModuleName(), m_snowfall);
    return true;
}
