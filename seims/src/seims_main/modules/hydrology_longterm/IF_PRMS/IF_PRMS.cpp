#include "IF_PRMS.h"
#include "text.h"

IF_PRMS::IF_PRMS():
m_nCells(-1), m_maxSoilLyrs(-1), m_isInitialized(false),
m_subbasins(nullptr), m_cellsMappingToSubbasinId(nullptr),
m_cellArea(nullptr), m_maxIfRate(nullptr), m_soilLayers(nullptr),
m_soilThickness(nullptr), m_porosity(nullptr), m_awcAmount(nullptr), m_soilWaterStorage(nullptr),
m_Q_SBIF(nullptr)
{
    SetModuleName(M_IF_PRMS[0]);
}

void IF_PRMS::InitialOutputs() {
    Initialize1DArray(m_nSubbasins + 1, m_Q_SBIF, 0.);
}

IF_PRMS::~IF_PRMS() {
    Release1DArray(m_Q_SBIF);
}
void IF_PRMS::SetValue(const char* key, int value) {
    string sk(key);
    if (StringMatch(sk, VAR_SUBBSNID_NUM[0])) m_nSubbasins = value;
    else {
        throw ModelException(GetModuleName(), "SetValue",
                             "Integer Parameter " + sk + " does not exist.");
    }
}
void IF_PRMS::Set1DData(const char* key, int n, int* data) {
    string sk(key);

    if (StringMatch(sk, VAR_SUBBSN[0])) {m_cellsMappingToSubbasinId = data;}
    else if (StringMatch(sk, VAR_SOILLAYERS[0])) {m_soilLayers = data;}
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}
void IF_PRMS::Set1DData(const char* key, int n, FLTPT* data) {
    string sk(key);
    if (StringMatch(sk, VAR_CELL_AREA[0])) { m_cellArea = data; }
    else {
        throw ModelException(GetModuleName(), "Set1DData", "Parameter " + sk
                             + " does not exist in current module. Please contact the module developer.");
    }
}
void IF_PRMS::Set2DData(const char* key, const int nrows, const int ncols, FLTPT** data) {
    string sk(key);
    if (StringMatch(sk, VAR_SOILTHICK[0])) {
        CheckInputSize2D(key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_soilThickness = data;
    } else if (StringMatch(sk, VAR_POROST[0])) {
        CheckInputSize2D(key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_porosity = data;
    } else if (StringMatch(sk, VAR_MAX_IF_RATE[0])) {
        CheckInputSize2D(key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_maxIfRate = data;
    } else if (StringMatch(sk, VAR_SOL_AWC_AMOUNT[0])) {
        CheckInputSize2D(key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_awcAmount = data;
    } else if (StringMatch(sk, VAR_SOL_ST[0])) {
        CheckInputSize2D(key, nrows, ncols, m_nCells, m_maxSoilLyrs);
        m_soilWaterStorage = data;
    } else {
        throw ModelException(GetModuleName(), "Set2DData",
                             "Parameter " + sk + " does not exist.");
    }
}
bool IF_PRMS::CheckInputSize(const char* key, int n) {
    return SimulationModule::CheckInputSize(key, n, m_nCells);
}

bool IF_PRMS::CheckInputData(void) {
    CHECK_POSITIVE(GetModuleName(), m_nCells);
    return true;
}

void IF_PRMS::Get1DData(const char* key, int* n, FLTPT** data) {
    string s(key);
    if (StringMatch(s, VAR_SBIF[0])) { *data = m_Q_SBIF; }
    else {
        throw ModelException(GetModuleName(), "Get1DData", "Output " + s
                             + " does not exist in current module. Please contact the module developer.");
    }
    *n = m_nSubbasins + 1;
}

int IF_PRMS::Execute() {
    InitialOutputs();
    CheckInputData();
    FLTPT s1 = 0;
    FLTPT s2 = 0;
    FLTPT s3 = 0;
    FLTPT s4 = 0;
     for (int n = 0; n <= m_nSubbasins; n++) {
        m_Q_SBIF[n] = 0;
     }
//#pragma omp parallel
    {
        FLTPT* tmp_ifSub = new FLTPT[m_nSubbasins + 1];
        for (int i = 0; i <= m_nSubbasins; i++) {
            tmp_ifSub[i] = 0.;
        }
//#pragma omp for
        for (int i = 0; i < m_nCells; i++) {
            FLTPT interflow = 0;
            for (int j = 0; j < m_soilLayers[i]; j++) {
                // soil water max volume
                FLTPT swMax = m_soilThickness[i][j] * m_porosity[i][j];
                FLTPT stor = m_soilWaterStorage[i][j];
                FLTPT tens = m_awcAmount[i][j];
                // drainable soil saturation
                FLTPT drainable = NonNeg((stor - tens) / (swMax - tens));
                s2 += tens;
                s3+=stor;
                s1 += m_soilWaterStorage[i][j];
                Convey(m_soilWaterStorage[i][j], interflow, m_maxIfRate[i][j]*drainable);
            }
            tmp_ifSub[CVT_INT(m_cellsMappingToSubbasinId[i])] += interflow * m_cellArea[i];
        }
//#pragma omp critical
        {
            for (int n = 1; n <= m_nSubbasins; n++) {
                m_Q_SBIF[n] += tmp_ifSub[n] * 0.001 / m_dt;
            }
        }
        delete[] tmp_ifSub;
        tmp_ifSub = nullptr;
    }
    for (int n = 1; n <= m_nSubbasins; n++) {
        m_Q_SBIF[0] += m_Q_SBIF[n];
    }
    
#ifdef PRINT_DEBUG
    printf("[IF_PRMS] AVG: tens(%f), stor(%f)\n", s2 / m_nCells, s3 / m_nCells);
    printf("[IF_PRMS] soilWater->m_Q_SBIF = %f->%f\n", s1, m_Q_SBIF[0]);
    fflush(stdout);
#endif
    return 0;
}
