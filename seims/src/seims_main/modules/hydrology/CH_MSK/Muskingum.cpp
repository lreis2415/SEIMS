#include "seims.h"
#include "Muskingum.h"

using namespace std;

Muskingum::Muskingum() : m_nCells(-1), m_chNumber(-1), m_dt(-1.0f), m_CellWidth(-1.0f), m_layeringMethod(UP_DOWN),
                         m_s0(nullptr), m_direction(nullptr), m_reachDownStream(nullptr), m_chWidth(nullptr),
                         m_qs(nullptr), m_qg(nullptr), m_qi(nullptr), m_chStorage(nullptr),
                         m_qCh(nullptr), m_qUpCh(nullptr),
                         m_prec(nullptr), m_qSubbasin(nullptr),
                         m_flowLen(nullptr), m_alpha(nullptr), m_streamLink(nullptr),
                         m_sourceCellIds(nullptr),
                         m_msk_x(0.2f), m_chS0(0.f),
                         m_idUpReach(-1), m_idOutlet(-1), m_qUpReach(0.f), m_beta(5.0f / 3), m_delta(1e-6f) {

}

Muskingum::~Muskingum() {
    /// reaches related variables will be released in ~clsReaches(). By lj, 2017-12-26.

    if (nullptr != m_chStorage) Release2DArray(m_chNumber, m_chStorage);
    if (nullptr != m_qUpCh) Release2DArray(m_chNumber, m_qUpCh);
    if (nullptr != m_qCh) Release2DArray(m_chNumber, m_qCh);
    if (nullptr != m_flowLen) Release2DArray(m_chNumber, m_flowLen);
    if (nullptr != m_alpha) Release2DArray(m_chNumber, m_alpha);

    if (nullptr != m_sourceCellIds) Release1DArray(m_sourceCellIds);
    if (nullptr != m_qSubbasin) Release1DArray(m_qSubbasin);
}

float Muskingum::GetDelta_t(float timeStep, float fmin, float fmax) {
    if (fmax >= timeStep) {
        return timeStep;
    }

    int i = int(timeStep / fmax);

    float dt = timeStep / i;
    if (dt > fmax) {
        dt = timeStep / (i + 1);
    }

    return dt;
}

void Muskingum::GetCofficients(float reachLength, float waterDepth, float s0, float v0, MuskWeights &weights) {

    //float x = 0.5f * (1 - waterDepth/(m_beta*reachLength*s0));
    //if (x < 0)
    //	x = 0.01f;
    float x = m_msk_x;
    //float v = sqrt(s0) * pow(waterDepth, TWO_THIRDS) / manning_n;
    //if (v < 0.2f)
    //	v = 0.2f;
    //else if (v > 3.f)
    //	v = 3.f;
    //float K = reachLength / (m_beta*v);
    float K = (4.64f - 3.64f * 0.7f) * reachLength / (m_beta * v0);
    //get delta t
    float min = 2.0f * K * x;
    float max = 2.0f * K * (1 - x);
    float delta_t = GetDelta_t(m_dt, min, max);
    weights.dt = delta_t;

    //get coefficient
    float temp = max + delta_t;
    weights.c1 = (delta_t - min) / temp;
    weights.c2 = (delta_t + min) / temp;
    weights.c3 = (max - delta_t) / temp;
    weights.c4 = 2 * delta_t / temp;

    //make sure any coefficient is positive
    if (weights.c1 < 0) {
        weights.c2 += weights.c1;
        weights.c1 = 0.0f;
    }
    if (weights.c3 < 0) {
        weights.c2 += weights.c1;
        weights.c3 = 0.0f;
    }
}

