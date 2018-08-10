#include "MUSK_CH.h"

#include "utils_math.h"
#include "text.h"
#include "ChannelRoutingCommon.h"

using namespace utils_math;

MUSK_CH::MUSK_CH() :
    m_dt(-1), m_nreach(-1), m_inputSubbsnID(-1), m_outletID(-1), m_ptSub(nullptr),
    m_chMan(nullptr), m_chSlope(nullptr),
    m_Kchb(nullptr), m_Kbank(nullptr), m_Epch(NODATA_VALUE), m_Bnk0(NODATA_VALUE),
    m_chSideSlope(nullptr), m_Chs0_perc(NODATA_VALUE),
    m_Vseep0(0.f), m_aBank(NODATA_VALUE), m_bBank(NODATA_VALUE),
    m_subbsnID(nullptr), m_area(nullptr),
    m_petCh(nullptr), m_qsSub(nullptr), m_qiSub(nullptr), m_qgSub(nullptr),
    m_gwSto(nullptr), m_qsCh(nullptr), m_qiCh(nullptr), m_qgCh(nullptr),
    m_chWth(nullptr), m_chWtrWth(nullptr),
    m_chBtmWth(nullptr), m_chDepth(nullptr),
    m_chWtrDepth(nullptr), m_preChWtrDepth(nullptr), m_chLen(nullptr),
    m_bankSto(nullptr), m_seepage(nullptr),
    m_reachDownStream(nullptr), m_mskX(NODATA_VALUE),
    m_mskCoef1(NODATA_VALUE), m_mskCoef2(NODATA_VALUE),
    m_flowIn(nullptr), m_flowOut(nullptr), m_chSto(nullptr), m_preChSto(nullptr),
    m_qRchOut(nullptr) {
}

MUSK_CH::~MUSK_CH() {
    /// reaches related variables will be released in ~clsReaches(). By lj, 2017-12-26.

    if (nullptr != m_chSto) Release1DArray(m_chSto);
    if (nullptr != m_preChSto) Release1DArray(m_preChSto);
    if (nullptr != m_qRchOut) Release1DArray(m_qRchOut);
    if (nullptr != m_bankSto) Release1DArray(m_bankSto);
    if (nullptr != m_seepage) Release1DArray(m_seepage);
    if (nullptr != m_qsCh) Release1DArray(m_qsCh);
    if (nullptr != m_qiCh) Release1DArray(m_qiCh);
    if (nullptr != m_qgCh) Release1DArray(m_qgCh);
    if (nullptr != m_chWtrDepth) Release1DArray(m_chWtrDepth);
    if (nullptr != m_preChWtrDepth) Release1DArray(m_preChWtrDepth);
    if (nullptr != m_chWtrWth) Release1DArray(m_chWtrWth);
    if (nullptr != m_chBtmWth) Release1DArray(m_chBtmWth);
    if (nullptr != m_ptSub) Release1DArray(m_ptSub);
    // m_ptSrcFactory will be released by DataCenter->Scenario. lj
}

bool MUSK_CH::CheckInputData() {
    CHECK_POSITIVE(MID_MUSK_CH, m_dt);
    CHECK_POSITIVE(MID_MUSK_CH, m_nreach);
    CHECK_NONNEGATIVE(MID_MUSK_CH, m_inputSubbsnID);
    CHECK_POSITIVE(MID_MUSK_CH, m_outletID);
    //CHECK_NODATA(MID_MUSK_CH, m_mskX); // Do not throw exception, since they have default values.
    //CHECK_NODATA(MID_MUSK_CH, m_mskCoef1);
    //CHECK_NODATA(MID_MUSK_CH, m_mskCoef2);
    CHECK_POINTER(MID_MUSK_CH, m_chMan);
    CHECK_POINTER(MID_MUSK_CH, m_chSlope);
    CHECK_POINTER(MID_MUSK_CH, m_Kchb);
    CHECK_POINTER(MID_MUSK_CH, m_Kbank);
    CHECK_NODATA(MID_MUSK_CH, m_Epch);
    CHECK_NODATA(MID_MUSK_CH, m_Bnk0);
    CHECK_NODATA(MID_MUSK_CH, m_Chs0_perc);
    CHECK_NODATA(MID_MUSK_CH, m_aBank);
    CHECK_NODATA(MID_MUSK_CH, m_bBank);
    CHECK_NODATA(MID_MUSK_CH, m_Vseep0);
    CHECK_POINTER(MID_MUSK_CH, m_subbsnID);
    CHECK_POINTER(MID_MUSK_CH, m_qsSub);
    CHECK_POINTER(MID_MUSK_CH, m_chWth);
    return true;
}

