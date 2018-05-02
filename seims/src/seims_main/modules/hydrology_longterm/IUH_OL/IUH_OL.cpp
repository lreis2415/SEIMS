#include "seims.h"
#include "IUH_OL.h"

using namespace std;

IUH_OL::IUH_OL() : m_TimeStep(-1), m_nCells(-1), m_CellWidth(NODATA_VALUE), m_cellArea(NODATA_VALUE),
                   m_nSubbasins(-1), m_subbasinID(-1), m_subbasin(nullptr),
                   m_iuhCell(nullptr), m_iuhCols(-1), m_rs(nullptr),
                   m_cellFlowCols(-1), m_cellFlow(nullptr), m_OL_Flow(nullptr), m_Q_SBOF(nullptr) {
}

IUH_OL::~IUH_OL() {
    Release1DArray(m_Q_SBOF);
    Release2DArray(m_nCells, m_cellFlow);
    Release1DArray(m_OL_Flow);
}

bool IUH_OL::CheckInputData() {
    CHECK_POSITIVE(MID_IUH_OL, m_date);
    CHECK_POSITIVE(MID_IUH_OL, m_nSubbasins);
    CHECK_NONNEGATIVE(MID_IUH_OL, m_subbasinID);
    CHECK_POSITIVE(MID_IUH_OL, m_nCells);
    CHECK_POSITIVE(MID_IUH_OL, m_CellWidth);
    CHECK_NONNEGATIVE(MID_IUH_OL, m_TimeStep);
    CHECK_POINTER(MID_IUH_OL, m_subbasin);
    CHECK_POINTER(MID_IUH_OL, m_iuhCell);
    CHECK_POINTER(MID_IUH_OL, m_rs);
    return true;
}

void IUH_OL::initialOutputs() {
    CHECK_POSITIVE(MID_IUH_OL, m_nSubbasins);

    if (m_cellArea <= 0.f) m_cellArea = m_CellWidth * m_CellWidth;
    if (nullptr == m_Q_SBOF) {
        Initialize1DArray(m_nSubbasins + 1, m_Q_SBOF, 0.f);
        for (int i = 0; i < m_nCells; i++) {
            m_cellFlowCols = max(int(m_iuhCell[i][1] + 1), m_cellFlowCols);
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
    initialOutputs();
    // delete value of last time step
    for (int n = 0; n <= m_nSubbasins; n++) {
        m_Q_SBOF[n] = 0.f;
    }
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        //forward one time step
        for (int j = 0; j < m_cellFlowCols; j++) {
            if (j != m_cellFlowCols - 1) {
                m_cellFlow[i][j] = m_cellFlow[i][j + 1];
            } else {
                m_cellFlow[i][j] = 0.f;
            }
        }

        float v_rs = m_rs[i];
        if (v_rs > 0.f) {
            int min = int(m_iuhCell[i][0]);
            int max = int(m_iuhCell[i][1]);
            int col = 2;
            for (int k = min; k <= max; k++) {
                m_cellFlow[i][k] += v_rs / 1000.f * m_iuhCell[i][col] * m_cellArea / m_TimeStep;
                col++;
            }
        }
    }
    // See https://github.com/lreis2415/SEIMS/issues/36 for more descriptions. By lj
#pragma omp parallel
    {
        float *tmp_qsSub = new float[m_nSubbasins + 1];
        for (int i = 0; i <= m_nSubbasins; i++) {
            tmp_qsSub[i] = 0.f;
        }
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            tmp_qsSub[(int) m_subbasin[i]] += m_cellFlow[i][0];    //get new value
            m_OL_Flow[i] = m_cellFlow[i][0];
            m_OL_Flow[i] = m_OL_Flow[i] * m_TimeStep * 1000.f / m_cellArea;     // m3/s -> mm
        }
#pragma omp critical
        {
            for (int i = 1; i <= m_nSubbasins; i++) {
                m_Q_SBOF[i] += tmp_qsSub[i];
            }
        }
        delete[] tmp_qsSub;
    }  /* END of #pragma omp parallel */

    for (int n = 1; n <= m_nSubbasins; n++) {
        //get overland flow routing for entire watershed.
        m_Q_SBOF[0] += m_Q_SBOF[n];
    }
    return 0;
}

bool IUH_OL::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_IUH_OL, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(MID_IUH_OL, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}

void IUH_OL::SetValue(const char *key, float value) {
    string sk(key);
    if (StringMatch(sk, Tag_TimeStep)) { m_TimeStep = int(value); }
    else if (StringMatch(sk, Tag_CellSize)) { m_nCells = int(value); }
    else if (StringMatch(sk, Tag_CellWidth)) { m_CellWidth = value; }
    else if (StringMatch(sk, VAR_SUBBSNID_NUM)) { m_nSubbasins = value; }
    else if (StringMatch(sk, Tag_SubbasinId)) { m_subbasinID = value; }
    else {
        throw ModelException(MID_IUH_OL, "SetValue", "Parameter " + sk + " does not exist in current method.");
    }
}

void IUH_OL::Set1DData(const char *key, int n, float *data) {
    CheckInputSize(key, n);
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN)) { m_subbasin = data; }
    else if (StringMatch(sk, VAR_SURU)) { m_rs = data; }
    else {
        throw ModelException(MID_IUH_OL, "Set1DData", "Parameter " + sk + " does not exist in current method.");
    }
}

void IUH_OL::Set2DData(const char *key, int nRows, int nCols, float **data) {
    string sk(key);
    if (StringMatch(sk, VAR_OL_IUH)) {
        CheckInputSize(VAR_OL_IUH, nRows);
        m_iuhCell = data;
        m_iuhCols = nCols;
    } else {
        throw ModelException(MID_IUH_OL, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void IUH_OL::GetValue(const char *key, float *value) {
    initialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SBOF) && m_subbasinID > 0) { /// For MPI version to transfer data across subbasins
        *value = m_Q_SBOF[m_subbasinID];
    } else {
        throw ModelException(MID_IUH_OL, "GetValue", "Result " + sk + " does not exist.");
    }
}

void IUH_OL::Get1DData(const char *key, int *n, float **data) {
    initialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SBOF)) {
        *data = m_Q_SBOF;
        *n = m_nSubbasins + 1;
    } else if (StringMatch(sk, VAR_OLFLOW)) {
        *data = m_OL_Flow;
        *n = m_nCells;
    } else {
        throw ModelException(MID_IUH_OL, "Get1DData", "Result " + sk + " does not exist.");
    }
}