bool Muskingum::CheckInputData() {
    if (this->m_date <= 0) {
        throw ModelException(MID_CH_MSK, "CheckInputData", "You have not set the Date variable.");
        return false;
    }

    if (this->m_nCells <= 0) {
        throw ModelException(MID_CH_MSK, "CheckInputData", "The cell number of the input can not be less than zero.");
        return false;
    }

    if (this->m_dt <= 0) {
        throw ModelException(MID_CH_MSK, "CheckInputData", "You have not set the TimeStep variable.");
        return false;
    }

    if (this->m_CellWidth <= 0) {
        throw ModelException(MID_CH_MSK, "CheckInputData", "You have not set the CellWidth variable.");
        return false;
    }

    if (nullptr == m_s0) {
        throw ModelException(MID_CH_MSK, "CheckInputData", "The parameter: slope has not been set.");
    }
    if (nullptr == m_direction) {
        throw ModelException(MID_CH_MSK, "CheckInputData", "The parameter: flow direction has not been set.");
    }
    if (nullptr == m_qs) {
        throw ModelException(MID_CH_MSK, "CheckInputData", "The parameter: H_TOCHANNEL has not been set.");
    }

    if (nullptr == m_chWidth) {
        throw ModelException(MID_CH_MSK, "CheckInputData", "The parameter: CHWIDTH has not been set.");
    }
    if (nullptr == m_streamLink) {
        throw ModelException(MID_CH_MSK, "CheckInputData", "The parameter: STREAM_LINK has not been set.");
    }

    if (nullptr == m_prec) {
        throw ModelException(MID_CH_MSK, "CheckInputData", "The parameter: D_P(precipitation) has not been set.");
    }

    return true;
}

void Muskingum::initialOutputs() {
    if (this->m_nCells <= 0) {
        throw ModelException(MID_CH_MSK, "initialOutputs", "The cell number of the input can not be less than zero.");
    }

    if (nullptr == m_chStorage) {
        // find source cells the reaches
        Initialize1DArray(m_chNumber, m_sourceCellIds, -1);
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

        m_chStorage = new float *[m_chNumber];
        m_qCh = new float *[m_chNumber];
        m_qUpCh = new float *[m_chNumber];
        m_flowLen = new float *[m_chNumber];
        m_alpha = new float *[m_chNumber];
        m_qSubbasin = new float[m_chNumber];
        for (int i = 0; i < m_chNumber; ++i) {
            int n = m_reachs[i].size();
            m_chStorage[i] = new float[n];
            m_qCh[i] = new float[n];
            m_qUpCh[i] = new float[n];
            m_flowLen[i] = new float[n];
            m_alpha[i] = new float[n];
            m_qSubbasin[i] = 0.f;

            int id;
            float dx;
            vector<MuskWeights> *vecWeights = new vector<MuskWeights>();
            for (int j = 0; j < n; ++j) {

                m_qCh[i][j] = 0.f;
                m_qUpCh[i][j] = 0.f;

                id = m_reachs[i][j];
                m_s0[id] = m_s0[id];
                if (m_s0[id] < MINI_SLOPE) {
                    m_s0[id] = MINI_SLOPE;
                }

                // slope length needs to be corrected by slope angle
                dx = m_CellWidth / cos(atan(m_s0[id]));
                int dir = (int) m_direction[id];
                //if ((int) m_diagonal[dir] == 1) {
                if (DiagonalCCW[dir] == 1) {
                    dx = SQ2 * dx;
                }
                m_flowLen[i][j] = dx;

                m_chStorage[i][j] = m_chS0 * m_flowLen[i][j];

            }
        }
    }
}