void MUSK_CH::InitialOutputs() {
    CHECK_POSITIVE(MID_MUSK_CH, m_nreach);
    if (nullptr != m_chSto) return; // DO NOT Initial Outputs repeatedly.
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
    //initial channel storage
    m_chSto = new(nothrow) float[m_nreach + 1];
    m_preChSto = new(nothrow) float[m_nreach + 1];
    //m_qIn = new(nothrow) float[m_nreach + 1];
    m_qRchOut = new(nothrow) float[m_nreach + 1];
    m_bankSto = new(nothrow) float[m_nreach + 1];
    m_seepage = new(nothrow) float[m_nreach + 1];
    m_qsCh = new(nothrow) float[m_nreach + 1];
    m_qiCh = new(nothrow) float[m_nreach + 1];
    m_qgCh = new(nothrow) float[m_nreach + 1];
    m_chWtrDepth = new(nothrow) float[m_nreach + 1];
    m_preChWtrDepth = new(nothrow) float[m_nreach + 1];
    m_chWtrWth = new(nothrow) float[m_nreach + 1];
    m_chBtmWth = new(nothrow) float[m_nreach + 1];
    m_flowIn = new(nothrow) float[m_nreach + 1];
    m_flowOut = new(nothrow) float[m_nreach + 1];

    for (int i = 1; i <= m_nreach; i++) {
        float qiSub = 0.f; // interflow (subsurface flow)
        float qgSub = 0.f; // groundwater outflow
        if (nullptr != m_qiSub) {
            qiSub = m_qiSub[i];
        }
        if (nullptr != m_qgSub) {
            qgSub = m_qgSub[i];
        }
        m_seepage[i] = 0.f;
        m_bankSto[i] = m_Bnk0 * m_chLen[i];
        updateChannleBottomWidth(i);
        m_chWtrDepth[i] = m_chDepth[i] * m_Chs0_perc;
        m_chWtrWth[i] = m_chBtmWth[i] + 2.f * m_chSideSlope[i] * m_chWtrDepth[i];
        m_preChWtrDepth[i] = m_chWtrWth[i];
        m_chSto[i] = m_chLen[i] * m_chWtrDepth[i] * (m_chBtmWth[i] + m_chSideSlope[i] * m_chWtrDepth[i]);
        m_preChSto[i] = m_chSto[i];
        m_flowIn[i] = m_chSto[i];
        m_flowOut[i] = m_chSto[i];
        //m_qIn[i] = 0.f;
        m_qRchOut[i] = m_qsSub[i] + qiSub + qgSub;
        m_qsCh[i] = m_qsSub[i];
        m_qiCh[i] = qiSub;
        m_qgCh[i] = qgSub;
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
        //#pragma omp parallel for reduction(+:errCount)
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

bool MUSK_CH::CheckInputSize(const char* key, const int n) {
    if (n <= 0) {
        throw ModelException(MID_MUSK_CH, "CheckInputSize", "Input data for " + string(key) +
                             " is invalid. The size could not be less than zero.");
    }
    if (m_nreach != n - 1) {
        if (m_nreach <= 0) {
            m_nreach = n - 1;
        } else {
            std::ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n << ". The origin size is " <<
                    m_nreach << ".\n";
            throw ModelException(MID_MUSK_CH, "CheckInputSize", oss.str());
        }
    }
    return true;
}

void MUSK_CH::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, VAR_OUTLETID)) m_outletID = CVT_INT(value);
    else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbsnID = CVT_INT(value);
    else if (StringMatch(sk, Tag_ChannelTimeStep)) m_dt = CVT_INT(value);
    else if (StringMatch(sk, VAR_EP_CH)) m_Epch = value;
    else if (StringMatch(sk, VAR_BNK0)) m_Bnk0 = value;
    else if (StringMatch(sk, VAR_CHS0_PERC)) m_Chs0_perc = value;
    else if (StringMatch(sk, VAR_VSEEP0)) m_Vseep0 = value;
    else if (StringMatch(sk, VAR_A_BNK)) m_aBank = value;
    else if (StringMatch(sk, VAR_B_BNK)) m_bBank = value;
    else if (StringMatch(sk, VAR_MSK_X)) m_mskX = value;
    else if (StringMatch(sk, VAR_MSK_CO1)) m_mskCoef1 = value;
        // else if (StringMatch(sk, VAR_MSK_CO2)) m_mskCoef2 = value;
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
    if (StringMatch(sk, VAR_SBOF)) m_qsSub[index] = value;
    else if (StringMatch(sk, VAR_SBIF)) m_qiSub[index] = value;
    else if (StringMatch(sk, VAR_SBQG)) m_qgSub[index] = value;
    else if (StringMatch(sk, VAR_SBPET)) m_petCh[index] = value;
    else if (StringMatch(sk, VAR_SBGS)) m_gwSto[index] = value;
        /// IN/OUTPUT variables
    else if (StringMatch(sk, VAR_QRECH)) m_qRchOut[index] = value;
    else if (StringMatch(sk, VAR_QS)) m_qsCh[index] = value;
    else if (StringMatch(sk, VAR_QI)) m_qiCh[index] = value;
    else if (StringMatch(sk, VAR_QG)) m_qgCh[index] = value;
    else {
        throw ModelException(MID_MUSK_CH, "SetValueByIndex", "Parameter " + sk + " does not exist.");
    }
}

