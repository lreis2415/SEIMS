#include "MUSK_CH.h"

#include "utils_math.h"
#include "text.h"

using namespace utils_math;

MUSK_CH::MUSK_CH() :
    m_dt(-1), m_nreach(-1), m_inputSubbsnID(-1), m_outletID(-1), m_ptSub(nullptr),
    m_Kchb(nullptr), m_Kbank(nullptr), m_Epch(NODATA_VALUE), m_Bnk0(NODATA_VALUE),
    m_chSideSlope(nullptr), m_Chs0_perc(NODATA_VALUE),
    m_Vseep0(0.f), m_aBank(NODATA_VALUE), m_bBank(NODATA_VALUE),
    m_subbsnID(nullptr), m_area(nullptr),
    m_petCh(nullptr), m_qsSub(nullptr), m_qiSub(nullptr), m_qgSub(nullptr),
    m_gwStorage(nullptr), m_qsCh(nullptr), m_qiCh(nullptr), m_qgCh(nullptr),
    m_chWidth(nullptr), m_chWTWidth(nullptr),
    m_chBtmWidth(nullptr), m_chDepth(nullptr),
    m_chWTdepth(nullptr), m_preChWTDepth(nullptr), m_chLen(nullptr),
    m_chVel(nullptr), m_bankStorage(nullptr), m_deepGroundwater(0.f), m_seepage(nullptr),
    m_reachDownStream(nullptr), m_x(NODATA_VALUE),
    m_co1(NODATA_VALUE), m_chStorage(nullptr), m_preChStorage(nullptr),
    m_qRchOut(nullptr), m_qIn(nullptr) {
}

MUSK_CH::~MUSK_CH() {
    /// reaches related variables will be released in ~clsReaches(). By lj, 2017-12-26.

    if (nullptr != m_chStorage) Release1DArray(m_chStorage);
    if (nullptr != m_preChStorage) Release1DArray(m_preChStorage);
    if (nullptr != m_qRchOut) Release1DArray(m_qRchOut);
    if (nullptr != m_bankStorage) Release1DArray(m_bankStorage);
    if (nullptr != m_seepage) Release1DArray(m_seepage);
    if (nullptr != m_qsCh) Release1DArray(m_qsCh);
    if (nullptr != m_qiCh) Release1DArray(m_qiCh);
    if (nullptr != m_qgCh) Release1DArray(m_qgCh);
    if (nullptr != m_chWTdepth) Release1DArray(m_chWTdepth);
    if (nullptr != m_preChWTDepth) Release1DArray(m_preChWTDepth);
    if (nullptr != m_chWTWidth) Release1DArray(m_chWTWidth);
    if (nullptr != m_chBtmWidth) Release1DArray(m_chBtmWidth);
    if (nullptr != m_ptSub) Release1DArray(m_ptSub);
    // m_ptSrcFactory will be released by DataCenter->Scenario. lj
}

bool MUSK_CH::CheckInputData() {
    CHECK_POSITIVE(MID_MUSK_CH, m_dt);
    CHECK_POSITIVE(MID_MUSK_CH, m_nreach);
    CHECK_NONNEGATIVE(MID_MUSK_CH, m_inputSubbsnID);
    CHECK_POSITIVE(MID_MUSK_CH, m_outletID);
    CHECK_NODATA(MID_MUSK_CH, m_x);
    CHECK_NODATA(MID_MUSK_CH, m_co1);
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
    CHECK_POINTER(MID_MUSK_CH, m_chWidth);
    return true;
}

