#include "DiffusiveWave.h"

#include "text.h"

DiffusiveWave::DiffusiveWave() :
    m_nCells(-1), m_dt(-1.0f), m_CellWidth(-1.0f), m_chNumber(-1),
    m_s0(nullptr), m_direction(nullptr), m_reachDownStream(nullptr), m_reachN(nullptr),
    m_chWidth(nullptr),
    m_qs(nullptr), m_hCh(nullptr), m_qCh(nullptr), m_prec(nullptr), m_qSubbasin(nullptr),
    m_elevation(nullptr),
    m_flowLen(nullptr), m_qi(nullptr), m_flowInIndex(nullptr), m_flowOutIdx(nullptr),
    m_streamLink(nullptr),
    m_sourceCellIds(nullptr),
    m_idUpReach(-1), m_idOutlet(-1), m_qUpReach(0.f) {
}

DiffusiveWave::~DiffusiveWave() {
    //Release1DArray(m_reachId);
    //Release1DArray(m_streamOrder);
    //Release1DArray(m_reachDownStream);
    //Release1DArray(m_reachN);

    Release2DArray(m_hCh);
    Release2DArray(m_qCh);
    Release2DArray(m_flowLen);
    Release1DArray(m_sourceCellIds);
    Release1DArray(m_qSubbasin);
}

//! Check input data
bool DiffusiveWave::CheckInputData(void) {
    if (this->m_date <= 0) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "You have not set the Date variable.");
        return false;
    }

    if (this->m_nCells <= 0) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The cell number of the input can not be less than zero.");
        return false;
    }

    if (this->m_dt <= 0) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "You have not set the TimeStep variable.");
        return false;
    }

    if (this->m_CellWidth <= 0) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "You have not set the CellWidth variable.");
        return false;
    }
    if (m_s0 == nullptr) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: slope has not been set.");
    }
    if (m_direction == nullptr) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: flow direction has not been set.");
    }

    if (m_chWidth == nullptr) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: CHWIDTH has not been set.");
    }
    if (m_streamLink == nullptr) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: STREAM_LINK has not been set.");
    }

    if (m_prec == nullptr) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: D_P(precipitation) has not been set.");
    }
    if (m_elevation == nullptr) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: Elevation has not been set.");
    }

    return true;
}

//! Initial outputs
void DiffusiveWave:: InitialOutputs() {
    if (m_nCells <= 0) {
        throw ModelException(M_CH_DW[0], "InitialOutputs", "The cell number of the input can not be less than zero.");
    }

    if (m_hCh == nullptr) {
        // find source cells the reaches
        m_sourceCellIds = new int[m_chNumber];
        for (int i = 0; i < m_chNumber; ++i) {
            m_sourceCellIds[i] = -1;
        }
        for (int i = 0; i < m_nCells; i++) {
            if (FloatEqual(m_streamLink[i], NODATA_VALUE)) {
                continue;
            }
            int reachId = (int) m_streamLink[i];
            bool isSource = true;
            for (int k = 1; k <= (int) m_flowInIndex[i][0]; ++k) {
                int flowInId = (int) m_flowInIndex[i][k];
                int flowInReachId = (int) m_streamLink[flowInId];
                if (flowInReachId == reachId) {
                    isSource = false;
                    break;
                }
            }

            if ((int) m_flowInIndex[i][0] == 0) {
                isSource = true;
            }

            if (isSource) {
                int reachIndex = m_idToIndex[reachId];
                m_sourceCellIds[reachIndex] = i;
            }
        }

        //for(int i = 0; i < m_chNumber; i++)
        //	cout << m_sourceCellIds[i] << endl;

        // get the cells in reaches according to flow direction
        for (int iCh = 0; iCh < m_chNumber; iCh++) {
            int iCell = m_sourceCellIds[iCh];
            int reachId = (int) m_streamLink[iCell];
            while ((int) m_streamLink[iCell] == reachId) {
                m_reachs[iCh].push_back(iCell);
                iCell = (int) m_flowOutIdx[iCell];
            }
        }

        m_hCh = new float *[m_chNumber];
        m_qCh = new float *[m_chNumber];

        m_flowLen = new float *[m_chNumber];

        m_qSubbasin = new float[m_chNumber];
        for (int i = 0; i < m_chNumber; ++i) {
            int n = m_reachs[i].size();
            m_hCh[i] = new float[n];
            m_qCh[i] = new float[n];

            m_flowLen[i] = new float[n];

            m_qSubbasin[i] = 0.f;

            int id;
            float s0, dx;
            for (int j = 0; j < n; ++j) {
                m_hCh[i][j] = 0.f;
                m_qCh[i][j] = 0.f;

                id = m_reachs[i][j];
                s0 = m_s0[id];
                if (FloatEqual(s0, 0.f)) {
                    s0 = MINI_SLOPE;
                }

                // slope length needs to be corrected by slope angle
                dx = m_CellWidth / cos(atan(s0));
                int dir = (int) m_direction[id];
                if (DiagonalCCW[dir] == 1) {
                    dx = SQ2 * dx;
                }
                m_flowLen[i][j] = dx;
            }
        }

    }
}

