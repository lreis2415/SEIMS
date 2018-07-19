#include "IO_test.h"

#include "text.h"

IO_TEST::IO_TEST() :
    m_nCells(-1), m_raster1D(nullptr), m_soilLayers(-1),
    m_nSoilLayrs(nullptr), m_raster2D(nullptr),
    m_output1Draster(nullptr), m_output2Draster(nullptr),
    m_scenario(nullptr), m_reaches(nullptr) {
}

IO_TEST::~IO_TEST() {
    if (m_output1Draster != nullptr) { Release1DArray(m_output1Draster); }
    if (m_output2Draster != nullptr) { Release2DArray(m_nCells, m_output2Draster); }
    // NOTE: m_scenario and m_reaches will be released in DataCenter!
}

void IO_TEST::Set1DData(const char* key, int n, float* data) {
    if (!CheckInputSize(key, n)) return;
    string sk(key);
    if (StringMatch(sk, VAR_CN2)) { m_raster1D = data; }
    if (StringMatch(sk, VAR_SOILLAYERS)) { m_nSoilLayrs = data; }
}

void IO_TEST::Set2DData(const char* key, int n, int col, float** data) {
    string sk(key);
    if (!CheckInputSize(key, n)) return;
    if (StringMatch(sk, VAR_CONDUCT)) {
        this->m_raster2D = data;
        this->m_soilLayers = col;
    }
}

void IO_TEST::SetScenario(bmps::Scenario* sce) {
    if (nullptr != sce) { m_scenario = sce; }
}

void IO_TEST::SetReaches(clsReaches* reaches) {
    if (nullptr != reaches) { m_reaches = reaches; }
}

bool IO_TEST::CheckInputSize(const char* key, int n) {
    CHECK_POSITIVE("IO_TEST", n);
    if (m_nCells != n) {
        if (m_nCells <= 0) { m_nCells = n; } else {
            throw ModelException("IO_TEST", "CheckInputSize", "Input data for " + string(key) +
                                 " is invalid. All the input data should have same size.");
        }
    }
    return true;
}

bool IO_TEST::CheckInputData() {
    /// m_date is protected variable member in base class SimulationModule.
    CHECK_POSITIVE("IO_TEST", m_date);
    CHECK_POSITIVE("IO_TEST", m_nCells);
    CHECK_POINTER("IO_TEST", m_raster1D);
    CHECK_POINTER("IO_TEST", m_raster2D);
    CHECK_POINTER("IO_TEST", m_nSoilLayrs);
    return true;
}

int IO_TEST::Execute() {
    /// Initialize output variables
    if (nullptr == m_output1Draster) { Initialize1DArray(m_nCells, m_output1Draster, 0.f); }

    if (nullptr == m_output2Draster) { Initialize2DArray(m_nCells, m_soilLayers, m_output2Draster, NODATA_VALUE); }

    /// Execute function
#pragma omp parallel for
    for (int i = 0; i < m_nCells; i++) {
        m_output1Draster[i] = m_raster1D[i] * 0.5f;
        for (int j = 0; j < m_nSoilLayrs[i]; j++) {
            m_output2Draster[i][j] = m_raster2D[i][j] + 2.f;
        }
    }
    /// Write Scenario Information
    // m_scenario->Dump("e:\\test\\bmpScenario2.txt");
    int nReaches = m_reaches->GetReachNumber();
    return 0;
}

void IO_TEST::Get1DData(const char* key, int* n, float** data) {
    string sk(key);
    if (StringMatch(sk, "CN2_M")) {
        *data = this->m_output1Draster;
        *n = this->m_nCells;
    }
}

void IO_TEST::Get2DData(const char* key, int* n, int* col, float*** data) {
    string sk(key);
    if (StringMatch(sk, "K_M")) {
        *data = this->m_output2Draster;
        *n = this->m_nCells;
        *col = this->m_soilLayers;
    }
}
