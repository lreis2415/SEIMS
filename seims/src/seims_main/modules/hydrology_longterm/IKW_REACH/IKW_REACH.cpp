#include "IKW_REACH.h"
#include "text.h"

//using namespace std;  // Avoid this statement! by lj.

IKW_REACH::IKW_REACH() : m_dt(-1), m_nreach(-1), m_Kchb(nullptr),
                         m_Kbank(nullptr), m_Epch(NODATA_VALUE), m_Bnk0(NODATA_VALUE), m_Chs0(NODATA_VALUE),
                         m_aBank(NODATA_VALUE),
                         m_bBank(NODATA_VALUE), m_subbasin(nullptr), m_qsSub(nullptr),
                         m_qiSub(nullptr), m_qgSub(nullptr), m_petCh(nullptr), m_gwStorage(nullptr), m_area(nullptr),
                         m_Vseep0(0.f), m_chManning(nullptr), m_chSlope(nullptr),m_chWTdepth(nullptr),
                         m_bankStorage(nullptr), m_seepage(nullptr),
                         m_qsCh(nullptr), m_qiCh(nullptr), m_qgCh(nullptr),
                         m_x(0.2f), m_co1(0.7f), m_qIn(nullptr), m_chStorage(nullptr),
                         m_qUpReach(0.f), m_deepGroudwater(0.f) {
}

IKW_REACH::~IKW_REACH() {
    /// reaches related variables will be released in ~clsReaches(). By lj, 2017-12-26.

    if (nullptr != m_chStorage) Release1DArray(m_chStorage);
    if (nullptr != m_qOut) Release1DArray(m_qOut);
    if (nullptr != m_bankStorage) Release1DArray(m_bankStorage);
    if (nullptr != m_seepage) Release1DArray(m_seepage);
    if (nullptr != m_chStorage) Release1DArray(m_chStorage);
    if (nullptr != m_qsCh) Release1DArray(m_qsCh);
    if (nullptr != m_qiCh) Release1DArray(m_qiCh);
    if (nullptr != m_qgCh) Release1DArray(m_qgCh);
    if (nullptr != m_chWTdepth) Release1DArray(m_chWTdepth);
}

bool IKW_REACH::CheckInputData() {
    if (m_dt < 0) {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: m_dt has not been set.");
    }

    if (m_nreach < 0) {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: m_nreach has not been set.");
    }

    if (nullptr == m_Kchb) {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: K_chb has not been set.");
    }
    if (nullptr == m_Kbank) {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: K_bank has not been set.");
    }
    if (FloatEqual(m_Epch, NODATA_VALUE)) {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: Ep_ch has not been set.");
    }
    if (FloatEqual(m_Bnk0, NODATA_VALUE)) {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: Bnk0 has not been set.");
    }
    if (FloatEqual(m_Chs0, NODATA_VALUE)) {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: Chs0 has not been set.");
    }
    if (FloatEqual(m_aBank, NODATA_VALUE)) {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: A_bnk has not been set.");
    }
    if (FloatEqual(m_bBank, NODATA_VALUE)) {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: B_bnk has not been set.");
    }
    if (FloatEqual(m_Vseep0, NODATA_VALUE)) {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: m_Vseep0 has not been set.");
    }
    if (nullptr == m_subbasin) {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: m_subbasin has not been set.");
    }
    if (nullptr == m_qsSub) {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: Q_SBOF has not been set.");
    }
    if (nullptr == m_chWidth) {
        throw ModelException("IKW_REACH", "CheckInputData", "The parameter: RchParam has not been set.");
    }
    return true;
}

