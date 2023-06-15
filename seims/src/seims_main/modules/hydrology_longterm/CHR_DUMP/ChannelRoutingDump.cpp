#include "ChannelRoutingDump.h"
#include "text.h"


ChannelRoutingDump::ChannelRoutingDump():  
    m_isInitialized(false),
    m_nCells(-1),
    m_nReaches(-1),
    m_inputSubbasinId(-1),
    m_outletID(-1),
    m_reachDownStream(nullptr),
    m_Q_SBOF(nullptr), 
    m_qRchOut(nullptr){
}

ChannelRoutingDump::~ChannelRoutingDump() {
    Release1DArray(m_qRchOut);
}

void ChannelRoutingDump::InitialOutputs() {

    Initialize1DArray(m_nReaches + 1, m_qRchOut, 0.);
    
    for (int i = 1; i <= m_nReaches; i++) {
        m_qRchOut[i] = m_Q_SBOF[i];
    }
    
}

bool ChannelRoutingDump::CheckInputData(void) {
    CHECK_POSITIVE(M_MUSK_CH[0], m_nReaches);
    CHECK_POSITIVE(M_MUSK_CH[0], m_outletID);
    CHECK_POINTER(M_MUSK_CH[0], m_Q_SBOF);
    return true;
}

void ChannelRoutingDump::SetValue(const char* key, int value) {
    string sk(key);
    if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbasinId = value;
    else if (StringMatch(sk, VAR_OUTLETID[0])) m_outletID = value;
    else {
        throw ModelException(M_CHR_DUMP[0], "SetValue",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void ChannelRoutingDump::SetValueByIndex(const char* key, const int index, const FLTPT value){
    if (m_inputSubbasinId == 0) return;           // Not for omp version
    if (index <= 0 || index > m_nReaches) return; // index should belong 1 ~ m_nreach
    if (nullptr == m_qRchOut) InitialOutputs();
    
    string sk(key);
    if (StringMatch(sk, VAR_QRECH[0])) m_qRchOut[index] = value;
    else {
        throw ModelException(M_CHR_DUMP[0], "SetValueByIndex",
                             "Parameter " + sk + " does not exist.");
    }
}

void ChannelRoutingDump::Set1DData(const char* key, const int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_SBOF[0])) {
        CheckInputSize(M_CHR_DUMP[0], key, n - 1, m_nReaches);
        m_Q_SBOF = data;
    } else {
        throw ModelException(M_CHR_DUMP[0], "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void ChannelRoutingDump::SetReaches(clsReaches* reaches) {
    if (nullptr == reaches) {
        throw ModelException(M_CHR_DUMP[0], "SetReaches", "The reaches input can not to be NULL.");
    }
    m_nReaches = reaches->GetReachNumber();

    if (nullptr == m_reachDownStream) {
        FLTPT* tmp = nullptr;
        reaches->GetReachesSingleProperty(REACH_DOWNSTREAM, &tmp);
        Initialize1DArray(m_nReaches + 1, m_reachDownStream, tmp);
        Release1DArray(tmp);
    }

    m_reachUpStream = reaches->GetUpStreamIDs();
    m_rteLyrs = reaches->GetReachLayers();
}

void ChannelRoutingDump::GetValue(const char* key, FLTPT* value){
    InitialOutputs();
    string sk(key);
    /// IN/OUTPUT variables
    if (StringMatch(sk, VAR_QRECH[0]) && m_inputSubbasinId > 0) *value = m_qRchOut[m_inputSubbasinId];
    else {
        throw ModelException(M_CHR_DUMP[0], "GetValue", "Parameter " + sk + " does not exist.");
    }
}

void ChannelRoutingDump::Get1DData(const char *key, int *nRows, FLTPT **data) {
    string s(key);
    if (StringMatch(s, VAR_QRECH[0])) {
        m_qRchOut[0] = m_qRchOut[m_outletID];
        *data = m_qRchOut;
    } else {
        throw ModelException(M_CHR_DUMP[0], "Get1DData", "Output " + s + " does not exist.");
    }
    *nRows = m_nReaches + 1;
}

int ChannelRoutingDump::Execute() {
    InitialOutputs();
    CheckInputData();

    for (auto it = m_rteLyrs.begin(); it != m_rteLyrs.end(); ++it) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int reachNum = CVT_INT(it->second.size());
        // the size of m_rteLyrs (map) is equal to the maximum stream order
#pragma omp parallel for
        for (int i = 0; i < reachNum; i++) {
            int reachIndex = it->second[i]; // index in the array, i.e., subbasinID
            if (m_inputSubbasinId == 0 || m_inputSubbasinId == reachIndex) {
                // for OpenMP version, all reaches will be executed,
                // for MPI version, only the current reach will be executed.
                ChannelFlow(reachIndex);
            }
        }

        printf("\n[ChannelRoutingDump]%d m_qRchOut:",it->first);
        for (int i = 0; i < m_nReaches; i++) {
            printf("%f, ", m_qRchOut[i]);
        }
        fflush(stdout);
    }
    printf("\n");

    return 0;
}


void ChannelRoutingDump::ChannelFlow(const int i){
    for (auto upRchID = m_reachUpStream.at(i).begin(); upRchID != m_reachUpStream.at(i).end(); ++upRchID) {
        if (m_qRchOut[*upRchID] > 0.) {
            m_qRchOut[i] += m_qRchOut[*upRchID];
        }
    }
}
