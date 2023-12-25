#include "OLR_CIUH_GAMMA.h"
#include "text.h"


OLR_CIUH_GAMMA::OLR_CIUH_GAMMA():
m_nCells(-1), m_cellArea(nullptr), m_convolveTransporter(nullptr),
m_subbasins(nullptr), m_cellsMappingToSubbasinId(nullptr),
m_Q_SBOF(nullptr){
    SetModuleName(M_OLR_CIUH_GAMMA[0]);
}

OLR_CIUH_GAMMA::~OLR_CIUH_GAMMA() {
    Release1DArray(m_Q_SBOF);
    Release1DArray(m_gammaScale);
    Release1DArray(m_gammaShape);
    m_convolveTransporter = nullptr;
}


void OLR_CIUH_GAMMA::InitialOutputs() {
    Initialize1DArray(m_nSubbasins + 1, m_Q_SBOF, 0.);
}

bool OLR_CIUH_GAMMA::CheckInputData(void) {
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    return true;
}

bool OLR_CIUH_GAMMA::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}
void OLR_CIUH_GAMMA::SetValue(const char* key, int value) {
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSNID_NUM[0])) m_nSubbasins = value;
    else {
        throw ModelException(GetModuleName(), "SetValue",
            "Integer Parameter " + sk + " does not exist.");
    }
}
void OLR_CIUH_GAMMA::Set1DData(const char* key, int n, int* data) {
    string sk(key);

    if (StringMatch(sk, VAR_SUBBSN[0])) m_cellsMappingToSubbasinId = data;
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
            + " does not exist in current module. Please contact the module developer.");
    }
}
void OLR_CIUH_GAMMA::Set1DData(const char* key, const int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_SURU[0])) m_surfaceRunoff = data;
    else if (StringMatch(sk, VAR_CELL_AREA[0])) { m_cellArea = data; }
    else if (StringMatch(sk, VAR_GAMMA_SCALE[0])) { m_gammaScale = data; }
    else if (StringMatch(sk, VAR_GAMMA_SHAPE[0])) { m_gammaShape = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData",
            "Parameter " + sk + " does not exist.");
    }
}
void OLR_CIUH_GAMMA::Get1DData(const char* key, int* nRows, FLTPT** data) {
    string s(key);
    if (StringMatch(s, VAR_SBOF[0])) { *data = m_Q_SBOF; }
    else {
        throw ModelException(GetModuleName(), "getResult", "Result " + s +
            " does not exist in OLR_CIUH_GAMMA method. Please contact the module developer.");
    }
    *nRows = m_nSubbasins + 1;
}

void OLR_CIUH_GAMMA::SetSubbasins(clsSubbasins* subbsns) {
    if (m_subbasins == nullptr) {
        m_subbasins = subbsns;
        m_subbasinIDs = m_subbasins->GetSubbasinIDs();
    }
}
void OLR_CIUH_GAMMA::InitUnitHydrograph(){
    if (m_convolveTransporter == nullptr) {
        m_convolveTransporter = new ConvolveTransporterGAMMA(m_nCells,m_gammaScale,m_gammaShape);
    }
}

//Execute module
int OLR_CIUH_GAMMA::Execute() {
    InitialOutputs();
    CheckInputData();
    InitUnitHydrograph();

#ifdef PRINT_DEBUG
    FLTPT s1 = 0;
    for (int i = 0; i < m_nCells; i++) {
        s1 += m_surfaceRunoff[i];
    }
    printf("[OLR_CIUH_GAMMA] Before. surface runoff: %f\n", s1);
    fflush(stdout);
#endif

    FLTPT* flowOut = nullptr;
    Initialize1DArray(m_nCells, flowOut, 0.);
    m_convolveTransporter->Convolve(m_surfaceRunoff, flowOut);
    for (int n = 0; n <= m_nSubbasins; n++) {
        m_Q_SBOF[n] = 0;
    }
#pragma omp parallel
    {
        FLTPT* tmp_qsSub = new FLTPT[m_nSubbasins + 1];
        for (int i = 0; i <= m_nSubbasins; i++) {
            tmp_qsSub[i] = 0.;
        }
#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            tmp_qsSub[CVT_INT(m_cellsMappingToSubbasinId[i])] += flowOut[i] * m_cellArea[i];
        }
#pragma omp critical
        {
            for (int n = 1; n <= m_nSubbasins; n++) {
                m_Q_SBOF[n] += tmp_qsSub[n] * 0.001 / m_dt;
            }
        }
        delete[] tmp_qsSub;
        tmp_qsSub = nullptr;
    } /* END of //#pragma omp parallel */
    for (int n = 1; n <= m_nSubbasins; n++) {
        //get overland flow routing for entire watershed.
        m_Q_SBOF[0] += m_Q_SBOF[n];
    }
#ifdef PRINT_DEBUG
    FLTPT s2 = 0;
    for (int i = 0; i < m_nCells; i++) {
        s2 += flowOut[i];
    }
    printf("[OLR_CIUH_GAMMA] After. flowOut: %f, m_Q_SBOF[0]: %f\n", s2, m_Q_SBOF[0]);
    fflush(stdout);
#endif
    delete[] flowOut;
    flowOut = nullptr;
    return 0;
}