void IKW_REACH:: InitialOutputs() {
    if (m_nreach <= 0) {
        throw ModelException("IKW_REACH", "initialOutputs", "The cell number of the input can not be less than zero.");
    }

    //initial channel storage
    if (nullptr == m_chStorage) {
        m_chStorage = new FLTPT[m_nreach + 1];
        m_qIn = new FLTPT[m_nreach + 1];
        m_qOut = new FLTPT[m_nreach + 1];
        m_bankStorage = new FLTPT[m_nreach + 1];
        m_seepage = new FLTPT[m_nreach + 1];
        m_qsCh = new FLTPT[m_nreach + 1];
        m_qiCh = new FLTPT[m_nreach + 1];
        m_qgCh = new FLTPT[m_nreach + 1];
        m_chWTdepth = new FLTPT[m_nreach + 1];

#pragma omp parallel for
        for (int i = 1; i <= m_nreach; i++) {
            FLTPT qiSub = 0.f;
            FLTPT qgSub = 0.f;
            if (nullptr != m_qiSub) {
                qiSub = m_qiSub[i];
            }
            if (nullptr != m_qgSub) {
                qgSub = m_qgSub[i];
            }
            m_seepage[i] = 0.f;
            m_bankStorage[i] = m_Bnk0 * m_chLen[i];
            m_chStorage[i] = m_Chs0 * m_chLen[i];
            m_qIn[i] = 0.f;
            m_qOut[i] = m_qsSub[i] + qiSub + qgSub;
            m_qsCh[i] = m_qsSub[i];
            m_qiCh[i] = qiSub;
            m_qgCh[i] = qgSub;
            m_chWTdepth[i] = 0.f;

        }
    }
}

int IKW_REACH::Execute() {
    InitialOutputs();

    for (auto it = m_reachLayers.begin(); it != m_reachLayers.end(); it++) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int nReaches = it->second.size();
        // the size of m_reachLayers (map) is equal to the maximum stream order
#pragma omp parallel for
        for (int i = 0; i < nReaches; ++i) {
            int reachIndex = it->second[i]; // index in the array
            ChannelFlow(reachIndex);
        }
    }

    return 0;
}

bool IKW_REACH::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        //StatusMsg("Input data for "+string(key) +" is invalid. The size could not be less than zero.");
        return false;
    }
#ifdef STORM_MODE
    if(m_nreach != n-1)
    {
        if(m_nreach <=0)
            m_nreach = n-1;
        else
        {
            //StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            ostringstream oss;
            oss << "Input data for "+string(key) << " is invalid with size: " << n << ". The origin size is " << m_nreach << ".\n";
            throw ModelException("IKW_REACH","CheckInputSize",oss.str());
        }
    }
#else
    if (m_nreach != n - 1) {
        if (m_nreach <= 0) {
            m_nreach = n - 1;
        } else {
            //StatusMsg("Input data for "+string(key) +" is invalid. All the input data should have same size.");
            std::ostringstream oss;
            oss << "Input data for " + string(key) << " is invalid with size: "
                    << n << ". The origin size is " << m_nreach << ".\n";
            throw ModelException("IKW_REACH", "CheckInputSize", oss.str());
        }
    }
#endif /* STORM_MODE */
    return true;
}

void IKW_REACH::SetValue(const char *key, FLTPT value) {
    string sk(key);

    //if (StringMatch(sk, VAR_QUPREACH)) {
    //    m_qUpReach = value;
    //} else
    //if (StringMatch(sk, Tag_LayeringMethod[0])) {
    //    m_layeringMethod = (LayeringMethod) int(value);
    //} else

    if (StringMatch(sk, VAR_EP_CH[0])) {
        m_Epch = value;
    } else if (StringMatch(sk, VAR_BNK0[0])) {
        m_Bnk0 = value;
    } else if (StringMatch(sk, VAR_CHS0[0])) {
        m_Chs0 = value;
    } else if (StringMatch(sk, VAR_VSEEP0[0])) {
        m_Vseep0 = value;
    } else if (StringMatch(sk, VAR_A_BNK[0])) {
        m_aBank = value;
    } else if (StringMatch(sk, VAR_B_BNK[0])) {
        m_bBank = value;
    } else if (StringMatch(sk, VAR_MSK_X[0])) {
        m_x = value;
    } else if (StringMatch(sk, VAR_MSK_CO1[0])) {
        m_co1 = value;
    } else {
        throw ModelException("IKW_REACH", "SetSingleData",
                             "Parameter " + sk + " does not exist. Please contact the module developer.");
    }

}

void IKW_REACH::SetValue(const char *key, const int value){
    string sk(key);
    if (StringMatch(sk, Tag_ChannelTimeStep[0])) {
            m_dt = int(value);
    }
}

