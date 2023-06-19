#include "ChannelRoutingMuskingum.h"
#include "text.h"


ChannelRoutingMuskingum::ChannelRoutingMuskingum():  
    m_isInitialized(false),
    m_nCells(-1),
    m_nReaches(-1),
    m_inputSubbasinId(-1),
    m_outletID(-1),
    m_reachDownStream(nullptr),
    m_Q_SBOF(nullptr),
    m_Q_in(nullptr),
    m_Q_inLast(nullptr),
    m_Q_outLast(nullptr),
    m_Q_out(nullptr){
}

ChannelRoutingMuskingum::~ChannelRoutingMuskingum() {
    Release1DArray(m_Q_in);
    Release1DArray(m_Q_inLast);
    Release1DArray(m_Q_outLast);
    Release1DArray(m_Q_out);
}

void ChannelRoutingMuskingum::InitialOutputs() {

    Initialize1DArray(m_nReaches + 1, m_Q_out, 0.);
    Initialize1DArray(m_nReaches + 1, m_Q_outLast, 0.);
    Initialize1DArray(m_nReaches + 1, m_Q_in, 0.);
    Initialize1DArray(m_nReaches + 1, m_Q_inLast, 0.);
    
    for (int i = 1; i <= m_nReaches; i++) {
        m_Q_inLast[i] = m_Q_in[i];
        m_Q_outLast[i] = m_Q_out[i];
        m_Q_out[i] = 0;
        m_Q_in[i] = 0;

        //m_Q_out[i] = m_Q_SBOF[i];
    }
}

bool ChannelRoutingMuskingum::CheckInputData(void) {
    CHECK_POSITIVE(M_CHR_MUSK[0], m_nReaches);
    CHECK_POSITIVE(M_CHR_MUSK[0], m_outletID);
    CHECK_POINTER(M_CHR_MUSK[0], m_Q_SBOF);
    // 2KX < t < 2K(1-X)

    //FLTPT t = m_dt/60/60/24;
    //if(2*K*X>=t || 2*K*(1-X)<=t){
    //    throw ModelException(M_CHR_MUSK[0], "CheckInputData",
    //                         "Muskingum routing parameter error.");
    //}
    return true;
}

