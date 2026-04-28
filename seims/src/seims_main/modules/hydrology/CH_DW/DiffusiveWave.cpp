#include "DiffusiveWave.h"
#include "text.h"
#include <queue>
#include <fstream>
#include <unordered_set>
#include <set>
#include <cstdlib>
//using namespace std;

namespace {
std::unordered_set<int> g_debugCells;
bool g_debugCellsLoaded = false;

void LoadDebugCellsOnce() {
    if (g_debugCellsLoaded) {
        return;
    }
    g_debugCellsLoaded = true;

    const char* path = std::getenv("SEIMS_DEBUG_CELLS_FILE");
    if (path == nullptr || *path == '\0') {
        return;
    }

    std::ifstream fin(path);
    if (!fin.is_open()) {
        return;
    }

    int id = 0;
    while (fin >> id) {
        g_debugCells.insert(id);
    }
}


}

DiffusiveWave::DiffusiveWave() :
    m_nCells(-1),  m_dt(-1.0f), m_CellWidth(-1.0f), m_chNumber(-1),
    m_s0(nullptr), m_direction(nullptr), m_reachDownStream(nullptr), m_reachN(nullptr),
    m_chWidth(nullptr), m_chDepth(nullptr), m_Chs0_perc(NODATA_VALUE), m_chSlope(nullptr),
    m_qs(nullptr), m_hCh(nullptr), m_qCh(nullptr), m_prec(nullptr), m_netPcp(nullptr), m_qSubbasin(nullptr),m_qsCh(nullptr),m_qiCh(nullptr),
    m_elevation(nullptr),
    m_flowLen(nullptr), m_qi(nullptr), m_qg(nullptr), m_flowInIndex(nullptr), m_flowOutIdx(nullptr),
    m_streamLink(nullptr),
    m_sourceCellIds(nullptr),
    m_idUpReach(-1), m_idOutlet(-1), m_qUpReach(0.f) {
}

DiffusiveWave::~DiffusiveWave() {
    //Release1DArray(m_reachId);
    //Release1DArray(m_streamOrder);
    //Release1DArray(m_reachDownStream);
    //Release1DArray(m_reachN);
    if (nullptr != m_reachN) Release1DArray(m_reachN);

    Release2DArray(m_hCh);
    Release2DArray(m_qCh);
    Release2DArray(m_flowLen);
    Release1DArray(m_sourceCellIds);
    Release1DArray(m_qSubbasin);

    //estimate qs and qi of the outlet
    Release1DArray(m_qiCh);
    Release1DArray(m_qsCh);
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
    if (m_chDepth == nullptr) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: CHDEPTH has not been set.");
    }
    if (m_chSlope == nullptr) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: CHSLOPE has not been set.");
    }
    if (m_streamLink == nullptr) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: STREAM_LINK has not been set.");
    }

    if (m_prec == nullptr) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: D_P(precipitation) has not been set.");
    }
    if (m_netPcp == nullptr) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: D_P(precipitation) has not been set.");
    }
    if (m_elevation == nullptr) {
        throw ModelException(M_CH_DW[0], "CheckInputData", "The parameter: Elevation has not been set.");
    }
    CHECK_NODATA(M_MUSK_CH[0], m_Chs0_perc);
    return true;
}

