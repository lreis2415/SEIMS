#include "SEDR_SBAGNOLD.h"

#include "text.h"
#include "ChannelRoutingCommon.h"
//#ifndef PRINT_DEBUG
//#define PRINT_DEBUG
//#endif

SEDR_SBAGNOLD::SEDR_SBAGNOLD() :
    m_dt(-1), m_nreach(-1), m_inputSubbsnID(-1),
    m_vcd(false), m_peakRateAdj(NODATA_VALUE), m_sedTransEqCoef(NODATA_VALUE),
    m_sedTransEqExp(NODATA_VALUE),
    m_critVelSedDep(NODATA_VALUE), m_sedtoCh(nullptr),
    m_initChStorage(NODATA_VALUE), m_initChSedConc(NODATA_VALUE), m_qRchOut(nullptr),
    m_chOrder(nullptr), m_chWth(nullptr),
    m_chDepth(nullptr), m_chLen(nullptr),
    m_chSlope(nullptr), m_chBnkCov(nullptr), m_chBnkErod(nullptr), m_reachDownStream(nullptr),
    m_ptSub(nullptr), m_chSto(nullptr),
    m_chWtrDepth(nullptr), m_chWtrWth(nullptr),
    m_sedRchOut(nullptr),
    m_sedSto(nullptr), m_sedDep(nullptr), m_sedDeg(nullptr), m_sedConcRchOut(nullptr),
    m_sandSto(nullptr), m_siltSto(nullptr), m_claySto(nullptr),
    m_sagSto(nullptr), m_lagSto(nullptr), m_gravelSto(nullptr),
    m_rchBnkEro(nullptr), m_rchDeg(nullptr), m_rchDep(nullptr), m_fldplnDep(nullptr) {
}

SEDR_SBAGNOLD::~SEDR_SBAGNOLD() {
    /// reaches related variables will be released in ~clsReaches(). By lj, 2017-12-26.

    if (nullptr != m_ptSub) Release1DArray(m_ptSub);
    if (nullptr != m_sedRchOut) Release1DArray(m_sedRchOut);
    if (nullptr != m_sedConcRchOut) Release1DArray(m_sedConcRchOut);
    if (nullptr != m_sedSto) Release1DArray(m_sedSto);
    if (nullptr != m_sedDeg) Release1DArray(m_sedDeg);
    if (nullptr != m_sedDep) Release1DArray(m_sedDep);

    if (nullptr != m_sandSto) Release1DArray(m_sandSto);
    if (nullptr != m_siltSto) Release1DArray(m_siltSto);
    if (nullptr != m_claySto) Release1DArray(m_claySto);
    if (nullptr != m_sagSto) Release1DArray(m_sagSto);
    if (nullptr != m_lagSto) Release1DArray(m_lagSto);
    if (nullptr != m_gravelSto) Release1DArray(m_gravelSto);
    if (nullptr != m_rchBnkEro) Release1DArray(m_rchBnkEro);
    if (nullptr != m_rchDeg) Release1DArray(m_rchDeg);
    if (nullptr != m_rchDep) Release1DArray(m_rchDep);
    if (nullptr != m_fldplnDep) Release1DArray(m_fldplnDep);
}

bool SEDR_SBAGNOLD::CheckInputData() {
    CHECK_POSITIVE(MID_SEDR_SBAGNOLD, m_dt);
    CHECK_POSITIVE(MID_SEDR_SBAGNOLD, m_nreach);
    CHECK_NONNEGATIVE(MID_SEDR_SBAGNOLD, m_inputSubbsnID);
    CHECK_NODATA(MID_SEDR_SBAGNOLD, m_initChStorage);
    CHECK_NODATA(MID_SEDR_SBAGNOLD, m_peakRateAdj);
    CHECK_NODATA(MID_SEDR_SBAGNOLD, m_sedTransEqCoef);
    CHECK_NODATA(MID_SEDR_SBAGNOLD, m_sedTransEqExp);
    CHECK_NODATA(MID_SEDR_SBAGNOLD, m_critVelSedDep);
    CHECK_POINTER(MID_SEDR_SBAGNOLD, m_chSto);
    CHECK_POINTER(MID_SEDR_SBAGNOLD, m_chWth);
    CHECK_POINTER(MID_SEDR_SBAGNOLD, m_sedtoCh);
    CHECK_POINTER(MID_SEDR_SBAGNOLD, m_chWtrDepth);
    CHECK_POINTER(MID_SEDR_SBAGNOLD, m_qRchOut);
    return true;
}

