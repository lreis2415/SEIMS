#include "SurfaceRunoffDump.h"
#include "text.h"

SurfaceRunoffDump::SurfaceRunoffDump():
    m_nCells(-1),
    //infiltration
    m_pcp(nullptr), m_surfaceRunoff(nullptr){
}

void SurfaceRunoffDump::InitialOutputs() {
    Initialize1DArray(m_nCells, m_surfaceRunoff, 0.);
}

SurfaceRunoffDump::~SurfaceRunoffDump() {
    Release1DArray(m_surfaceRunoff);
}

void SurfaceRunoffDump::SetValue(const char* key, int value) {
    string sk(key);
    if (StringMatch(sk, Tag_CellSize[0])) m_nCells = value;
    else {
        throw ModelException(M_SUR_DUMP[0], "SetValue",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void SurfaceRunoffDump::Set1DData(const char* key, int n, FLTPT* data) {
    CheckInputSize(key, n);
    string sk(key);

    if (StringMatch(sk, VAR_PCP[0])) { m_pcp = data; }
    else {
        throw ModelException(M_SUR_DUMP[0], "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

bool SurfaceRunoffDump::CheckInputSize(const char* key, int n) {
    if (n <= 0) {
        throw ModelException(M_SUR_DUMP[0], "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(M_SUR_DUMP[0], "CheckInputSize", "Input data for " + string(key) +
                                 " is invalid. All the input data should have same size.");
            return false;
        }
    }
    return true;
}

bool SurfaceRunoffDump::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException(M_SUR_DUMP[0], "CheckInputData", "Input data is invalid. The size could not be less than zero.");
        return false;
    }
    CHECK_POINTER(M_SUR_DUMP[0], m_pcp);
    return true;
}

void SurfaceRunoffDump::Get1DData(const char* key, int* n, FLTPT** data) {
    string sk(key);
    if (StringMatch(sk, VAR_SURU[0])) { *data = m_surfaceRunoff; }
    else {
        throw ModelException(M_SUR_DUMP[0], "Get1DData",
                             "Result " + sk +
                             " does not exist in current module. Please contact the module developer.");
    }
    *n = m_nCells;
}

int SurfaceRunoffDump::Execute() {
    InitialOutputs();
    CheckInputData();
    
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_surfaceRunoff[i] = m_pcp[i];
    }

    FLTPT sum = 0;
    for (int i = 0; i < m_nCells; i++) {
        sum += m_surfaceRunoff[i];
    }
    printf("\n[SurfaceRunoffDump] m_surfaceRunoff=%f\n", sum);
    fflush(stdout);

    return 0;
}