void IKW_REACH::Set1DData(const char *key, int n, FLTPT *value) {
    string sk(key);
    //check the input data
    if (StringMatch(sk, VAR_SUBBSN[0])) {
        m_subbasin = value;   //m_subbasin
    } else if (StringMatch(sk, VAR_SBOF[0])) {
        CheckInputSize(key, n);
        m_qsSub = value;
    } else if (StringMatch(sk, VAR_SBIF[0])) {
        CheckInputSize(key, n);
        m_qiSub = value;
    } else if (StringMatch(sk, VAR_SBQG[0])) {
        m_qgSub = value;
    } else if (StringMatch(sk, VAR_SBPET[0])) {
        m_petCh = value;
    } else if (StringMatch(sk, VAR_SBGS[0])) {
        m_gwStorage = value;
    } else {
        throw ModelException("IKW_REACH", "Set1DData", "Parameter " + sk
            + " does not exist. Please contact the module developer.");
    }

}

//void IKW_REACH::GetValue(const char *key, FLTPT *value) {
//    string sk(key);
//    int iOutlet = m_reachLayers.rbegin()->second[0];
//    if (StringMatch(sk, VAR_QOUTLET)) {
//        //*value = m_qsCh[iOutlet];
//        m_qOut[0] = m_qOut[iOutlet] + m_deepGroudwater;
//        *value = m_qOut[0];
//    } else if (StringMatch(sk, VAR_QSOUTLET)) {
//        *value = m_qsCh[iOutlet];
//    }
//
//}

void IKW_REACH::Get1DData(const char *key, int *n, FLTPT **data) {
    string sk(key);
    *n = m_nreach + 1;
    int iOutlet = m_reachLayers.rbegin()->second[0];
    if (StringMatch(sk, VAR_QRECH[0])) {
        m_qOut[0] = m_qOut[iOutlet] + m_deepGroudwater;
        *data = m_qOut;
    } else if (StringMatch(sk, VAR_QS[0])) {
        m_qsCh[0] = m_qsCh[iOutlet];
        *data = m_qsCh;
    } else if (StringMatch(sk, VAR_QI[0])) {
        m_qiCh[0] = m_qiCh[iOutlet];
        *data = m_qiCh;
    } else if (StringMatch(sk, VAR_QG[0])) {
        m_qgCh[0] = m_qgCh[iOutlet];
        *data = m_qgCh;
    } else if (StringMatch(sk, VAR_BKST[0])) {
        m_bankStorage[0] = m_bankStorage[iOutlet];
        *data = m_bankStorage;
    } else if (StringMatch(sk, VAR_CHST[0])) {
        m_chStorage[0] = m_chStorage[iOutlet];
        *data = m_chStorage;
    } else if (StringMatch(sk, VAR_SEEPAGE[0])) {
        m_seepage[0] = m_seepage[iOutlet];
        *data = m_seepage;
    } else if (StringMatch(sk, VAR_CHWTRDEPTH[0])) {
        m_chWTdepth[0] = m_chWTdepth[iOutlet];
        *data = m_chWTdepth;
    } else {
        throw ModelException("IKW_REACH", "Get1DData", "Output " + sk
            +
                " does not exist in the IKW_REACH module. Please contact the module developer.");
    }

}

void IKW_REACH::Get2DData(const char *key, int *nRows, int *nCols, FLTPT ***data) {
    string sk(key);
    throw ModelException("IKW_REACH", "Get2DData", "Output " + sk +
            " does not exist in the IKW_REACH module. Please contact the module developer.");
}

void IKW_REACH::SetReaches(clsReaches *reaches) {
    if (nullptr == reaches) {
        throw ModelException("IKW_REACH", "SetReaches", "The reaches input can not to be NULL.");
    }
    m_nreach = reaches->GetReachNumber();

    if (nullptr == m_reachDownStream) reaches->GetReachesSingleProperty(REACH_DOWNSTREAM, &m_reachDownStream);
    if (nullptr == m_chWidth) reaches->GetReachesSingleProperty(REACH_WIDTH, &m_chWidth);
    if (nullptr == m_chLen) reaches->GetReachesSingleProperty(REACH_LENGTH, &m_chLen);
    if (nullptr == m_chDepth) reaches->GetReachesSingleProperty(REACH_DEPTH, &m_chDepth);
    // if (nullptr == m_chVel) reaches->GetReachesSingleProperty(REACH_V0, &m_chVel); // todo fix this
    if (nullptr == m_area) reaches->GetReachesSingleProperty(REACH_AREA, &m_area);
    if (nullptr == m_chManning) reaches->GetReachesSingleProperty(REACH_MANNING, &m_chManning);
    if (nullptr == m_chSlope) reaches->GetReachesSingleProperty(REACH_SLOPE, &m_chSlope);

    m_reachUpStream = reaches->GetUpStreamIDs();
    m_reachLayers = reaches->GetReachLayers();
}