void SEDR_SBAGNOLD::InitialOutputs() {
    CHECK_POSITIVE(MID_SEDR_SBAGNOLD, m_nreach);
    //initial channel storage
    if (nullptr == m_sedRchOut) {
        Initialize1DArray(m_nreach + 1, m_sedRchOut, 0.f);
        Initialize1DArray(m_nreach + 1, m_sedConcRchOut, 0.f);
        Initialize1DArray(m_nreach + 1, m_sedDep, 0.f);
        Initialize1DArray(m_nreach + 1, m_sedDeg, 0.f);
        Initialize1DArray(m_nreach + 1, m_sandSto, 0.f);
        Initialize1DArray(m_nreach + 1, m_siltSto, 0.f);
        Initialize1DArray(m_nreach + 1, m_claySto, 0.f);
        Initialize1DArray(m_nreach + 1, m_sagSto, 0.f);
        Initialize1DArray(m_nreach + 1, m_lagSto, 0.f);
        Initialize1DArray(m_nreach + 1, m_gravelSto, 0.f);
        Initialize1DArray(m_nreach + 1, m_rchBnkEro, 0.f);
        Initialize1DArray(m_nreach + 1, m_rchDeg, 0.f);
        Initialize1DArray(m_nreach + 1, m_rchDep, 0.f);
        Initialize1DArray(m_nreach + 1, m_fldplnDep, 0.f);
    }
    /// initialize point source loadings
    if (nullptr == m_ptSub) Initialize1DArray(m_nreach + 1, m_ptSub, 0.f);
}

void SEDR_SBAGNOLD::PointSourceLoading() {
    /// load point source water discharge (m3/s) on current day from Scenario
    for (auto it = m_ptSrcFactory.begin(); it != m_ptSrcFactory.end(); ++it) {
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
            float per_sed = curPtMgt->GetSedment(); /// g/cm3, or Mg/m3
            // 1.3 Sum up all point sources
            for (auto locIter = ptSrcIDs.begin(); locIter != ptSrcIDs.end(); ++locIter) {
                if (pointSrcLocsMap.find(*locIter) != pointSrcLocsMap.end()) {
                    PointSourceLocations* curPtLoc = pointSrcLocsMap.at(*locIter);
                    int curSubID = curPtLoc->GetSubbasinID();
                    /// Mg/m3 ==> kg / timestep, 1. / 86400. = 1.1574074074074073e-05
                    m_ptSub[curSubID] += per_sed * curPtLoc->GetSize() * 1000.f * m_dt * 1.1574074074074073e-05f;
                }
            }
        }
    }
}

int SEDR_SBAGNOLD::Execute() {
    CheckInputData();
    InitialOutputs();
    /// load point source water volume from m_ptSrcFactory
    PointSourceLoading();
    for (auto it = m_reachLayers.begin(); it != m_reachLayers.end(); ++it) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int nReaches = CVT_INT(it->second.size());
#pragma omp parallel for
        for (int i = 0; i < nReaches; i++) {
            int reachIndex = it->second[i]; // index in the array, which is equal to reach ID
            if (m_inputSubbsnID == 0 || m_inputSubbsnID == reachIndex) {
                // for OpenMP version, all reaches will be executed,
                // for MPI version, only the current reach will be executed.
                SedChannelRouting(reachIndex);
                // compute changes in channel dimensions caused by downcutting and widening
                if (m_vcd) {
                    ChannelDowncuttingWidening(reachIndex);
                }
            }
        }
    }
    return 0;
}

bool SEDR_SBAGNOLD::CheckInputSize(const char* key, const int n) {
    if (n <= 0) return false;
    if (m_nreach != n) {
        if (m_nreach <= 0) {
            m_nreach = n;
        } else {
            std::ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: " << n <<
                    ". The origin size is " << m_nreach << ".\n";
            throw ModelException(MID_SEDR_SBAGNOLD, "CheckInputSize", oss.str());
        }
    }
    return true;
}

void SEDR_SBAGNOLD::GetValue(const char* key, float* value) {
    InitialOutputs();
    string sk(key);
    // Get value for transferring across subbasins
    if (StringMatch(sk, VAR_SED_RECH)) *value = m_sedRchOut[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_SED_RECHConc)) *value = m_sedConcRchOut[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_RCH_BANKERO)) *value = m_rchBnkEro[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_RCH_DEG)) *value = m_rchDeg[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_RCH_DEP)) *value = m_rchDep[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_FLDPLN_DEP)) *value = m_fldplnDep[m_inputSubbsnID];
    else {
        throw ModelException(MID_SEDR_SBAGNOLD, "GetValue", "Parameter " + sk + " does not exist.");
    }
}

void SEDR_SBAGNOLD::SetValue(const char* key, const float value) {
    string sk(key);
    if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbsnID = CVT_INT(value);
#ifdef STORM_MODE
    else if (StringMatch(sk, Tag_ChannelTimeStep)) m_dt = CVT_INT(value);
#else
    else if (StringMatch(sk, Tag_TimeStep)) m_dt = CVT_INT(value);
#endif /* STORM_MODE */
    else if (StringMatch(sk, VAR_P_RF)) m_peakRateAdj = value;
    else if (StringMatch(sk, VAR_SPCON)) m_sedTransEqCoef = value;
    else if (StringMatch(sk, VAR_SPEXP)) m_sedTransEqExp = value;
    else if (StringMatch(sk, VAR_VCRIT)) m_critVelSedDep = value;
    else if (StringMatch(sk, VAR_CHS0)) m_initChStorage = value;
    else if (StringMatch(sk, VAR_SED_CHI0)) m_initChSedConc = value;
    else if (StringMatch(sk, VAR_VCD)) m_vcd = FloatEqual(value, 1.f);
    else {
        throw ModelException(MID_SEDR_SBAGNOLD, "SetValue", "Parameter " + sk + " does not exist.");
    }
}

