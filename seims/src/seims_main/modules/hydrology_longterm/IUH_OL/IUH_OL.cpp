#include "IUH_OL.h"

#include "text.h"

IUH_OL::IUH_OL() :
    m_TimeStep(-1), m_nCells(-1), m_cellArea(nullptr),
    m_nSubbsns(-1), m_inputSubbsnID(-1), m_subbsnID(nullptr),
    m_iuhCell(nullptr), m_iuhCols(-1), m_surfRf(nullptr),
    m_cellFlow(nullptr), m_cellFlowCols(-1), m_Q_SBOF(nullptr), m_OL_Flow(nullptr) {
}

IUH_OL::~IUH_OL() {
    if (m_Q_SBOF != nullptr) Release1DArray(m_Q_SBOF);
    if (m_cellFlow != nullptr) Release2DArray(m_cellFlow);
    if (m_OL_Flow != nullptr) Release1DArray(m_OL_Flow);
}

bool IUH_OL::CheckInputData() {
    CHECK_POSITIVE(M_IUH_OL[0], m_date);
    CHECK_POSITIVE(M_IUH_OL[0], m_nSubbsns);
    CHECK_NONNEGATIVE(M_IUH_OL[0], m_inputSubbsnID);
    CHECK_POSITIVE(M_IUH_OL[0], m_nCells);
    CHECK_POINTER(M_IUH_OL[0], m_cellArea);
    CHECK_NONNEGATIVE(M_IUH_OL[0], m_TimeStep);
    CHECK_POINTER(M_IUH_OL[0], m_subbsnID);
    CHECK_POINTER(M_IUH_OL[0], m_iuhCell);
    CHECK_POINTER(M_IUH_OL[0], m_surfRf);
    return true;
}

void IUH_OL::InitialOutputs() {
    CHECK_POSITIVE(M_IUH_OL[0], m_nSubbsns);
    
    if (nullptr == m_Q_SBOF) {
        Initialize1DArray(m_nSubbsns + 1, m_Q_SBOF, 0.);
        for (int i = 0; i < m_nCells; i++) {
            m_cellFlowCols = Max(CVT_INT(m_iuhCell[i][1]) + 1, m_cellFlowCols);
        }
        //get m_cellFlowCols, i.e. the maximum of second column of OL_IUH plus 1.
        Initialize2DArray(m_nCells, m_cellFlowCols, m_cellFlow, 0.);
    }
    if (nullptr == m_OL_Flow) {
        Initialize1DArray(m_nCells, m_OL_Flow, 0.);
    }
}

int IUH_OL::Execute() {
    CheckInputData();
    InitialOutputs();
    // delete value of last time step
    for (int n = 0; n <= m_nSubbsns; n++) {
        m_Q_SBOF[n] = 0.;
    }
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        //forward one time step
        for (int j = 0; j < m_cellFlowCols - 1; j++) {
            m_cellFlow[i][j] = m_cellFlow[i][j + 1];
        }
        m_cellFlow[i][m_cellFlowCols - 1] = 0.;

        if (m_surfRf[i] <= 0.) continue;

        int min = CVT_INT(m_iuhCell[i][0]);
        int max = CVT_INT(m_iuhCell[i][1]);
        int col = 2;
        for (int k = min; k <= max; k++) {
            m_cellFlow[i][k] += m_surfRf[i] * 0.001 * m_iuhCell[i][col] * m_cellArea[i];
            col++;
        }
    }
    // See https://github.com/lreis2415/SEIMS/issues/36 for more descriptions. By lj
#pragma omp parallel
    {
        FLTPT* tmp_qsSub = new FLTPT[m_nSubbsns + 1];
        for (int i = 0; i <= m_nSubbsns; i++) {
            tmp_qsSub[i] = 0.;
        }
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            tmp_qsSub[CVT_INT(m_subbsnID[i])] += m_cellFlow[i][0]; //get new value
            m_OL_Flow[i] = m_cellFlow[i][0];
            m_OL_Flow[i] = m_OL_Flow[i] * m_TimeStep * 1000. / m_cellArea[i]; // m3/s -> mm
        }
#pragma omp critical
        {
            for (int i = 1; i <= m_nSubbsns; i++) {
                m_Q_SBOF[i] += tmp_qsSub[i];
            }
        }
        delete[] tmp_qsSub;
        tmp_qsSub = nullptr;
    } /* END of #pragma omp parallel */

    for (int n = 1; n <= m_nSubbsns; n++) {
        //get overland flow routing for entire watershed.
        m_Q_SBOF[0] += m_Q_SBOF[n];
    }
    return 0;
}


void IUH_OL::SetValue(const char* key, const int value) {
    string sk(key);
    if (StringMatch(sk, Tag_TimeStep[0])) m_TimeStep = value;
    else if (StringMatch(sk, Tag_CellSize[0])) m_nCells = value;
    else if (StringMatch(sk, VAR_SUBBSNID_NUM[0])) m_nSubbsns = value;
    else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbsnID = value;
    else {
        throw ModelException(M_IUH_OL[0], "SetValue",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void IUH_OL::Set1DData(const char* key, const int n, FLTPT* data) {
    CheckInputSize(M_IUH_OL[0], key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_SURU[0])) {
        m_surfRf = data;
    } else if (StringMatch(sk, VAR_CELL_AREA[0])) {
        m_cellArea = data;
    } else {
        throw ModelException(M_IUH_OL[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void IUH_OL::Set1DData(const char* key, const int n, int* data) {
    CheckInputSize(M_IUH_OL[0], key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN[0])) m_subbsnID = data;
    else {
        throw ModelException(M_IUH_OL[0], "Set1DData",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void IUH_OL::Set2DData(const char* key, const int nrows, const int ncols, FLTPT** data) {
    string sk(key);
    if (StringMatch(sk, VAR_OL_IUH[0])) {
        CheckInputSize2D(M_IUH_OL[0], VAR_OL_IUH[0], nrows, ncols, m_nCells, m_iuhCols);
        m_iuhCell = data;
        m_iuhCols = ncols;
    } else {
        throw ModelException(M_IUH_OL[0], "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void IUH_OL::GetValue(const char* key, FLTPT* value) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SBOF[0]) && m_inputSubbsnID > 0) {
        /// For MPI version to transfer data across subbasins
        *value = m_Q_SBOF[m_inputSubbsnID];
    } else {
        throw ModelException(M_IUH_OL[0], "GetValue", "Result " + sk + " does not exist.");
    }
}

void IUH_OL::Get1DData(const char* key, int* n, FLTPT** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SBOF[0])) {
        *data = m_Q_SBOF;
        *n = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_OLFLOW[0])) {
        *data = m_OL_Flow;
        *n = m_nCells;
    } else {
        throw ModelException(M_IUH_OL[0], "Get1DData", "Result " + sk + " does not exist.");
    }
}
