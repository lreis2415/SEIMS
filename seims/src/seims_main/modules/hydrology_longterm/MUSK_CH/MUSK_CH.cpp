#include "MUSK_CH.h"

#include "utils_math.h"
#include "text.h"
#include "ChannelRoutingCommon.h"

using namespace utils_math;

MUSK_CH::MUSK_CH() :
    m_dt(-1), m_inputSubbsnID(-1), m_nreach(-1), m_outletID(-1),
    m_Epch(NODATA_VALUE), m_Bnk0(NODATA_VALUE), m_Chs0_perc(NODATA_VALUE),
    m_aBank(NODATA_VALUE), m_bBank(NODATA_VALUE), m_subbsnID(nullptr),
    m_mskX(NODATA_VALUE), m_mskCoef1(NODATA_VALUE), m_mskCoef2(NODATA_VALUE),
    m_chWth(nullptr), m_chDepth(nullptr), m_chLen(nullptr), m_chArea(nullptr),
    m_chSideSlope(nullptr), m_chSlope(nullptr), m_chMan(nullptr),
    m_Kchb(nullptr), m_Kbank(nullptr), m_reachDownStream(nullptr),
    // Inputs from other modules
    m_petSubbsn(nullptr), m_gwSto(nullptr),
    m_olQ2Rch(nullptr), m_ifluQ2Rch(nullptr), m_gndQ2Rch(nullptr),
    // Temporary variables
    m_ptSub(nullptr), m_flowIn(nullptr), m_flowOut(nullptr), m_seepage(nullptr),
    // Outputs
    m_qRchOut(nullptr), m_qsRchOut(nullptr), m_qiRchOut(nullptr), m_qgRchOut(nullptr),
    m_chSto(nullptr), m_rteWtrIn(nullptr), m_rteWtrOut(nullptr), m_bankSto(nullptr),
    m_chWtrDepth(nullptr), m_chWtrWth(nullptr), m_chBtmWth(nullptr), m_chCrossArea(nullptr) {
}

MUSK_CH::~MUSK_CH() {
    /// 1. reaches related variables will be released in ~clsReaches(). By lj, 2017-12-26.
    /// 2. m_ptSrcFactory will be released by DataCenter->Scenario. lj

    if (nullptr != m_ptSub) Release1DArray(m_ptSub);
    if (nullptr != m_flowIn) Release1DArray(m_flowIn);
    if (nullptr != m_flowOut) Release1DArray(m_flowOut);
    if (nullptr != m_seepage) Release1DArray(m_seepage);

    if (nullptr != m_qRchOut) Release1DArray(m_qRchOut);
    if (nullptr != m_qsRchOut) Release1DArray(m_qsRchOut);
    if (nullptr != m_qiRchOut) Release1DArray(m_qiRchOut);
    if (nullptr != m_qgRchOut) Release1DArray(m_qgRchOut);

    if (nullptr != m_chSto) Release1DArray(m_chSto);
    if (nullptr != m_rteWtrIn) Release1DArray(m_rteWtrIn);
    if (nullptr != m_rteWtrOut) Release1DArray(m_rteWtrOut);
    if (nullptr != m_bankSto) Release1DArray(m_bankSto);

    if (nullptr != m_chWtrDepth) Release1DArray(m_chWtrDepth);
    if (nullptr != m_chWtrWth) Release1DArray(m_chWtrWth);
    if (nullptr != m_chBtmWth) Release1DArray(m_chBtmWth);
    if (nullptr != m_chCrossArea) Release1DArray(m_chCrossArea);
}

bool MUSK_CH::CheckInputData() {
    CHECK_POSITIVE(MID_MUSK_CH, m_dt);
    CHECK_NONNEGATIVE(MID_MUSK_CH, m_inputSubbsnID);
    CHECK_POSITIVE(MID_MUSK_CH, m_nreach);
    CHECK_POSITIVE(MID_MUSK_CH, m_outletID);
    CHECK_NODATA(MID_MUSK_CH, m_Epch);
    CHECK_NODATA(MID_MUSK_CH, m_Bnk0);
    CHECK_NODATA(MID_MUSK_CH, m_Chs0_perc);
    CHECK_NODATA(MID_MUSK_CH, m_aBank);
    CHECK_NODATA(MID_MUSK_CH, m_bBank);
    //CHECK_NODATA(MID_MUSK_CH, m_mskX); // Do not throw exception, since they have default values.
    //CHECK_NODATA(MID_MUSK_CH, m_mskCoef1);
    //CHECK_NODATA(MID_MUSK_CH, m_mskCoef2);
    CHECK_POINTER(MID_MUSK_CH, m_subbsnID);

    CHECK_POINTER(MID_MUSK_CH, m_petSubbsn);
    CHECK_POINTER(MID_MUSK_CH, m_gwSto);
    CHECK_POINTER(MID_MUSK_CH, m_olQ2Rch);
    CHECK_POINTER(MID_MUSK_CH, m_ifluQ2Rch);
    CHECK_POINTER(MID_MUSK_CH, m_gndQ2Rch);
    return true;
}