void SEDR_SBAGNOLD::SetValueByIndex(const char* key, const int index, const float value) {
    if (m_inputSubbsnID == 0) return;           // Not for omp version
    if (index <= 0 || index > m_nreach) return; // index should belong 1 ~ m_nreach
    if (nullptr == m_sedRchOut) InitialOutputs();
    string sk(key);
    // transferred single value in MPI version�� IN/OUTPUT variables
    if (StringMatch(sk, VAR_SED_RECH)) m_sedRchOut[index] = value;
    else if (StringMatch(sk, VAR_SED_RECHConc)) m_sedConcRchOut[index] = value;
    else if (StringMatch(sk, VAR_RCH_BANKERO)) m_rchBnkEro[index] = value;
    else if (StringMatch(sk, VAR_RCH_DEG)) m_rchDeg[index] = value;
    else if (StringMatch(sk, VAR_RCH_DEP)) m_rchDep[index] = value;
    else if (StringMatch(sk, VAR_FLDPLN_DEP)) m_fldplnDep[index] = value;
    else {
        throw ModelException(MID_SEDR_SBAGNOLD, "SetValueByIndex", "Parameter " + sk + " does not exist");
    }
}

void SEDR_SBAGNOLD::Set1DData(const char* key, const int n, float* data) {
    string sk(key);
    if (StringMatch(sk, VAR_SED_TO_CH)) m_sedtoCh = data;        //for longterm model
    else if (StringMatch(sk, VAR_SUB_SEDTOCH)) m_sedtoCh = data; //for storm model // TODO
    else if (StringMatch(sk, VAR_QRECH)) m_qRchOut = data;
    else if (StringMatch(sk, VAR_CHST)) {
        m_chSto = data;
        if (m_nreach + 1 != n) m_nreach = n - 1;
        if (nullptr == m_sedSto) Initialize1DArray(m_nreach + 1, m_sedSto, 0.f);
        for (int i = 1; i <= m_nreach; i++) {
            // m_Chs0 is initial channel storage per meter, not sediment! By LJ
            m_sedSto[i] = m_initChSedConc * m_chSto[i] * 1000.f; /// ton/m3 * m3/m * m * 1000 = kg
        }
    } else if (StringMatch(sk, VAR_RTWTR)) m_rteWtrOut = data;
    else if (StringMatch(sk, VAR_CHBTMWIDTH)) m_chBtmWth = data;
    else if (StringMatch(sk, VAR_CHWTRDEPTH)) m_chWtrDepth = data;
    else if (StringMatch(sk, VAR_CHWTRWIDTH)) m_chWtrWth = data;
    else {
        throw ModelException(MID_SEDR_SBAGNOLD, "Set1DData", "Parameter " + sk + " does not exist");
    }
}

void SEDR_SBAGNOLD::Get1DData(const char* key, int* n, float** data) {
    InitialOutputs();
    string sk(key);
    *n = m_nreach + 1;
    if (StringMatch(sk, VAR_SED_RECH)) *data = m_sedRchOut;
    else if (StringMatch(sk, VAR_SED_RECHConc)) *data = m_sedConcRchOut;
    else if (StringMatch(sk, VAR_RCH_BANKERO)) *data = m_rchBnkEro;
    else if (StringMatch(sk, VAR_RCH_DEG)) *data = m_rchDeg;
    else if (StringMatch(sk, VAR_RCH_DEP)) *data = m_rchDep;
    else if (StringMatch(sk, VAR_FLDPLN_DEP)) *data = m_fldplnDep;
    else {
        throw ModelException(MID_SEDR_SBAGNOLD, "Get1DData", "Output " + sk + " does not exist.");
    }
}

void SEDR_SBAGNOLD::SetScenario(Scenario* sce) {
    if (nullptr == sce) {
        throw ModelException(MID_SEDR_SBAGNOLD, "SetScenario", "The scenario can not to be NULL.");
    }
    map<int, BMPFactory *>& tmpBMPFactories = sce->GetBMPFactories();
    for (auto it = tmpBMPFactories.begin(); it != tmpBMPFactories.end(); ++it) {
        /// Key is uniqueBMPID, which is calculated by BMP_ID * 100000 + subScenario;
        if (it->first / 100000 != BMP_TYPE_POINTSOURCE) continue;
#ifdef HAS_VARIADIC_TEMPLATES
        m_ptSrcFactory.emplace(it->first, static_cast<BMPPointSrcFactory *>(it->second));
#else
        m_ptSrcFactory.insert(make_pair(it->first, static_cast<BMPPointSrcFactory *>(it->second)));
#endif
    }
}

