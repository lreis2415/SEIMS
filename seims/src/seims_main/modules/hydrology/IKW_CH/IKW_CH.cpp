#include "IKW_CH.h"
#include "text.h"
#include <cstdlib>
#include <fstream>
#include <queue>

//using namespace std;  // Avoid this statement! by lj.

ImplicitKinematicWave_CH::ImplicitKinematicWave_CH() :
    m_nCells(-1), m_chNumber(-1), m_dt(-1.0f), m_substeps(2),
    m_channelBaseflowInitialized(false),
    m_CellWidth(-1.0f), //m_layeringMethod(DOWNUP),
    m_sRadian(nullptr), m_direction(nullptr), m_reachDownStream(nullptr),
    m_chWidth(nullptr),
    m_qs(nullptr), m_hCh(nullptr), m_qCh(nullptr), m_prec(nullptr),
    m_qSubbasin(nullptr), m_qg(nullptr),
    m_flowLen(nullptr), m_qi(nullptr), m_streamLink(nullptr),
    m_sourceCellIds(nullptr),
    m_idUpReach(-1), m_qUpReach(0.f),
    m_qgDeep(0.f),
    m_idOutlet(-1)//, m_qsInput(nullptr)
{
}

ImplicitKinematicWave_CH::~ImplicitKinematicWave_CH(void) {
    Release2DArray(m_hCh);
    Release2DArray(m_qCh);
    Release2DArray(m_flowLen);

    Release1DArray(m_sourceCellIds);
    Release1DArray(m_qSubbasin);
}

//---------------------------------------------------------------------------
// modified from OpenLISEM
/** Newton Rapson iteration for new water flux in cell, based on Ven Te Chow 1987
\param qIn      summed Q new from upstream
\param qLast    current discharge in the cell
\param surplus        infiltration surplus flux (in m2/s), has value <= 0
\param alpha    alpha calculated in LISEM from before kinematic wave
\param dt   time step
\param dx   length of the cell corrected for slope
*/
float ImplicitKinematicWave_CH::GetNewQ(float qIn, float qLast, float surplus, float alpha, float dt, float dx) {
    /* Using Newton-Raphson Method */
    float ab_pQ, dtX, C;  //auxillary vars
    int count;
    float Qkx; //iterated discharge, becomes Qnew
    float fQkx; //function
    float dfQkx;  //derivative
    const float _epsilon = 1e-12f;
    const float beta = 0.6f;

    /* if no input then output = 0 */
    if ((qIn + qLast) <= -surplus * dx)//0)
    {
        //itercount = -1;
        return (0);
    }

    /* common terms */
    ab_pQ = alpha * beta * CalPow(((qLast + qIn) / 2), beta - 1);
    // derivative of diagonal average (space-time)

    dtX = dt / dx;
    C = dtX * qIn + alpha * CalPow(qLast, beta) + dt * surplus;
    //dt/dx*Q = m3/s*s/m=m2; a*Q^b = A = m2; surplus*dt = s*m2/s = m2
    //C is unit volume of water
    // first gues Qkx
    Qkx = (dtX * qIn + qLast * ab_pQ + dt * surplus) / (dtX + ab_pQ);

    // VJ 050704, 060830 infil so big all flux is gone
    //VJ 110114 without this de iteration cannot be solved for very small values
    if (Qkx < MIN_FLUX) {
        //itercount = -2;
        return (0);
    }

    Qkx = Max(Qkx, MIN_FLUX);

    count = 0;
    do {
        fQkx = dtX * Qkx + alpha * Power(Qkx, beta) - C;   /* Current k */
        dfQkx = dtX + alpha * beta * Power(Qkx, beta - 1);  /* Current k */
        Qkx -= fQkx / dfQkx;                                /* Next k */
        Qkx = Max(Qkx, MIN_FLUX);
        count++;
        //qDebug() << count << fQkx << Qkx;
    } while (Abs(fQkx) > _epsilon && count < MAX_ITERS_KW);

    if (Qkx != Qkx) {
        throw ModelException(M_IKW_CH[0], "GetNewQ", "Error in iteration!");
    }

    //itercount = count;
    return Qkx;
}

// end code form LISEM

