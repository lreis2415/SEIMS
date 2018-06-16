#include "SEDR_SBAGNOLD.h"

#include "text.h"

SEDR_SBAGNOLD::SEDR_SBAGNOLD() :
    m_dt(-1), m_nreach(-1), m_inputSubbsnID(-1),
    m_vcd(false), m_peakRateAdj(NODATA_VALUE), m_sedTransEqCoef(NODATA_VALUE),
    m_sedTransEqExp(NODATA_VALUE),
    m_critVelSedDep(NODATA_VALUE), m_sedtoCh(nullptr),
    m_initChStorage(NODATA_VALUE), m_initChSedConc(NODATA_VALUE), m_qRchOut(nullptr),
    m_chOrder(nullptr), m_chWidth(nullptr),
    m_chDepth(nullptr), m_chLen(nullptr), m_chVel(nullptr),
    m_chSlope(nullptr), m_chCover(nullptr), m_chErod(nullptr), m_reachDownStream(nullptr),
    m_ptSub(nullptr), m_chStorage(nullptr),
    m_preChStorage(nullptr),
    m_chWtrDepth(nullptr), m_preChWtrDepth(nullptr), m_chWtrWth(nullptr),
    m_sedRchOut(nullptr),
    m_sedStorage(nullptr), m_sedDep(nullptr), m_sedDeg(nullptr), m_sedConcRchOut(nullptr),
    m_rchSand(nullptr), m_rchSilt(nullptr), m_rchClay(nullptr),
    m_rchSag(nullptr), m_rchLag(nullptr), m_rchGra(nullptr),
    m_rchBankEro(nullptr), m_rchDeg(nullptr), m_rchDep(nullptr), m_fldPlainDep(nullptr) {
}

SEDR_SBAGNOLD::~SEDR_SBAGNOLD() {
    /// reaches related variables will be released in ~clsReaches(). By lj, 2017-12-26.

    if (nullptr != m_ptSub) Release1DArray(m_ptSub);
    if (nullptr != m_sedRchOut) Release1DArray(m_sedRchOut);
    if (nullptr != m_sedConcRchOut) Release1DArray(m_sedConcRchOut);
    if (nullptr != m_sedStorage) Release1DArray(m_sedStorage);
    if (nullptr != m_sedDeg) Release1DArray(m_sedDeg);
    if (nullptr != m_sedDep) Release1DArray(m_sedDep);

    if (nullptr != m_rchSand) Release1DArray(m_rchSand);
    if (nullptr != m_rchSilt) Release1DArray(m_rchSilt);
    if (nullptr != m_rchClay) Release1DArray(m_rchClay);
    if (nullptr != m_rchSag) Release1DArray(m_rchSag);
    if (nullptr != m_rchLag) Release1DArray(m_rchLag);
    if (nullptr != m_rchGra) Release1DArray(m_rchGra);
    if (nullptr != m_rchBankEro) Release1DArray(m_rchBankEro);
    if (nullptr != m_rchDeg) Release1DArray(m_rchDeg);
    if (nullptr != m_rchDep) Release1DArray(m_rchDep);
    if (nullptr != m_fldPlainDep) Release1DArray(m_fldPlainDep);
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
    CHECK_POINTER(MID_SEDR_SBAGNOLD, m_chStorage);
    CHECK_POINTER(MID_SEDR_SBAGNOLD, m_chWidth);
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
        Initialize1DArray(m_nreach + 1, m_sedStorage, 0.f);
        Initialize1DArray(m_nreach + 1, m_sedDep, 0.f);
        Initialize1DArray(m_nreach + 1, m_sedDeg, 0.f);

        for (int i = 1; i <= m_nreach; i++) {
            // m_Chs0 is initial channel storage per meter, not sediment! By LJ
            m_sedStorage[i] = m_initChSedConc * m_chStorage[i] * 1000.f; /// ton/m3 * m3/m * m * 1000 = kg
        }
        Initialize1DArray(m_nreach + 1, m_rchSand, 0.f);
        Initialize1DArray(m_nreach + 1, m_rchSilt, 0.f);
        Initialize1DArray(m_nreach + 1, m_rchClay, 0.f);
        Initialize1DArray(m_nreach + 1, m_rchSag, 0.f);
        Initialize1DArray(m_nreach + 1, m_rchLag, 0.f);
        Initialize1DArray(m_nreach + 1, m_rchGra, 0.f);
        Initialize1DArray(m_nreach + 1, m_rchBankEro, 0.f);
        Initialize1DArray(m_nreach + 1, m_rchDeg, 0.f);
        Initialize1DArray(m_nreach + 1, m_rchDep, 0.f);
        Initialize1DArray(m_nreach + 1, m_fldPlainDep, 0.f);
    }
    /// initialize point source loadings
    if (nullptr == m_ptSub) Initialize1DArray(m_nreach + 1, m_ptSub, 0.f);
}