//! Initial outputs
void DiffusiveWave:: InitialOutputs() {
    if (m_nCells <= 0) {
        throw ModelException(M_CH_DW[0], "InitialOutputs", "The cell number of the input can not be less than zero.");
    }

    /*if (m_stormMode)
    {
        struct tm* date_info = new tm();
        LocalTime(m_date, date_info);
        int hour = date_info->tm_hour;
        int min = date_info->tm_min;

        if (hour == 8 && min == 0)
        {
            m_qCh = nullptr;
            m_hCh = nullptr;
            m_qSubbasin = nullptr;
            m_flowLen = nullptr;
            m_qUpReach = 0.f;
            m_qs = 0;
            m_qi = 0;
        }
    }*/


    if (m_qCh == nullptr) {

        // find source cells the reaches
        m_sourceCellIds = new int[m_chNumber + 1];
        for (int i = 1; i <= m_chNumber; ++i) {
            m_sourceCellIds[i] = -1;
        }
        /*int reachIndex = 0;*/
        for (int i = 0; i < m_nCells; i++) {
            //std::cout << "the total number of Cells is  " << m_nCells << endl;
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
                //if (m_idToIndex.find(reachId) == m_idToIndex.end())
                //{
                //    m_idToIndex.insert(pair<int, int>(reachId, reachIndex));
                //}
                /*int reachIndex = m_idToIndex[reachId];*/
                m_sourceCellIds[reachId] = i;

/*                reachIndex++;    */       
            }
        }


        m_hCh = new float *[m_chNumber + 1];
        m_qCh = new float *[m_chNumber + 1];


        m_flowLen = new float *[m_chNumber + 1];

        m_qSubbasin = new float[m_chNumber + 1];
        memset(m_qSubbasin, 0, sizeof(float) * (m_chNumber + 1));

        //estimate qs and qi of the outlet
        m_qiCh = new float[m_chNumber + 1];
        memset(m_qiCh, 0, sizeof(float) * (m_chNumber + 1));
        m_qsCh = new float[m_chNumber + 1];
        memset(m_qsCh, 0, sizeof(float) * (m_chNumber + 1));


        for (int i = 1; i <= m_chNumber; ++i) {
            int n = m_reachs[i].size();
            m_hCh[i] = new float[n];
            m_qCh[i] = new float[n];
            memset(m_qCh[i], 0, sizeof(float) * n);

            m_flowLen[i] = new float[n];

            m_qSubbasin[i] = 0.f;



            // initial channel depth
            float h0 = m_chDepth[i] * m_Chs0_perc;
            float area = m_chWidth[i] * h0;



            int id;
            float s0, dx;
            for (int j = 0; j < n; ++j) {


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

                m_hCh[i][j] = h0;
            }
        }

    }
}