bool ImplicitKinematicWave_CH::CheckInputData(void) {
    if (m_date <= 0) {
        throw ModelException(M_IKW_CH[0], "CheckInputData", "You have not set the Date variable.");
    }

    if (m_nCells <= 0) {
        throw ModelException(M_IKW_CH[0], "CheckInputData", "The cell number of the input can not be less than zero.");
    }

    if (m_dt <= 0) {
        throw ModelException(M_IKW_CH[0], "CheckInputData", "You have not set the TimeStep variable.");
    }

    if (m_CellWidth <= 0) {
        throw ModelException(M_IKW_CH[0], "CheckInputData", "You have not set the CellWidth variable.");
    }

    if (m_sRadian == nullptr) {
        throw ModelException(M_IKW_CH[0], "CheckInputData", "The parameter: RadianSlope has not been set.");
    }
    if (m_direction == nullptr) {
        throw ModelException(M_IKW_CH[0], "CheckInputData", "The parameter: flow direction has not been set.");
    }

    if (m_chWidth == nullptr) {
        throw ModelException(M_IKW_CH[0], "CheckInputData", "The parameter: CHWIDTH has not been set.");
    }
    if (m_streamLink == nullptr) {
        throw ModelException(M_IKW_CH[0], "CheckInputData", "The parameter: STREAM_LINK has not been set.");
    }

    if (m_prec == nullptr) {
        throw ModelException(M_IKW_CH[0], "CheckInputData", "The parameter: D_P(precipitation) has not been set.");
    }

    return true;
}

void ImplicitKinematicWave_CH:: InitialOutputs() {
    if (m_nCells <= 0) {
        throw ModelException(M_IKW_CH[0], "InitialOutputs", "The cell number of the input can not be less than zero.");
    }

    if (m_hCh == nullptr) {
        // find source cells the reaches
        m_sourceCellIds = new int[m_chNumber + 1];
        //m_qsInput = new float[m_chNumber+1];

        for (int i = 1; i <= m_chNumber; ++i) {
            m_sourceCellIds[i] = -1;
            //m_qsInput[i] = 0.f;
        }

        for (int i = 0; i < m_nCells; i++) {
            if (FloatEqual(m_streamLink[i], NODATA_VALUE)) {
                continue;
            }
            int reachId = (int) m_streamLink[i];
            bool isSource = true;
            for (int k = 1; k <= (int) m_flowInIdx[i][0]; ++k) {
                int flowInId = (int) m_flowInIdx[i][k];
                int flowInReachId = (int) m_streamLink[flowInId];
                if (flowInReachId == reachId) {
                    isSource = false;
                    break;
                }
            }

            if ((int) m_flowInIdx[i][0] == 0) {
                isSource = true;
            }

            if (isSource) {
                m_sourceCellIds[reachId] = i;
            }
        }

        //for(int i = 0; i < m_chNumber; i++)
        //    cout << m_sourceCellIds[i] << endl;

        // get the cells in reaches according to flow direction
        for (int iCh = 1; iCh <= m_chNumber; iCh++) {
            std::queue<int> q;
            int iCell = m_sourceCellIds[iCh];

            if (iCell < 0) continue; // invalid source cell

            int reachId = (int)m_streamLink[iCell]; //get current reachID
            q.push(iCell); // push source cell

            while (!q.empty())
            {
                int curCell = q.front();
                q.pop(); // dequeue the first cell

                if ((int)m_streamLink[curCell] != reachId) {
                    continue; //skip the cell not belong to this reach
                }
                m_reachs[iCh].push_back(curCell); // add the cell to the reach list
                int num_outflows = m_flowOutIdx[curCell][0];
                for (int k = 1; k <= num_outflows; ++k)
                {
                    int nextCell = m_flowOutIdx[curCell][k]; //get downstream cell
                    if (nextCell >= 0)
                    {
                        q.push(nextCell);
                    }
                }
            }

            /*while ((int)m_streamLink[iCell] == reachId ) {
                m_reachs[iCh].push_back(iCell);

                if(m_flowOutIdx[iCell][1] <0 )
                {
                    break;
                }

                iCell = (int)m_flowOutIdx[iCell][1];
            }*/
        }

        m_hCh = new float *[m_chNumber + 1];
        m_qCh = new float *[m_chNumber + 1];

        //m_flowLen = new float *[m_chNumber + 1];

        m_qSubbasin = new float[m_chNumber + 1];
        for (int i = 1; i <= m_chNumber; ++i) {
            int n = CVT_INT(m_reachs[i].size());
            m_hCh[i] = new float[n];
            m_qCh[i] = new float[n];

            //m_flowLen[i] = new float[n];

            m_qSubbasin[i] = 0.f;

            //int id;
            //float dx;
            for (int j = 0; j < n; ++j) {
                m_hCh[i][j] = 0.f;
                m_qCh[i][j] = 0.f;
            }
        }

    }

}