//! Channel flow
void DiffusiveWave::ChannelFlow(int iReach, int iCell, int id) {

    float qUp = 0.f;
    float hUp = 0.f;

    if (iReach == 0 && iCell == 0) {
        qUp = m_qUpReach;
    }

    // inflow from upstream channel
    if (iCell == 0) { // inflow of this cell is the last cell of the upstream reach
        for (size_t i = 0; i < m_reachUpStream[iReach].size(); ++i) {
            int upReachId = m_reachUpStream[iReach][i];
            if (upReachId >= 0) {
                int upCellsNum = m_reachs[upReachId].size();
                int upCellId = m_reachs[upReachId][upCellsNum - 1];
                qUp += m_qCh[upReachId][upCellsNum - 1];

                float hWater = m_elevation[upCellId] + m_hCh[upReachId][upCellsNum - 1];
                if (hWater > hUp) {
                    hUp = hWater;
                }
            }
        }
    } else {
        qUp = m_qCh[iReach][iCell - 1];
        hUp = m_hCh[iReach][iCell - 1];
    }

    //float b = 0.6f;
    float h = m_hCh[iReach][iCell];
    float dx = m_flowLen[iReach][iCell];

    float qLat = m_prec[id] / 1000.f * m_chWidth[id];
    if (m_qs != nullptr) {
        qLat += m_qs[id] / dx;
    }
    if (m_qi != nullptr) {
        qLat += m_qi[id] / dx;
    }

    if (qLat < MIN_FLUX && qUp < MIN_FLUX) {
        m_hCh[iReach][iCell] = 0.f;
        m_qCh[iReach][iCell] = 0.f;
        return;
    }

    float perim = 2.f * h + m_chWidth[iReach];
    float sf = (hUp - m_elevation[id] - h) / dx;
    if (sf < MINI_SLOPE) {
        sf = MINI_SLOPE;
    }
    float c = 1.f / 3600.f * m_reachN[iReach] * pow(perim, _2div3) / sqrt(sf);
    c = pow(c, 0.6f);

    float d = 1.f;
    int counter = 0;
    float qLast = m_qCh[iReach][iCell];
    float qNew = qLast;
    if (qNew < MIN_FLUX) {
        qNew = qLat;
    }

    while (abs(d) > MIN_FLUX && counter < 10) {
        d = (qNew * m_dt / dx + c * Power(qNew, 0.6f) - qUp * m_dt / dx - c * Power(qLast, 0.6f) - qLat * m_dt) /
            (m_dt / dx + c * 0.6f / Power(qNew, 0.4f));

        //if(d != d)
        //	int test = 1;
        qNew -= d;
        counter++;
    }

    if (qNew < 0.f) {
        qNew = 0.f;
    }

    float qAvail = m_hCh[iReach][iCell] * m_chWidth[iReach] * dx / m_dt + qLat * dx + qUp;

    if (qNew > qAvail) {
        m_qCh[iReach][iCell] = qAvail;
        m_hCh[iReach][iCell] = 0.f;
    } else {
        m_qCh[iReach][iCell] = qNew;//m_hCh[iReach][iCell]
        m_hCh[iReach][iCell] = c * pow(qNew, 0.6f) / m_chWidth[iReach];
        //float hh = (qUp + qLat*dx - qNew)*m_dt/(m_chWidth[iReach]*dx) + m_hCh[iReach][iCell];
    }
}

//! Main execute function
int DiffusiveWave::Execute() {
    CheckInputData();
    InitialOutputs();
    for (auto it = m_reachLayers.begin(); it != m_reachLayers.end(); ++it) {
        // There are no flow relationships within each routing layer.
        //   So parallelization can be done here.
        int nReaches = it->second.size();
        // the size of m_reachLayers (map) is equal to the maximum stream order
#pragma omp parallel for
        for (int i = 0; i < nReaches; ++i) {
            int reachIndex = it->second[i]; // index in the array
            vector<int> &vecCells = m_reachs[reachIndex];
            int n = vecCells.size();
            for (int iCell = 0; iCell < n; iCell++) {
                ChannelFlow(reachIndex, iCell, vecCells[iCell]);
            }
            m_qSubbasin[reachIndex] = m_qCh[reachIndex][n - 1];
        }
    }
    return 0;
}

