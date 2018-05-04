#include "seims.h"
#include "clsTSD_RD.h"

using namespace std;

void clsTSD_RD::Set1DData(const char *key, int n, float *data) {
    m_nStations = n;
    m_stationData = data;
}

void clsTSD_RD::Get1DData(const char *key, int *n, float **data) {
    if (m_nStations < 0 || nullptr == m_stationData) {
        throw ModelException(MID_TSD_RD, "Get1DData", "The data " + string(key) + " is NULL.");
    }
    *data = m_stationData;
    *n = m_nStations;
}