void MUSK_CH::InitialOutputs() {
    CHECK_POSITIVE(MID_MUSK_CH, m_nreach);
    if (nullptr != m_qRchOut) return; // DO NOT Initial Outputs repeatedly.
    if (m_mskX < 0.f) m_mskX = 0.2f;
    if (m_mskCoef1 < 0.f || m_mskCoef1 > 1.f) {
        m_mskCoef1 = 0.75f;
        m_mskCoef2 = 0.25f;
    } else {
        // There is no need to use mskCoef2 as input parameter.
        // Make sure m_mskCoef1 + m_mskCoef2 = 1.
        //float msk1 = m_mskCoef1 / (m_mskCoef1 + m_mskCoef2);
        //float msk2 = m_mskCoef2 / (m_mskCoef1 + m_mskCoef2);
        //m_mskCoef1 = msk1;
        //m_mskCoef2 = msk2;
    }
    m_mskCoef2 = 1.f - m_mskCoef1;

    m_flowIn = new(nothrow) float[m_nreach + 1];
    m_flowOut = new(nothrow) float[m_nreach + 1];
    m_seepage = new(nothrow) float[m_nreach + 1];

    m_qRchOut = new(nothrow) float[m_nreach + 1];
    m_qsRchOut = new(nothrow) float[m_nreach + 1];
    m_qiRchOut = new(nothrow) float[m_nreach + 1];
    m_qgRchOut = new(nothrow) float[m_nreach + 1];

    m_chSto = new(nothrow) float[m_nreach + 1];
    m_rteWtrIn = new(nothrow) float[m_nreach + 1];
    m_rteWtrOut = new(nothrow) float[m_nreach + 1];
    m_bankSto = new(nothrow) float[m_nreach + 1];

    m_chWtrDepth = new(nothrow) float[m_nreach + 1];
    m_chWtrWth = new(nothrow) float[m_nreach + 1];
    m_chBtmWth = new(nothrow) float[m_nreach + 1];
    m_chCrossArea = new(nothrow) float[m_nreach + 1];

    for (int i = 1; i <= m_nreach; i++) {
        m_qRchOut[i] = m_olQ2Rch[i];
        m_qsRchOut[i] = m_olQ2Rch[i];
        if (nullptr != m_ifluQ2Rch) {
            m_qRchOut[i] += m_ifluQ2Rch[i];
            m_qiRchOut[i] = m_ifluQ2Rch[i];
        } else {
            m_qiRchOut[i] = 0.f;
        }
        if (nullptr != m_gndQ2Rch) {
            m_qRchOut[i] += m_gndQ2Rch[i];
            m_qgRchOut[i] = m_gndQ2Rch[i];
        } else {
            m_qgRchOut[i] = 0.f;
        }
        m_seepage[i] = 0.f;
        m_bankSto[i] = m_Bnk0 * m_chLen[i];
        m_chBtmWth[i] = ChannleBottomWidth(m_chWth[i], m_chSideSlope[i], m_chDepth[i]);
        m_chCrossArea[i] = ChannelCrossSectionalArea(m_chBtmWth[i], m_chDepth[i], m_chSideSlope[i]);
        m_chWtrDepth[i] = m_chDepth[i] * m_Chs0_perc;
        m_chWtrWth[i] = m_chBtmWth[i] + 2.f * m_chSideSlope[i] * m_chWtrDepth[i];
        m_chSto[i] = m_chLen[i] * m_chWtrDepth[i] * (m_chBtmWth[i] + m_chSideSlope[i] * m_chWtrDepth[i]);
        m_flowIn[i] = m_chSto[i];
        m_flowOut[i] = m_chSto[i];
        m_rteWtrIn[i] = 0.f;
        m_rteWtrOut[i] = 0.f;
    }
    /// initialize point source loadings
    if (nullptr == m_ptSub) {
        Initialize1DArray(m_nreach + 1, m_ptSub, 0.f);
    }
}