void SEDR_SBAGNOLD::PointSourceLoading() {
    /// load point source water discharge (m3/s) on current day from Scenario
    for (auto it = m_ptSrcFactory.begin(); it != m_ptSrcFactory.end(); ++it) {
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
            float per_sed = curPtMgt->GetSedment(); /// g/cm3, or Mg/m3
            // 1.3 Sum up all point sources
            for (auto locIter = m_ptSrcIDs.begin(); locIter != m_ptSrcIDs.end(); ++locIter) {
                if (m_pointSrcLocsMap.find(*locIter) != m_pointSrcLocsMap.end()) {
                    PointSourceLocations* curPtLoc = m_pointSrcLocsMap.at(*locIter);
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
                    DoChannelDowncuttingAndWidening(reachIndex);
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
    else if (StringMatch(sk, VAR_RCH_BANKERO)) *value = m_rchBankEro[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_RCH_DEG)) *value = m_rchDeg[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_RCH_DEP)) *value = m_rchDep[m_inputSubbsnID];
    else if (StringMatch(sk, VAR_FLPLAIN_DEP)) *value = m_fldPlainDep[m_inputSubbsnID];
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
    InitialOutputs();
    string sk(key);
    // transferred single value in MPI version
    if (StringMatch(sk, VAR_SED_TO_CH)) m_sedtoCh[index] = value;        // for longterm model
    else if (StringMatch(sk, VAR_SUB_SEDTOCH)) m_sedtoCh[index] = value; // for storm model // TODO
    else if (StringMatch(sk, VAR_QRECH)) m_qRchOut[index] = value;
}

void SEDR_SBAGNOLD::Set1DData(const char* key, const int n, float* data) {
    string sk(key);
    if (StringMatch(sk, VAR_SED_TO_CH)) m_sedtoCh = data;        //for longterm model
    else if (StringMatch(sk, VAR_SUB_SEDTOCH)) m_sedtoCh = data; //for storm model // TODO
    else if (StringMatch(sk, VAR_QRECH)) m_qRchOut = data;
    else if (StringMatch(sk, VAR_CHST)) m_chStorage = data;
    else if (StringMatch(sk, VAR_PRECHST)) m_preChStorage = data;
    else if (StringMatch(sk, VAR_CHWTDEPTH)) m_chWtrDepth = data;
    else if (StringMatch(sk, VAR_CHWTWIDTH)) m_chWtrWth = data;
    else if (StringMatch(sk, VAR_PRECHWTDEPTH)) m_preChWtrDepth = data;
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
    else if (StringMatch(sk, VAR_RCH_BANKERO)) *data = m_rchBankEro;
    else if (StringMatch(sk, VAR_RCH_DEG)) *data = m_rchDeg;
    else if (StringMatch(sk, VAR_RCH_DEP)) *data = m_rchDep;
    else if (StringMatch(sk, VAR_FLPLAIN_DEP)) *data = m_fldPlainDep;
    else {
        throw ModelException(MID_SEDR_SBAGNOLD, "Get1DData", "Output " + sk + " does not exist.");
    }
}

void SEDR_SBAGNOLD::SetScenario(Scenario* sce) {
    if (nullptr != sce) {
        map<int, BMPFactory *>& tmpBMPFactories = sce->GetBMPFactories();
        for (auto it = tmpBMPFactories.begin(); it != tmpBMPFactories.end(); ++it) {
            /// Key is uniqueBMPID, which is calculated by BMP_ID * 100000 + subScenario;
            if (it->first / 100000 == BMP_TYPE_POINTSOURCE) {
                m_ptSrcFactory[it->first] = static_cast<BMPPointSrcFactory *>(it->second);
            }
        }
    } else {
        throw ModelException(MID_SEDR_SBAGNOLD, "SetScenario", "The scenario can not to be NULL.");
    }
}

void SEDR_SBAGNOLD::SetReaches(clsReaches* reaches) {
    if (nullptr == reaches) {
        throw ModelException(MID_SEDR_SBAGNOLD, "SetReaches", "The reaches input can not to be NULL.");
    }
    m_nreach = reaches->GetReachNumber();

    if (nullptr == m_reachDownStream) reaches->GetReachesSingleProperty(REACH_DOWNSTREAM, &m_reachDownStream);
    if (nullptr == m_chOrder) reaches->GetReachesSingleProperty(REACH_UPDOWN_ORDER, &m_chOrder);
    if (nullptr == m_chWidth) reaches->GetReachesSingleProperty(REACH_WIDTH, &m_chWidth);
    if (nullptr == m_chLen) reaches->GetReachesSingleProperty(REACH_LENGTH, &m_chLen);
    if (nullptr == m_chDepth) reaches->GetReachesSingleProperty(REACH_DEPTH, &m_chDepth);
    if (nullptr == m_chVel) reaches->GetReachesSingleProperty(REACH_V0, &m_chVel);
    if (nullptr == m_chSlope) reaches->GetReachesSingleProperty(REACH_SLOPE, &m_chSlope);
    if (nullptr == m_chCover) reaches->GetReachesSingleProperty(REACH_COVER, &m_chCover);
    if (nullptr == m_chErod) reaches->GetReachesSingleProperty(REACH_EROD, &m_chErod);

    m_reachUpStream = reaches->GetUpStreamIDs();
    m_reachLayers = reaches->GetReachLayers();
}

void SEDR_SBAGNOLD::SedChannelRouting(const int i) {
    float qOutV = 0.f;    // water volume (m^3) of flow out
    float allWater = 0.f; // // water in reach during time step, qdin in SWAT
    // initialize sediment in reach during time step
    /// sediment from upstream reaches
    float allSediment = 0.f; // all sediment in reach, kg
    float sedUp = 0.f;       // sediment from upstream channels, kg
    for (size_t j = 0; j < m_reachUpStream[i].size(); j++) {
        int upReachId = m_reachUpStream[i][j];
        sedUp += m_sedRchOut[upReachId];
    }
    allSediment = sedUp + m_sedtoCh[i] + m_sedStorage[i];
    /// add point source loadings
    if (nullptr != m_ptSub && m_ptSub[i] > 0.f) {
        allSediment += m_ptSub[i];
    }
    // initialize water in reach during time step
    qOutV = m_qRchOut[i] * m_dt; // m^3
    allWater = m_preChStorage[i];
    if (((m_qRchOut[i] < UTIL_ZERO) && (m_chWtrDepth[i] < UTIL_ZERO)) || (allWater < 0.01f)) {
        /// do not perform sediment routing when:
        /// 1. whether is no water flow out of channel and water depth is nearly zero
        /// 2. if no water in reach
        m_sedDeg[i] = 0.f;
        m_sedDep[i] = 0.f;
        m_sedRchOut[i] = 0.f;
        m_sedConcRchOut[i] = 0.f;
        m_rchSand[i] = 0.f;
        m_rchSilt[i] = 0.f;
        m_rchClay[i] = 0.f;
        m_rchSag[i] = 0.f;
        m_rchLag[i] = 0.f;
        m_rchGra[i] = 0.f;
        m_rchBankEro[i] = 0.f;
        m_rchDeg[i] = 0.f;
        m_rchDep[i] = 0.f;
        m_fldPlainDep[i] = 0.f;
        m_sedStorage[i] = allSediment;
        return;
    }
    // initialize reach peak runoff rate and calculate flow velocity. SWAT Theory 2009, p.448
    float peakFlowRate = m_qRchOut[i] * m_peakRateAdj; // unit: m3/s
    float crossarea = allWater / m_chLen[i];           // unit: m2, p.432, eq.7:1.2.3
    float peakVelocity = 0.f;
    if (m_preChWtrDepth[i] < 0.01f) {
        peakVelocity = 0.01f;
    } else {
        peakVelocity = peakFlowRate / crossarea;
    }
    if (peakVelocity > 5.f) {
        // cout << "subbasin id: " << i <<", peakVelocity: " << peakVelocity << endl;
        peakVelocity = 5.f;
    } /// calculate tbase  |none  |flow duration (fraction of 24 hr)
    float tbase = m_chLen[i] / (m_dt * peakVelocity);
    if (tbase > 1.f) tbase = 1.f;

    //if (i == 4) cout << "qchOut: " << m_qchOut[i] << ", allwater: " << allWater <<
    //    ", chLen: " << m_chLen[i] << ", peakVelocity: " << peakVelocity <<
    //    ", m_preChWTDepth: " << m_preChWTDepth[i] << ", tbase: "<< tbase << endl;

    /// New improved method for sediment transport
    float initCon = 0.f;         // cyin
    float maxCon = 0.f;          // cych
    float sedDeposition = 0.f;   // depnet, and dep
    float sedDegradation = 0.f;  // deg
    float sedDegradation1 = 0.f; // deg1
    float sedDegradation2 = 0.f; // deg2

    //deposition and degradation
    initCon = allSediment / allWater; // kg/m^3
    //max concentration
    maxCon = m_sedTransEqCoef * pow(peakVelocity, m_sedTransEqExp) * 1000.f; // kg/m^3
    //if (i == 4) cout<<"iniCon: "<<initCon<<", maxCon: "<<maxCon<<endl;
    //initial concentration,mix sufficiently
    sedDeposition = allWater * (initCon - maxCon); // kg
    if (peakVelocity < m_critVelSedDep) {
        sedDeposition = 0.f;
    }

    if (sedDeposition < 0.f) {
        //degradation
        //cout << "initCon < maxCon, subbasin id: " << i << endl;
        sedDegradation = -sedDeposition * tbase;
        // first the deposited material will be degraded before channel bed
        if (sedDegradation >= m_sedDep[i]) {
            sedDegradation1 = m_sedDep[i];
            sedDegradation2 = (sedDegradation - sedDegradation1) * m_chErod[i] * m_chCover[i];
        } else {
            sedDegradation1 = sedDegradation;
            sedDegradation2 = 0.f;
        }
        sedDeposition = 0.f;
    } else {
        sedDegradation = 0.f;
        sedDegradation1 = 0.f;
        sedDegradation2 = 0.f;
    }
    //update sed deposition
    m_sedDep[i] += sedDeposition - sedDegradation1;
    if (m_sedDep[i] < UTIL_ZERO) m_sedDep[i] = 0.f;
    m_sedDeg[i] += sedDegradation1 + sedDegradation2;

    //get sediment after deposition and degradation
    allSediment += sedDegradation1 + sedDegradation2 - sedDeposition;
    if (allSediment < UTIL_ZERO) allSediment = 0.f;
    //get out flow water fraction
    float outFraction = qOutV / allWater;
    if (outFraction > 1.f) outFraction = 1.f;
    m_sedRchOut[i] = allSediment * outFraction;
    if (m_sedRchOut[i] < UTIL_ZERO) m_sedRchOut[i] = 0.f;
    // update sediment storage
    m_sedStorage[i] = allSediment - m_sedRchOut[i];
    if (m_sedStorage[i] < UTIL_ZERO) m_sedStorage[i] = 0.f;

    // get final sediment in water, cannot large than 0.848 ton/m3
    float maxSedinWt = 0.848f * qOutV * 1000.f; /// kg
    if (m_sedRchOut[i] > maxSedinWt) {
        m_sedDep[i] += m_sedRchOut[i] - maxSedinWt;
        m_sedRchOut[i] = maxSedinWt;
    }
    /// calculate sediment concentration
    m_sedConcRchOut[i] = m_sedRchOut[i] / qOutV; /// kg/m3, i.e., g/L
    /// in this default sediment routing method, sediment is not tracked by particle size
    m_rchSand[i] = 0.f;
    m_rchSilt[i] = m_sedRchOut[i]; // the sediments is assumed to be silt for mass conservation
    m_rchClay[i] = 0.f;
    m_rchSag[i] = 0.f;
    m_rchLag[i] = 0.f;
    m_rchGra[i] = 0.f;

    m_rchBankEro[i] = 0.f;
    m_rchDeg[i] = sedDegradation2;
    m_rchDep[i] = sedDeposition;
    m_fldPlainDep[i] = 0.f;
}

void SEDR_SBAGNOLD::DoChannelDowncuttingAndWidening(const int id) {
    /// TODO, lj
    float depdeg = m_preChWtrDepth[id] - m_chWtrDepth[id]; // depth of degradation/deposition from original
    if (depdeg < m_chSlope[id] * m_chLen[id]) {
        //float storage = m_chStorage[id];
        //float vout = m_qchOut[id] * m_dt;
        if (m_preChStorage[id] > 1.4e6f) {
            /// downcutting depth, m
            float cutDepth = 358.6f * m_chWtrDepth[id] * m_chSlope[id] * m_chErod[id];
            m_chWtrDepth[id] += cutDepth;
            m_chWtrWth[id] = m_chWtrDepth[id] * (m_chWidth[id] / m_chDepth[id]);

            m_chSlope[id] -= cutDepth / m_chLen[id];
            m_chSlope[id] = Max(0.0001f, m_chSlope[id]);
        }
    }
    // call ttcoef(jrch) // TODO
}