//! Channel flow
void DiffusiveWave::ChannelFlow(int iReach, int iCell, int id, float qgEachCell) {
   
    //debug
    //int TARGET_DEBUG_ID = 4197;
    //bool isDebug = (id == TARGET_DEBUG_ID) || (iReach == 4 && iCell == 0);
    //if (isDebug) {
    //    std::cout << "\n========== [DEBUG] Reach:" << iReach << " | Cell:" << iCell << " | ID:" << id << " ==========" << std::endl;
    //    std::cout << std::fixed << std::setprecision(4); // 设置小数位数
    //}

    static const std::set<int> targetCells = { 1944,2052,2159,2158,2157,2156,
        2264,2372,2481,2591,2590,2589,
        2588,2698,2810,2923,2922,2921,
        3036,3035,3034,3148,3147,3146,3145,
        3144,3143,3142,3141,3025,3024,3023,
        3022,3021,3020,3134,3247,3361,3472,
        3582,3691,3797,3901,4006,4110,4209,
        4208,4207,4299,4391,4481,4569,4653,
        4735,4816,4896,4975,5053,5128,5200,
        5199,5267,5336,5335,5404,5472,5471,
        5530,5584,5635,5686,5737,5786,5833,
        5832,5831,5830,5829,5874,5913,5939,
        5950,5956,5962,5966,5970,5973,5975 };
    bool isInterestCell = targetCells.find(id) != targetCells.end();

    
    
    float qUp = 0.f;
    float hUp = 0.f; // h for the previous time step

    if (iReach == 0 && iCell == 0) {
        qUp = m_qUpReach;
    }

    // inflow from upstream channel
    if (iCell == 0) { // inflow of this cell is the last cell of the upstream reach
        m_qsCh[iReach] = 0.f;
        m_qiCh[iReach] = 0.f;

        for (size_t i = 0; i < m_reachUpStream[iReach].size(); ++i) {

            

            int upReachId = m_reachUpStream[iReach][i];
            if (upReachId >= 0) {
                int upCellsNum = m_reachs[upReachId].size();
                int upCellId = m_reachs[upReachId][upCellsNum - 1];


                qUp += m_qCh[upReachId][upCellsNum - 1];

                m_qsCh[iReach] += m_qsCh[upReachId];
                m_qiCh[iReach] += m_qiCh[upReachId];

                float hWater = m_elevation[upCellId] + m_hCh[upReachId][upCellsNum - 1];
                if (hWater > hUp) {
                    hUp = hWater;
                }
                //debug
                //if (isDebug) {
                //    std::cout << "  -> Upstream Reach Found: ID=" << upReachId
                //        << " Q_out=" << m_qCh[upReachId][upCellsNum - 1]
                //        << " H_water=" << hWater << std::endl;
                //}
            }
        }
    } else {
        qUp = m_qCh[iReach][iCell - 1];
        hUp = m_hCh[iReach][iCell - 1];
    }

    //float b = 0.6f;
    float h = m_hCh[iReach][iCell];
    float dx = m_flowLen[iReach][iCell];

    float local_qs = (m_qs != nullptr) ? m_qs[id][0] : 0.f;
    float local_qi = (m_qi != nullptr) ? m_qi[id] : 0.f;

    ////debug
    //bool isInvalid = false;

    //if (local_qi < -0.0001f) {
    //    std::cout << "[ERROR] Negative Qi Detected! (Possible NoData)" << std::endl;
    //    isInvalid = true;
    //}
    //else if (std::isnan(local_qi)) {
    //    std::cout << "[ERROR] NaN Qi Detected! (Math Error)" << std::endl;
    //    isInvalid = true;
    //}
    //else if (local_qi > 1e10f) { 
    //    std::cout << "[ERROR] Huge Qi Detected! (Possible Uninitialized Memory)" << std::endl;
    //    isInvalid = true;
    //}


    //if (isInvalid) {
    //    std::cout << "  -> Location: Reach " << iReach << ", CellIndex " << iCell << ", CellID " << id << std::endl;
    //    std::cout << "  -> Value: " << local_qi << std::endl;

    //    if (m_qi == nullptr) {
    //        std::cout << "  -> m_qi pointer is NULL (Logic handled, but value shouldn't be random)" << std::endl;
    //    }
    //    else {
    //        std::cout << "  -> Raw m_qi[" << id << "] = " << m_qi[id] << std::endl;
    //    }
    //}

    float rain_flux = 0.f;

    float qLat_rain = m_prec[id] / 1000.f / m_dt * m_chWidth[id];
    float qLat_qs = (m_qs != nullptr) ? (m_qs[id][0] / dx) : 0.f;
    float qLat_qi = (m_qi != nullptr) ? (m_qi[id] / dx) : 0.f;
    float qLat_qg = (qgEachCell > 0.f) ? (qgEachCell / dx) : 0.f;
    float qLat = qLat_rain + qLat_qs + qLat_qi + qLat_qg;

    rain_flux = qLat_rain * dx;

    m_qsCh[iReach] += local_qs + rain_flux;
    m_qiCh[iReach] += local_qi;
    //debug
    //if (isDebug) {
    //    std::cout << "[1. Inflows]" << std::endl;
    //    std::cout << "  qUp (Upstream)   = " << qUp << std::endl;
    //    std::cout << "  hUp (Up-Head/Dep)= " << hUp << std::endl;
    //    std::cout << "  qLat (Lateral)   = " << qLat  << std::endl;
    //    std::cout << "  Current Depth h  = " << h << std::endl;
    //    std::cout << "  Flow Len dx      = " << dx << std::endl;
    //}

    if (qLat < MIN_FLUX && qUp < MIN_FLUX) {
        m_hCh[iReach][iCell] = 0.f;
        m_qCh[iReach][iCell] = 0.f;
        //if (isDebug) std::cout << "[Result] Dry condition. Q=0, H=0" << std::endl;
        if (isInterestCell) {
            /*std::cout << "[TRACE_CSV],Step,Step,DRY_SKIP" 
                << ",Module,ChannelFlow"
                << ",Reach," << iReach
                << ",Cell," << id
                << ",CellIdx," << iCell
                << ",Q_Up," << qUp
                << ",Q_Lat_Total," << (qLat * dx)
                << ",Q_Out,0"
                << ",H_Ch,0"
                << std::endl;*/
        }
        return;
    }

    float perim = 2.f * h + m_chWidth[iReach];
    float sf = (hUp - m_elevation[id] - h) / dx;

    //debug
    //if (isDebug) {
    //    std::cout << "[2. Physics]" << std::endl;
    //    std::cout << "  Elevation Cur    = " << m_elevation[id] << std::endl;
    //    std::cout << "  Slope (sf) Raw   = " << sf << std::endl;
    //}

    if (sf < MINI_SLOPE) {
        sf = MINI_SLOPE;
    }
    float c = 1.f / 3600.f * m_reachN[iReach] * CalPow(perim, _2div3) / CalSqrt(sf);
    c = CalPow(c, 0.6f);

    //debug
    //if (isDebug) {
    //    std::cout << "  Slope (sf) Used  = " << sf << std::endl;
    //    std::cout << "  Perimeter        = " << perim << std::endl;
    //    std::cout << "  Conductance (c)  = " << c << std::endl;
    //}

    float d = 1.f;
    int counter = 0;
    float qLast = m_qCh[iReach][iCell];
    float qNew = qLast;
    if (qNew < MIN_FLUX) {
        qNew = qLat;
    }

    //debug
    //if (isDebug) std::cout << "[3. Iteration]" << std::endl;

    while (abs(d) > MIN_FLUX && counter < 10) {
        d = (qNew * m_dt / dx + c * Power(qNew, 0.6f) - qUp * m_dt / dx - c * Power(qLast, 0.6f) - qLat * m_dt) /
            (m_dt / dx + c * 0.6f / Power(qNew, 0.4f));
        //debug
        //if (isDebug) {
        //    std::cout << "  Iter " << counter << ": qNew=" << qNew << ", d=" << d << std::endl;
        //}

        //if(d != d)
        //	int test = 1;
        qNew -= d;
        counter++;
    }

    if (qNew < 0.f) {
        qNew = 0.f;
    }

    float qAvail = m_hCh[iReach][iCell] * m_chWidth[iReach] * dx / m_dt + qLat * dx + qUp;

    //debug
    //if (isDebug) {
    //    std::cout << "[4. Balance Check]" << std::endl;
    //    std::cout << "  qCalculated = " << qNew << std::endl;
    //    std::cout << "  qAvailable  = " << qAvail << std::endl;
    //}

    if (qNew > qAvail) {
        m_qCh[iReach][iCell] = qAvail;
        m_hCh[iReach][iCell] = 0.f;
    } else {
        m_qCh[iReach][iCell] = qNew;//m_hCh[iReach][iCell]
        m_hCh[iReach][iCell] = c * CalPow(qNew, 0.6f) / m_chWidth[iReach];
        //float hh = (qUp + qLat*dx - qNew)*m_dt/(m_chWidth[iReach]*dx) + m_hCh[iReach][iCell];
    }



    if (isInterestCell) {
        float qLat_total = qLat * dx;
        /*std::cout << "[TRACE_CSV],Step,Step,UNKNOWN"
            << ",Module,ChannelFlow"
            << ",Reach," << iReach
            << ",Cell," << id
            << ",CellIdx," << iCell
            << ",Q_Up," << qUp
            << ",Q_Lat_Total," << qLat_total
            << ",Q_Lat_Rain," << (qLat_rain * dx)
            << ",Q_Lat_QS," << (qLat_qs * dx)
            << ",Q_Lat_QI," << (qLat_qi * dx)
            << ",Q_Lat_QG," << (qLat_qg * dx)
            << ",Q_Out," << m_qCh[iReach][iCell]
            << ",H_Ch," << m_hCh[iReach][iCell]
            << std::endl;*/
    }
    //debug
    //if (isDebug) {
    //    std::cout << "[Final Output]" << std::endl;
    //    std::cout << "  Q_out = " << m_qCh[iReach][iCell] << std::endl;
    //    std::cout << "  H_new = " << m_hCh[iReach][iCell] << std::endl;
    //    std::cout << "==========================================\n" << std::endl;
    //}

    int nCells = m_reachs[iReach].size();
    if (iCell == nCells - 1) {
        float output_flow = m_qCh[iReach][iCell];

        float total_input = m_qsCh[iReach] + m_qiCh[iReach];

        float ratio = 0.f;
        if (total_input > MIN_FLUX) {
            ratio = output_flow / total_input;
        }
        else if (output_flow > MIN_FLUX) {
            ratio = 0.f;
        }
        m_qsCh[iReach] *= ratio;
        m_qiCh[iReach] *= ratio;
    }

}

