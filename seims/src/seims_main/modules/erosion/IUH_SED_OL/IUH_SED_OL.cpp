#include "seims.h"
#include "IUH_SED_OL.h"

using namespace std;

IUH_SED_OL::IUH_SED_OL() : m_TimeStep(-1), m_nCells(-1), m_CellWidth(NODATA_VALUE), m_cellArea(NODATA_VALUE),
                           m_nSubbasins(-1), m_subbasinID(-1), m_subbasin(nullptr),
                           m_iuhCell(nullptr), m_iuhCols(-1), m_cellFlowCols(-1),
                           m_sedYield(nullptr), m_cellSed(nullptr), m_sedtoCh(nullptr), m_sedOL(nullptr) {
}

IUH_SED_OL::~IUH_SED_OL() {
    Release1DArray(m_sedtoCh);
    Release2DArray(m_nCells, m_cellSed);
}

bool IUH_SED_OL::CheckInputData() {
    CHECK_POSITIVE(MID_IUH_SED_OL, m_date);
    CHECK_POSITIVE(MID_IUH_SED_OL, m_nSubbasins);
    CHECK_NONNEGATIVE(MID_IUH_SED_OL, m_subbasinID);
    CHECK_POSITIVE(MID_IUH_SED_OL, m_nCells);
    CHECK_POSITIVE(MID_IUH_SED_OL, m_CellWidth);
    CHECK_NONNEGATIVE(MID_IUH_SED_OL, m_TimeStep);
    CHECK_POINTER(MID_IUH_SED_OL, m_subbasin);
    CHECK_POINTER(MID_IUH_SED_OL, m_iuhCell);
    return true;
}

void IUH_SED_OL::initialOutputs() {
    CHECK_POSITIVE(MID_IUH_SED_OL, m_nSubbasins);
    CHECK_POSITIVE(MID_IUH_SED_OL, m_nCells);
    CHECK_POINTER(MID_IUH_SED_OL, m_iuhCell);
    if (m_cellArea <= 0.f) m_cellArea = m_CellWidth * m_CellWidth;
    if (nullptr == m_sedtoCh) {
        Initialize1DArray(m_nSubbasins + 1, m_sedtoCh, 0.f);
        Initialize1DArray(m_nCells, m_sedOL, 0.f);
        //get m_cellFlowCols, i.e. the maximum of second column of OL_IUH plus 1.
        for (int i = 0; i < m_nCells; i++) {
            m_cellFlowCols = max(int(m_iuhCell[i][1] + 1), m_cellFlowCols);
        }
        Initialize2DArray(m_nCells, m_cellFlowCols, m_cellSed, 0.f);
    }
}

int IUH_SED_OL::Execute() {
    CheckInputData();
    initialOutputs();
    // delete value of last time step
    for (int i = 0; i < m_nSubbasins + 1; i++) {
        m_sedtoCh[i] = 0.f;
    }

#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        //forward one time step
        for (int j = 0; j < m_cellFlowCols; j++) {
            if (j != m_cellFlowCols - 1) {
                m_cellSed[i][j] = m_cellSed[i][j + 1];
            } else {
                m_cellSed[i][j] = 0.f;
            }
        }

        if (m_sedYield[i] > 0.f) {
            int min = int(m_iuhCell[i][0]);
            int max = int(m_iuhCell[i][1]);
            int col = 2;
            for (int k = min; k <= max; k++) {
                m_cellSed[i][k] += m_sedYield[i] * m_iuhCell[i][col];
                col++;
            }
        }
    }
    // See https://github.com/lreis2415/SEIMS/issues/36 for more descriptions. By lj
#pragma omp parallel
    {
        float *tmp_sed2ch = new float[m_nSubbasins + 1];
        for (int i = 0; i <= m_nSubbasins; i++) {
            tmp_sed2ch[i] = 0.f;
        }
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            tmp_sed2ch[(int) m_subbasin[i]] += m_cellSed[i][0];
            m_sedOL[i] = m_cellSed[i][0];
        }
#pragma omp critical
        {
            for (int i = 1; i <= m_nSubbasins; i++) {
                m_sedtoCh[i] += tmp_sed2ch[i];
            }
        }
        delete[] tmp_sed2ch;
    }  /* END of #pragma omp parallel */
    for (int i = 1; i < m_nSubbasins + 1; i++) {
        m_sedtoCh[0] += m_sedtoCh[i]; //get overland flow routing for entire watershed.
    }
    return 0;
}

bool IUH_SED_OL::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_IUH_SED_OL, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; }
        else {
            throw ModelException(MID_IUH_SED_OL, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}

void IUH_SED_OL::SetValue(const char *key, float value) {
    string sk(key);

    if (StringMatch(sk, Tag_TimeStep)) { m_TimeStep = (int) value; }
    else if (StringMatch(sk, Tag_CellSize)) { m_nCells = (int) value; }
    else if (StringMatch(sk, Tag_CellWidth)) { m_CellWidth = value; }
    else if (StringMatch(sk, VAR_SUBBSNID_NUM)) { m_nSubbasins = value; }
    else if (StringMatch(sk, Tag_SubbasinId)) { m_subbasinID = value; }
    else if (StringMatch(sk, VAR_OMP_THREADNUM)) { SetOpenMPThread((int) value); }
    else {
        throw ModelException(MID_IUH_SED_OL, "SetValue", "Parameter " + sk + " does not exist in current method.");
    }
}

void IUH_SED_OL::Set1DData(const char *key, int n, float *data) {
    CheckInputSize(key, n);
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN)) { m_subbasin = data; }
    else if (StringMatch(sk, VAR_SOER)) { m_sedYield = data; }
    else {
        throw ModelException(MID_IUH_SED_OL, "Set1DData", "Parameter " + sk + " does not exist in current method.");
    }
}

void IUH_SED_OL::Set2DData(const char *key, int nRows, int nCols, float **data) {
    string sk(key);
    if (StringMatch(sk, VAR_OL_IUH)) {
        CheckInputSize(VAR_OL_IUH, nRows);
        m_iuhCell = data;
        m_iuhCols = nCols;
    } else {
        throw ModelException(MID_IUH_SED_OL, "Set2DData", "Parameter " + sk + " does not exist in current method.");
    }
}

void IUH_SED_OL::GetValue(const char *key, float *value) {
    initialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SED_TO_CH) && m_subbasinID > 0) { /// For MPI version to transfer data across subbasins
        *value = m_sedtoCh[m_subbasinID];
    }
    else {
        throw ModelException(MID_IUH_SED_OL, "GetValue", "Result " + sk + " does not exist.");
    }
}

void IUH_SED_OL::Get1DData(const char *key, int *n, float **data) {
    initialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SED_TO_CH)) {
        *data = m_sedtoCh;   // from each subbasin to channel
        *n = m_nSubbasins + 1;
        return;
    } else if (StringMatch(sk, VAR_SEDYLD)) {
        *data = m_sedOL;
        *n = m_nCells;
    } else {
        throw ModelException(MID_IUH_SED_OL, "Get1DData", "Result " + sk + " does not exist.");
    }
}
