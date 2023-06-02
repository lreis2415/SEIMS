#include "HydroVarsUpdate.h"
#include "text.h"


HydroVarsUpdate::HydroVarsUpdate():
m_nCells(-1),
m_subbasins(nullptr),
m_petSubbsn(nullptr), m_gwSto(nullptr) {
}

HydroVarsUpdate::~HydroVarsUpdate() {
}

void HydroVarsUpdate::InitialOutputs() {
    Initialize1DArray(m_nSubbasins + 1, m_petSubbsn, 0.);
    Initialize1DArray(m_nSubbasins + 1, m_gwSto, 0.);
}

void HydroVarsUpdate::Get1DData(const char *key, int *nRows, FLTPT **data) {
    string s(key);
    if (StringMatch(s, VAR_SBPET[0])) {
        *data = m_petSubbsn;
    }
    else if (StringMatch(s, VAR_SBGS[0])) {
        *data = m_gwSto;
    }
    else {
        throw ModelException("HydroVarsUpdate", "getResult", "Result " + s +
            " does not exist in HydroVarsUpdate method. Please contact the module developer.");
    }

    *nRows = m_nSubbasins + 1;
}

void HydroVarsUpdate::SetValue(const char *key, int value) {
    string s(key);
    if (StringMatch(s, Tag_CellSize[0])) m_nCells = value;
    else if (StringMatch(s, VAR_SUBBSNID_NUM[0])) {m_nSubbasins = value;}
    else if (StringMatch(s, Tag_SubbasinId)) {m_inputSubbsnId = value;}
    else {
        throw ModelException("HydroVarsUpdate", "SetValue", "Parameter " + s +
            " does not exist in HydroVarsUpdate method. Please contact the module developer.");
    }
}
void HydroVarsUpdate::SetSubbasins(clsSubbasins* subbsns) {
    if (m_subbasins == nullptr) {
        m_subbasins = subbsns;
        m_subbasinIds = m_subbasins->GetSubbasinIDs();
    }
}
void HydroVarsUpdate::Set1DData(const char* key, int n, int* data) {
    string sk(key);

    if (StringMatch(sk, VAR_SUBBSN[0])) m_cellsMappingToSubbasinId = data;
    else {
        throw ModelException(CM_GR4J[0], "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}
void HydroVarsUpdate::Set1DData(const char* key, int n, FLTPT* data) {
    string sk(key);

    if (StringMatch(sk, VAR_PET[0])) { m_pet = data; }
    else {
        throw ModelException(CM_GR4J[0], "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}
//Execute module
int HydroVarsUpdate::Execute() {
    CheckInputData();
    InitialOutputs();
    
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_petSubbsn[CVT_INT(m_cellsMappingToSubbasinId[i])] += m_pet[i];
    }

    for (auto it = m_subbasinIds.begin(); it != m_subbasinIds.end(); ++it) {
        Subbasin* sub = m_subbasins->GetSubbasinByID(*it);


        m_gwSto[*it] = sub->GetGw();
    }
    return 0;
}
