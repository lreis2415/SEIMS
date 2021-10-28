#include "IUH_SED_OL.h"

#include "text.h"

IUH_SED_OL::IUH_SED_OL() :
    m_TimeStep(-1), m_nCells(-1), m_CellWidth(NODATA_VALUE), m_cellArea(NODATA_VALUE),
    m_nSubbsns(-1), m_inputSubbsnID(-1), m_subbsnID(nullptr),
    m_iuhCell(nullptr), m_iuhCols(-1), m_sedYield(nullptr),
    m_cellFlowCols(-1), m_cellSed(nullptr), m_sedtoCh(nullptr), m_olWtrEroSed(nullptr) {
}

IUH_SED_OL::~IUH_SED_OL() {
    if (m_sedtoCh != nullptr) Release1DArray(m_sedtoCh);
    if (m_cellSed != nullptr) Release2DArray(m_nCells, m_cellSed);
}

bool IUH_SED_OL::CheckInputData() {
    CHECK_POSITIVE(M_IUH_SED_OL[0], m_date);
    CHECK_POSITIVE(M_IUH_SED_OL[0], m_nSubbsns);
    CHECK_NONNEGATIVE(M_IUH_SED_OL[0], m_inputSubbsnID);
    CHECK_POSITIVE(M_IUH_SED_OL[0], m_nCells);
    CHECK_POSITIVE(M_IUH_SED_OL[0], m_CellWidth);
    CHECK_NONNEGATIVE(M_IUH_SED_OL[0], m_TimeStep);
    CHECK_POINTER(M_IUH_SED_OL[0], m_subbsnID);
    CHECK_POINTER(M_IUH_SED_OL[0], m_iuhCell);
    return true;
}

void IUH_SED_OL::InitialOutputs() {
    CHECK_POSITIVE(M_IUH_SED_OL[0], m_nSubbsns);
    CHECK_POSITIVE(M_IUH_SED_OL[0], m_nCells);
    CHECK_POINTER(M_IUH_SED_OL[0], m_iuhCell);
    if (m_cellArea <= 0.f) m_cellArea = m_CellWidth * m_CellWidth;
    if (nullptr == m_sedtoCh) {
        Initialize1DArray(m_nSubbsns + 1, m_sedtoCh, 0.f);
        Initialize1DArray(m_nCells, m_olWtrEroSed, 0.f);
        //get m_cellFlowCols, i.e. the maximum of second column of OL_IUH plus 1.
        for (int i = 0; i < m_nCells; i++) {
            m_cellFlowCols = Max(CVT_INT(m_iuhCell[i][1] + 1), m_cellFlowCols);
        }
        Initialize2DArray(m_nCells, m_cellFlowCols, m_cellSed, 0.f);
    }
}

int IUH_SED_OL::Execute() {
    CheckInputData();
    InitialOutputs();
    // delete value of last time step
    for (int i = 0; i < m_nSubbsns + 1; i++) {
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
            int min = CVT_INT(m_iuhCell[i][0]);
            int max = CVT_INT(m_iuhCell[i][1]);
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
        float* tmp_sed2ch = new float[m_nSubbsns + 1];
        for (int i = 0; i <= m_nSubbsns; i++) {
            tmp_sed2ch[i] = 0.f;
        }
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            tmp_sed2ch[CVT_INT(m_subbsnID[i])] += m_cellSed[i][0];
            m_olWtrEroSed[i] = m_cellSed[i][0];
        }
#pragma omp critical
        {
            for (int i = 1; i <= m_nSubbsns; i++) {
                m_sedtoCh[i] += tmp_sed2ch[i];
            }
        }
        delete[] tmp_sed2ch;
        tmp_sed2ch = nullptr;
    } /* END of #pragma omp parallel */
    for (int i = 1; i < m_nSubbsns + 1; i++) {
        m_sedtoCh[0] += m_sedtoCh[i]; //get overland flow routing for entire watershed.
    }
    return 0;
}

void IUH_SED_OL::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, Tag_TimeStep[0])) m_TimeStep = CVT_INT(value);
    else if (StringMatch(sk, Tag_CellSize[0])) m_nCells = CVT_INT(value);
    else if (StringMatch(sk, Tag_CellWidth[0])) m_CellWidth = value;
    else if (StringMatch(sk, VAR_SUBBSNID_NUM[0])) m_nSubbsns = CVT_INT(value);
    else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbsnID = CVT_INT(value);
    else {
        throw ModelException(M_IUH_SED_OL[0], "SetValue", "Parameter " + sk + " does not exist in current method.");
    }
}

void IUH_SED_OL::Set1DData(const char* key, const int n, float* data) {
    CheckInputSize(M_IUH_SED_OL[0], key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN[0])) m_subbsnID = data;
    else if (StringMatch(sk, VAR_SOER[0])) m_sedYield = data;
    else {
        throw ModelException(M_IUH_SED_OL[0], "Set1DData", "Parameter " + sk + " does not exist in current method.");
    }
}

void IUH_SED_OL::Set2DData(const char* key, const int nrows, const int ncols, float** data) {
    string sk(key);
    if (StringMatch(sk, VAR_OL_IUH[0])) {
        CheckInputSize2D(M_IUH_SED_OL[0], key, nrows, ncols, m_nCells, m_iuhCols);
        m_iuhCell = data;
    } else {
        throw ModelException(M_IUH_SED_OL[0], "Set2DData", "Parameter " + sk + " does not exist in current method.");
    }
}

void IUH_SED_OL::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SED_TO_CH[0])) {
        *data = m_sedtoCh; // from each subbasin to channel
        *n = m_nSubbsns + 1;
    } else if (StringMatch(sk, VAR_SEDYLD[0])) {
        *data = m_olWtrEroSed;
        *n = m_nCells;
    } else {
        throw ModelException(M_IUH_SED_OL[0], "Get1DData", "Result " + sk + " does not exist.");
    }
}