void SEDR_SBAGNOLD::SetReaches(clsReaches* reaches) {
    if (nullptr == reaches) {
        throw ModelException(MID_SEDR_SBAGNOLD, "SetReaches", "The reaches input can not to be NULL.");
    }
    m_nreach = reaches->GetReachNumber();

    if (nullptr == m_reachDownStream) reaches->GetReachesSingleProperty(REACH_DOWNSTREAM, &m_reachDownStream);
    if (nullptr == m_chOrder) reaches->GetReachesSingleProperty(REACH_UPDOWN_ORDER, &m_chOrder);
    if (nullptr == m_chWth) reaches->GetReachesSingleProperty(REACH_WIDTH, &m_chWth);
    if (nullptr == m_chLen) reaches->GetReachesSingleProperty(REACH_LENGTH, &m_chLen);
    if (nullptr == m_chDepth) reaches->GetReachesSingleProperty(REACH_DEPTH, &m_chDepth);
    if (nullptr == m_chSlope) reaches->GetReachesSingleProperty(REACH_SLOPE, &m_chSlope);
    if (nullptr == m_chSideSlope) reaches->GetReachesSingleProperty(REACH_SIDESLP, &m_chSideSlope);
    if (nullptr == m_chBnkCov) reaches->GetReachesSingleProperty(REACH_BNKCOV, &m_chBnkCov);
    if (nullptr == m_chBnkErod) reaches->GetReachesSingleProperty(REACH_BEDEROD, &m_chBnkErod);

    m_reachUpStream = reaches->GetUpStreamIDs();
    m_reachLayers = reaches->GetReachLayers();
}

