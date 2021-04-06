#include "clsTSD_RD.h"

#include "text.h"

clsTSD_RD::clsTSD_RD() : m_nStations(-1), m_stationData(nullptr) {
}

clsTSD_RD::~clsTSD_RD() {
}

void clsTSD_RD::Set1DData(const char *key, const int n, float *data) {
    m_nStations = n;
    m_stationData = data;
}

void clsTSD_RD::Get1DData(const char *key, int *n, float **data) {
    if (m_nStations < 0 || nullptr == m_stationData) {
        throw ModelException(M_TSD_RD[0], "Get1DData", "The data " + string(key) + " is NULL.");
    }
    *data = m_stationData;
    *n = m_nStations;
}