void DiffusiveWave::SetValue(const char *key, const float value) {
    string sk(key);
    if (StringMatch(sk, Tag_HillSlopeTimeStep[0])) {
        m_dt = value;
    } else if (StringMatch(sk, Tag_CellSize[0])) {
        m_nCells = CVT_INT(value);
    } else if (StringMatch(sk, Tag_CellWidth[0])) {
        m_CellWidth = value;
    } else {
        throw ModelException(M_CH_DW[0], "SetValue", "Parameter " + sk
                             + " does not exist. Please contact the module developer.");
    }
}

void DiffusiveWave::Set1DData(const char *key, int n, float *data) {
    string sk(key);
    //check the input data
    CheckInputSize(M_CH_DW[0], key, n, m_nCells);

    if (StringMatch(sk, VAR_SLOPE[0])) {
        m_s0 = data;
    } else if (StringMatch(sk, VAR_DEM[0])) {
        m_elevation = data;
    } else if (StringMatch(sk, VAR_FLOWDIR[0])) {
        m_direction = data;
    } else if (StringMatch(sk, VAR_PCP[0])) {
        m_prec = data;
    } else if (StringMatch(sk, VAR_QSOIL[0])) {
        m_qi = data;
    } else if (StringMatch(sk, VAR_QOVERLAND[0])) {
        m_qs = data;
    } else if (StringMatch(sk, VAR_CHWIDTH[0])) {
        m_chWidth = data;
    } else if (StringMatch(sk, VAR_STREAM_LINK[0])) {
        m_streamLink = data;
    } else if (StringMatch(sk, Tag_FLOWOUT_INDEX[0])) { // TODO: Use a simple way to get outlet index
        m_flowOutIdx = data;
        for (int i = 0; i < m_nCells; i++) {
            if (m_flowOutIdx[i] < 0) {
                m_idOutlet = i;
                break;
            }
        }
    } else {
        throw ModelException(M_CH_DW[0], "Set1DData", "Parameter " + sk
                             + " does not exist.");
    }
}

void DiffusiveWave::SetReaches(clsReaches *reaches) {
    if (nullptr == reaches) {
        throw ModelException(M_CH_DW[0], "SetReaches", "The reaches input can not to be NULL.");
    }
    m_chNumber = reaches->GetReachNumber();

    if (nullptr == m_reachDownStream) reaches->GetReachesSingleProperty(REACH_DOWNSTREAM, &m_reachDownStream);
    if (nullptr == m_chWidth) reaches->GetReachesSingleProperty(REACH_WIDTH, &m_chWidth);
    if (nullptr == m_reachN) reaches->GetReachesSingleProperty(REACH_MANNING, &m_reachN);

    m_reachUpStream = reaches->GetUpStreamIDs();
    m_reachLayers = reaches->GetReachLayers();
}

void DiffusiveWave::Get1DData(const char *key, int *n, float **data) {
    string sk(key);
    //*n = m_nCells;
    *n = m_chNumber;
    if (StringMatch(sk, VAR_QSUBBASIN[0])) {
        *data = m_qSubbasin;
    }
        /*else if (StringMatch(sk, "CHWATH"))
        {
        *data = m_chwath;
        }
        else if (StringMatch(sk, "CHQCH"))
        {
        *data = m_chwath;
        }*/
    else {
        throw ModelException(M_CH_DW[0], "Get1DData",
                             "Output " + sk + " does not exist.");
    }
}

void DiffusiveWave::Get2DData(const char *key, int *nrows, int *ncols, float ***data) {
    string sk(key);
    *nrows = m_chNumber;
    if (StringMatch(sk, VAR_QCH[0])) {
        *data = m_qCh;
    } else if (StringMatch(sk, VAR_HCH[0])) {
        *data = m_hCh;
    } else {
        throw ModelException(M_CH_DW[0], "Get2DData",
                             "Output " + sk + " does not exist.");
    }

}

void DiffusiveWave::Set2DData(const char *key, int nrows, int ncols, float **data) {
    string sk(key);
    if (StringMatch(sk, Tag_FLOWIN_INDEX[0])) {
        m_flowInIndex = data;
    } else {
        throw ModelException(M_CH_DW[0], "Set2DData",
                             "Parameter " + sk + " does not exist.");
    }
}
