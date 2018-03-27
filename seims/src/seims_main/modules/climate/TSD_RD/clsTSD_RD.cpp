#include "seims.h"
#include "clsTSD_RD.h"

using namespace std;

clsTSD_RD::clsTSD_RD() : m_Rows(-1), m_Data(nullptr) {
}

clsTSD_RD::~clsTSD_RD() {
}

void clsTSD_RD::Set1DData(const char *key, int n, float *data) {
    m_Rows = n;
    m_Data = data;
}

void clsTSD_RD::Get1DData(const char *key, int *n, float **data) {
    string sk(key);
    if (m_Rows < 0 || nullptr == m_Data) {
        throw ModelException(MID_TSD_RD, "Get1DData", "The data " + string(key) + " is NULL.");
    }
    *data = m_Data;
    *n = m_Rows;
}