void MUSK_CH::PointSourceLoading() {
    /// load point source water discharge (m3/s) on current day from Scenario
    for (auto it = m_ptSrcFactory.begin(); it != m_ptSrcFactory.end(); ++it) {
        /// reset point source loading water to 0.f
        for (int i = 0; i <= m_nreach; i++) {
            m_ptSub[i] = 0.f;
        }
        //cout<<"unique Point Source Factory ID: "<<it->first<<endl;
        vector<int>& ptSrcMgtSeqs = it->second->GetPointSrcMgtSeqs();
        map<int, PointSourceMgtParams *>& pointSrcMgtMap = it->second->GetPointSrcMgtMap();
        vector<int>& ptSrcIDs = it->second->GetPointSrcIDs();
        map<int, PointSourceLocations *>& pointSrcLocsMap = it->second->GetPointSrcLocsMap();
        // 1. looking for management operations from m_pointSrcMgtMap
        for (auto seqIter = ptSrcMgtSeqs.begin(); seqIter != ptSrcMgtSeqs.end(); ++seqIter) {
            PointSourceMgtParams* curPtMgt = pointSrcMgtMap.at(*seqIter);
            // 1.1 If current day is beyond the date range, then continue to next management
            if (curPtMgt->GetStartDate() != 0 && curPtMgt->GetEndDate() != 0) {
                if (m_date < curPtMgt->GetStartDate() || m_date > curPtMgt->GetEndDate()) {
                    continue;
                }
            }
            // 1.2 Otherwise, get the water volume
            float per_wtrVol = curPtMgt->GetWaterVolume(); /// m3/'size'/day
            // 1.3 Sum up all point sources
            for (auto locIter = ptSrcIDs.begin(); locIter != ptSrcIDs.end(); ++locIter) {
                if (pointSrcLocsMap.find(*locIter) != pointSrcLocsMap.end()) {
                    PointSourceLocations* curPtLoc = pointSrcLocsMap.at(*locIter);
                    int curSubID = curPtLoc->GetSubbasinID();
                    m_ptSub[curSubID] += per_wtrVol * curPtLoc->GetSize() / 86400.f; /// m3/'size'/day ==> m3/s
                }
            }
        }
    }
}

int MUSK_CH::Execute() {
    InitialOutputs();
    /// load point source water volume from m_ptSrcFactory
    PointSourceLoading();
    for (auto it = m_rteLyrs.begin(); it != m_rteLyrs.end(); ++it) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int reachNum = CVT_INT(it->second.size());
        size_t errCount = 0;
        // the size of m_rteLyrs (map) is equal to the maximum stream order
#pragma omp parallel for reduction(+:errCount)
        for (int i = 0; i < reachNum; i++) {
            int reachIndex = it->second[i]; // index in the array, i.e., subbasinID
            if (m_inputSubbsnID == 0 || m_inputSubbsnID == reachIndex) {
                // for OpenMP version, all reaches will be executed,
                // for MPI version, only the current reach will be executed.
                if (!ChannelFlow(reachIndex)) {
                    errCount++;
                }
            }
        }
        if (errCount > 0) {
            throw ModelException(MID_MUSK_CH, "Execute", "Error occurred!");
        }
    }
    return 0;
}

