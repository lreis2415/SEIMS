#include "IUH_OL.h"

#include "text.h"

IUH_OL::IUH_OL() :
    m_TimeStep(-1), m_nCells(-1), m_CellWth(NODATA_VALUE), m_cellArea(NODATA_VALUE),
    m_nSubbsns(-1), m_inputSubbsnID(-1), m_subbsnID(nullptr),
    m_iuhCell(nullptr), m_iuhCols(-1), m_surfRf(nullptr),
    m_cellFlow(nullptr), m_cellFlowCols(-1), m_Q_SBOF(nullptr), m_OL_Flow(nullptr) {
}

IUH_OL::~IUH_OL() {
    if (m_Q_SBOF != nullptr) Release1DArray(m_Q_SBOF);
    if (m_cellFlow != nullptr) Release2DArray(m_nCells, m_cellFlow);
    if (m_OL_Flow != nullptr) Release1DArray(m_OL_Flow);
}

bool IUH_OL::CheckInputData() {
    CHECK_POSITIVE(MID_IUH_OL, m_date);
    CHECK_POSITIVE(MID_IUH_OL, m_nSubbsns);
    CHECK_NONNEGATIVE(MID_IUH_OL, m_inputSubbsnID);
    CHECK_POSITIVE(MID_IUH_OL, m_nCells);
    CHECK_POSITIVE(MID_IUH_OL, m_CellWth);
    CHECK_NONNEGATIVE(MID_IUH_OL, m_TimeStep);
    CHECK_POINTER(MID_IUH_OL, m_subbsnID);
    CHECK_POINTER(MID_IUH_OL, m_iuhCell);
    CHECK_POINTER(MID_IUH_OL, m_surfRf);
    return true;
}

void IUH_OL::InitialOutputs() {
    CHECK_POSITIVE(MID_IUH_OL, m_nSubbsns);

    if (m_cellArea <= 0.f) m_cellArea = m_CellWth * m_CellWth;
    if (nullptr == m_Q_SBOF) {
        Initialize1DArray(m_nSubbsns + 1, m_Q_SBOF, 0.f);
        for (int i = 0; i < m_nCells; i++) {
            m_cellFlowCols = Max(CVT_INT(m_iuhCell[i][1]) + 1, m_cellFlowCols);
        }
        //get m_cellFlowCols, i.e. the maximum of second column of OL_IUH plus 1.
        Initialize2DArray(m_nCells, m_cellFlowCols, m_cellFlow, 0.f);
    }
    if (nullptr == m_OL_Flow) {
        Initialize1DArray(m_nCells, m_OL_Flow, 0.f);
    }
}

int IUH_OL::Execute() {
    CheckInputData();
    InitialOutputs();
    // delete value of last time step
    for (int n = 0; n <= m_nSubbsns; n++) {
        m_Q_SBOF[n] = 0.f;
    }
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        //forward one time step
        for (int j = 0; j < m_cellFlowCols - 1; j++) {
            m_cellFlow[i][j] = m_cellFlow[i][j + 1];
        }
        m_cellFlow[i][m_cellFlowCols - 1] = 0.f;

        if (m_surfRf[i] <= 0.f) continue;

        int min = CVT_INT(m_iuhCell[i][0]);
        int max = CVT_INT(m_iuhCell[i][1]);
        int col = 2;
        for (int k = min; k <= max; k++) {
            m_cellFlow[i][k] += m_surfRf[i] * 0.001f * m_iuhCell[i][col] * m_cellArea / m_TimeStep;
            col++;
        }
    }
    // See https://github.com/lreis2415/SEIMS/issues/36 for more descriptions. By lj
#pragma omp parallel
    {
        float* tmp_qsSub = new float[m_nSubbsns + 1];
        for (int i = 0; i <= m_nSubbsns; i++) {
            tmp_qsSub[i] = 0.f;
        }
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            tmp_qsSub[CVT_INT(m_subbsnID[i])] += m_cellFlow[i][0]; //get new value
            m_OL_Flow[i] = m_cellFlow[i][0];
            m_OL_Flow[i] = m_OL_Flow[i] * m_TimeStep * 1000.f / m_cellArea; // m3/s -> mm
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

void IUH_OL::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, Tag_TimeStep)) m_TimeStep = CVT_INT(value);
    else if (StringMatch(sk, Tag_CellSize)) m_nCells = CVT_INT(value);
    else if (StringMatch(sk, Tag_CellWidth)) m_CellWth = value;
    else if (StringMatch(sk, VAR_SUBBSNID_NUM)) m_nSubbsns = CVT_INT(value);
    else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbsnID = CVT_INT(value);
    else {
        throw ModelException(MID_IUH_OL, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void IUH_OL::Set1DData(const char* key, const int n, float* data) {
    CheckInputSize(MID_IUH_OL, key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN)) m_subbsnID = data;
    else if (StringMatch(sk, VAR_SURU)) m_surfRf = data;
    else {
        throw ModelException(MID_IUH_OL, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void IUH_OL::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    string sk(key);
    if (StringMatch(sk, VAR_OL_IUH)) {
        CheckInputSize2D(MID_IUH_OL, VAR_OL_IUH, nrows, ncols, m_nCells, m_iuhCols);
        m_iuhCell = data;
        m_iuhCols = ncols;
    } else {
        throw ModelException(MID_IUH_OL, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void IUH_OL::GetValue(const char* key, float* value) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SBOF) && m_inputSubbsnID > 0) {
        /// For MPI version to transfer data across subbasins
        *value = m_Q_SBOF[m_inputSubbsnID];
    } else {
        throw ModelException(MID_IUH_OL, "GetValue", "Result " + sk + " does not exist.");
    }
}

void IUH_OL::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SBOF)) {
        *data = m_Q_SBOF;
        *n = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_OLFLOW)) {
        *data = m_OL_Flow;
        *n = m_nCells;
    } else {
        throw ModelException(MID_IUH_OL, "Get1DData", "Result " + sk + " does not exist.");
    }
}
