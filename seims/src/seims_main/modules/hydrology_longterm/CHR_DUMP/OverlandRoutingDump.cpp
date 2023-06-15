#include "OverlandRoutingDump.h"
#include "text.h"


OverlandRoutingDump::OverlandRoutingDump():  
    m_nCells(-1), m_timeStep(-1), m_cellWidth(-1), m_cellArea(-1), m_isInitialized(false),
    m_subbasins(nullptr), m_cellsMappingToSubbasinId(nullptr),
    m_Q_SBOF(nullptr), m_Q_SB_ZEROS(nullptr){
}

OverlandRoutingDump::~OverlandRoutingDump() {
    Release1DArray(m_Q_SBOF);
    Release1DArray(m_Q_SB_ZEROS);
}

void OverlandRoutingDump::InitialOutputs() {
    
    if(m_isInitialized) return;
    Initialize1DArray(m_nSubbasins + 1, m_Q_SB_ZEROS, 0.);
    Initialize1DArray(m_nSubbasins + 1, m_Q_SBOF, 0.);

    if (m_cellArea <= 0.) m_cellArea = m_cellWidth * m_cellWidth;
    m_isInitialized = true;
}

bool OverlandRoutingDump::CheckInputData(void) {
    if (m_nCells <= 0) {
        throw ModelException(M_OLR_DUMP[0], "CheckInputData", "Input data is invalid. The size could not be less than zero.");
        return false;
    }
    return true;
}

void OverlandRoutingDump::SetValue(const char* key, int value) {
    string sk(key);
    if (StringMatch(sk, Tag_CellSize[0])) m_nCells = value;
    else if (StringMatch(sk, Tag_TimeStep[0])) m_timeStep = value;
    else if (StringMatch(sk, VAR_SUBBSNID_NUM[0])) m_nSubbasins = value;
    else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbasinId = value;
    else {
        throw ModelException(M_OLR_DUMP[0], "SetValue",
                             "Integer Parameter " + sk + " does not exist.");
    }
}
void OverlandRoutingDump::SetValue(const char* key, FLTPT value) {
     string sk(key);
     if (StringMatch(sk, Tag_CellWidth[0])) m_cellWidth = value;
     else {
         throw ModelException(M_OLR_DUMP[0], "SetValue",
                              "Integer Parameter " + sk + " does not exist.");
     }
}

void OverlandRoutingDump::Set1DData(const char* key, int n, int* data) {
    string sk(key);

    if (StringMatch(sk, VAR_SUBBSN[0])) m_cellsMappingToSubbasinId = data;
    else {
        throw ModelException(M_OLR_DUMP[0], "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}

void OverlandRoutingDump::Set1DData(const char* key, const int n, FLTPT* data){
    string sk(key);
    if (StringMatch(sk, VAR_SURU[0])) m_surfaceRunoff = data;
    else {
        throw ModelException(M_OLR_DUMP[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}
void OverlandRoutingDump::Get1DData(const char *key, int *nRows, FLTPT **data) {
    string s(key);
    if (StringMatch(s, VAR_SBOF[0])) {*data = m_Q_SBOF;} 
    else if (StringMatch(s, VAR_SBIF[0])) { *data = m_Q_SB_ZEROS; }
    else if (StringMatch(s, VAR_SBQG[0])) { *data = m_Q_SB_ZEROS; }
    else {
        throw ModelException(M_OLR_DUMP[0], "getResult", "Result " + s +
            " does not exist in OverlandRoutingDump method. Please contact the module developer.");
    }
    *nRows = m_nSubbasins + 1;
}

void OverlandRoutingDump::SetSubbasins(clsSubbasins* subbsns) {
    if (m_subbasins == nullptr) {
        m_subbasins = subbsns;
        m_subbasinIDs = m_subbasins->GetSubbasinIDs();
    }
}

//Execute module
int OverlandRoutingDump::Execute() {
    InitialOutputs();
    CheckInputData();
    for (int n = 0; n <= m_nSubbasins; n++) {
        m_Q_SBOF[n] = 0;
    }
//#pragma omp parallel
    {
        FLTPT* tmp_qsSub = new FLTPT[m_nSubbasins + 1];
        for (int i = 0; i <= m_nSubbasins; i++) {
            tmp_qsSub[i] = 0.;
        }
//#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            tmp_qsSub[CVT_INT(m_cellsMappingToSubbasinId[i])] += m_surfaceRunoff[i];
            m_surfaceRunoff[i] = 0.0;
        }
//#pragma omp critical
        {
            for (int n = 1; n <= m_nSubbasins; n++) {
                m_Q_SBOF[n] = tmp_qsSub[n];
                m_Q_SBOF[n] = m_Q_SBOF[n] * 0.001 * m_cellArea / m_timeStep;
            }
        }
        delete[] tmp_qsSub;
        tmp_qsSub = nullptr;
    } /* END of //#pragma omp parallel */

    for (int n = 1; n <= m_nSubbasins; n++) {
        //get overland flow routing for entire watershed.
        m_Q_SBOF[0] += m_Q_SBOF[n];
    }

    printf("\n[OverlandRoutingDump] m_Q_SBOF[0]=%f->%f\n",
        m_Q_SBOF[0] / 0.001 / m_cellArea * m_timeStep,
        m_Q_SBOF[0]);
    fflush(stdout);
    return 0;
}