void MUSK_CH::Set1DData(const char* key, const int n, float* data) {
    string sk(key);
    //check the input data
    if (StringMatch(sk, VAR_SUBBSN)) {
        m_subbsnID = data;
    } else if (StringMatch(sk, VAR_SBOF)) {
        CheckInputSize(key, n);
        m_qsSub = data;
    } else if (StringMatch(sk, VAR_SBIF)) {
        CheckInputSize(key, n);
        m_qiSub = data;
    } else if (StringMatch(sk, VAR_SBQG)) {
        CheckInputSize(key, n);
        m_qgSub = data;
    } else if (StringMatch(sk, VAR_SBPET)) {
        CheckInputSize(key, n);
        m_petCh = data;
    } else if (StringMatch(sk, VAR_SBGS)) {
        CheckInputSize(key, n);
        m_gwSto = data;
    } else {
        throw ModelException(MID_MUSK_CH, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void MUSK_CH::GetValue(const char* key, float* value) {
    InitialOutputs();
    string sk(key);
    /// IN/OUTPUT variables
    if (StringMatch(sk, VAR_QRECH) && m_inputSubbsnID > 0) *value = m_qRchOut[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_QS) && m_inputSubbsnID > 0) *value = m_qsCh[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_QI) && m_inputSubbsnID > 0) *value = m_qiCh[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_QG) && m_inputSubbsnID > 0) *value = m_qgCh[m_inputSubbsnID];
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
        m_qsCh[0] = m_qsCh[m_outletID];
        *data = m_qsCh;
    } else if (StringMatch(sk, VAR_QI)) {
        m_qiCh[0] = m_qiCh[m_outletID];
        *data = m_qiCh;
    } else if (StringMatch(sk, VAR_QG)) {
        m_qgCh[0] = m_qgCh[m_outletID];
        *data = m_qgCh;
    } else if (StringMatch(sk, VAR_BKST)) {
        m_bankSto[0] = m_bankSto[m_outletID];
        *data = m_bankSto;
    } else if (StringMatch(sk, VAR_CHST)) {
        m_chSto[0] = m_chSto[m_outletID];
        *data = m_chSto;
    } else if (StringMatch(sk, VAR_PRECHST)) {
        m_preChSto[0] = m_preChSto[m_outletID];
        *data = m_preChSto;
    } else if (StringMatch(sk, VAR_SEEPAGE)) {
        m_seepage[0] = m_seepage[m_outletID];
        *data = m_seepage;
    } else if (StringMatch(sk, VAR_CHWTDEPTH)) {
        m_chWtrDepth[0] = m_chWtrDepth[m_outletID];
        *data = m_chWtrDepth;
    } else if (StringMatch(sk, VAR_PRECHWTDEPTH)) {
        m_preChWtrDepth[0] = m_preChWtrDepth[m_outletID];
        *data = m_preChWtrDepth;
    } else if (StringMatch(sk, VAR_CHWTWIDTH)) {
        m_chWtrWth[0] = m_chWtrWth[m_outletID];
        *data = m_chWtrWth;
    } else if (StringMatch(sk, VAR_CHBTMWIDTH)) {
        m_chBtmWth[0] = m_chBtmWth[m_outletID];
        *data = m_chBtmWth;
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

    if (nullptr == m_reachDownStream) reaches->GetReachesSingleProperty(REACH_DOWNSTREAM, &m_reachDownStream);
    if (nullptr == m_chWth) reaches->GetReachesSingleProperty(REACH_WIDTH, &m_chWth);
    if (nullptr == m_chLen) reaches->GetReachesSingleProperty(REACH_LENGTH, &m_chLen);
    if (nullptr == m_chDepth) reaches->GetReachesSingleProperty(REACH_DEPTH, &m_chDepth);
    if (nullptr == m_area) reaches->GetReachesSingleProperty(REACH_AREA, &m_area);
    if (nullptr == m_chSideSlope) reaches->GetReachesSingleProperty(REACH_SIDESLP, &m_chSideSlope);
    if (nullptr == m_Kbank) reaches->GetReachesSingleProperty(REACH_KBANK, &m_Kbank);
    if (nullptr == m_Kchb) reaches->GetReachesSingleProperty(REACH_KBED, &m_Kchb);
    if (nullptr == m_chMan) reaches->GetReachesSingleProperty(REACH_MANNING, &m_chMan);
    if (nullptr == m_chSlope) reaches->GetReachesSingleProperty(REACH_SLOPE, &m_chSlope);

    m_reachUpStream = reaches->GetUpStreamIDs();
    m_rteLyrs = reaches->GetReachLayers();
}