//---------------------------------------------------------------------------
// modified from OpenLISEM
/** Newton Rapson iteration for new water flux in cell, based on Ven Te Chow 1987
\param qIn      summed Q new from upstream
\param qLast    current discharge in the cell
\param surplus        infiltration surplus flux (in m2/s), has value <= 0
\param alpha    alpha calculated in LISEM from before kinematic wave
\param dt   timestep
\param dx   length of the cell corrected for slope
*/
FLTPT IKW_REACH::GetNewQ(FLTPT qIn, FLTPT qLast, FLTPT surplus, FLTPT alpha, FLTPT dt, FLTPT dx) {
    /* Using Newton-Raphson Method */
    FLTPT ab_pQ, dtX, C;  //auxillary vars
    int count;
    FLTPT Qkx; //iterated discharge, becomes Qnew
    FLTPT fQkx; //function
    FLTPT dfQkx;  //derivative
    const FLTPT _epsilon = 1e-12f;
    const FLTPT beta = 0.6f;

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
        fQkx = dtX * Qkx + alpha * CalPow(Qkx, beta) - C;   /* Current k */
        dfQkx = dtX + alpha * beta * CalPow(Qkx, beta - 1);  /* Current k */
        Qkx -= fQkx / dfQkx;                                /* Next k */
        Qkx = Max(Qkx, MIN_FLUX);
        count++;
        //qDebug() << count << fQkx << Qkx;
    } while (Abs(fQkx) > _epsilon && count < MAX_ITERS_KW);

    if (Qkx != Qkx) {
        throw ModelException("IKW_OL", "GetNewQ", "Error in iteration!");
    }

    //itercount = count;
    return Qkx;
}