void MUSK_CH::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, Tag_ChannelTimeStep)) m_dt = CVT_INT(value);
    else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbsnID = CVT_INT(value);
    else if (StringMatch(sk, VAR_OUTLETID)) m_outletID = CVT_INT(value);
    else if (StringMatch(sk, VAR_EP_CH)) m_Epch = value;
    else if (StringMatch(sk, VAR_BNK0)) m_Bnk0 = value;
    else if (StringMatch(sk, VAR_CHS0_PERC)) m_Chs0_perc = value;
    else if (StringMatch(sk, VAR_A_BNK)) m_aBank = value;
    else if (StringMatch(sk, VAR_B_BNK)) m_bBank = value;
    else if (StringMatch(sk, VAR_MSK_X)) m_mskX = value;
    else if (StringMatch(sk, VAR_MSK_CO1)) m_mskCoef1 = value;
    else {
        throw ModelException(MID_MUSK_CH, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void MUSK_CH::SetValueByIndex(const char* key, const int index, const float value) {
    if (m_inputSubbsnID == 0) return;           // Not for omp version
    if (index <= 0 || index > m_nreach) return; // index should belong 1 ~ m_nreach
    if (nullptr == m_qRchOut) InitialOutputs();
    string sk(key);
    /// Set single value of array1D of current subbasin
    /// IN/OUTPUT variables
    if (StringMatch(sk, VAR_QRECH)) m_qRchOut[index] = value;
    else if (StringMatch(sk, VAR_QS)) m_qsRchOut[index] = value;
    else if (StringMatch(sk, VAR_QI)) m_qiRchOut[index] = value;
    else if (StringMatch(sk, VAR_QG)) m_qgRchOut[index] = value;
    else {
        throw ModelException(MID_MUSK_CH, "SetValueByIndex", "Parameter " + sk + " does not exist.");
    }
}

void MUSK_CH::Set1DData(const char* key, const int n, float* data) {
    string sk(key);
    //check the input data
    if (StringMatch(sk, VAR_SUBBSN)) {
        m_subbsnID = data;
    } else if (StringMatch(sk, VAR_SBPET)) {
        CheckInputSize(MID_MUSK_CH, key, n - 1, m_nreach);
        m_petSubbsn = data;
    } else if (StringMatch(sk, VAR_SBGS)) {
        CheckInputSize(MID_MUSK_CH, key, n - 1, m_nreach);
        m_gwSto = data;
    } else if (StringMatch(sk, VAR_SBOF)) {
        CheckInputSize(MID_MUSK_CH, key, n - 1, m_nreach);
        m_olQ2Rch = data;
    } else if (StringMatch(sk, VAR_SBIF)) {
        CheckInputSize(MID_MUSK_CH, key, n - 1, m_nreach);
        m_ifluQ2Rch = data;
    } else if (StringMatch(sk, VAR_SBQG)) {
        CheckInputSize(MID_MUSK_CH, key, n - 1, m_nreach);
        m_gndQ2Rch = data;
    } else {
        throw ModelException(MID_MUSK_CH, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void MUSK_CH::GetValue(const char* key, float* value) {
    InitialOutputs();
    string sk(key);
    /// IN/OUTPUT variables
    if (StringMatch(sk, VAR_QRECH) && m_inputSubbsnID > 0) *value = m_qRchOut[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_QS) && m_inputSubbsnID > 0) *value = m_qsRchOut[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_QI) && m_inputSubbsnID > 0) *value = m_qiRchOut[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_QG) && m_inputSubbsnID > 0) *value = m_qgRchOut[m_inputSubbsnID];
    else {
        throw ModelException(MID_MUSK_CH, "GetValue", "Parameter " + sk + " does not exist.");
    }
}

void MUSK_CH::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nreach + 1;
    if (StringMatch(sk, VAR_QRECH)) {
        m_qRchOut[0] = m_qRchOut[m_outletID];
        *data = m_qRchOut;
    } else if (StringMatch(sk, VAR_QS)) {
        m_qsRchOut[0] = m_qsRchOut[m_outletID];
        *data = m_qsRchOut;
    } else if (StringMatch(sk, VAR_QI)) {
        m_qiRchOut[0] = m_qiRchOut[m_outletID];
        *data = m_qiRchOut;
    } else if (StringMatch(sk, VAR_QG)) {
        m_qgRchOut[0] = m_qgRchOut[m_outletID];
        *data = m_qgRchOut;
    } else if (StringMatch(sk, VAR_CHST)) {
        m_chSto[0] = m_chSto[m_outletID];
        *data = m_chSto;
    } else if (StringMatch(sk, VAR_RTE_WTRIN)) {
        m_rteWtrIn[0] = m_rteWtrIn[m_outletID];
        *data = m_rteWtrIn;
    } else if (StringMatch(sk, VAR_RTE_WTROUT)) {
        m_rteWtrOut[0] = m_rteWtrOut[m_outletID];
        *data = m_rteWtrOut;
    } else if (StringMatch(sk, VAR_BKST)) {
        m_bankSto[0] = m_bankSto[m_outletID];
        *data = m_bankSto;
    } else if (StringMatch(sk, VAR_CHWTRDEPTH)) {
        m_chWtrDepth[0] = m_chWtrDepth[m_outletID];
        *data = m_chWtrDepth;
    } else if (StringMatch(sk, VAR_CHWTRWIDTH)) {
        m_chWtrWth[0] = m_chWtrWth[m_outletID];
        *data = m_chWtrWth;
    } else if (StringMatch(sk, VAR_CHBTMWIDTH)) {
        m_chBtmWth[0] = m_chBtmWth[m_outletID];
        *data = m_chBtmWth;
    } else if (StringMatch(sk, VAR_CHCROSSAREA)) {
        m_chCrossArea[0] = m_chCrossArea[m_outletID];
        *data = m_chCrossArea;
    } else {
        throw ModelException(MID_MUSK_CH, "Get1DData", "Output " + sk + " does not exist.");
    }
}

void MUSK_CH::SetScenario(Scenario* sce) {
    if (nullptr != sce) {
        map<int, BMPFactory *>& tmpBMPFactories = sce->GetBMPFactories();
        for (auto it = tmpBMPFactories.begin(); it != tmpBMPFactories.end(); ++it) {
            /// Key is uniqueBMPID, which is calculated by BMP_ID * 100000 + subScenario;
            if (it->first / 100000 == BMP_TYPE_POINTSOURCE) {
#ifdef HAS_VARIADIC_TEMPLATES
                m_ptSrcFactory.emplace(it->first, static_cast<BMPPointSrcFactory*>(it->second));
#else
                m_ptSrcFactory.insert(make_pair(it->first, static_cast<BMPPointSrcFactory*>(it->second)));
#endif
            }
        }
    } else {
        throw ModelException(MID_MUSK_CH, "SetScenario", "The scenario can not to be NULL.");
    }
}

void MUSK_CH::SetReaches(clsReaches* reaches) {
    if (nullptr == reaches) {
        throw ModelException(MID_MUSK_CH, "SetReaches", "The reaches input can not to be NULL.");
    }
    m_nreach = reaches->GetReachNumber();

    if (nullptr == m_chWth) reaches->GetReachesSingleProperty(REACH_WIDTH, &m_chWth);
    if (nullptr == m_chDepth) reaches->GetReachesSingleProperty(REACH_DEPTH, &m_chDepth);
    if (nullptr == m_chLen) reaches->GetReachesSingleProperty(REACH_LENGTH, &m_chLen);
    if (nullptr == m_chArea) reaches->GetReachesSingleProperty(REACH_AREA, &m_chArea);
    if (nullptr == m_chSideSlope) reaches->GetReachesSingleProperty(REACH_SIDESLP, &m_chSideSlope);
    if (nullptr == m_chSlope) reaches->GetReachesSingleProperty(REACH_SLOPE, &m_chSlope);
    if (nullptr == m_chMan) reaches->GetReachesSingleProperty(REACH_MANNING, &m_chMan);
    if (nullptr == m_Kbank) reaches->GetReachesSingleProperty(REACH_BNKK, &m_Kbank);
    if (nullptr == m_Kchb) reaches->GetReachesSingleProperty(REACH_BEDK, &m_Kchb);
    if (nullptr == m_reachDownStream) reaches->GetReachesSingleProperty(REACH_DOWNSTREAM, &m_reachDownStream);

    m_reachUpStream = reaches->GetUpStreamIDs();
    m_rteLyrs = reaches->GetReachLayers();
}

bool MUSK_CH::ChannelFlow(const int i) {
    // 1. first add all the inflow water
    float qIn = 0.f; /// Water entering reach on current day from both current subbasin and upstreams
    // 1.1. water from this subbasin
    qIn += m_olQ2Rch[i]; /// surface flow
    float qiSub = 0.f;   /// interflow flow
    if (nullptr != m_ifluQ2Rch && m_ifluQ2Rch[i] >= 0.f) {
        qiSub = m_ifluQ2Rch[i];
        qIn += qiSub;
    }
    float qgSub = 0.f; /// groundwater flow
    if (nullptr != m_gndQ2Rch && m_gndQ2Rch[i] >= 0.f) {
        qgSub = m_gndQ2Rch[i];
        qIn += qgSub;
    }
    float ptSub = 0.f; /// point sources flow
    if (nullptr != m_ptSub && m_ptSub[i] >= 0.f) {
        ptSub = m_ptSub[i];
        qIn += ptSub;
    }
    // 1.2. water from upstream reaches
    float qsUp = 0.f;
    float qiUp = 0.f;
    float qgUp = 0.f;
    for (auto upRchID = m_reachUpStream.at(i).begin(); upRchID != m_reachUpStream.at(i).end(); ++upRchID) {
        if (m_qsRchOut[*upRchID] != m_qsRchOut[*upRchID]) {
            cout << "DayOfYear: " << m_dayOfYear << ", rchID: " << i << ", upRchID: " << *upRchID <<
                    ", surface part illegal!" << endl;
            return false;
        }
        if (m_qiRchOut[*upRchID] != m_qiRchOut[*upRchID]) {
            cout << "DayOfYear: " << m_dayOfYear << ", rchID: " << i << ", upRchID: " << *upRchID <<
                    ", subsurface part illegal!" << endl;
            return false;
        }
        if (m_qgRchOut[*upRchID] != m_qgRchOut[*upRchID]) {
            cout << "DayOfYear: " << m_dayOfYear << ", rchID: " << i << ", upRchID: " << *upRchID <<
                    ", groundwater part illegal!" << endl;
            return false;
        }
        if (m_qsRchOut[*upRchID] > 0.f) qsUp += m_qsRchOut[*upRchID];
        if (m_qiRchOut[*upRchID] > 0.f) qiUp += m_qiRchOut[*upRchID];
        if (m_qgRchOut[*upRchID] > 0.f) qgUp += m_qgRchOut[*upRchID];
    }
    qIn += qsUp + qiUp + qgUp;
#ifdef PRINT_DEBUG
    cout << "ID: " << i << ", surfaceQ: " << m_qsSub[i] << ", subsurfaceQ: " << qiSub <<
        ", groundQ: " << qgSub << ", pointQ: " << ptSub <<
        ", UPsurfaceQ: " << qsUp << ", UPsubsurface: " << qiUp << ", UPground: " << qgUp << endl;
#endif
    // 1.3. water from bank storage
    float bankOut = m_bankSto[i] * (1.f - exp(-m_aBank));
    m_bankSto[i] -= bankOut;
    qIn += bankOut / m_dt;

    // loss the water from bank storage to the adjacent unsaturated zone and groundwater storage
    float bankOutGw = m_bankSto[i] * (1.f - exp(-m_bBank));
    m_bankSto[i] -= bankOutGw;
    if (nullptr != m_gwSto) {
        m_gwSto[i] += bankOutGw / m_chArea[i] * 1000.f; // updated groundwater storage
    }

    // Compute storage time constant (ratio of storage to discharge) for reach
    // Wetting perimeter at bankfull
    float wet_perimeter = ChannelWettingPerimeter(m_chBtmWth[i], m_chDepth[i], m_chSideSlope[i]);
    // Cross-sectional area at bankfull
    float cross_area = ChannelCrossSectionalArea(m_chBtmWth[i], m_chDepth[i], m_chSideSlope[i]);
    // Hydraulic radius
    float radius = cross_area / wet_perimeter;
    // The storage time constant calculated for the reach segment with bankfull flows.
    float k_bankfull = StorageTimeConstant(m_chMan[i], m_chSlope[i], m_chLen[i], radius); // Hour

    // The storage time constant calculated for the reach segment with one-tenth of the bankfull flows.
    float wet_perimeter2 = ChannelWettingPerimeter(m_chBtmWth[i], 0.1f * m_chDepth[i], m_chSideSlope[i]);
    float cross_area2 = ChannelCrossSectionalArea(m_chBtmWth[i], 0.1f * m_chDepth[i], m_chSideSlope[i]);
    float radius2 = cross_area2 / wet_perimeter2;
    float k_bankfull2 = StorageTimeConstant(m_chMan[i], m_chSlope[i], m_chLen[i], radius2); // Hour

    float xkm = k_bankfull * m_mskCoef1 + k_bankfull2 * m_mskCoef2;
    // Eq. 7:1.4.9 in SWAT Theory 2009.
    // Check Muskingum numerical stability
    float detmax = 2.f * xkm * (1.f - m_mskX);
    float detmin = 2.f * xkm * m_mskX;
    // Discretize time interval to meet the stability criterion
    float det = 24.f; // hours, time step
    int nn = 0;       // number of subdaily computation points for stable routing
    if (det > detmax) {
        if (det / 2.f <= detmax) {
            det = 12.f;
            nn = 2;
        } else if (det / 4.f <= detmax) {
            det = 6.f;
            nn = 4;
        } else {
            det = 1;
            nn = 24;
        }
    } else {
        det = 24;
        nn = 1;
    }
    // get coefficients of Muskingum
    float temp = detmax + det;
    float c1 = (det - detmin) / temp;
    float c2 = (det + detmin) / temp;
    float c3 = (detmax - det) / temp;
    // make sure any coefficient is positive. Not sure whether this is needed.
    //if (c1 < 0) {
    //    c2 += c1;
    //    c1 = 0.f;
    //}
    //if (c3 < 0) {
    //    c2 += c1;
    //    c3 = 0.f;
    //}

#ifdef PRINT_DEBUG
    cout << " chStorage before routing " << m_chStorage[i] << endl;
#endif
    m_rteWtrOut[i] = qIn * m_dt;   // m^3
    float wtrin = qIn * m_dt / nn; // Inflow during a sub time interval, m^3
    float vol = 0.f;               // volume of water in reach, m^3
    float volrt = 0.f;             // flow rate, m^3/s
    float max_rate = 0.f;          // maximum flow capacity of the channel at bank full (m^3/s)
    float sdti = 0.f;              // average flow on day in reach, m^3/s, i.e., m_qRchOut
    float vc = 0.f;                // average flow velocity in channel, m/s
    float rchp = 0.f;              // wet perimeter, m
    float rcharea = 0.f;           // cross-sectional area, m^2
    float rchradius = 0.f;         // hydraulic radius
    float rtwtr = 0.f;             // water leaving reach on day, m^3, i.e., m_rteWtrOut
    float rttlc = 0.f;             // transmission losses from reach on day, m^3
    float qinday = 0.f;            // m^3
    float qoutday = 0.f;           // m^3

    // Iterate for the day
    for (int ii = 0; ii < nn; ii++) {
        // Calculate volume of water in reach
        vol = m_chSto[i] + wtrin; // m^3
        // Find average flowrate in a sub time interval, m^3/s
        volrt = vol / (86400.f / nn);
        // Find maximum flow capacity of the channel at bank full, m^3/s
        max_rate = manningQ(cross_area, radius, m_chMan[i], m_chSlope[i]);
        sdti = 0.f;
        m_chWtrDepth[i] = 0.f;
        // If average flowrate is greater than the channel capacity at bank full,
        //   then simulate flood plain flow, else simulate the regular channel flow.
        if (volrt > max_rate) {
            m_chWtrDepth[i] = m_chDepth[i];
            sdti = max_rate;
            // Find the cross-sectional area and depth for volrt by iteration method at 1cm interval depth.
            // Find the depth until the discharge rate is equal to volrt
            while (sdti < volrt) {
                m_chWtrDepth[i] += 0.01f; // Increase 1cm at each interation
                rcharea = ChannelCrossSectionalArea(m_chBtmWth[i], m_chDepth[i], m_chWtrDepth[i],
                                                    m_chSideSlope[i], m_chWth[i], 4.f);
                rchp = ChannelWettingPerimeter(m_chBtmWth[i], m_chDepth[i], m_chWtrDepth[i],
                                               m_chSideSlope[i], m_chWth[i], 4.f);
                radius = rcharea / rchp;
                sdti = manningQ(rcharea, radius, m_chMan[i], m_chSlope[i]);
            }
            sdti = volrt;
        } else {
            // Find the cross-sectional area and depth for volrt by iteration method at 1cm interval depth
            // Find the depth until the discharge rate is equal to volrt.
            while (sdti < volrt) {
                m_chWtrDepth[i] += 0.01f;
                rcharea = ChannelCrossSectionalArea(m_chBtmWth[i], m_chWtrDepth[i], m_chSideSlope[i]);
                rchp = ChannelWettingPerimeter(m_chBtmWth[i], m_chWtrDepth[i], m_chSideSlope[i]);
                rchradius = rcharea / rchp;
                sdti = manningQ(rcharea, rchradius, m_chMan[i], m_chSlope[i]);
            }
            sdti = volrt;
        }
        // Calculate top width of channel at water level, topw in SWAT
        if (m_chWtrDepth[i] <= m_chDepth[i]) {
            m_chWtrWth[i] = m_chBtmWth[i] + 2.f * m_chWtrDepth[i] * m_chSideSlope[i];
        } else {
            m_chWtrWth[i] = 5.f * m_chWth[i] + 2.f * (m_chWtrDepth[i] - m_chDepth[i]) * 4.f;
        }
        if (sdti > 0.f) {
            // Calculate velocity and travel time
            vc = sdti / rcharea;                       // vel_chan(:) in SWAT
            float rttime = m_chLen[i] / (3600.f * vc); // reach travel time, hr
            // Compute water leaving reach on day
            rtwtr = c1 * wtrin + c2 * m_flowIn[i] + c3 * m_flowOut[i];
            if (rtwtr < 0.f) rtwtr = 0.f;
            rtwtr = Min(rtwtr, wtrin + m_chSto[i]);
            // Calculate amount of water in channel at end of day
            m_chSto[i] += wtrin - rtwtr;
            // Add if statement to keep m_chStorage from becoming negative
            if (m_chSto[i] < 0.f) m_chSto[i] = 0.f;

            // Transmission and evaporation losses are proportionally taken from the channel storage
            //   and from volume flowing out
            if (rtwtr > 0.f) {
                // Total time in hours to clear the water
                rttlc = det * m_Kchb[i] * 0.001f * m_chLen[i] * rchp; // m^3
                float rttlc2 = rttlc * m_chSto[i] / (rtwtr + m_chSto[i]);
                float rttlc1 = 0.f;
                if (m_chSto[i] <= rttlc2) {
                    rttlc2 = Min(rttlc2, m_chSto[i]);
                }
                m_chSto[i] -= rttlc2;
                rttlc1 = rttlc - rttlc2;
                if (rtwtr <= rttlc1) {
                    rttlc1 = Min(rttlc1, rtwtr);
                }
                rtwtr -= rttlc1;
                rttlc = rttlc1 + rttlc2; // Total water loss by transmission
            }
            // Calculate evaporation
            float rtevp = 0.f;
            float rtevp1 = 0.f;
            float rtevp2 = 0.f;
            if (rtwtr > 0.f) {
                /// In SWAT source code, line 306 of rtmusk.f, I think aaa should be divided by nn! By lj.
                float aaa = m_Epch * m_petSubbsn[i] * 0.001f / nn; // m
                if (m_chWtrDepth[i] <= m_chDepth[i]) {
                    rtevp = aaa * m_chLen[i] * m_chWtrWth[i]; // m^3
                } else {
                    if (aaa <= m_chWtrDepth[i] - m_chDepth[i]) {
                        rtevp = aaa * m_chLen[i] * m_chWtrWth[i];
                    } else {
                        rtevp = aaa;
                        m_chWtrWth[i] = m_chBtmWth[i] + 2.f * m_chDepth[i] * m_chSideSlope[i];
                        rtevp *= m_chLen[i] * m_chWtrWth[i]; // m^3
                    }
                }
                rtevp2 = rtevp * m_chSto[i] / (rtwtr + m_chSto[i]);
                if (m_chSto[i] <= rtevp2) {
                    rtevp2 = Min(rtevp2, m_chSto[i]);
                }
                m_chSto[i] -= rtevp2;
                rtevp1 = rtevp - rtevp2;
                if (rtwtr <= rtevp1) {
                    rtevp1 = Min(rtevp1, rtwtr);
                }
                rtwtr -= rtevp1;
                rtevp = rtevp1 + rtevp2; // Total water loss by evaporation
            }
            // Define flow parameters for current iteration
            m_flowIn[i] = wtrin;
            m_flowOut[i] = rtwtr;
            // Define flow parameters for current day
            qinday += wtrin;
            qoutday += rtwtr;
            // Total outflow for the day
            rtwtr = qoutday;
        } else {
            rtwtr = 0.f;
            sdti = 0.f;
            m_chSto[i] = 0.f;
            m_flowIn[i] = 0.f;
            m_flowOut[i] = 0.f;
        }
    } /* Iterate for the day */
    if (rtwtr < 0.f) rtwtr = 0.f;
    if (m_chSto[i] < 0.f) m_chSto[i] = 0.f;
    if (m_chSto[i] < 10.f) {
        rtwtr += m_chSto[i];
        m_chSto[i] = 0.f;
    }
    m_qRchOut[i] = sdti;
    m_rteWtrOut[i] = rtwtr;
    m_chCrossArea[i] = rcharea;

    float qInSum = m_olQ2Rch[i] + qiSub + qgSub + qsUp + qiUp + qgUp;
    if (qInSum < UTIL_ZERO) {
        // In case of divided by zero.
        m_qsRchOut[i] = 0.f;
        m_qiRchOut[i] = 0.f;
        m_qgRchOut[i] = 0.f;
        m_qRchOut[i] = 0.f;
    } else {
        // In my opinion, these lines should use `qIn` instead of `qInSum`. By lj.
        m_qsRchOut[i] = m_qRchOut[i] * (m_olQ2Rch[i] + qsUp) / qIn;
        m_qiRchOut[i] = m_qRchOut[i] * (qiSub + qiUp) / qIn;
        m_qgRchOut[i] = m_qRchOut[i] * (qgSub + qgUp) / qIn;
    }

    // Add transmission losses to bank storage/deep aquifer (i.e., groundwater in current version)
    if (rttlc > 0.f) {
        float trnsrch = 0.5f;
        if (rchp > 0.f) {
            trnsrch = m_chBtmWth[i] / rchp; // Use bottom width / wetting perimeter to estimate.
        }
        m_bankSto[i] += rttlc * (1.f - trnsrch); // m^3
        if (nullptr != m_gwSto) {
            m_gwSto[i] += rttlc * trnsrch / m_chArea[i] * 1000.f; // mm
        }
    }

    // todo, compute revap from bank storage. In SWAT, revap coefficient is equal to gw_revap.

#ifdef PRINT_DEBUG
    cout << " chStorage after routing " << m_chStorage[i] << endl;
    cout << " surfq: " << m_qsCh[i] << ", ifluq: " << m_qiCh[i] << ", groudq: " << m_qgCh[i] << endl;
#endif
    return true;
}