void ImplicitKinematicWave_CH::initialOutputs2() {
    if (m_flowLen != nullptr) {
        return;
    }

    m_flowLen = new float *[m_chNumber + 1];

    for (int i = 1; i <= m_chNumber; ++i) {
        int n = m_reachs[i].size();
        m_flowLen[i] = new float[n];

        int id;
        float dx;
        for (int j = 0; j < n; ++j) {
            id = m_reachs[i][j];
            // slope length needs to be corrected by slope angle
            dx = m_CellWidth / cos(m_sRadian[id][1]);
            int dir = (int) m_direction[id];
            //if ((int) m_diagonal[dir] == 1) {
            if (DiagonalCCW[dir] == 1) {
                dx = SQ2 * dx;
            }
            m_flowLen[i][j] = dx;
        }
    }
}

void ImplicitKinematicWave_CH::InitializeChannelWithBaseflow() {
    if (m_channelBaseflowInitialized || m_qg == nullptr || m_hCh == nullptr ||
        m_qCh == nullptr || m_flowLen == nullptr) {
        return;
    }

    for (auto it = m_reachLayers.begin(); it != m_reachLayers.end(); ++it) {
        for (size_t i = 0; i < it->second.size(); ++i) {
            int reachIndex = it->second[i];
            vector<int>& vecCells = m_reachs[reachIndex];
            int n = CVT_INT(vecCells.size());
            if (n <= 0) {
                continue;
            }

            float qUp = 0.f;
            for (size_t iUp = 0; iUp < m_reachUpStream[reachIndex].size(); ++iUp) {
                int upReachId = m_reachUpStream[reachIndex][iUp];
                if (upReachId >= 0 && !m_reachs[upReachId].empty()) {
                    int upCellsNum = CVT_INT(m_reachs[upReachId].size());
                    qUp += m_qCh[upReachId][upCellsNum - 1];
                }
            }

            float localBaseflow = Max(m_qg[reachIndex], 0.f);
            for (int iCell = 0; iCell < n; ++iCell) {
                int id = vecCells[iCell];
                float qBase = qUp + localBaseflow * (iCell + 1.f) / n;
                if (qBase < MIN_FLUX) {
                    m_qCh[reachIndex][iCell] = 0.f;
                    m_hCh[reachIndex][iCell] = 0.f;
                    continue;
                }

                float sSin = CalSqrt(sin(Max(m_sRadian[id][1], 0.001f)));
                if (sSin < 1e-6f) {
                    sSin = 1e-6f;
                }
                float alpha = CalPow(m_reachN[reachIndex] / sSin *
                                     CalPow(m_chWidth[id], _2div3), 0.6f);
                m_qCh[reachIndex][iCell] = qBase;
                m_hCh[reachIndex][iCell] = alpha * CalPow(qBase, 0.6f) / m_chWidth[id];
            }
            m_qSubbasin[reachIndex] = m_qCh[reachIndex][n - 1];
        }
    }

    m_channelBaseflowInitialized = true;
}

