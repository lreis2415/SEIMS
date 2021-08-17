#include "DiffusiveWave.h"

#include "text.h"
//using namespace std;  // Avoid this statement! by lj.

DiffusiveWave::DiffusiveWave() :
    m_nCells(-1), m_chNumber(-1), m_dt(-1.0f), m_CellWidth(-1.0f),
    m_s0(NULL), m_direction(NULL), m_reachDownStream(NULL), m_reachN(nullptr),
    m_chWidth(NULL),
    m_qs(NULL), m_hCh(NULL), m_qCh(NULL), m_prec(NULL), m_qSubbasin(NULL),
    m_elevation(NULL),
    m_flowLen(NULL), m_qi(NULL), m_flowInIndex(nullptr), m_flowOutIndex(nullptr),
    m_streamLink(NULL),
    m_sourceCellIds(NULL), m_layeringMethod(UP_DOWN),
    m_idUpReach(-1), m_idOutlet(-1), m_qUpReach(0.f) {
}

DiffusiveWave::~DiffusiveWave(void) {
    //Release1DArray(m_reachId);
    //Release1DArray(m_streamOrder);
    //Release1DArray(m_reachDownStream);
    //Release1DArray(m_reachN);

    Release2DArray(m_chNumber, m_hCh);
    Release2DArray(m_chNumber, m_qCh);
    Release2DArray(m_chNumber, m_flowLen);
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
    if (m_s0 == NULL) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: slope has not been set.");
    }
    if (m_direction == NULL) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: flow direction has not been set.");
    }

    if (m_chWidth == NULL) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: CHWIDTH has not been set.");
    }
    if (m_streamLink == NULL) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: STREAM_LINK has not been set.");
    }

    if (m_prec == NULL) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: D_P(precipitation) has not been set.");
    }
    if (m_elevation == NULL) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: Elevation has not been set.");
    }

    return true;
}

//! Initial outputs
void DiffusiveWave:: InitialOutputs() {
    if (this->m_nCells <= 0) {
        throw ModelException(M_CH_DW[0], "initialOutputs", "The cell number of the input can not be less than zero.");
    }

    if (m_hCh == NULL) {
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
                iCell = (int) m_flowOutIndex[iCell];
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
                //if ((int) m_diagonal[dir] == 1) {
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
    if (iCell == 0)// inflow of this cell is the last cell of the upstream reach
    {
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
    if (m_qs != NULL) {
        qLat += m_qs[id] / dx;
    }
    if (m_qi != NULL) {
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
    //check the data
    CheckInputData();

    InitialOutputs();
    //Output1DArray(m_nCells, m_prec, "f:\\p2.txt");
    for (auto it = m_reachLayers.begin(); it != m_reachLayers.end(); it++) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int nReaches = it->second.size();
        // the size of m_reachLayers (map) is equal to the maximum stream order
#pragma omp parallel for
        for (int i = 0; i < nReaches; ++i) {
            int reachIndex = it->second[i]; // index in the array
            vector<int> &vecCells = m_reachs[reachIndex];
            int n = vecCells.size();
            for (int iCell = 0; iCell < n; ++iCell) {
                ChannelFlow(reachIndex, iCell, vecCells[iCell]);
            }
            m_qSubbasin[reachIndex] = m_qCh[reachIndex][n - 1];
        }
    }
    return 0;
}
/*
//! Check input size
bool DiffusiveWave::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        //this->StatusMsg("Input data for "+string(key) +" is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nCells != n) {
        if (this->m_nCells <= 0) { this->m_nCells = n; }
        else {
            //this->StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n << ". The origin size is " <<
                m_nCells << ".\n";
            throw ModelException(M_CH_DW[0], "CheckInputSize", oss.str());
        }
    }

    return true;
}
*/
//! Check input size channel
bool DiffusiveWave::CheckInputSizeChannel(const char *key, int n) {
    if (n <= 0) {
        //this->StatusMsg("Input data for "+string(key) +" is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_chNumber != n) {
        if (this->m_chNumber <= 0) { this->m_chNumber = n; }
        else {
            //this->StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            return false;
        }
    }

    return true;
}

//! Get value of Qoutlet. Deprecated. The Metadata function may need to be updated!
//void DiffusiveWave::GetValue(const char *key, float *value) {
//    string sk(key);
//    if (StringMatch(sk, VAR_QOUTLET)) {
//        auto it = m_reachLayers.end();
//        it--;
//        int reachId = it->second[0];
//        int iLastCell = m_reachs[reachId].size() - 1;
//        *value = m_qCh[reachId][iLastCell];
//        //*value = m_hToChannel[m_idOutlet];
//        //*value = m_qs[m_idOutlet];
//        //*value = m_qs[m_idOutlet] + m_qCh[reachId][iLastCell];
//    }
//}

//! Set value
void DiffusiveWave::SetValue(const char *key, float data) {
    string sk(key);
    if (StringMatch(sk, Tag_HillSlopeTimeStep[0])) {
        m_dt = data;
    } else if (StringMatch(sk, Tag_CellSize[0])) {
        m_nCells = (int) data;
    } else if (StringMatch(sk, Tag_CellWidth[0])) {
        m_CellWidth = data;
    } else if (StringMatch(sk, Tag_LayeringMethod[0])) {
        m_layeringMethod = (LayeringMethod) int(data);
    } else {
        throw ModelException(M_CH_DW[0], "SetValue", "Parameter " + sk
            + " does not exist. Please contact the module developer.");
    }
}

//! Set 1D data
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
    } else if (StringMatch(sk, Tag_FLOWOUT_INDEX_D8[0])) {
        m_flowOutIndex = data;
        for (int i = 0; i < m_nCells; i++) {
            if (m_flowOutIndex[i] < 0) {
                m_idOutlet = i;
                break;
            }
        }
    } else {
        throw ModelException(M_CH_DW[0], "Set1DData", "Parameter " + sk
            + " does not exist. Please contact the module developer.");
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

//! Get 1D data
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
        throw ModelException(M_CH_DW[0], "Get1DData", "Output " + sk
            + " does not exist in the current module. Please contact the module developer.");
    }
}

//! Get 2D data
void DiffusiveWave::Get2DData(const char *key, int *nRows, int *nCols, float ***data) {
    string sk(key);
    *nRows = m_chNumber;
    if (StringMatch(sk, VAR_QCH[0])) {
        *data = m_qCh;
    } else if (StringMatch(sk, VAR_HCH[0])) {
        *data = m_hCh;
    } else {
        throw ModelException(M_CH_DW[0], "Get2DData", "Output " + sk
            + " does not exist in the current module. Please contact the module developer.");
    }

}

//! Set 2D data
void DiffusiveWave::Set2DData(const char *key, int nrows, int ncols, float **data) {
    string sk(key);
    if (StringMatch(sk, Tag_FLOWIN_INDEX_D8[0])) {
        m_flowInIndex = data;
    } else {
        throw ModelException(M_CH_DW[0], "Set1DData", "Parameter " + sk
            + " does not exist. Please contact the module developer.");
    }
}
