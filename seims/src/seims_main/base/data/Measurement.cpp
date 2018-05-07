#include "Measurement.h"

#include "utils_string.h"
#include "utils_array.h"

Measurement::Measurement(MongoClient* conn, string& hydroDBName, string& sitesList, string& siteType, time_t startTime,
                         time_t endTime) : m_conn(conn), m_hydroDBName(hydroDBName), m_type(siteType),
                                           m_startTime(startTime), m_endTime(endTime), pData(nullptr) {
    utils_string::SplitStringForValues(sitesList, ',', m_siteIDList);
    sort(m_siteIDList.begin(), m_siteIDList.end());
    pData = new float[m_siteIDList.size()];
}

Measurement::~Measurement() {
    if (pData != nullptr) utils_array::Release1DArray(pData);
}