void ImplicitKinematicWave_CH::ChannelFlow(int iReach, int iCell, int id, float qgEachCell) {
    float qUp = 0.f;

    if (iReach == 0 && iCell == 0) {
        qUp = m_qUpReach;
    }

    // inflow from upstream channel
    if (iCell == 0)// inflow of this cell is the last cell of the upstream reach
    {
        for (size_t i = 0; i < m_reachUpStream[iReach].size(); ++i) {
            int upReachId = m_reachUpStream[iReach][i];
            if (upReachId >= 0) {
                int upCellsNum = CVT_INT(m_reachs[upReachId].size());
                int upCellId = m_reachs[upReachId][upCellsNum - 1];
                qUp += m_qCh[upReachId][upCellsNum - 1];
            }
        }
        //cout << qUp << "\t";
    } else {
        qUp = m_qCh[iReach][iCell - 1];
    }

    float dx = m_flowLen[iReach][iCell];

    float qLatPrec = (m_prec[id] / m_substeps) / 1000.f * m_chWidth[id] * dx / m_dt;
    float qLatQg = qgEachCell;
    float qLatQs = m_qs[id][0] / m_substeps;
    float qLatQi = 0.f;
    float qLat = qLatPrec + qLatQg + qLatQs;
    if (m_qi != nullptr) {
        qLatQi = m_qi[id] / m_substeps;
        qLat += qLatQi;
    }

    if (qLat < MIN_FLUX && qUp < MIN_FLUX) {
        m_hCh[iReach][iCell] = 0.f;
        m_qCh[iReach][iCell] = 0.f;
        return;
    }

    qUp += qLat;

    float Perim = 2.f * m_hCh[iReach][iCell] + m_chWidth[id];

    float sSin = CalSqrt(sin(Max(m_sRadian[id][1], 0.001f)));
    if (sSin < 1e-6f) {
        sSin = 1e-6f;
    }
    float alpha = CalPow(m_reachN[iReach] / sSin * CalPow(Perim, _2div3), 0.6f);

    float qIn = m_qCh[iReach][iCell];

    m_qCh[iReach][iCell] = GetNewQ(qUp, qIn, 0.f, alpha, m_dt, dx);

    float hTest = m_hCh[iReach][iCell] + (qUp - m_qCh[iReach][iCell]) * m_dt / m_chWidth[id] / dx;
    float hNew = (alpha * CalPow(m_qCh[iReach][iCell], 0.6f)) / m_chWidth[id]; // unit m
    m_hCh[iReach][iCell] = (alpha * CalPow(m_qCh[iReach][iCell], 0.6f)) / m_chWidth[id]; // unit m
}

int ImplicitKinematicWave_CH::Execute() {
    //check the data
    CheckInputData();

    InitialOutputs();
    initialOutputs2();
    InitializeChannelWithBaseflow();
    //Output1DArray(m_size, m_prec, "f:\\p2.txt");
    //cout << m_reachLayers.size() << "\t" << m_chNumber << endl;

    float dtOriginal = m_dt;
    m_dt = dtOriginal / m_substeps;
    const char* diagEnv = std::getenv("SEIMS_IKW_CH_DIAG");
    const bool writeDiag = diagEnv != nullptr && string(diagEnv) != "0" && !m_outpath.empty();
    double diagPrecVolume = 0.0;
    double diagQsVolume = 0.0;
    double diagQiVolume = 0.0;
    double diagQgVolume = 0.0;
    int diagCount = 0;

    for (int sub = 0; sub < m_substeps; sub++) {
        for (auto it = m_reachLayers.begin(); it != m_reachLayers.end(); it++) {
            // There are not any flow relationship within each routing layer.
            // So parallelization can be done here.
            int nReaches = it->second.size();
            //cout << "Number of reaches: " << nReaches << endl;
            // the size of m_reachLayers (map) is equal to the maximum stream order
#pragma omp parallel for reduction(+:diagPrecVolume,diagQsVolume,diagQiVolume,diagQgVolume,diagCount)
            for (int i = 0; i < nReaches; ++i) {
                int reachIndex = it->second[i]; // index in the array, from 0
                //m_qsInput[reachIndex+1] = 0.f;

                vector<int> &vecCells = m_reachs[reachIndex];
                int n = vecCells.size();
                if (n <= 0) {
                    m_qSubbasin[reachIndex] = 0.f;
                    continue;
                }
                //cout << "\tNumber of cells in reach " << reachIndex << ": " << n << endl;
                float qgEachCell = 0.f;
                if (m_qg != nullptr) {
                    qgEachCell = m_qg[reachIndex] / n / m_substeps;
                }
                //cout << "\tGroundwater: " << qgEachCell << endl;
                for (int iCell = 0; iCell < n; ++iCell) {
                    int idCell = vecCells[iCell];
                    //m_qsInput[reachIndex+1] += m_qs[idCell];
                    if (writeDiag) {
                        float dx = m_flowLen[reachIndex][iCell];
                        float qLatPrec = (m_prec[idCell] / m_substeps) / 1000.f *
                                         m_chWidth[idCell] * dx / m_dt;
                        float qLatQs = m_qs[idCell][0] / m_substeps;
                        float qLatQi = m_qi != nullptr ? m_qi[idCell] / m_substeps : 0.f;
                        diagPrecVolume += qLatPrec * m_dt;
                        diagQsVolume += qLatQs * m_dt;
                        diagQiVolume += qLatQi * m_dt;
                        diagQgVolume += qgEachCell * m_dt;
                        diagCount++;
                    }
                    ChannelFlow(reachIndex, iCell, idCell, qgEachCell);
                }
                m_qSubbasin[reachIndex] = m_qCh[reachIndex][n - 1];
            }
        }
    }
    m_dt = dtOriginal;

    if (writeDiag) {
        const string diagPath = m_outpath + SEP + "IKW_CH_diag.csv";
        std::ifstream existing(diagPath.c_str());
        const bool needHeader = !existing.good();
        existing.close();
        std::ofstream fs(diagPath.c_str(), std::ios::out | std::ios::app);
        if (fs.is_open()) {
            if (needHeader) {
                fs << "time,prec_cms,qs_cms,qi_cms,qg_cms,total_cms,"
                   << "prec_m3,qs_m3,qi_m3,qg_m3,count\n";
            }
            const double precCms = diagPrecVolume / dtOriginal;
            const double qsCms = diagQsVolume / dtOriginal;
            const double qiCms = diagQiVolume / dtOriginal;
            const double qgCms = diagQgVolume / dtOriginal;
            fs << ConvertToString2(m_date) << ","
               << precCms << "," << qsCms << "," << qiCms << "," << qgCms << ","
               << (precCms + qsCms + qiCms + qgCms) << ","
               << diagPrecVolume << "," << diagQsVolume << ","
               << diagQiVolume << "," << diagQgVolume << ","
               << diagCount << "\n";
        }
    }

    return 0;
}