//float MUSK_CH::manningQ(const float x1, const float x2, const float x3, const float x4) {
//    return x1 * pow(x2, 0.6666f) * sqrt(x4) / x3;
//}

//
//void MUSK_CH::GetDt(float timeStep, float fmin, float fmax, float& dt, int& n) {
//    if (fmax >= timeStep) {
//        dt = timeStep;
//        n = 1;
//        return;
//    }
//
//    n = CVT_INT(timeStep / fmax);
//    dt = timeStep / n;
//
//    if (dt > fmax) {
//        n++;
//        dt = timeStep / n;
//    }
//}
//
//void MUSK_CH::GetCoefficients(const float reachLength, const float v0, MuskWeights& weights) {
//    float K = (4.64f - 3.64f * m_mskCoef1) * reachLength / (5.f * v0 / 3.f);
//
//    float min = 2.f * K * m_mskX;
//    float max = 2.f * K * (1.f - m_mskX);
//    float dt;
//    int n;
//    GetDt(CVT_FLT(m_dt), min, max, dt, n);
//    weights.dt = dt;
//
//    //get coefficient
//    float temp = max + dt;
//    weights.c1 = (dt - min) / temp;
//    weights.c2 = (dt + min) / temp;
//    weights.c3 = (max - dt) / temp;
//    weights.c4 = 2 * dt / temp;
//    weights.n = n;
//
//    //make sure any coefficient is positive
//    if (weights.c1 < 0) {
//        weights.c2 += weights.c1;
//        weights.c1 = 0.f;
//    }
//    if (weights.c3 < 0) {
//        weights.c2 += weights.c1;
//        weights.c3 = 0.f;
//    }
//}

void MUSK_CH::updateWaterWidthDepth(const int i) {
    /// update channel water depth and width according to channel water storage
    float crossArea = m_chSto[i] / m_chLen[i];
    m_chWtrDepth[i] = (sqrt(m_chBtmWth[i] * m_chBtmWth[i] + 4.f * m_chSideSlope[i] * crossArea) -
        m_chBtmWth[i]) * 0.5f / m_chSideSlope[i];
    if (m_chWtrDepth[i] < UTIL_ZERO) {
        m_chWtrWth[i] = m_chBtmWth[i];
    } else {
        m_chWtrWth[i] = m_chBtmWth[i] + 2.f * m_chSideSlope[i] * m_chWtrDepth[i];
    }
}

void MUSK_CH::updateChannleBottomWidth(const int i) {
    // Code from ttcoef.f of SWAT source code.
    m_chBtmWth[i] = m_chWth[i] - 2.f * m_chSideSlope[i] * m_chDepth[i];
    if (m_chBtmWth[i] <= UTIL_ZERO) {
        m_chBtmWth[i] = 0.5f * m_chWth[i];
        m_chSideSlope[i] = (m_chWth[i] - m_chBtmWth[i]) * 0.5f / m_chDepth[i];
    }
}

