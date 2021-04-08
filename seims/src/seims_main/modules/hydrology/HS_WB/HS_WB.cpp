#include "HS_WB.h"
#include "text.h"

HS_WB::HS_WB(void) : m_dtHs(-1.f), m_dtCh(-1.f), m_nReaches(-1), m_nCells(-1), m_qs(NULL), m_qi(NULL), m_subbasin(NULL),
                     m_streamLink(NULL), m_subbasinID(-1),
                     m_qsInput(NULL), m_qiInput(NULL), m_qiTemp(NULL), m_qsTemp(NULL) {
}

HS_WB::~HS_WB(void) {
    if (m_qsInput != NULL) Release1DArray(m_qsInput);
    if (m_qiInput != NULL) Release1DArray(m_qiInput);
    if (m_qsTemp != NULL) Release1DArray(m_qsTemp);
    if (m_qiTemp != NULL) Release1DArray(m_qiTemp);
}

void HS_WB:: InitialOutputs() {
    if (m_qsInput == NULL) {
        CheckInputData();
        //cout << "Number of reaches: " << m_nReaches << endl;
        Initialize1DArray(m_nReaches + 1, m_qsInput, 0.f);
        Initialize1DArray(m_nReaches + 1, m_qiInput, 0.f);
        Initialize1DArray(m_nReaches + 1, m_qsTemp, 0.f);
        Initialize1DArray(m_nReaches + 1, m_qiTemp, 0.f);
    }
}

//Execute module
int HS_WB::Execute() {

    InitialOutputs();

    /// get the input of each subbasin in current step
#pragma omp parallel for
    for (int i = 1; i <= m_nReaches; i++) {
        m_qsTemp[i] = 0.f;
        m_qiTemp[i] = 0.f;
    }

    int reachIdx = 1;
    for (int i = 0; i < m_nCells; i++) {
        if (m_subbasinID == 0) { // deprecate the previously used macro MULTIPLY_REACHES
            reachIdx = int(m_streamLink[i]);
        }
        if (m_streamLink[i] > 0) {
            m_qsTemp[reachIdx] += m_qs[i];
            if (m_qi != NULL) {
                m_qiTemp[reachIdx] += m_qi[i];
            }
        }
    }

    m_qsInput[0] = 0.f;
    m_qiInput[0] = 0.f;
    for (int i = 1; i <= m_nReaches; i++) {
        m_qsInput[i] = (m_qsInput[i] * (m_tsCounter - 1) + m_qsTemp[i]) / m_tsCounter;
        m_qiInput[i] = (m_qiInput[i] * (m_tsCounter - 1) + m_qiTemp[i]) / m_tsCounter;

        m_qsInput[0] += m_qsInput[i];
        m_qiInput[0] += m_qiInput[i];
    }

    m_tsCounter++;

    return 0;
}

void HS_WB::SetValue(const char *key, float data) {
    string s(key);
    if (StringMatch(s, Tag_SubbasinId)) {
        m_subbasinID = data;
    } else {
        throw ModelException(MID_HS_WB, "SetValue", "Parameter " + s
            + " does not exist in current module. Please contact the module developer.");
    }
}

void HS_WB::Set1DData(const char *key, int nRows, float *data) {
    string s(key);

    this->CheckInputSize(key, nRows);

    if (StringMatch(s, VAR_QOVERLAND)) { this->m_qs = data; }
    else if (StringMatch(s, VAR_QSOIL)) { this->m_qi = data; }
    else if (StringMatch(s, VAR_SUBBSN)) { this->m_subbasin = data; }
    else if (StringMatch(s, VAR_STREAM_LINK)) { this->m_streamLink = data;}
    else if (StringMatch(s, VAR_SOILDEPTH)) { this->m_rootdepth = data;}
    else if (StringMatch(s, VAR_POROST)) { this->m_porosity = data; }
    else if (StringMatch(s, VAR_FIELDCAP)) { this->m_fieldCapacity = data; }
    else if (StringMatch(s, VAR_NEPR)) { m_pNet = data; }
    else {
        throw ModelException(MID_HS_WB, "Set1DData", "Parameter " + s +
            " does not exist in current module. Please contact the module developer.");
    }
}

void HS_WB::Get1DData(const char *key, int *nRows, float **data) {
    *nRows = m_nReaches + 1;
    string s(key);
    if (StringMatch(s, VAR_SBOF)) {
        *data = m_qsInput;
    } else if (StringMatch(s, VAR_SBIF)) {
        *data = m_qiInput;
    } else {
        throw ModelException(MID_HS_WB, "Get1DData", "Result " + string(key) +
            " does not exist in current module. Please contact the module developer.");
    }
}

void HS_WB::Get2DData(const char *key, int *nRows, int *nCols, float ***data) {
    string s(key);
    if (StringMatch(s, VAR_SOWB)) {
        *nCols = 17;
        *data = m_soilWaterBalance;
    } else {
        throw ModelException(MID_HS_WB, "Get2DData", "Result " + s +
            " does not exist in current module. Please contact the module developer.");
    }
}

void HS_WB::SetReaches(clsReaches *reaches) {
    assert(NULL != reaches);
    m_nReaches = reaches->GetReachNumber();
}

bool HS_WB::CheckInputData() {
    CHECK_NONNEGATIVE(MID_HS_WB, m_subbasinID);
    CHECK_POSITIVE(MID_HS_WB, m_nCells);
    CHECK_POINTER(MID_HS_WB, m_qs);
    CHECK_POINTER(MID_HS_WB, m_subbasin);
    CHECK_POINTER(MID_HS_WB, m_streamLink);
    return true;
}

bool HS_WB::CheckInputSize(const char *key, int n) {
    if (n <= 0) {
        throw ModelException(MID_HS_WB, "CheckInputSize",
                             "Input data for " + string(key) + " is invalid. The size could not be less than zero.");
        return false;
    }
    if (this->m_nCells != n) {
        if (this->m_nCells <= 0) {
            this->m_nCells = n;
        } else {
            cout << key << "\t" << n << "\t" << m_nCells << endl;
            throw ModelException(MID_HS_WB, "CheckInputSize", "Input data for " + string(key) +
                " is invalid. All the input data should have same size.");
            return false;
        }
    }

    return true;
}