bool ImplicitKinematicWave_CH::CheckInputSizeChannel(const char *key, int n) {
    if (n <= 0) {
        //StatusMsg("Input data for "+string(key) +" is invalid. The size could not be less than zero.");
        return false;
    }
    if (m_chNumber != n) {
        if (m_chNumber <= 0) { m_chNumber = n; }
        else {
            //StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            return false;
        }
    }

    return true;
}

void ImplicitKinematicWave_CH::GetValue(const char *key, float *value) {
    string sk(key);
    //if (StringMatch(sk, VAR_QOUTLET)) { // TODO: clean up.
    //    auto it = m_reachLayers.end();
    //    it--;
    //    int reachId = it->second[0];
    //    int iLastCell = m_reachs[reachId].size() - 1;
    //    *value = m_qCh[reachId][iLastCell];
    //} else
    if (StringMatch(sk, VAR_QTOTAL[0])) {
        auto it = m_reachLayers.end();
        --it;
        int reachId = it->second[0];
        int iLastCell = CVT_INT(m_reachs[reachId].size()) - 1;
        *value = m_qCh[reachId][iLastCell] + m_qgDeep;
    }
    else {
        throw ModelException(M_IKW_CH[0], "GetValue",
                             "Output " + sk + " does not exist.");
    }
}

void ImplicitKinematicWave_CH::SetValue(const char *key, FLTPT value) {
    string sk(key);
    if (StringMatch(sk, Tag_CellWidth[0])) {
        m_CellWidth = value;
    } else {
        throw ModelException(M_IKW_CH[0], "SetValue",
                             "Parameter " + sk + " does not exist.");
    }

}

void ImplicitKinematicWave_CH::SetValue(const char* key, int value) {
    string sk(key);
    if (StringMatch(sk, Tag_HillSlopeTimeStep[0])) {
        m_dt = value;
    }
    else {
        throw ModelException(M_IKW_CH[0], "SetValue",
            "Parameter " + sk + " does not exist.");
    }

}

