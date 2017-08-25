#include "seims.h"
#include "IUH_OL.h"

using namespace std;

IUH_OL::IUH_OL(void) : m_TimeStep(-1), m_nCells(-1), m_CellWidth(NODATA_VALUE), m_cellArea(NODATA_VALUE),
                       m_nSubbasins(-1), m_subbasin(NULL), m_subbasinsInfo(NULL),
                       m_iuhCell(NULL), m_iuhCols(-1), m_rs(NULL),
                       m_cellFlowCols(-1), m_cellFlow(NULL), m_OL_Flow(NULL), m_Q_SBOF(NULL) {
}

IUH_OL::~IUH_OL(void) {
    Release1DArray(m_Q_SBOF);
    Release2DArray(m_nCells, m_cellFlow);
    Release1DArray(m_OL_Flow);
}

bool IUH_OL::CheckInputData(void) {
    if (m_date < 0) {
        throw ModelException(MID_IUH_OL, "CheckInputData", "The parameter: m_date has not been set.");
    }
    if (m_nCells < 0) {
        throw ModelException(MID_IUH_OL, "CheckInputData", "The parameter: m_nCells has not been set.");
    }
    if (FloatEqual(m_CellWidth, NODATA_VALUE)) {
        throw ModelException(MID_IUH_OL, "CheckInputData", "The parameter: m_CellWidth has not been set.");
    }
    if (m_TimeStep <= 0) {
        throw ModelException(MID_IUH_OL, "CheckInputData", "The parameter: m_TimeStep has not been set.");
    }
    if (m_subbasin == NULL) {
        throw ModelException(MID_IUH_OL, "CheckInputData", "The parameter: m_subbasin has not been set.");
    }
    if (m_nSubbasins <= 0) {
        throw ModelException(MID_IUH_OL, "CheckInputData", "The subbasins number must be greater than 0.");
    }
    if (m_subbasinIDs.empty()) throw ModelException(MID_IUH_OL, "CheckInputData", "The subbasin IDs can not be EMPTY.");
    if (m_subbasinsInfo == NULL) {
        throw ModelException(MID_IUH_OL, "CheckInputData", "The parameter: m_subbasinsInfo has not been set.");
    }
    if (m_iuhCell == NULL) {
        throw ModelException(MID_IUH_OL, "CheckInputData", "The parameter: m_iuhCell has not been set.");
    }
    if (m_rs == NULL) {
        throw ModelException(MID_IUH_OL, "CheckInputData", "The parameter: surface runoff (m_rs) has not been set.");
    }
    //if (m_landcover == NULL)
    //	throw ModelException(MID_IUH_OL, "CheckInputData", "The parameter: landcover has not been set.");
    return true;
}

void IUH_OL::initialOutputs() {
    if (m_cellArea <= 0.f) m_cellArea = m_CellWidth * m_CellWidth;
    if (this->m_Q_SBOF == NULL) {
        Initialize1DArray(m_nSubbasins + 1, m_Q_SBOF, 0.f);
        for (int i = 0; i < m_nCells; i++) {
            m_cellFlowCols = max(int(m_iuhCell[i][1] + 1), m_cellFlowCols);
        }
        //get m_cellFlowCols, i.e. the maximum of second column of OL_IUH plus 1.
        Initialize2DArray(m_nCells, m_cellFlowCols, m_cellFlow, 0.f);
    }
    if (m_OL_Flow == NULL) {
        Initialize1DArray(m_nCells, m_OL_Flow, 0.f);
    }
}

int IUH_OL::Execute() {
    this->CheckInputData();
    this->initialOutputs();
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
            int min = int(this->m_iuhCell[i][0]);
            int max = int(this->m_iuhCell[i][1]);
            int col = 2;
            for (int k = min; k <= max; k++) {
                m_cellFlow[i][k] += v_rs / 1000.f * m_iuhCell[i][col] * m_cellArea / m_TimeStep;
                col++;
            }
        }
    }
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        //add today's flow
        int subi = (int) m_subbasin[i];
        if (m_nSubbasins == 1) {
            subi = 1;
        } else if (subi >= m_nSubbasins + 1) {
            throw ModelException(MID_IUH_OL, "Execute", "The subbasin " + ValueToString(subi) + " is invalid.");
        }

        m_Q_SBOF[subi] += m_cellFlow[i][0];    //get new value

        m_OL_Flow[i] = m_cellFlow[i][0];
        m_OL_Flow[i] = m_OL_Flow[i] * m_TimeStep * 1000.f / m_cellArea;     // m3/s -> mm
        //if(i == 1000) cout << m_OL_Flow[i] << endl;
    }

    //float tmp = 0.f;
//#pragma omp parallel for reduction(+:tmp) // there is no need to use omp
    for (int n = 1; n <= m_nSubbasins; n++) {
        //get overland flow routing for entire watershed.
        //tmp += m_Q_SBOF[n];
        m_Q_SBOF[0] += m_Q_SBOF[n];
    }
    
    return 0;
}

bool IUH_OL::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_IUH_OL, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
    }
    if (this->m_nCells != n) {
        if (this->m_nCells <= 0) { this->m_nCells = n; }
        else {
            throw ModelException(MID_IUH_OL, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
        }
    }
    return true;
}

void IUH_OL::SetValue(const char *key, float value) {
    string sk(key);

    if (StringMatch(sk, Tag_TimeStep)) { m_TimeStep = (int) value; }
    else if (StringMatch(sk, Tag_CellSize)) { m_nCells = (int) value; }
    else if (StringMatch(sk, Tag_CellWidth)) { m_CellWidth = value; }
    else if (StringMatch(sk, VAR_OMP_THREADNUM)) { SetOpenMPThread((int) value); }
    else {
        throw ModelException(MID_IUH_OL, "SetValue", "Parameter " + sk + " does not exist in current method.");
    }
}

void IUH_OL::Set1DData(const char *key, int n, float *data) {
    this->CheckInputSize(key, n);
    //set the value
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN)) { m_subbasin = data; }
    else if (StringMatch(sk, VAR_SURU)) {
        m_rs = data;
        //else if (StringMatch(sk, VAR_LANDCOVER)) m_landcover = data;
    } else {
        throw ModelException(MID_IUH_OL, "Set1DData", "Parameter " + sk + " does not exist in current method.");
    }
}

void IUH_OL::Set2DData(const char *key, int nRows, int nCols, float **data) {

    string sk(key);
    if (StringMatch(sk, VAR_OL_IUH)) {
        this->CheckInputSize(VAR_OL_IUH, nRows);
        m_iuhCell = data;
        m_iuhCols = nCols;
    } else {
        throw ModelException(MID_IUH_OL, "Set2DData", "Parameter " + sk + " does not exist.");
    }
}

void IUH_OL::SetSubbasins(clsSubbasins *subbasins) {
    if (m_subbasinsInfo == NULL) {
        m_subbasinsInfo = subbasins;
        m_nSubbasins = m_subbasinsInfo->GetSubbasinNumber();
        m_subbasinIDs = m_subbasinsInfo->GetSubbasinIDs();
    }
}

void IUH_OL::Get1DData(const char *key, int *n, float **data) {
    initialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_SBOF)) {
        *data = this->m_Q_SBOF;
        *n = this->m_nSubbasins + 1;
    } else if (StringMatch(sk, VAR_OLFLOW)) {
        *data = this->m_OL_Flow;
        *n = this->m_nCells;
    } else {
        throw ModelException(MID_IUH_OL, "Get1DData", "Result " + sk + " does not exist.");
    }
}
