#include "ChannelRoutingDump.h"
#include "text.h"


ChannelRoutingDump::ChannelRoutingDump():  
    m_isInitialized(false),
    m_nReaches(-1),
    m_inputSubbasinId(-1),
    m_outletID(-1),
    m_reachDownStream(nullptr),
    m_Q_SBOF(nullptr), 
    m_Q_outlet(nullptr){
        SetModuleName(M_CHR_DUMP[0]);
}

ChannelRoutingDump::~ChannelRoutingDump() {
    Release1DArray(m_Q_outlet);
}

void ChannelRoutingDump::InitialOutputs() {

    Initialize1DArray(m_nReaches + 1, m_Q_outlet, 0.);
    
}

bool ChannelRoutingDump::CheckInputData(void) {
    CHECK_POSITIVE(GetModuleName(), m_nReaches);
    CHECK_POSITIVE(GetModuleName(), m_outletID);
    CHECK_POINTER(GetModuleName(), m_Q_SBOF);
    return true;
}

void ChannelRoutingDump::SetValue(const char* key, int value) {
    string sk(key);
    if (StringMatch(sk, Tag_SubbasinId)) m_inputSubbasinId = value;
    else if (StringMatch(sk, VAR_OUTLETID[0])) m_outletID = value;
    else {
        throw ModelException(GetModuleName(), "SetValue",
                             "Integer Parameter " + sk + " does not exist.");
    }
}

void ChannelRoutingDump::SetValueByIndex(const char* key, const int index, const FLTPT value){
    if (m_inputSubbasinId == 0) return;           // Not for omp version
    if (index <= 0 || index > m_nReaches) return; // index should belong 1 ~ m_nreach
    if (nullptr == m_Q_outlet) InitialOutputs();
    
    string sk(key);
    if (StringMatch(sk, VAR_QRECH[0])) m_Q_outlet[index] = value;
    else {
        throw ModelException(GetModuleName(), "SetValueByIndex",
                             "Parameter " + sk + " does not exist.");
    }
}

void ChannelRoutingDump::Set1DData(const char* key, const int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_SBOF[0])) {
        CheckInputSize(key, n - 1, m_nReaches);
        m_Q_SBOF = data;
    } else {
        throw ModelException(GetModuleName(), "Set1DData",
                             "Parameter " + sk + " does not exist.");
    }
}

void ChannelRoutingDump::SetReaches(clsReaches* reaches) {
    if (nullptr == reaches) {
        throw ModelException(GetModuleName(), "SetReaches", "The reaches input can not to be NULL.");
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

void ChannelRoutingDump::GetValue(const char* key, FLTPT* value){
    InitialOutputs();
    string sk(key);
    /// IN/OUTPUT variables
    if (StringMatch(sk, VAR_QRECH[0]) && m_inputSubbasinId > 0) *value = m_Q_outlet[m_inputSubbasinId];
    else {
        throw ModelException(GetModuleName(), "GetValue", "Parameter " + sk + " does not exist.");
    }
}

void ChannelRoutingDump::Get1DData(const char *key, int *nRows, FLTPT **data) {
    string s(key);
    if (StringMatch(s, VAR_QRECH[0])) {
        m_Q_outlet[0] = m_Q_outlet[m_outletID];
        *data = m_Q_outlet;
    } else {
        throw ModelException(GetModuleName(), "Get1DData", "Output " + s + " does not exist.");
    }
    *nRows = m_nReaches + 1;
}

int ChannelRoutingDump::Execute() {
    InitialOutputs();
    CheckInputData();

    for (auto it = m_routeLayers.begin(); it != m_routeLayers.end(); ++it) {
        // There are not any flow relationship within each routing layer.
        // So parallelization can be done here.
        int reachNum = CVT_INT(it->second.size());
        // the size of m_routeLayers (map) is equal to the maximum stream order
//#pragma omp parallel for
        for (int i = 0; i < reachNum; i++) {
            int reachIndex = it->second[i]; // index in the array, i.e., subbasinID
            if (m_inputSubbasinId == 0 || m_inputSubbasinId == reachIndex) {
                // for OpenMP version, all reaches will be executed,
                // for MPI version, only the current reach will be executed.
                ChannelFlow(reachIndex);
            }
        }

#ifdef PRINT_DEBUG
        printf("[ChannelRoutingDump]%d m_Q_outlet:",it->first);
        for (int i = 0; i <= m_nReaches; i++) {
            printf("%f, ", m_Q_outlet[i]);
        }
        printf("\n");
        fflush(stdout);
#endif
    }

    return 0;
}


void ChannelRoutingDump::ChannelFlow(const int i){
#ifdef PRINT_DEBUG
    printf("Q[%d](%f) += ", i, m_Q_outlet[i]);
#endif
    m_Q_outlet[i] = m_Q_SBOF[i];
    for (auto upReachId = m_reachUpStream.at(i).begin(); upReachId != m_reachUpStream.at(i).end(); ++upReachId) {
        if (m_Q_outlet[*upReachId] > 0.) {
            m_Q_outlet[i] += m_Q_outlet[*upReachId];
#ifdef PRINT_DEBUG
            printf("+ Q[%d](%f) ", *upReachId, m_Q_outlet[*upReachId]);
#endif
        }
    }
#ifdef PRINT_DEBUG
    printf("\n");
    fflush(stdout);
#endif
}
