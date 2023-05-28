#include "OverlandRoutingDump.h"
#include "text.h"


OverlandRoutingDump::OverlandRoutingDump():  
m_cellSize(-1), 
m_subbasins(nullptr),
m_petSubbsn(nullptr), m_gwSto(nullptr) {
}

OverlandRoutingDump::~OverlandRoutingDump() {
}

void OverlandRoutingDump::Get1DData(const char *key, int *nRows, float **data) {
    string s(key);
    if (StringMatch(s, VAR_SBPET[0])) {*data = m_petSubbsn;} 
    if (StringMatch(s, VAR_SBGS[0])) {*data = m_gwSto;} 
    else {
        throw ModelException("OverlandRoutingDump", "getResult", "Result " + s +
            " does not exist in OverlandRoutingDump method. Please contact the module developer.");
    }

    *nRows = this->m_cellSize;
}

void OverlandRoutingDump::SetValue(const char *key, float value) {
    string s(key);
    if (StringMatch(s, VAR_SUBBSNID_NUM[0])) {m_nSubbasins = value;}
    else {
        throw ModelException("OverlandRoutingDump", "SetValue", "Parameter " + s +
            " does not exist in OverlandRoutingDump method. Please contact the module developer.");
    }
}
void OverlandRoutingDump::SetSubbasins(clsSubbasins* subbsns) {
    if (m_subbasins == nullptr) {
        m_subbasins = subbsns;
        m_subbasinIDs = m_subbasins->GetSubbasinIDs();
    }
}

//Execute module
int OverlandRoutingDump::Execute() {
    CheckInputData();
    for (auto it = m_subbasinIDs.begin(); it != m_subbasinIDs.end(); ++it) {
        Subbasin* sub = m_subbasins->GetSubbasinByID(*it);
        
        m_petSubbsn[*it] = sub->GetPet();
        m_gwSto[*it] = sub->GetGw();
    }
    return 0;
}