//! Main execute function
int DiffusiveWave::Execute() {
    CheckInputData();
    InitialOutputs();
    float total_qs = 0.0;
    float total_qi = 0.0;
    for (auto it = m_reachLayers.begin(); it != m_reachLayers.end(); ++it) {
        // There are no flow relationships within each routing layer.
        //   So parallelization can be done here.
        int nReaches = it->second.size();
        // the size of m_reachLayers (map) is equal to the maximum stream order
//#pragma omp parallel for
       
        for (int i = 0; i < nReaches; ++i) {
            int reachIndex = it->second[i]; // index in the array           
            vector<int> &vecCells = m_reachs[reachIndex];
            int n = vecCells.size();
            
            // Distribute groundwater baseflow equally to each channel cell
            float qgEachCell = 0.f;
            if (m_qg != nullptr) {
                qgEachCell = m_qg[reachIndex] / n;
            }
            
            for (int iCell = 0; iCell < n; iCell++) {
                ChannelFlow(reachIndex, iCell, vecCells[iCell], qgEachCell);
            }


            m_qSubbasin[reachIndex] = m_qCh[reachIndex][n - 1];


        }
    }


    return 0;
}

void DiffusiveWave::SetValue(const char *key, const FLTPT value) {
    string sk(key);
    if(StringMatch(sk, Tag_CellWidth[0])) {
        m_CellWidth = value;
    }
    else if (StringMatch(sk, VAR_CHS0_PERC[0])) {
        m_Chs0_perc = value;
    }
    else {
        throw ModelException(M_CH_DW[0], "SetValue", "Parameter " + sk
                             + " does not exist. Please contact the module developer.");
    }
}
void DiffusiveWave::SetValue(const char* key, const int value) {
    string sk(key);
    if (StringMatch(sk, Tag_HillSlopeTimeStep[0])) {
        m_dt = value;
    }
    else if (StringMatch(sk, Tag_CellSize[0])) {
        m_nCells = CVT_INT(value);
    }
    else {
        throw ModelException(M_CH_DW[0], "SetValue", "Parameter " + sk
            + " does not exist. Please contact the module developer.");
    }
}

