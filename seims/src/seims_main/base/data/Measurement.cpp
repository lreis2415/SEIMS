#include "Measurement.h"

Measurement::Measurement(MongoClient *conn, string& hydroDBName, string& sitesList, string& siteType, time_t startTime,
                         time_t endTime) : m_conn(conn), m_hydroDBName(hydroDBName), m_type(siteType),
                                           m_startTime(startTime), m_endTime(endTime), pData(nullptr) {
    m_siteIDList = SplitStringForInt(sitesList, ',');
    sort(m_siteIDList.begin(), m_siteIDList.end());
    pData = new float[m_siteIDList.size()];
}

Measurement::~Measurement() {
    Release1DArray(pData);
}