void MUSK_CH::InitialOutputs() {
    CHECK_POSITIVE(MID_MUSK_CH, m_nreach);
    //initial channel storage
    if (nullptr == m_chStorage) {
        m_chStorage = new float[m_nreach + 1];
        m_preChStorage = new float[m_nreach + 1];
        m_qIn = new float[m_nreach + 1];
        m_qRchOut = new float[m_nreach + 1];
        m_bankStorage = new float[m_nreach + 1];
        m_seepage = new float[m_nreach + 1];
        m_qsCh = new float[m_nreach + 1];
        m_qiCh = new float[m_nreach + 1];
        m_qgCh = new float[m_nreach + 1];
        m_chWTdepth = new float[m_nreach + 1];
        m_preChWTDepth = new float[m_nreach + 1];
        m_chWTWidth = new float[m_nreach + 1];
        m_chBtmWidth = new float[m_nreach + 1];

        for (int i = 1; i <= m_nreach; i++) {
            float qiSub = 0.f;
            float qgSub = 0.f;
            if (nullptr != m_qiSub) {
                // interflow (subsurface flow)
                qiSub = m_qiSub[i];
            }
            if (nullptr != m_qgSub) {
                // groundwater
                qgSub = m_qgSub[i];
            }
            m_seepage[i] = 0.f;
            m_bankStorage[i] = m_Bnk0 * m_chLen[i];
            m_chBtmWidth[i] = m_chWidth[i] - 2.f * m_chSideSlope[i] * m_chDepth[i];
            if (m_chBtmWidth[i] <= UTIL_ZERO) {
                m_chBtmWidth[i] = 0.5f * m_chWidth[i];
                m_chSideSlope[i] = (m_chWidth[i] - m_chBtmWidth[i]) * 0.5f / m_chDepth[i];
            }
            m_chWTdepth[i] = m_chDepth[i] * m_Chs0_perc;
            m_chWTWidth[i] = m_chBtmWidth[i] + 2.f * m_chSideSlope[i] * m_chWTdepth[i];
            m_preChWTDepth[i] = m_chWTWidth[i];
            m_chStorage[i] = m_chLen[i] * m_chWTdepth[i] * (m_chBtmWidth[i] + m_chSideSlope[i] * m_chWTdepth[i]);
            m_preChStorage[i] = m_chStorage[i];
            m_qIn[i] = 0.f;
            m_qRchOut[i] = m_qsSub[i] + qiSub + qgSub;
            m_qsCh[i] = m_qsSub[i];
            m_qiCh[i] = qiSub;
            m_qgCh[i] = qgSub;
        }
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
        vector<int>& m_ptSrcMgtSeqs = it->second->GetPointSrcMgtSeqs();
        map<int, PointSourceMgtParams *>& m_pointSrcMgtMap = it->second->GetPointSrcMgtMap();
        vector<int>& m_ptSrcIDs = it->second->GetPointSrcIDs();
        map<int, PointSourceLocations *>& m_pointSrcLocsMap = it->second->GetPointSrcLocsMap();
        // 1. looking for management operations from m_pointSrcMgtMap
        for (auto seqIter = m_ptSrcMgtSeqs.begin(); seqIter != m_ptSrcMgtSeqs.end(); ++seqIter) {
            PointSourceMgtParams* curPtMgt = m_pointSrcMgtMap.at(*seqIter);
            // 1.1 If current day is beyond the date range, then continue to next management
            if (curPtMgt->GetStartDate() != 0 && curPtMgt->GetEndDate() != 0) {
                if (m_date < curPtMgt->GetStartDate() || m_date > curPtMgt->GetEndDate()) {
                    continue;
                }
            }
            // 1.2 Otherwise, get the water volume
            float per_wtrVol = curPtMgt->GetWaterVolume(); /// m3/'size'/day
            // 1.3 Sum up all point sources
            for (auto locIter = m_ptSrcIDs.begin(); locIter != m_ptSrcIDs.end(); ++locIter) {
                if (m_pointSrcLocsMap.find(*locIter) != m_pointSrcLocsMap.end()) {
                    PointSourceLocations* curPtLoc = m_pointSrcLocsMap.at(*locIter);
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
        // the size of m_reachLayers (map) is equal to the maximum stream order
#pragma omp parallel for
        for (int i = 0; i < reachNum; i++) {
            int reachIndex = it->second[i]; // index in the array, i.e., subbasinID
            if (m_inputSubbsnID == 0 || m_inputSubbsnID == reachIndex) {
                // for OpenMP version, all reaches will be executed,
                // for MPI version, only the current reach will be executed.
                ChannelFlow(reachIndex);
            }
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
    else if (StringMatch(sk, VAR_MSK_X)) m_x = value;
    else if (StringMatch(sk, VAR_MSK_CO1)) m_co1 = value;
    else if (StringMatch(sk, VAR_GWRQ)) m_deepGroundwater = value;
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
    else if (StringMatch(sk, VAR_SBGS)) m_gwStorage[index] = value;
        /// IN/OUTPUT variables
    else if (StringMatch(sk, VAR_QRECH)) m_qRchOut[index] = value;
    else if (StringMatch(sk, VAR_QS)) m_qsCh[index] = value;
    else if (StringMatch(sk, VAR_QI)) m_qiCh[index] = value;
    else if (StringMatch(sk, VAR_QG)) m_qgCh[index] = value;
    else {
        throw ModelException(MID_MUSK_CH, "SetValueByIndex", "Parameter " + sk + " does not exist.");
    }
}

void MUSK_CH::Set1DData(const char* key, const int n, float* value) {
    string sk(key);
    //check the input data
    if (StringMatch(sk, VAR_SUBBSN)) {
        m_subbsnID = value;
    } else if (StringMatch(sk, VAR_SBOF)) {
        CheckInputSize(key, n);
        m_qsSub = value;
    } else if (StringMatch(sk, VAR_SBIF)) {
        CheckInputSize(key, n);
        m_qiSub = value;
    } else if (StringMatch(sk, VAR_SBQG)) {
        CheckInputSize(key, n);
        m_qgSub = value;
    } else if (StringMatch(sk, VAR_SBPET)) {
        CheckInputSize(key, n);
        m_petCh = value;
    } else if (StringMatch(sk, VAR_SBGS)) {
        CheckInputSize(key, n);
        m_gwStorage = value;
    } else {
        throw ModelException(MID_MUSK_CH, "Set1DData", "Parameter " + sk + " does not exist.");
    }
}

void MUSK_CH::GetValue(const char* key, float* value) {
    InitialOutputs();
    string sk(key);
    if (StringMatch(sk, VAR_QOUTLET)) *value = m_qRchOut[m_outletID];
    else if (StringMatch(sk, VAR_QSOUTLET)) *value = m_qsCh[m_outletID];
        /// IN/OUTPUT variables
    else if (StringMatch(sk, VAR_QRECH) && m_inputSubbsnID > 0) *value = m_qRchOut[m_inputSubbsnID];
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
        m_bankStorage[0] = m_bankStorage[m_outletID];
        *data = m_bankStorage;
    } else if (StringMatch(sk, VAR_CHST)) {
        m_chStorage[0] = m_chStorage[m_outletID];
        *data = m_chStorage;
    } else if (StringMatch(sk, VAR_PRECHST)) {
        m_preChStorage[0] = m_preChStorage[m_outletID];
        *data = m_preChStorage;
    } else if (StringMatch(sk, VAR_SEEPAGE)) {
        m_seepage[0] = m_seepage[m_outletID];
        *data = m_seepage;
    } else if (StringMatch(sk, VAR_CHWTDEPTH)) {
        m_chWTdepth[0] = m_chWTdepth[m_outletID];
        *data = m_chWTdepth;
    } else if (StringMatch(sk, VAR_PRECHWTDEPTH)) {
        m_preChWTDepth[0] = m_preChWTDepth[m_outletID];
        *data = m_preChWTDepth;
    } else if (StringMatch(sk, VAR_CHWTWIDTH)) {
        m_chWTWidth[0] = m_chWTWidth[m_outletID];
        *data = m_chWTWidth;
    } else if (StringMatch(sk, VAR_CHBTMWIDTH)) {
        m_chBtmWidth[0] = m_chBtmWidth[m_outletID];
        *data = m_chBtmWidth;
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
    if (nullptr == m_chWidth) reaches->GetReachesSingleProperty(REACH_WIDTH, &m_chWidth);
    if (nullptr == m_chLen) reaches->GetReachesSingleProperty(REACH_LENGTH, &m_chLen);
    if (nullptr == m_chDepth) reaches->GetReachesSingleProperty(REACH_DEPTH, &m_chDepth);
    if (nullptr == m_chVel) reaches->GetReachesSingleProperty(REACH_V0, &m_chVel);
    if (nullptr == m_area) reaches->GetReachesSingleProperty(REACH_AREA, &m_area);
    if (nullptr == m_chSideSlope) reaches->GetReachesSingleProperty(REACH_SIDESLP, &m_chSideSlope);
    if (nullptr == m_Kbank) reaches->GetReachesSingleProperty(REACH_KBANK, &m_Kbank);
    if (nullptr == m_Kchb) reaches->GetReachesSingleProperty(REACH_KBED, &m_Kchb);

    m_reachUpStream = reaches->GetUpStreamIDs();
    m_rteLyrs = reaches->GetReachLayers();
}

void MUSK_CH::GetDt(float timeStep, float fmin, float fmax, float& dt, int& n) {
    if (fmax >= timeStep) {
        dt = timeStep;
        n = 1;
        return;
    }

    n = int(timeStep / fmax);
    dt = timeStep / n;

    if (dt > fmax) {
        n++;
        dt = timeStep / n;
    }
}

void MUSK_CH::GetCoefficients(const float reachLength, const float v0, MuskWeights& weights) {
    float K = (4.64f - 3.64f * m_co1) * reachLength / (5.f * v0 / 3.f);

    float min = 2.f * K * m_x;
    float max = 2.f * K * (1.f - m_x);
    float dt;
    int n;
    GetDt(float(m_dt), min, max, dt, n);
    weights.dt = dt;

    //get coefficient
    float temp = max + dt;
    weights.c1 = (dt - min) / temp;
    weights.c2 = (dt + min) / temp;
    weights.c3 = (max - dt) / temp;
    weights.c4 = 2 * dt / temp;
    weights.n = n;

    //make sure any coefficient is positive
    if (weights.c1 < 0) {
        weights.c2 += weights.c1;
        weights.c1 = 0.f;
    }
    if (weights.c3 < 0) {
        weights.c2 += weights.c1;
        weights.c3 = 0.f;
    }
}

void MUSK_CH::updateWaterWidthDepth(const int i) {
    /// update channel water depth and width according to channel water storage
    float crossArea = m_chStorage[i] / m_chLen[i];
    m_chWTdepth[i] =
            (sqrt(m_chBtmWidth[i] * m_chBtmWidth[i] + 4.f * m_chSideSlope[i] * crossArea) - m_chBtmWidth[i]) / 2.f /
            m_chSideSlope[i];
    if (m_chWTdepth[i] < UTIL_ZERO) {
        m_chWTWidth[i] = m_chBtmWidth[i];
    } else {
        m_chWTWidth[i] = m_chBtmWidth[i] + 2.f * m_chSideSlope[i] * m_chWTdepth[i];
    }
}

void MUSK_CH::ChannelFlow(const int i) {
    float st0 = m_chStorage[i];
    float qiSub = 0.f; /// interflow flow
    if (nullptr != m_qiSub && m_qiSub[i] >= 0.f) {
        qiSub = m_qiSub[i];
    }
    float qgSub = 0.f; /// groundwater flow
    if (nullptr != m_qgSub && m_qgSub[i] >= 0.f) {
        qgSub = m_qgSub[i];
    }
    float ptSub = 0.f; /// point sources flow
    if (nullptr != m_ptSub && m_ptSub[i] >= 0.f) {
        ptSub = m_ptSub[i];
    }
    //////////////////////////////////////////////////////////////////////////
    // first add all the inflow water
    // 1. water from this subbasin
    float qIn = m_qsSub[i] + qiSub + qgSub + ptSub + m_deepGroundwater; /// m^3/s
    // 2. water from upstream reaches
    float qsUp = 0.f;
    float qiUp = 0.f;
    float qgUp = 0.f;
    for (size_t j = 0; j < m_reachUpStream[i].size(); ++j) {
        int upReachId = m_reachUpStream[i][j];
        qsUp += m_qsCh[upReachId];
        qiUp += m_qiCh[upReachId];
        qgUp += m_qgCh[upReachId];
    }
    qIn += qsUp + qiUp + qgUp; //qIn is equivalent to the wtrin variable in rtmusk.f of SWAT
    //if(i == 12)
    //	cout <<"surfaceQ: "<< m_qsSub[i] << ", subsurfaceQ: " << qiSub << ", groundQ: " << qgSub << ", pointQ: " << ptSub <<
    //	", UPsurfaceQ: "<<qsUp<<", UPsubsurface: "<<qiUp<<", UPground: "<<qgUp<<", \n";
    // 3. water from bank storage
    float bankOut = m_bankStorage[i] * (1.f - exp(-m_aBank));

    m_bankStorage[i] -= bankOut;
    qIn += bankOut / m_dt;

    // add inflow water to storage
    //if (i==12) cout<<"initial chStorage: "<<m_chStorage[i]<<", ";
    m_chStorage[i] += qIn * m_dt;
    /// update channel water depth and width according to channel water storage
    updateWaterWidthDepth(i);
    //if (i==12) cout<<"added chStorage: "<<m_chStorage[i]<<endl;
    //if(i == 2) cout <<"qIn:"<< qIn<<", chStorage: "<<m_chStorage[i]<<endl;
    //////////////////////////////////////////////////////////////////////////
    // then subtract all the outflow water
    // 1. transmission losses to deep aquifer, which is lost from the system
    // the unit of kchb is mm/hr, 1. / 1000. / 3600. = 2.7777777777777776e-07
    float seepage = m_Kchb[i] * 2.7777777777777776e-07f * m_chBtmWidth[i] * m_chLen[i] * m_dt;
    //if(i == 2) cout << "seepage: " << seepage << endl;
    if (qgSub < UTIL_ZERO) {
        if (m_chStorage[i] > seepage) {
            m_seepage[i] = seepage;
            m_chStorage[i] -= seepage;
        } else {
            m_seepage[i] = m_chStorage[i];
            m_chStorage[i] = 0.f;
        }
    } else {
        m_seepage[i] = 0.f;
    }

    // 2. calculate transmission losses to bank storage
    //float dch = m_chStorage[i] / (m_chWTWidth[i] * m_chLen[i]);
    float dch = m_chWTdepth[i];
    float bankLen = dch * sqrt(1.f + m_chSideSlope[i] * m_chSideSlope[i]);
    float bankInLoss = 2.f * m_Kbank[i] * 2.7777777777777776e-07f * bankLen * m_chLen[i] * m_dt; // m^3
    //bankInLoss = 0.f; //TODO, why not consider bankage loss?
    if (m_chStorage[i] > bankInLoss) {
        m_chStorage[i] -= bankInLoss;
    } else {
        bankInLoss = m_chStorage[i];
        m_chStorage[i] = 0.f;
    }
    // water balance of the bank storage
    // loss the water from bank storage to the adjacent unsaturated zone and groundwater storage
    float bankOutGw = m_bankStorage[i] * (1.f - exp(-m_bBank));
    //bankOutGw = 0.f; //TODO, why not consider?
    m_bankStorage[i] += bankInLoss - bankOutGw;
    if (nullptr != m_gwStorage) {
        m_gwStorage[i] += bankOutGw / m_area[i] * 1000.f;
    } // updated groundwater storage

    // 3. evaporation losses
    float et = 0.f;
    if (nullptr != m_petCh) {
        et = m_Epch * m_petCh[i] * 0.001f * m_chWTWidth[i] * m_chLen[i]; //m3
        if (m_chStorage[i] > et) {
            m_chStorage[i] -= et;
        } else {
            et = m_chStorage[i];
            m_chStorage[i] = 0.f;
        }
    }
    if (FloatEqual(m_chStorage[i], 0.f)) {
        m_qRchOut[i] = 0.f;
        m_qsCh[i] = 0.f;
        m_qiCh[i] = 0.f;
        m_qgCh[i] = 0.f;
        m_chWTdepth[i] = 0.f;
        m_chWTWidth[i] = 0.f;
        return;
    }

    //if(i == 2) cout << "chStorage before routing " << m_chStorage[i] << endl;
    //////////////////////////////////////////////////////////////////////////
    // routing, there are water in the channel after inflow and transmission loss
    float totalLoss = m_seepage[i] + bankInLoss + et;

    m_preChStorage[i] = m_chStorage[i];
    m_preChWTDepth[i] = m_chWTdepth[i];
    m_chStorage[i] = st0;

    // calculate coefficients
    MuskWeights wt;
    if (FloatEqual(m_chVel[i], 0.f)) m_chVel[i] = MIN_SLOPE;
    GetCoefficients(m_chLen[i], m_chVel[i], wt);
    int n = wt.n;
    float q = 0.f;
    for (int j = 0; j < n; j++) {
        m_qRchOut[i] = wt.c1 * qIn + wt.c2 * m_qIn[i] + wt.c3 * m_qRchOut[i];
        m_qIn[i] = qIn;
        float tmp = m_chStorage[i] + (qIn - totalLoss / m_dt - m_qRchOut[i]) * wt.dt;
        if (tmp < 0.f) {
            m_qRchOut[i] = m_chStorage[i] / wt.dt + qIn;
            m_chStorage[i] = 0.f;
        } else {
            m_chStorage[i] = tmp;
        }
        q += m_qRchOut[i];
    }
    m_qRchOut[i] = q / n;

    float qInSum = m_qsSub[i] + qiSub + qgSub + qsUp + qiUp + qgUp;
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

    // set variables for next time step
    m_qIn[i] = qIn;
    updateWaterWidthDepth(i);
}