void DiffusiveWave::Set1DData(const char *key, int n, FLTPT *data) {
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
    }
    else if (StringMatch(sk, VAR_NEPR[0])) {
        m_netPcp = data;
    }
    else if (StringMatch(sk, VAR_QSOIL[0])) {
        m_qi = data;
    }
    else if (StringMatch(sk, VAR_SBQG[0])) {
        m_qg = data;
    }
    else if (StringMatch(sk, VAR_CHWIDTH[0])) {
        m_chWidth = data;
    }
    //else if (StringMatch(sk, Tag_FLOWOUT_INDEX[0])) { // TODO: Use a simple way to get outlet index
    //    m_flowOutIdx = data;
    //    for (int i = 0; i < m_nCells; i++) {
    //        if (m_flowOutIdx[i] < 0) {
    //            m_idOutlet = i;
    //            break;
    //        }
    //    }
    //}
    else {
        throw ModelException(M_CH_DW[0], "Set1DData", "Parameter " + sk
                             + " does not exist.");
    }
}

void DiffusiveWave::Set1DData(const char* key, int n, int* data) {
    string sk(key);
    //check the input data
    CheckInputSize(M_CH_DW[0], key, n, m_nCells);

    if (StringMatch(sk, VAR_STREAM_LINK[0])) {
        m_streamLink = data;
    }

    else {
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
    if (nullptr == m_chDepth) reaches->GetReachesSingleProperty(REACH_DEPTH, &m_chDepth);
    if (nullptr == m_chSlope) reaches->GetReachesSingleProperty(REACH_SLOPE, &m_chSlope);

    m_reachUpStream = reaches->GetUpStreamIDs();
    m_reachLayers = reaches->GetReachLayers();

    m_reachs.clear();

    // get the cells in each reach
    for (int i = 1; i <= m_chNumber; ++i) {
        clsReach* pReach = reaches->GetReachByID(i);
        if (pReach != nullptr) {
            int nCells = pReach->GetCellCount();     // cells_num_
            int* cells = pReach->GetPositions();     // positions_

            if (cells != nullptr && nCells > 0) {
                vector<int> cellVec(cells, cells + nCells);
                m_reachs[i] = cellVec;
            }
        }
    }
}

void DiffusiveWave::Get1DData(const char *key, int *n, float **data) {
    string sk(key);
    //*n = m_nCells;
    *n = m_chNumber + 1;
    int iOutlet = m_reachLayers.rbegin()->second[0];
    if (StringMatch(sk, VAR_QSUBBASIN[0])) {
        *data = m_qSubbasin;
    }
    
     else if (StringMatch(sk, VAR_QS[0])) {
        *data = m_qsCh;
        }
     else if (StringMatch(sk, VAR_QI[0])) {
        *data = m_qiCh;
        }
     /*else if (StringMatch(sk, VAR_QG[0])) {
        m_qgCh[0] = m_qg[iOutlet];
        *data = m_qg;
    }*/
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
    *nrows = m_chNumber + 1;
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
    if (StringMatch(sk, VAR_QOVERLAND[0])) {
    m_qs = data;
    }
    /*if (StringMatch(sk, Tag_FLOWIN_INDEX[0])) {
        m_flowInIndex = data;
    } else {
        throw ModelException(M_CH_DW[0], "Set2DData",
                             "Parameter " + sk + " does not exist.");
    }*/
}

void DiffusiveWave::Set2DData(const char* key, int nrows, int ncols, int** data) {
    string sk(key);
    if (StringMatch(sk, Tag_FLOWIN_INDEX[0])) {
        m_flowInIndex = data;
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
        throw ModelException(M_CH_DW[0], "Set2DData",
            "Parameter " + sk + " does not exist.");
    }
}