bool Muskingum::ChannelFlow(int iReach, int iCell, int id, float qgEachCell) {
    float qUpNew = 0.f;

    if (iReach == 0 && iCell == 0) {
        qUpNew = m_qUpReach;
    }

    // inflow from upstream channel
    if (iCell == 0)// inflow of this cell is the last cell of the upstream reach
    {
        for (size_t i = 0; i < m_reachUpStream[iReach].size(); ++i) {
            int upReachId = m_reachUpStream[iReach][i];
            if (upReachId >= 0) {
                int upCellsNum = m_reachs[upReachId].size();
                int upCellId = m_reachs[upReachId][upCellsNum - 1];
                qUpNew += m_qCh[upReachId][upCellsNum - 1];
            }
        }
    } else {
        qUpNew = m_qCh[iReach][iCell - 1];
    }

    float dx = m_flowLen[iReach][iCell];
    float area = dx * m_chWidth[id];

    // lateral flow
    float qLat = m_prec[id] / 1000.f * area / m_dt;

    if (nullptr != m_qs) {
        qLat += m_qs[id];
    }

    if (nullptr != m_qi) {
        qLat += m_qi[id];
    }

    qLat += qgEachCell;

    //if (iReach == 364)
    //	cout << qLat << qUpNew << endl;
    qUpNew += qLat;

    float waterDepth = m_chStorage[iReach][iCell] / area;

    MuskWeights weights;
    GetCofficients(dx, waterDepth, m_s0[id], m_v0[iReach], weights);
    int stepCount = int(m_dt / weights.dt);
    //start the loop for every delta t
    float qNew = 0.f;

    float q = 0.f;
    for (int i = 0; i < stepCount; i++) {

        //GetCofficients(dx, waterDepth, m_s0[i], m_v0[iReach], weights);
        // unit discharge
        qNew = weights.c1 * qUpNew + weights.c2 * m_qUpCh[iReach][iCell] + weights.c3 * qNew;

        if (qNew != qNew) {
            cout << "Error in function Muskingum::ChannelFlow. \n";
            cout << "The weights are: " << weights.c1 << ", " << weights.c2 << ", " << weights.c3 << endl;
            cout << "qUpNew: " << qUpNew << "\tqUpPre: " << m_qUpCh[iReach][iCell] << "\tqNew: " << qNew << endl;
            //throw ModelException(MID_CH_MSK, "ChannelFlow", "Error occurred.");
            return false;
        }

        float tmp = m_chStorage[iReach][iCell] + (qUpNew - qNew) * weights.dt;
        if (tmp > 0.f) {
            m_chStorage[iReach][iCell] = tmp;
            m_qCh[iReach][iCell] = qNew;
        } else {
            m_qCh[iReach][iCell] = m_chStorage[iReach][iCell] / weights.dt;
            m_chStorage[iReach][iCell] = 0.f;
        }

        m_qUpCh[iReach][iCell] = qUpNew;
        q += m_qCh[iReach][iCell];
    }

    q /= stepCount;
    m_qCh[iReach][iCell] = q;
    return true;
}

int Muskingum::Execute() {
    //check the data
    initialOutputs();
    CheckInputData();

    //Output1DArray(m_nCells, m_prec, "f:\\p2.txt");
    map<int, vector<int> >::iterator it;
    //cout << "reach layer number: " << m_reachLayers.size() << endl;
    for (it = m_reachLayers.begin(); it != m_reachLayers.end(); it++) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int nReaches = it->second.size();
        //cout << "reach number:" << nReaches << endl;
        // the size of m_reachLayers (map) is equal to the maximum stream order
        bool exceptionOccurredFlag = false;
#pragma omp parallel for
        for (int i = 0; i < nReaches; ++i) {
            int reachIndex = it->second[i]; // index in the array
            vector<int> &vecCells = m_reachs[reachIndex];
            int n = vecCells.size();
            float qgEachCell = m_qg[reachIndex + 1] / n;
            for (int iCell = 0; iCell < n; ++iCell) {
#pragma omp flush (exceptionOccurredFlag)
                if (!exceptionOccurredFlag) {
                    exceptionOccurredFlag = ChannelFlow(reachIndex, iCell, vecCells[iCell], qgEachCell);
#pragma omp flush (exceptionOccurredFlag)
                }
                m_qSubbasin[reachIndex] = m_qCh[reachIndex][n - 1];
            }
        }
        if (exceptionOccurredFlag) {
            throw ModelException(MID_CH_MSK, "ChannelFlow", "Error occurred.");
        }
    }
    return 0;
}

bool Muskingum::CheckInputSize(const char *key, int n) {
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
            throw ModelException(MID_CH_MSK, "CheckInputSize", oss.str());
        }
    }

    return true;
}