void IKW_REACH::ChannelFlow(int i) {
    FLTPT st0 = m_chStorage[i];

    FLTPT qiSub = 0.f;
    if (nullptr != m_qiSub) {
        qiSub = m_qiSub[i];
    }
    FLTPT qgSub = 0.f;
    if (nullptr != m_qgSub) {
        qgSub = m_qgSub[i];
    }

    //////////////////////////////////////////////////////////////////////////
    // first add all the inflow water
    // 1. water from this subbasin
    FLTPT qIn = m_qsSub[i] + qiSub + qgSub;

    // 2. water from upstream reaches
    FLTPT qsUp = 0.f;
    FLTPT qiUp = 0.f;
    FLTPT qgUp = 0.f;
    for (size_t j = 0; j < m_reachUpStream[i].size(); ++j) {
        int upReachId = m_reachUpStream[i][j];
        qsUp += m_qsCh[upReachId];
        qiUp += m_qiCh[upReachId];
        qgUp += m_qgCh[upReachId];
    }
    qIn += qsUp + qiUp + qgUp;
    qIn += m_qUpReach;
    // m_qUpReach is zero for not-parallel program and qsUp, qiUp and qgUp are zero for parallel computing

    // 3. water from bank storage
    FLTPT bankOut = m_bankStorage[i] * (1 - CalExp(-m_aBank));

    m_bankStorage[i] -= bankOut;
    qIn += bankOut / m_dt;

    // add inflow water to storage
    m_chStorage[i] += qIn * m_dt;

    //////////////////////////////////////////////////////////////////////////
    // then subtract all the outflow water
    // 1. transmission losses to deep aquifer, which is lost from the system
    // the unit of kchb is mm/hr
    FLTPT seepage = m_Kchb[i] / 1000.f / 3600.f * m_chWidth[i] * m_chLen[i] * m_dt;
    if (qgSub < 0.001f) {
        if (m_chStorage[i] > seepage) {
            m_seepage[i] = seepage;
            m_chStorage[i] -= seepage;
        } else {
            m_seepage[i] = m_chStorage[i];
            m_chStorage[i] = 0.f;
            m_qOut[i] = 0.f;
            m_qsCh[i] = 0.f;
            m_qiCh[i] = 0.f;
            m_qgCh[i] = 0.f;
            return;
        }
    } else {
        m_seepage[i] = 0.f;
    }

    // 2. calculate transmission losses to bank storage
    FLTPT dch = m_chStorage[i] / (m_chWidth[i] * m_chLen[i]);
    FLTPT bankInLoss = 2 * m_Kbank[i] / 1000.f / 3600.f * dch * m_chLen[i] * m_dt;   // m3/s
    bankInLoss = 0.f;
    if (m_chStorage[i] > bankInLoss) {
        m_chStorage[i] -= bankInLoss;
    } else {
        bankInLoss = m_chStorage[i];
        m_chStorage[i] = 0.f;
    }
    // water balance of the bank storage
    // loss the water from bank storage to the adjacent unsaturated zone and groundwater storage
    FLTPT bankOutGw = m_bankStorage[i] * (1 - CalExp(-m_bBank));
    bankOutGw = 0.f;
    m_bankStorage[i] = m_bankStorage[i] + bankInLoss - bankOutGw;
    if (nullptr != m_gwStorage) {
        m_gwStorage[i] += bankOutGw / m_area[i] * 1000.f;
    }   // updated groundwater storage

    if (m_chStorage[i] <= 0.f) {
        m_qOut[i] = 0.f;
        m_qsCh[i] = 0.f;
        m_qiCh[i] = 0.f;
        m_qgCh[i] = 0.f;
        return;
    }

    // 3. evaporation losses
    FLTPT et = 0.f;
    if (nullptr != m_petCh) {
        et = m_Epch * m_petCh[i] / 1000.0f * m_chWidth[i] * m_chLen[i];    //m3
        if (m_chStorage[i] > et) {
            m_chStorage[i] -= et;
        } else {
            et = m_chStorage[i];
            m_chStorage[i] = 0.f;
            m_qOut[i] = 0.f;
            m_qsCh[i] = 0.f;
            m_qiCh[i] = 0.f;
            m_qgCh[i] = 0.f;
            return;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // routing, there are water in the channel after inflow and transmission loss
    FLTPT totalLoss = m_seepage[i] + bankInLoss + et;

    if (m_chStorage[i] >= 0.f) {
        //qIn -= totalLoss/m_dt;
        m_chStorage[i] = st0;

        FLTPT h = m_chStorage[i] / m_chWidth[i] / m_chLen[i];
        FLTPT Perim = 2.f * h + m_chWidth[i];
        FLTPT sSin = CalSqrt(sin(m_chSlope[i]));
        FLTPT alpha = CalPow(m_chManning[i] / sSin * CalPow(Perim, _2div3), 0.6f);

        FLTPT lossRate = -totalLoss / m_dt / m_chWidth[i];
        //lossRate = 0.f;
        m_qOut[i] = GetNewQ(qIn, m_qOut[i], lossRate, alpha, m_dt, m_chLen[i]);

        FLTPT hNew = (alpha * CalPow(m_qOut[i], 0.6f)) / m_chWidth[i]; // unit m

        m_chStorage[i] += (qIn - m_qOut[i]) * m_dt;
        //FLTPT hTest = h + (qIn-m_qOut[i])*m_dt/m_chWidth[i]/m_chLen[i];

        if (m_chStorage[i] < 0.f) {
            m_qOut[i] = qIn;
            m_chStorage[i] = 0.f;
        }
    } else {
        m_qOut[i] = 0.f;
        m_chStorage[i] = 0.f;
        qIn = 0.f;
    }

    FLTPT qInSum = m_qsSub[i] + qiSub + qgSub + qsUp + qiUp + qgUp;
    m_qsCh[i] = m_qOut[i] * (m_qsSub[i] + qsUp) / qInSum;
    m_qiCh[i] = m_qOut[i] * (qiSub + qiUp) / qInSum;
    m_qgCh[i] = m_qOut[i] * (qgSub + qgUp) / qInSum;

    // set variables for next time step
    m_qIn[i] = qIn;

    m_chWTdepth[i] = dch;
}
