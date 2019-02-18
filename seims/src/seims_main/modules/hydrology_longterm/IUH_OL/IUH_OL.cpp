#include "IUH_OL.h"

#include "text.h"

IUH_OL::IUH_OL() :
    m_TimeStep(-1), m_nCells(-1), m_CellWth(NODATA_VALUE), m_cellArea(NODATA_VALUE),
    m_nSubbsns(-1), m_inputSubbsnID(-1), m_subbsnID(nullptr),
    m_iuhCell(nullptr), m_iuhCols(-1), m_surfRf(nullptr),
    m_cellFlow(nullptr), m_cellFlowCols(-1), m_Q_SBOF(nullptr), m_OL_Flow(nullptr),
    m_unitArea(nullptr), m_landUse(nullptr), m_flowPond(nullptr), m_pondSort(nullptr){
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
    if (m_inputSubbsnID == 9999){
        CHECK_POINTER(MID_IUH_OL, m_landUse);
        CHECK_POINTER(MID_IUH_OL, m_flowPond);
        CHECK_POINTER(MID_IUH_OL, m_pondSort);
        CHECK_POINTER(MID_IUH_OL, m_unitArea);
    }
    return true;
}

void IUH_OL::InitialOutputs() {
    CHECK_POSITIVE(MID_IUH_OL, m_nSubbsns);

    //if (m_cellArea <= 0.f) m_cellArea = m_CellWth * m_CellWth;
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
        // if (m_inputSubbsnID == 9999 && FloatEqual(CVT_INT(m_landUse[i]), LANDUSE_ID_POND))  continue;
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
            m_cellFlow[i][k] += m_surfRf[i] * 0.001f * m_iuhCell[i][col] * GetUnitArea(i) / m_TimeStep;
            col++;
        }
    }
    // See https://github.com/lreis2415/SEIMS/issues/36 for more descriptions. By lj
    if (m_inputSubbsnID != 9999) { //raster version
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
            m_OL_Flow[i] = m_OL_Flow[i] * m_TimeStep * 1000.f / GetUnitArea(i); // m3/s -> mm
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
    }
    else { //field version
        float* tmp_qsSub = new float[m_nSubbsns + 1];
        for (int i = 0; i <= m_nSubbsns; i++) {
            tmp_qsSub[i] = 0.f;
        }
#pragma omp parallel for
        for (int i = 0; i < m_nCells; i++) {
            // skip ponds in this step and process them later
            if (FloatEqual(CVT_INT(m_landUse[i]), LANDUSE_ID_POND)) { continue; }

            // judge whether the overland flow of one cell flow to a river or a pond
            if (m_flowPond[i] < 0) // to river 
                {
                    tmp_qsSub[CVT_INT(m_subbsnID[i])] += m_cellFlow[i][0]; //get new value
                    m_OL_Flow[i] = m_cellFlow[i][0];
                    m_OL_Flow[i] = m_OL_Flow[i] * m_TimeStep * 1000.f / GetUnitArea(i); // m3/s -> mm
                }
            else // to pond, add to down pond flow
                {
                    m_cellFlow[CVT_INT(m_flowPond[i])][0] += m_cellFlow[i][0];
                    // if a field flow to pond, should update its m_OL_Flow???
                    m_OL_Flow[i] = m_cellFlow[i][0];  
                    m_OL_Flow[i] = m_OL_Flow[i] * m_TimeStep * 1000.f / GetUnitArea(i); // m3/s -> mm
                }
        }
        for (int i = 0; i < m_nCells; i++) {
            // process the ponds if their water volume exceeds their capacity
            // the upstream and downstream topology among ponds should be considered here
            if (m_pondSort[i] > 0){ // since pond topological sort has been read, calculate the cellflow directly
                int pondId = CVT_INT(m_pondSort[i]);
                float pond_area = m_unitArea[pondId];
                float excCap = m_cellFlow[pondId][0] - pond_area * 2.f / m_TimeStep; // suppose the capacity of the pond is area*2m.
                if (excCap > 0){ // if their water volume exceed their capacity
                    if (m_flowPond[pondId] < 0) tmp_qsSub[CVT_INT(m_subbsnID[pondId])] += m_cellFlow[pondId][0]; // the down is river
                    else { // the down is pond
                        m_cellFlow[CVT_INT(m_flowPond[pondId])][0] += excCap;
                    }
                }
                m_OL_Flow[pondId] = m_cellFlow[pondId][0];
                m_OL_Flow[pondId] = m_OL_Flow[pondId] * m_TimeStep * 1000.f / pond_area; // m3/s -> mm
            }
        }
    }

    for (int n = 1; n <= m_nSubbsns; n++) {
        //get overland flow routing for entire watershed.
        m_Q_SBOF[0] += m_Q_SBOF[n];
    }
    return 0;
}

bool IUH_OL::CheckInputSize(const char* key, const int n) {
    if (n <= 0) {
        throw ModelException(MID_IUH_OL, "CheckInputSize", "Input data for " + string(key) +
                             " is invalid. The size could not be less than zero.");
    }
    if (m_nCells != n) {
        if (m_nCells <= 0) {
            m_nCells = n;
        } else {
            throw ModelException(MID_IUH_OL, "CheckInputSize", "Input data for " + string(key) +
                                 " is invalid. All the input data should have same size.");
        }
    }
    return true;
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
    CheckInputSize(key, n);
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN)) m_subbsnID = data;
    else if (StringMatch(sk, VAR_SURU)) m_surfRf = data;
    else if (StringMatch(sk, VAR_LANDUSE)) m_landUse = data;
    else if (StringMatch(sk, VAR_FLOWPOND)) m_flowPond = data;
    else if (StringMatch(sk, VAR_FIELDAREA)) m_unitArea = data;
    else if (StringMatch(sk, VAR_PONDSORT)) m_pondSort = data;
    else {
        throw ModelException(MID_IUH_OL, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void IUH_OL::Set2DData(const char* key, const int nRows, const int nCols, float** data) {
    string sk(key);
    if (StringMatch(sk, VAR_OL_IUH)) {
        CheckInputSize(VAR_OL_IUH, nRows);
        m_iuhCell = data;
        m_iuhCols = nCols;
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

float IUH_OL::GetUnitArea(int i) {
    if (m_inputSubbsnID == 9999) return m_unitArea[i];
    else return m_CellWth * m_CellWth;
}