bool Muskingum::CheckInputSizeChannel(const char *key, int n) {
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

void Muskingum::GetValue(const char *key, float *value) {
    string sk(key);
    if (StringMatch(sk, VAR_QOUTLET)) {
        auto it = m_reachLayers.end();
        it--;
        int reachId = it->second[0];
        //int iLastCell = m_reachs[reachId].size() - 1;
        //*value = m_qCh[reachId][iLastCell];
        *value = m_qSubbasin[reachId];

        //*value = m_hToChannel[m_idOutlet];
        //*value = m_qs[m_idOutlet];
        //*value = m_qs[m_idOutlet] + m_qCh[reachId][iLastCell];
    }
}

void Muskingum::SetValue(const char *key, float data) {
    string sk(key);
    if (StringMatch(sk, Tag_HillSlopeTimeStep)) {
        m_dt = data;
    } else if (StringMatch(sk, Tag_CellSize)) {
        m_nCells = (int) data;
    } else if (StringMatch(sk, Tag_CellWidth)) {
        m_CellWidth = data;
    } else if (StringMatch(sk, Tag_LayeringMethod)) {
        m_layeringMethod = (LayeringMethod) int(data);
    } else if (StringMatch(sk, VAR_CHS0)) {
        m_chS0 = data;
    } else if (StringMatch(sk, VAR_MSK_X)) {
        m_msk_x = data;
    } else if (StringMatch(sk, VAR_OMP_THREADNUM)) {
        SetOpenMPThread((int) data);
    } else {
        throw ModelException(MID_CH_MSK, "SetValue", "Parameter " + sk
            + " does not exist. Please contact the module developer.");
    }

}

void Muskingum::Set1DData(const char *key, int n, float *data) {
    string sk(key);
    if (StringMatch(sk, VAR_SBQG)) {
        m_qg = data;
        return;
    }

    //check the input data
    CheckInputSize(key, n);

    if (StringMatch(sk, VAR_SLOPE)) {
        m_s0 = data;
    } else if (StringMatch(sk, VAR_FLOWDIR)) {
        m_direction = data;
    } else if (StringMatch(sk, VAR_PCP)) {
        m_prec = data;
    } else if (StringMatch(sk, VAR_QSOIL)) {
        m_qi = data;
    } else if (StringMatch(sk, VAR_QOVERLAND)) {
        m_qs = data;
    } else if (StringMatch(sk, VAR_CHWIDTH)) {
        m_chWidth = data;
    } else if (StringMatch(sk, VAR_STREAM_LINK)) {
        m_streamLink = data;
    } else if (StringMatch(sk, Tag_FLOWOUT_INDEX_D8)) {
        m_flowOutIndex = data;
        for (int i = 0; i < m_nCells; i++) {
            if (m_flowOutIndex[i] < 0) {
                m_idOutlet = i;
                break;
            }
        }
    } else {
        throw ModelException(MID_CH_MSK, "Set1DData", "Parameter " + sk
            + " does not exist. Please contact the module developer.");
    }

}

void Muskingum::Get1DData(const char *key, int *n, float **data) {
    string sk(key);
    *n = m_chNumber;
    if (StringMatch(sk, VAR_QSUBBASIN)) {
        *data = m_qSubbasin;
    } else {
        throw ModelException(MID_CH_MSK, "Get1DData", "Output " + sk + " does not exist.");
    }

}

void Muskingum::Get2DData(const char *key, int *nRows, int *nCols, float ***data) {
    string sk(key);
    *nRows = m_chNumber;
    if (StringMatch(sk, VAR_QCH)) {
        *data = m_qCh;
    } else if (StringMatch(sk, VAR_CHST)) {
        *data = m_chStorage;
    } else {
        throw ModelException(MID_CH_MSK, "Get2DData", "Output " + sk
            + " does not exist in the current module. Please contact the module developer.");
    }
}

void Muskingum::Set2DData(const char *key, int nrows, int ncols, float **data) {
    string sk(key);
    if (StringMatch(sk, Tag_FLOWIN_INDEX_D8)) {
        m_flowInIndex = data;
    } else {
        throw ModelException(MID_CH_MSK, "Set1DData", "Parameter " + sk
            + " does not exist. Please contact the module developer.");
    }
}

void Muskingum::SetReaches(clsReaches *reaches) {
    if (nullptr == reaches) {
        throw ModelException(MID_CH_MSK, "SetReaches", "The reaches input can not to be NULL.");
    }
    m_chNumber = reaches->GetReachNumber();

    if (nullptr == m_reachDownStream) reaches->GetReachesSingleProperty(REACH_DOWNSTREAM, &m_reachDownStream);
    if (nullptr == m_v0) reaches->GetReachesSingleProperty(REACH_V0, &m_v0);

    m_reachUpStream = reaches->GetUpStreamIDs();
    m_reachLayers = reaches->GetReachLayers(m_layeringMethod);
}