void SEDR_SBAGNOLD::SedChannelRouting(const int i) {
    if (m_rteWtrOut[i] <= UTIL_ZERO || m_chWtrDepth[i] <= UTIL_ZERO) {
        m_sedRchOut[i] = 0.f;
        m_sedConcRchOut[i] = 0.f;
        m_sandRchOut[i] = 0.f;
        m_siltRchOut[i] = 0.f;
        m_clayRchOut[i] = 0.f;
        m_sagRchOut[i] = 0.f;
        m_lagRchOut[i] = 0.f;
        m_gravelRchOut[i] = 0.f;
        m_rchBnkEro[i] = 0.f;
        return;
    }
    // initialize sediment in reach during time step
    /// sediment from upstream reaches
    float sedin = 0.f;    ///< all sediment, kg, sedin * 1000 in SWAT
    float sandin = 0.f;   ///< all sand, kg, sanin * 1000 in SWAT
    float siltin = 0.f;   ///< all silt, kg, silin * 1000 in SWAT
    float clayin = 0.f;   ///< all clay, kg, clain * 1000 in SWAT
    float sagin = 0.f;    ///< all small aggregate, kg, sagin * 1000 in SWAT
    float lagin = 0.f;    ///< all large aggregate, kg, lagin * 1000 in SWAT
    float gravelin = 0.f; ///< all gravel, kg, grain * 1000 in SWAT
    for (auto upRchID = m_reachUpStream.at(i).begin(); upRchID != m_reachUpStream.at(i).end(); ++upRchID) {
        sedin += m_sedRchOut[*upRchID];
        sandin += m_sandRchOut[*upRchID];
        siltin += m_siltRchOut[*upRchID];
        clayin += m_clayRchOut[*upRchID];
        sagin += m_sagRchOut[*upRchID];
        lagin += m_lagRchOut[*upRchID];
        gravelin += m_gravelRchOut[*upRchID];
    }
    sedin += m_sedtoCh[i] + m_sedSto[i];
    sandin += m_sandtoCh[i] + m_sandSto[i];
    siltin += m_silttoCh[i] + m_siltSto[i];
    clayin += m_claytoCh[i] + m_claySto[i];
    sagin += m_sagtoCh[i] + m_sagSto[i];
    lagin += m_lagtoCh[i] + m_lagSto[i];
    gravelin += m_graveltoCh[i] + m_gravelSto[i];
    /// add point source loadings
    if (nullptr != m_ptSub && m_ptSub[i] > 0.f) {
        sedin += m_ptSub[i];
    }

    // initialize water in reach during time step
    float allWater = m_chSto[i] + m_rteWtrOut[i]; ///< water in reach during time step, m^3, qdin in SWAT

    if (((m_rteWtrOut[i] <= UTIL_ZERO) || (m_chWtrDepth[i] <= UTIL_ZERO)) || (allWater <= 0.01f)) {
        /// do not perform sediment routing when:
        /// 1. whether is no water flow out of channel and water depth is nearly zero
        /// 2. if no water in reach
        m_sedDeg[i] = 0.f;
        m_sedDep[i] = 0.f;
        m_sedRchOut[i] = 0.f;
        m_sedConcRchOut[i] = 0.f;
        m_sandSto[i] = 0.f;
        m_siltSto[i] = 0.f;
        m_claySto[i] = 0.f;
        m_sagSto[i] = 0.f;
        m_lagSto[i] = 0.f;
        m_gravelSto[i] = 0.f;
        m_rchBnkEro[i] = 0.f;
        m_rchDeg[i] = 0.f;
        m_rchDep[i] = 0.f;
        m_fldplnDep[i] = 0.f;
        m_sedSto[i] = sedin;
        return;
    }
    // initialize reach peak runoff rate and calculate flow velocity. SWAT Theory 2009, p.448
    float cross_area = ChannelCrossSectionalArea(m_chBtmWth[i], m_chDepth[i], m_chWtrDepth[i],
                                                 m_chSideSlope[i], m_chWth[i], 4.f);
    float peakRfRate = m_qRchOut[i] * m_peakRateAdj; ///< peak runoff rate, unit: m^3/s
    float peakVel = 0.f;                             ///< peak flow velocity, vc in swat, unit: m/s
    if (cross_area < 0.01f) {
        peakVel = 0.01f;
    } else {
        peakVel = peakRfRate / cross_area;
    }
    if (peakVel > 5.f) {
        peakVel = 5.f;
    }
    /// calculate tbase  |none  |flow duration (fraction of 24 hr)
    float tbase = m_chLen[i] / (m_dt * peakVel);
    if (tbase > 1.f) tbase = 1.f;
#ifdef PRINT_DEBUG
    cout << "ID: " << i << ", qchOut: " << m_qRchOut[i] << ", allwater: " << allWater <<
            ", chStorage: " << m_chStorage[i] <<
            ", sedUp: " << sedUp << ", sedtoCh: " << m_sedtoCh[i] <<
            ", sedStorage: " << m_sedStorage[i] << ", allSediment: " << allSediment <<
            ", chLen: " << m_chLen[i] << ", peakVelocity: " << peakVel << ", tbase: " << tbase;
#endif
    // New improved method for sediment transport
    float cyin = 0.f;          ///< concentrate of flow in sediment, kg/m^3
    float cych = 0.f;          ///< concentrate of sediment in channel
    float sedDepNet = 0.f;     ///< depnet
    float sedDeg1 = 0.f;       ///< deg1
    float sedDeg1Sand = 0.f;   ///< deg1san
    float sedDeg1Silt = 0.f;   ///< deg1sil
    float sedDeg1Clay = 0.f;   ///< deg1cla
    float sedDeg1Sag = 0.f;    ///< deg1sag, small aggregate
    float sedDeg1Lag = 0.f;    ///< deg1lag, large aggregate
    float sedDeg1Gravel = 0.f; ///< deg1gra
    float sedDegRemain = 0.f;  ///< degremain
    float sedDegSand = 0.f;    ///< degsan
    float sedDegSilt = 0.f;    ///< degsil
    float sedDegClay = 0.f;    ///< degcla
    float sedDegGravel = 0.f;  ///< deggra
    float bnkSand = 0.f;       ///< bnksan
    float bnkSilt = 0.f;       ///< bnksil
    float bnkClay = 0.f;       ///< bnkcla
    float bnkGravel = 0.f;     ///< bnkgra
    float sedDep = 0.f;        ///< dep
    float sedDepSand = 0.f;    ///< depsan
    float sedDepSilt = 0.f;    ///< depsil
    float sedDepClay = 0.f;    ///< depcla
    float sedDepSag = 0.f;     ///< depsag
    float sedDepLag = 0.f;     ///< deplag
    float sedDepGravel = 0.f;  ///< depgra

    // Hydraulic radius
    float wet_peri = ChannelWettingPerimeter(m_chBtmWth[i], m_chDepth[i], m_chWtrDepth[i],
                                             m_chSideSlope[i], m_chWth[i], 4.f);
    float pbank = wet_peri - m_chBtmWth[i];
    float rh = cross_area - wet_peri;

    // Area ratio of water in flood plain to total cross-sectional area
    float fpratio = 0.f;
    if (m_chWtrDepth[i] > m_chDepth[i]) {
        float adddep = m_chWtrDepth[i] - m_chDepth[i];
        float cross_area_bankful = ChannelCrossSectionalArea(m_chBtmWth[i], m_chDepth[i], m_chSideSlope[i]);
        fpratio = 1.f - (cross_area_bankful + adddep * m_chWth[i]) / cross_area;
        fpratio = Max(0.f, fpratio);
    }

    // Applied bank shear stress, equations from Eaton and Millar (2004)
    float sfbank = pow(10.f, -1.4026f * log10(m_chBtmWth[i] / pbank + 1.5f) + 2.247f);
    float tou = 9800.f * m_chWtrDepth[i] * m_chSlope[i];
    float asinea = 1.f / sqrt(1.f + m_chSideSlope[i] * m_chSideSlope[i]);
    float tbank = tou * sfbank / 100.f * (m_chWtrWth[i] + m_chBtmWth[i]) * asinea / (4.f * m_chWtrDepth[i]);
    float tbed = tou * (1.f - sfbank / 100.f) * (m_chWtrWth[i] / 2.f / m_chBtmWth[i] + 0.5f);

    // Potential bank erosion rate in kg per day
    // Assumed on an average only one bank eroding due to meandering of channel
    float bnkrte = m_chBnkErod[i] * (tbank - m_chBnkTc[i]) * 1.e-6f; // cm^3/N/s * N/m^2 * 1.e-6  = m/s
    if (bnkrte < 0.f) bnkrte = 0.f;
    bnkrte *= m_chLen[i] * (m_chWtrDepth[i] * sqrt(1.f + m_chSideSlope[i] * m_chSideSlope[i])) *
            m_chBnkBD[i] * m_dt * 1000.f; // m/s * m * m * g/cm^3 = 1000 kg
    // Potential bed degradation rate in kg per day
    float degrte = m_chBedErod[i] * (tbed - m_chBedTc[i]) * 1.e-6f;
    if (degrte < 0.f) degrte = 0.f;
    degrte *= m_chLen[i] * m_chBtmWth[i] * m_chBedBD[i] * m_dt * 1000.f;

    // Relative potential for bank/bed erosion
    float bnkrt = 1.f;
    if (bnkrte + degrte > 1.e-6f) {
        bnkrt = bnkrte / (bnkrte + degrte);
    }
    bnkrt = Min(1.f, bnkrt);
    float bedrt = 1.f - bnkrt; ///< Relative potential for bed erosion

    // Incoming sediment concentration
    cyin = sedin / allWater; // kg/m^3

    // Streampower for sediment calculated based on Bagnold (1977) concept
    cych = m_sedTransEqCoef * pow(peakVel, m_sedTransEqExp) * 1000.f; // kg/m^3
#ifdef PRINT_DEBUG
    cout << ", cyin: " << cyin << ", cych: " << cych << endl;
#endif
    // Potential sediment Transport capacity, kg
    sedDepNet = allWater * (cych - cyin);
    if (sedDepNet <= UTIL_ZERO) {
        sedDepNet = 0.f;
        bnkrte = 0.f;
        degrte = 0.f;
    } else {
        // First the deposited material will be degraded before channel bed or band erosion
        if (sedDepNet >= m_rchDep[i]) {
            // Effective erosion
            float effbnkbed = sedDepNet - m_rchDep[i];
            // Effective bank erosion
            bnkrte = Min(effbnkbed * bnkrt, bnkrte);
            bnkSand = bnkrte * m_chBnkSand[i];
            bnkSilt = bnkrte * m_chBnkSilt[i];
            bnkClay = bnkrte * m_chBnkClay[i];
            bnkGravel = bnkrte * m_chBnkGravel[i];
            // Effective bed erosion (degradation)
            degrte = Min(effbnkbed * bedrt, degrte);
            sedDegSand = degrte * m_chBedSand[i];
            sedDegSilt = degrte * m_chBedSilt[i];
            sedDegClay = degrte * m_chBedClay[i];
            sedDegGravel = degrte * m_chBedGravel[i];

            sedDeg1 = m_rchDep[i];
            sedDeg1Sand = m_rchDepSand[i];
            sedDeg1Silt = m_rchDepSilt[i];
            sedDeg1Clay = m_rchDepClay[i];
            sedDeg1Sag = m_rchDepSag[i];
            sedDeg1Lag = m_rchDepLag[i];
            sedDeg1Gravel = m_rchDepGravel[i];

            m_rchDep[i] = 0.f;
            m_rchDepSand[i] = 0.f;
            m_rchDepSilt[i] = 0.f;
            m_rchDepClay[i] = 0.f;
            m_rchDepSag[i] = 0.f;
            m_rchDepLag[i] = 0.f;
            m_rchDepGravel[i] = 0.f;
        } else {
            bnkrte = 0.f;
            degrte = 0.f;
            sedDegSand = 0.f;
            sedDegSilt = 0.f;
            sedDegClay = 0.f;
            sedDegGravel = 0.f;
            bnkSand = 0.f;
            bnkSilt = 0.f;
            bnkClay = 0.f;
            bnkGravel = 0.f;
            m_rchDep[i] -= sedDepNet;
            sedDeg1 = sedDepNet;
            if (m_rchDepClay[i] >= sedDepNet) {
                m_rchDepClay[i] -= sedDepNet;
                sedDeg1Clay = sedDepNet;
                sedDegRemain = 0.f;
            } else {
                sedDegRemain = sedDepNet - m_rchDepClay[i];
                sedDeg1Clay = m_rchDepClay[i];
                m_rchDepClay[i] = 0.f;
                if (m_rchDepSilt[i] >= sedDegRemain) {
                    m_rchDepSilt[i] -= sedDegRemain;
                    sedDeg1Silt = sedDegRemain;
                    sedDegRemain = 0.f;
                } else {
                    sedDegRemain -= m_rchDepSilt[i];
                    sedDeg1Silt = m_rchDepSilt[i];
                    m_rchDepSilt[i] = 0.f;
                    if (m_rchDepSag[i] >= sedDegRemain) {
                        m_rchDepSag[i] -= sedDegRemain;
                        sedDeg1Sag = sedDegRemain;
                        sedDegRemain = 0.f;
                    } else {
                        sedDegRemain -= m_rchDepSag[i];
                        sedDeg1Sag = m_rchDepSag[i];
                        m_rchDepSag[i] = 0.f;
                        if (m_rchDepSand[i] >= sedDegRemain) {
                            m_rchDepSand[i] -= sedDegRemain;
                            sedDeg1Sand = sedDegRemain;
                            sedDegRemain = 0.f;
                        } else {
                            sedDegRemain -= m_rchDepSand[i];
                            sedDeg1Sand = m_rchDepSand[i];
                            m_rchDepSand[i] = 0.f;
                            if (m_rchDepLag[i] >= sedDegRemain) {
                                m_rchDepLag[i] -= sedDegRemain;
                                sedDeg1Lag = sedDegRemain;
                                sedDegRemain = 0.f;
                            } else {
                                sedDegRemain -= m_rchDepLag[i];
                                sedDeg1Lag = m_rchDepLag[i];
                                m_rchDepLag[i] = 0.f;
                                if (m_rchDepGravel[i] >= sedDegRemain) {
                                    m_rchDepGravel[i] -= sedDegRemain;
                                    sedDeg1Gravel = sedDegRemain;
                                    sedDegRemain = 0.f;
                                } else {
                                    sedDegRemain -= m_rchDepGravel[i];
                                    sedDeg1Gravel = m_rchDepGravel[i];
                                    m_rchDepGravel[i] = 0.f;
                                } /* m_rchDepGravel */
                            }     /* m_rchDepLag */
                        }         /* m_rchDepSand */
                    }             /* m_rchDepSag */
                }                 /* m_rchDepSilt */
            }                     /* m_rchDepClay */
        }                         /* sedDepNet < m_rchDep[i] */
    }                             /* sedDepNet > UTIL_ZERO */

    if (m_rchDep[i] < UTIL_ZERO) {
        m_rchDep[i] = 0.f;
        m_rchDepSand[i] = 0.f;
        m_rchDepSilt[i] = 0.f;
        m_rchDepClay[i] = 0.f;
        m_rchDepSag[i] = 0.f;
        m_rchDepLag[i] = 0.f;
        m_rchDepGravel[i] = 0.f;
    }

    // Fall velocity based on equation 1.36 from SWRRB mannual
    //vgra = 411.0 * ((2.00)**2.) / (3600.)  ==> 0.45666667f
    //vsan = 411.0 * ((0.20)**2.) / (3600.)  ==> 0.00456667f
    //vsil = 411.0 * ((0.01)**2.) / (3600.)  ==> 1.14166667e-05f
    //vcla = 411.0 * ((0.002)**2.) / (3600.) ==> 4.56666667e-07f
    //vsag = 411.0 * ((0.03)**2.) / (3600.)  ==> 0.00010275f
    //vlag = 411.0 * ((0.50)**2.) / (3600.)  ==> 0.02854167f

    // Deposition calculated based on Einsten equation
    float x = 0.f;

    /// Gravel deposition
    x = 1.055f * m_chLen[i] * 0.45666667f / (peakVel * m_chWtrDepth[i]);
    if (x > 20.f) x = 20.f;
    sedDepGravel = gravelin * Min(1.f - exp(-x), 1.f);

    /// Sand deposition
    x = 1.055f * m_chLen[i] * 0.00456667f / (peakVel * m_chWtrDepth[i]);
    if (x > 20.f) x = 20.f;
    sedDepSand = sandin * Min(1.f - exp(-x), 1.f);

    /// Silt deposition
    x = 1.055f * m_chLen[i] * 1.14166667e-05f / (peakVel * m_chWtrDepth[i]);
    if (x > 20.f) x = 20.f;
    sedDepSilt = siltin * Min(1.f - exp(-x), 1.f);

    /// Clay deposition
    x = 1.055f * m_chLen[i] * 4.56666667e-07f / (peakVel * m_chWtrDepth[i]);
    if (x > 20.f) x = 20.f;
    sedDepClay = clayin * Min(1.f - exp(-x), 1.f);

    /// Small aggregate deposition
    x = 1.055f * m_chLen[i] * 0.00010275f / (peakVel * m_chWtrDepth[i]);
    if (x > 20.f) x = 20.f;
    sedDepSag = sagin * Min(1.f - exp(-x), 1.f);

    /// Large aggregate deposition
    x = 1.055f * m_chLen[i] * 0.02854167f / (peakVel * m_chWtrDepth[i]);
    if (x > 20.f) x = 20.f;
    sedDepLag = lagin * Min(1.f - exp(-x), 1.f);

    sedDep = sedDepSand + sedDepSilt + sedDepClay + sedDepSag + sedDepLag + sedDepGravel;

    // Particles deposited on Floodplain (only silt and clay type particles)
    m_fldplnDep[i] += (sedDepSilt + sedDepClay) * fpratio;
    m_fldplnDepSilt[i] += sedDepSilt * fpratio;
    m_fldplnDepClay[i] += sedDepClay * fpratio;

    // Remaining is deposited in the channel
    m_rchDep[i] += sedDep - (sedDepSilt + sedDepClay) * fpratio;
    m_rchDepSilt[i] += sedDepSilt * (1.f - fpratio);
    m_rchDepClay[i] += sedDepClay * (1.f - fpratio);
    m_rchDepSand[i] += sedDepSand;
    m_rchDepSag[i] += sedDepSag;
    m_rchDepLag[i] += sedDepLag;
    m_rchDepGravel[i] += sedDepGravel;

    sedin += degrte + bnkrte + sedDeg1 - sedDep;
    sandin += sedDegSand + bnkSand + sedDeg1Sand - sedDepSand;
    siltin += sedDegSilt + bnkSilt + sedDeg1Silt - sedDepSilt;
    clayin += sedDegClay + bnkClay + sedDeg1Clay - sedDepClay;
    sagin += sedDeg1Sag - sedDepSag;
    lagin += sedDeg1Lag - sedDepLag;
    gravelin += sedDegGravel + bnkGravel + sedDeg1Gravel - sedDepGravel;

    if (sedin < UTIL_ZERO) {
        sedin = 0.f;
        sandin = 0.f;
        siltin = 0.f;
        clayin = 0.f;
        sagin = 0.f;
        lagin = 0.f;
        gravelin = 0.f;
    }

    // Routing out sediment (kg)
    float outfract = m_rteWtrOut[i] / allWater;
    m_sedRchOut[i] = sedin * outfract;                    // sedrch in SWAT
    m_sedConcRchOut[i] = m_sedRchOut[i] / m_rteWtrOut[i]; /// kg/m^3, i.e., g/L
    m_sandRchOut[i] = sandin * outfract;                  // rch_san in SWAT
    m_siltRchOut[i] = siltin * outfract;                  // rch_sil in SWAT
    m_clayRchOut[i] = clayin * outfract;                  // rch_cla in SWAT
    m_sagRchOut[i] = sagin * outfract;                    // rch_sag in SWAT
    m_lagRchOut[i] = lagin * outfract;                    // rch_lag in SWAT
    m_gravelRchOut[i] = gravelin * outfract;              // rch_gra in SWAT

    if (m_sedRchOut[i] < UTIL_ZERO) {
        m_sedRchOut[i] = 0.f;
        m_sedConcRchOut[i] = 0.f;
        m_sandRchOut[i] = 0.f;
        m_siltRchOut[i] = 0.f;
        m_clayRchOut[i] = 0.f;
        m_sagRchOut[i] = 0.f;
        m_lagRchOut[i] = 0.f;
        m_gravelRchOut[i] = 0.f;
    }

    // Channel storage (kg)
    m_sedSto[i] = sedin - m_sedRchOut[i];
    m_sandSto[i] = sandin - m_sandRchOut[i];
    m_siltSto[i] = siltin - m_siltRchOut[i];
    m_claySto[i] = clayin - m_clayRchOut[i];
    m_sagSto[i] = sagin - m_sagRchOut[i];
    m_lagSto[i] = lagin - m_lagRchOut[i];
    m_gravelSto[i] = gravelin - m_gravelRchOut[i];
    if (m_sedSto[i] < UTIL_ZERO) {
        m_sedSto[i] = 0.f;
        m_sandSto[i] = 0.f;
        m_siltSto[i] = 0.f;
        m_claySto[i] = 0.f;
        m_sagSto[i] = 0.f;
        m_lagSto[i] = 0.f;
        m_gravelSto[i] = 0.f;
    }

    // Bank erosion
    m_rchBnkEro[i] = bnkrte;
    // Channel degradation
    m_rchDeg[i] = degrte;

#ifdef PRINT_DEBUG
    cout << ", sedRchOut: " << m_sedRchOut[i] << endl;
#endif
}

void SEDR_SBAGNOLD::ChannelDowncuttingWidening(const int i) {
    float depdeg = m_chDepth[i] - m_initChDepth[i];
    if (depdeg < m_initChSlope[i] * m_initChLen[i]) {
        if (m_chSto[i] + m_rteWtrOut[i] > 1.4e6f) {
            /// downcutting depth, m
            float cutdepth = 358.6f * m_chWtrDepth[i] * m_chSlope[i] * m_chBnkCov[i];
            m_chDepth[i] += cutdepth;
            m_chWth[i] = m_chDepth[i] * m_chWthDepthRt[i];
            m_chSlope[i] -= cutdepth / m_chLen[i];
            m_chSlope[i] = Max(0.0001f, m_chSlope[i]);
            // Update channel bottom width
            m_chBtmWth[i] = ChannleBottomWidth(m_chWth[i], m_chSideSlope[i], m_chDepth[i]);
        }
    }
}
