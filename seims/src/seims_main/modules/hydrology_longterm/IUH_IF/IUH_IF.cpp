#include "IUH_IF.h"
#include "text.h"

// using namespace std;  // Avoid this statement! by lj.

IUH_IF::IUH_IF(void) : m_TimeStep(-1), m_nCells(-1), m_cellArea(nullptr), m_nsub(-1), m_subbasin(NULL),
                       m_iuhCell(NULL), m_ssru(NULL), m_iuhCols(-1), m_cellFlowCols(-1) {

    m_Q_SBIF = NULL;
    m_cellFlow = NULL;
    SetModuleName("IUH_IF");
}

IUH_IF::~IUH_IF(void) {
    Release1DArray(m_Q_SBIF);
    Release2DArray(m_cellFlow);
}

bool IUH_IF::CheckInputData(void) {
    if (m_nCells < 0) {
        throw ModelException(GetModuleName(), "CheckInputData", "The parameter: m_nCells has not been set.");
        return false;
    }
    CHECK_POINTER(M_IUH_IF[0], m_cellArea);
    if (m_TimeStep <= 0) {
        throw ModelException(GetModuleName(), "CheckInputData", "The parameter: m_TimeStep has not been set.");
        return false;
    }

    if (m_subbasin == NULL) {
        throw ModelException(GetModuleName(), "CheckInputData", "The parameter: m_subbasin has not been set.");
        return false;
    }
    /*if (m_uhmaxCell == NULL)
    {
    throw ModelException(GetModuleName(),"CheckInputData","The parameter: m_uhmaxCell has not been set.");
    return false;
    }
    if (m_uhminCell == NULL)
    {
    throw ModelException(GetModuleName(),"CheckInputData","The parameter: m_uhminCell has not been set.");
    return false;
    }*/
    if (m_iuhCell == NULL) {
        throw ModelException(GetModuleName(), "CheckInputData", "The parameter: m_iuhCell has not been set.");
        return false;
    }
    if (m_ssru == NULL) {
        throw ModelException(GetModuleName(), "CheckInputData", "The parameter: m_rs has not been set.");
        return false;
    }
    if (m_date < 0) {
        throw ModelException(GetModuleName(), "CheckInputData", "The parameter: m_date has not been set.");
        return false;
    }

    return true;
}

void IUH_IF:: InitialOutputs() {
    if (this->m_nCells <= 0 || this->m_subbasin == NULL) {
        throw ModelException(GetModuleName(), "CheckInputData", "The dimension of the input data can not be less than zero.");
    }
    // allocate the output variables

    if (m_nsub <= 0) {
        map<int, int> subs;
        for (int i = 0; i < this->m_nCells; i++) {
            subs[int(this->m_subbasin[i])] += 1;
        }
        this->m_nsub = CVT_INT(subs.size());
    }

    if (m_cellFlow == NULL) {
        m_Q_SBIF = new FLTPT[m_nsub + 1];
        for (int i = 0; i <= m_nsub; i++) {
            m_Q_SBIF[i] = 0.f;
        }
        m_cellFlow = new FLTPT *[this->m_nCells];

        for (int i = 0; i < this->m_nCells; i++) {
            m_cellFlowCols = Max(int(m_iuhCell[i][1] + 1), m_cellFlowCols);
        }

        //get m_cellFlowCols, i.e. the maximum of second column of iuh add 1.
#pragma omp parallel for
        for (int i = 0; i < this->m_nCells; i++) {
            m_cellFlow[i] = new FLTPT[m_cellFlowCols];
            for (int j = 0; j < m_cellFlowCols; j++) {
                m_cellFlow[i][j] = 0.0f;
            }
        }
    }
}

int IUH_IF::Execute() {
    this->CheckInputData();

    this-> InitialOutputs();

#pragma omp parallel for
    for (int n = 0; n < m_nsub + 1; n++) {
        m_Q_SBIF[n] = 0.0f;    // delete value of last time step
    }


    //int nt = 0;
    //FLTPT qs_cell = 0.0f;

    #pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        //forward one time step
        for (int j = 0; j < m_cellFlowCols; j++) {
            if (j != m_cellFlowCols - 1) {
                m_cellFlow[i][j] = m_cellFlow[i][j + 1];
            } else {
                m_cellFlow[i][j] = 0.0f;
            }
        }

        //add today's flow
        int subi = (int) m_subbasin[i];

        if (m_nsub == 1) {
            subi = 1;
        } else if (subi >= m_nsub + 1) {
            throw ModelException(GetModuleName(), "Execute", "The subbasin " + ValueToString(subi) + " is invalid.");
        }

        FLTPT v_rs = m_ssru[i];
        if (v_rs > 0.f) {
            int min = int(this->m_iuhCell[i][0]);
            int max = int(this->m_iuhCell[i][1]);
            int col = 2;
            for (int k = min; k <= max; k++) {
                this->m_cellFlow[i][k] += v_rs / 1000.0f * m_iuhCell[i][col] * m_cellArea[i] / m_TimeStep;
                col++;
            }
        }
        //#pragma omp critical
        {
            m_Q_SBIF[subi] += this->m_cellFlow[i][0];    //get new value
        }
    }

    FLTPT tmp = 0.f;
    //#pragma omp parallel for reduction(+:tmp)
    for (int n = 1; n < m_nsub + 1; n++) {
        tmp += m_Q_SBIF[n];        //get overland flow routing for entire watershed.
    }
    m_Q_SBIF[0] = tmp;

    return 0;
}

bool IUH_IF::CheckInputSize(const char *key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

void IUH_IF::SetValue(const char *key, FLTPT value) {
    string sk(key);
    if (StringMatch(sk, Tag_TimeStep[0])) {
        m_TimeStep = (int) value;
    } else if (StringMatch(sk, Tag_CellSize[0])) {
        m_nCells = int(value);
    } else {
        throw ModelException(GetModuleName(), "SetValue", "Parameter " + sk
            + " does not exist in IUH_IF method. Please contact the module developer.");
    }
}

void IUH_IF::Set1DData(const char *key, int n, FLTPT *data) {
    //set the value
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSN[0])) {
        m_subbasin = data;
    } else if (StringMatch(sk, VAR_CELL_AREA[0])) {
        m_cellArea = data;
    } else if (StringMatch(sk, VAR_SSRU[0])) {
        m_ssru = data;
    } else {
        throw ModelException(GetModuleName(), "SetValue", "Parameter " + sk +
            " does not exist in IUH_IF method. Please contact the module developer.");
    }
}

void IUH_IF::Set2DData(const char *key, int nRows, int nCols, FLTPT **data) {
    string sk(key);
    if (StringMatch(sk, VAR_OL_IUH[0])) {
        CheckInputSize(key, nRows);
        m_iuhCell = data;
        m_iuhCols = nCols;
    } else {
        throw ModelException(GetModuleName(), "SetValue", "Parameter " + sk +
            " does not exist in IUH_IF method. Please contact the module developer.");
    }
}

void IUH_IF::Get1DData(const char *key, int *n, FLTPT **data) {
    string sk(key);
    if (StringMatch(sk, VAR_SBIF[0])) {
        *data = m_Q_SBIF;
    } else {
        throw ModelException(GetModuleName(), "getResult", "Result " + sk +
            " does not exist in IUH_IF method. Please contact the module developer.");
    }
    *n = this->m_nsub + 1;
}