void ChannelRoutingMuskingum::SetValue(const char* key, int value) {
    string sk(key);
    if (StringMatch(sk, Tag_ChannelTimeStep[0])) m_dt = value;
    else if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbasinId = value;
    else if (StringMatch(sk, VAR_OUTLETID[0])) m_outletID = value;
    else {
        throw ModelException(M_CHR_MUSK[0], "SetValue",
                             "Integer Parameter " + sk + " does not exist.");
    }
}
void ChannelRoutingMuskingum::SetValue(const char* key, FLTPT value) {
    string sk(key);
    if (StringMatch(sk, VAR_MSK_X[0])) X = value;
    else if (StringMatch(sk, VAR_MSK_K[0])) K = value;
    else {
        throw ModelException(M_CHR_MUSK[0], "SetValue",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void ChannelRoutingMuskingum::SetValueByIndex(const char* key, const int index, const FLTPT value){
    if (m_inputSubbasinId == 0) return;           // Not for omp version
    if (index <= 0 || index > m_nReaches) return; // index should belong 1 ~ m_nreach
    if (nullptr == m_Q_out) InitialOutputs();
    
    string sk(key);
    if (StringMatch(sk, VAR_QRECH[0])) m_Q_out[index] = value;
    else {
        throw ModelException(M_CHR_MUSK[0], "SetValueByIndex",
                             "Parameter " + sk + " does not exist.");
    }
}

void ChannelRoutingMuskingum::Set1DData(const char* key, const int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_SBOF[0])) {
        CheckInputSize(M_CHR_MUSK[0], key, n - 1, m_nReaches);
        m_Q_SBOF = data;
    } else {
        throw ModelException(M_CHR_MUSK[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void ChannelRoutingMuskingum::SetReaches(clsReaches* reaches) {
    if (nullptr == reaches) {
        throw ModelException(M_CHR_MUSK[0], "SetReaches", "The reaches input can not to be NULL.");
    }
    m_nReaches = reaches->GetReachNumber();

    if (nullptr == m_reachDownStream) {
        FLTPT* tmp = nullptr;
        reaches->GetReachesSingleProperty(REACH_DOWNSTREAM, &tmp);
        Initialize1DArray(m_nReaches + 1, m_reachDownStream, tmp);
        Release1DArray(tmp);
    }

    m_reachUpStream = reaches->GetUpStreamIDs();
    m_routeLayers = reaches->GetReachLayers();
}

void ChannelRoutingMuskingum::GetValue(const char* key, FLTPT* value){
    //InitialOutputs();
    string sk(key);
    /// IN/OUTPUT variables
    if (StringMatch(sk, VAR_QRECH[0]) && m_inputSubbasinId > 0) *value = m_Q_out[m_inputSubbasinId];
    else {
        throw ModelException(M_CHR_MUSK[0], "GetValue", "Parameter " + sk + " does not exist.");
    }
}

void ChannelRoutingMuskingum::Get1DData(const char *key, int *nRows, FLTPT **data) {
    string s(key);
    if (StringMatch(s, VAR_QRECH[0])) {
        m_Q_out[0] = m_Q_out[m_outletID];
        *data = m_Q_out;
    } else {
        throw ModelException(M_CHR_MUSK[0], "Get1DData", "Output " + s + " does not exist.");
    }
    *nRows = m_nReaches + 1;
}

int ChannelRoutingMuskingum::Execute() {
    InitialOutputs();
    CheckInputData();
    printf("\n[ChannelRoutingMuskingum] m_Q_SBOF: ");
    for (int i = 0; i < m_nReaches; i++) {
        printf("%f, ", m_Q_SBOF[i]);
    }
    printf("\n");

    for (auto it = m_routeLayers.begin(); it != m_routeLayers.end(); ++it) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int reachNum = CVT_INT(it->second.size());
        // the size of m_routeLayers (map) is equal to the maximum stream order
#pragma omp parallel for
        for (int i = 0; i < reachNum; i++) {
            int reachIndex = it->second[i]; // index in the array, i.e., subbasinID
            if ((m_inputSubbasinId == 0 || m_inputSubbasinId == reachIndex)) {
                // for OpenMP version, all reaches will be executed,
                // for MPI version, only the current reach will be executed.
                ChannelFlow(reachIndex);
            }
            m_Q_out[reachIndex] += m_Q_SBOF[reachIndex];
        }

        printf("\n[ChannelRoutingMuskingum]%d m_Q_out:",it->first);
        for (int i = 0; i < m_nReaches; i++) {
            printf("%f, ", m_Q_out[i]);
        }
        fflush(stdout);
    }
    printf("\n");

    return 0;
}


void ChannelRoutingMuskingum::ChannelFlow(const int i){

    FLTPT tstep = m_dt / 60.0 / 60.0 / 24.0;
    FLTPT dt = Min(K, tstep);
    //FLTPT Q_stored= m_Q_in[i];

    for (auto upReachID = m_reachUpStream.at(i).begin(); upReachID != m_reachUpStream.at(i).end(); ++upReachID) {
        m_Q_in[i] += m_Q_out[*upReachID];
    }

    for (FLTPT t = 0; t < tstep; t += dt) {
        if (dt > tstep - t) { dt = tstep - t; }

        FLTPT denom = 2 * K * (1.0 - X) + dt;
        FLTPT c1 = (dt - 2 * K * X) / denom;
        FLTPT c2 = (dt + 2 * K * X) / denom;
        FLTPT c3 = (-dt + 2 * K * (1 - X)) / denom;
        //FLTPT c4 = dt / denom;


        FLTPT diff = m_Q_in[i] - m_Q_inLast[i];
        FLTPT qIn = m_Q_inLast[i] + (t / tstep) * diff;
        FLTPT qInNew = m_Q_inLast[i] + ((t + dt) / tstep) * diff;

        m_Q_out[i] = c1 * qInNew + c2 * qIn + c3 * m_Q_outLast[i];

        m_Q_outLast[i] = m_Q_out[i];
    }

    //m_Q_in[i] = Q_stored;
    
}