bool MUSK_CH::ChannelFlow(const int i) {
    // 1. first add all the inflow water
    float qIn = 0.f; /// Water entering reach on current day from both current subbasin and upstreams
    // 1.1. water from this subbasin
    qIn += m_qsSub[i]; /// surface flow
    float qiSub = 0.f; /// interflow flow
    if (nullptr != m_qiSub && m_qiSub[i] >= 0.f) {
        qiSub = m_qiSub[i];
        qIn += qiSub;
    }
    float qgSub = 0.f; /// groundwater flow
    if (nullptr != m_qgSub && m_qgSub[i] >= 0.f) {
        qgSub = m_qgSub[i];
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
        if (m_qsCh[*upRchID] != m_qsCh[*upRchID]) {
            cout << "DayOfYear: " << m_dayOfYear << ", rchID: " << i << ", upRchID: " << *upRchID <<
                    ", surface part illegal!" << endl;
            return false;
        }
        if (m_qiCh[*upRchID] != m_qiCh[*upRchID]) {
            cout << "DayOfYear: " << m_dayOfYear << ", rchID: " << i << ", upRchID: " << *upRchID <<
                    ", subsurface part illegal!" << endl;
            return false;
        }
        if (m_qgCh[*upRchID] != m_qgCh[*upRchID]) {
            cout << "DayOfYear: " << m_dayOfYear << ", rchID: " << i << ", upRchID: " << *upRchID <<
                    ", groundwater part illegal!" << endl;
            return false;
        }
        if (m_qsCh[*upRchID] > 0.f) qsUp += m_qsCh[*upRchID];
        if (m_qiCh[*upRchID] > 0.f) qiUp += m_qiCh[*upRchID];
        if (m_qgCh[*upRchID] > 0.f) qgUp += m_qgCh[*upRchID];
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

    // Previous implementation. Remove in future.
    // add inflow water to storage
    //float st0 = m_chStorage[i];
    //m_chStorage[i] += qIn * m_dt;
    /// update channel water depth and width according to channel water storage
    //updateWaterWidthDepth(i);

    // Compute storage time constant for reach
    // The storage time constant calculated for the reach segment with bankfull flows(s).
    float k_bankfull = 0.f;
    // The storage time constant calculated for the reach segment with one-tenth of the bankfull flows(s).
    float k_bankfull2 = 0.f;
    // Compute flow and travel time at bankfull depth, from ttcoef.f of SWAT source code.
    // TODO, these two calculation should be moved to common_algorithm lib, which will be used when channel changes.
    float d = m_chDepth[i];
    float p = m_chBtmWth[i] + 2.f * d * sqrt(m_chSideSlope[i] * m_chSideSlope[i] + 1.f);
    float a = m_chBtmWth[i] * d + m_chSideSlope[i] * d * d;
    float rh = a / p;
    k_bankfull = m_chLen[i] / (manningQ(1.f, rh, m_chMan[i], m_chSlope[i]) * 5.f / 3.f) / 3600.f;
    // compute flow and travel time at 0.1 bankfull depth
    d = 0.1f * m_chDepth[i];
    p = m_chBtmWth[i] + 2.f * d * sqrt(m_chSideSlope[i] * m_chSideSlope[i] + 1.f);
    a = m_chBtmWth[i] * d + m_chSideSlope[i] * d * d;
    rh = a / p;
    k_bankfull2 = m_chLen[i] / (manningQ(1.f, rh, m_chMan[i], m_chSlope[i]) * 5.f / 3.f) / 3600.f; // sec->hour
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

    //////////////////////////////////////////////////////////////////////////
    // 2. Then subtract all the outflow water
    // 2.1. transmission losses to deep aquifer, which is lost from the system
    // the unit of kchb is mm/hr, 1. / 1000. / 3600. = 2.7777777777777776e-07
    //float seepage = m_Kchb[i] * 2.7777777777777776e-07f * m_chBtmWidth[i] * m_chLen[i] * m_dt;
    //if (qgSub < UTIL_ZERO) {
    //    if (m_chStorage[i] > seepage) {
    //        m_seepage[i] = seepage;
    //        m_chStorage[i] -= seepage;
    //    } else {
    //        m_seepage[i] = m_chStorage[i];
    //        m_chStorage[i] = 0.f;
    //    }
    //} else {
    //    m_seepage[i] = 0.f;
    //}

    // 2.2. calculate transmission losses to bank storage
    //float dch = m_chWTdepth[i];
    //float bankLen = dch * sqrt(1.f + m_chSideSlope[i] * m_chSideSlope[i]);
    //float bankInLoss = 2.f * m_Kbank[i] * 2.7777777777777776e-07f * bankLen * m_chLen[i] * m_dt; // m^3
    //if (m_chStorage[i] > bankInLoss) {
    //    m_chStorage[i] -= bankInLoss;
    //} else {
    //    bankInLoss = m_chStorage[i];
    //    m_chStorage[i] = 0.f;
    //}
    // water balance of the bank storage
    // loss the water from bank storage to the adjacent unsaturated zone and groundwater storage
    //float bankOutGw = m_bankStorage[i] * (1.f - exp(-m_bBank));
    //m_bankStorage[i] += bankInLoss - bankOutGw;
    //if (nullptr != m_gwStorage) {
    //    m_gwStorage[i] += bankOutGw / m_area[i] * 1000.f;
    //} // updated groundwater storage

    // 2.3. evaporation losses
    //float et = 0.f;
    //if (nullptr != m_petCh) {
    //    et = m_Epch * m_petCh[i] * 0.001f * m_chWTWidth[i] * m_chLen[i]; //m3
    //    if (m_chStorage[i] > et) {
    //        m_chStorage[i] -= et;
    //    } else {
    //        et = m_chStorage[i];
    //        m_chStorage[i] = 0.f;
    //    }
    //}
    //if (FloatEqual(m_chStorage[i], 0.f)) {
    //    m_qRchOut[i] = 0.f;
    //    m_qsCh[i] = 0.f;
    //    m_qiCh[i] = 0.f;
    //    m_qgCh[i] = 0.f;
    //    m_chWTdepth[i] = 0.f;
    //    m_chWTWidth[i] = 0.f;
    //    return true;
    //}
#ifdef PRINT_DEBUG
    cout << " chStorage before routing " << m_chStorage[i] << endl;
#endif
    //////////////////////////////////////////////////////////////////////////
    // routing, there are water in the channel after inflow and transmission loss
    //float totalLoss = m_seepage[i] + bankInLoss + et;

    //m_preChStorage[i] = m_chStorage[i];
    //m_preChWTDepth[i] = m_chWTdepth[i];
    //m_chStorage[i] = st0;

    float wtrin = qIn * m_dt / nn; // Inflow during a sub time interval, m^3
    float vol = 0.f;               // volume of water in reach, m^3
    float volrt = 0.f;             // flow rate, m^3/s
    float wet_p = 0.f;             // wetted perimeter
    float rcharea = 0.f;           // cross-sectional area
    float hydro_r = 0.f;           // hydraulic radius = cross-sectional area / wetted perimeter
    float max_rate = 0.f;          // maximum flow capacity of the channel at bank full
    float sdti = 0.f;              // average flow on day in reach, m^3/s
    float vc = 0.f;                // average flow velocity in channel, m/s
    float adddep = 0.f;            // added depth, m
    float addarea = 0.f;           // added cross-sectional area
    float addp = 0.f;              // added wetted perimeter
    float rtwtr = 0.f;             // water leaving reach on day, m^3
    float rttlc = 0.f;             // transmission losses from reach on day, m^3
    float qinday = 0.f;            // m^3
    float qoutday = 0.f;           // m^3
    // Iterate for the day
    for (int ii = 0; ii < nn; ii++) {
        // Calculate volume of water in reach
        vol = m_chSto[i] + wtrin; // m^3
        // Find average flowrate in a sub time interval
        volrt = vol / (86400.f / nn);
        // Find maximum flow capacity of the channel at bank full
        wet_p = m_chBtmWth[i] * 2.f * m_chDepth[i] * sqrt(1.f + m_chSideSlope[i] * m_chSideSlope[i]);
        rcharea = m_chBtmWth[i] * m_chDepth[i] + m_chSideSlope[i] * m_chDepth[i] * m_chDepth[i];
        hydro_r = rcharea / wet_p;
        max_rate = manningQ(rcharea, hydro_r, m_chMan[i], m_chSlope[i]);
        sdti = 0.f;
        m_chWtrDepth[i] = 0.f;
        rcharea = 0.f;
        vc = 0.f;
        // If average flowrate is greater than the channel capacity at bank full,
        //   then simulate flood plain flow, else simulate the regular channel flow.
        if (volrt > max_rate) {
            m_chWtrDepth[i] = m_chDepth[i];
            sdti = max_rate;
            adddep = 0.f;
            // Find the cross-sectional area and depth for volrt by iteration method at 1cm interval depth.
            // Find the depth until the discharge rate is equal to volrt
            while (sdti < volrt) {
                adddep += 0.01f;
                addarea = rcharea + (m_chWth[i] * 5.f + 4.f * adddep) * adddep;
                addp = wet_p + m_chWth[i] * 4.f + 2.f * adddep * sqrt(1.f + 4.f * 4.f);
                hydro_r = addarea / addp;
                sdti = manningQ(addarea, hydro_r, m_chMan[i], m_chSlope[i]);
            }
            rcharea = addarea;
            m_chWtrDepth[i] = m_chDepth[i] + adddep;
            wet_p = addp;
            sdti = volrt;
        } else {
            // Find the cross-sectional area and depth for volrt by iteration method at 1cm interval depth
            // Find the depth until the discharge rate is equal to volrt.
            while (sdti < volrt) {
                m_chWtrDepth[i] += 0.01f;
                rcharea = (m_chBtmWth[i] + m_chSideSlope[i] * m_chWtrDepth[i]) * m_chWtrDepth[i];
                wet_p = m_chBtmWth[i] + 2.f * m_chWtrDepth[i] * sqrt(1.f + m_chSideSlope[i] * m_chSideSlope[i]);
                hydro_r = rcharea / wet_p;
                sdti = manningQ(rcharea, hydro_r, m_chMan[i], m_chSlope[i]);
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
                rttlc = det * m_Kchb[i] * 0.001f * m_chLen[i] * wet_p; // m^3
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
                rttlc = rttlc1 + rttlc2;
            }
            // Calculate evaporation
            float rtevp = 0.f;
            float rtevp1 = 0.f;
            float rtevp2 = 0.f;
            if (rtwtr > 0.f) {
                float aaa = m_Epch * m_petCh[i] * 0.001f; // m
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
                rtevp = rtevp1 + rtevp2;
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
            m_qRchOut[i] = 0.f;
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
    m_qRchOut[i] = rtwtr / m_dt; // m^3/s

    // Previous implementation. Remove in future.
    //float q = 0.f;
    //for (int j = 0; j < nn; j++) {
    //    m_qRchOut[i] = c1 * qIn + c2 * m_qIn[i] + c3 * m_qRchOut[i];
    //    m_qIn[i] = qIn; // m^3/s
    //    float tmp = m_chStorage[i] + (qIn - totalLoss / m_dt - m_qRchOut[i]) * det * 3600.f;
    //    if (tmp < 0.f) {
    //        m_qRchOut[i] = m_chStorage[i] / (det * 3600.f) + qIn;
    //        m_chStorage[i] = 0.f;
    //    } else {
    //        m_chStorage[i] = tmp;
    //    }
    //    q += m_qRchOut[i];
    //}
    //m_qRchOut[i] = q / nn;

    float qInSum = m_qsSub[i] + qiSub + qgSub + qsUp + qiUp + qgUp + bankOut;
    if (qInSum < UTIL_ZERO) {
        // In case of divided by zero.
        m_qsCh[i] = 0.f;
        m_qiCh[i] = 0.f;
        m_qgCh[i] = 0.f;
        m_qRchOut[i] = 0.f;
    } else {
        m_qsCh[i] = m_qRchOut[i] * (m_qsSub[i] + qsUp) / qInSum;
        m_qiCh[i] = m_qRchOut[i] * (qiSub + qiUp) / qInSum;
        m_qgCh[i] = m_qRchOut[i] * (qgSub + qgUp) / qInSum;
    }

    // Previous implementation. Remove in future.
    // set variables for next time step
    //m_qIn[i] = qIn;
    //updateWaterWidthDepth(i);

#ifdef PRINT_DEBUG
    cout << " chStorage after routing " << m_chStorage[i] << endl;
    cout << " surfq: " << m_qsCh[i] << ", ifluq: " << m_qiCh[i] << ", groudq: " << m_qgCh[i] << endl;
#endif
    return true;
}