void ImplicitKinematicWave_CH::Set1DData(const char *key, int n, float *data) {
    string sk(key);

    if (StringMatch(sk, VAR_SBQG[0])) {
        m_qg = data;
        return;
    }

    CheckInputSize(M_IKW_CH[0], key, n, m_nCells);

    if (StringMatch(sk, VAR_FLOWDIR[0])) {
        m_direction = data;
    } else if (StringMatch(sk, VAR_PCP[0])) {
        m_prec = data;
    } else if (StringMatch(sk, VAR_QSOIL[0])) {
        m_qi = data;
    } else if (StringMatch(sk, VAR_CHWIDTH[0])) {
        m_chWidth = data;
    } else {
        throw ModelException(M_IKW_CH[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void ImplicitKinematicWave_CH::Set1DData(const char* key, int n, int* data) {
    CheckInputSize(M_IKW_CH[0], key, n, m_nCells);
    string sk(key);
    if (StringMatch(sk, VAR_STREAM_LINK[0])) {
        m_streamLink = data;
    } else {
        throw ModelException(M_IKW_CH[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void ImplicitKinematicWave_CH::Get1DData(const char *key, int *n, float **data) {
    string sk(key);
    *n = m_chNumber + 1;
    // TODO. Check.
    if (StringMatch(sk, VAR_QSUBBASIN[0])) {
        *data = m_qSubbasin;
    }
    else if (StringMatch(sk, VAR_QRECH[0])) {
        auto it = m_reachLayers.end();
        --it;
        int reachId = it->second[0];
        *data = m_qCh[reachId];
    } else {
        throw ModelException(M_IKW_CH[0], "Get1DData",
                             "Output " + sk + " does not exist.");
    }
}

void ImplicitKinematicWave_CH::Get2DData(const char *key, int *nrows, int *ncols, float ***data) {
    if (m_hCh == nullptr) { //|| m_qCh == nullptr
        InitialOutputs();
    }
    string sk(key);
    *nrows = m_chNumber + 1;
    //if (StringMatch(sk, VAR_QRECH)) {  //TODO QRECH is DT_array1D? LJ
        //*data = m_qCh;
    //}
    if (StringMatch(sk, VAR_HCH[0])) {
        *data = m_hCh;
    }
    else if (StringMatch(sk, VAR_QCH[0])) {
        *data = m_qCh;
    }
    else {
        throw ModelException(M_IKW_CH[0], "Get2DData",
                             "Output " + sk + " does not exist.");
    }
}

void ImplicitKinematicWave_CH::Set2DData(const char *key, int nrows, int ncols, FLTPT **data) {
    string sk(key);
    CheckInputSize(M_IKW_CH[0], key, nrows, m_nCells);
    if (StringMatch(sk, VAR_QOVERLAND[0])) {
        m_qs = data;
    } else if (StringMatch(sk, VAR_RadianSlope[0])) {
        m_sRadian = data;
    } else {
        throw ModelException(M_IKW_CH[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void ImplicitKinematicWave_CH::Set2DData(const char* key, int nrows, int ncols, int** data) {
    string sk(key);
    if (StringMatch(sk, Tag_FLOWIN_INDEX[0])) {
        m_flowInIdx = data;
    }
    else if (StringMatch(sk, Tag_FLOWOUT_INDEX[0])) {
        m_flowOutIdx = data;
        for (int i = 0; i < m_nCells; i++) {
            if (m_flowOutIdx[i][0] == 0 && m_flowOutIdx[i][1] < 0) {

                m_idOutlet = i;
                if (m_idOutlet < 0)
                {
                    throw ModelException(M_CH_DW[0], "Set2DData",
                        "m_idOutlet does not exist.");
                }
                break;
            }
        }
    }
    else {
        throw ModelException(M_IKW_CH[0], "Set1DData",
            "Parameter " + sk + " does not exist.");
    }
}

void ImplicitKinematicWave_CH::SetReaches(clsReaches *reaches) {
    if (nullptr == reaches) {
        throw ModelException(M_IKW_CH[0], "SetReaches",
                             "The reaches input can not to be nullptr.");
    }
    m_chNumber = reaches->GetReachNumber();

    if (nullptr == m_reachDownStream) reaches->GetReachesSingleProperty(REACH_DOWNSTREAM, &m_reachDownStream);
    if (nullptr == m_chWidth) reaches->GetReachesSingleProperty(REACH_WIDTH, &m_chWidth);
    if (nullptr == m_reachN) reaches->GetReachesSingleProperty(REACH_MANNING, &m_reachN);

    m_reachUpStream = reaches->GetUpStreamIDs();
    m_reachLayers = reaches->GetReachLayers();
}
