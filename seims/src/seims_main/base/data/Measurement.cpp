#include "Measurement.h"

#include "utils_string.h"
#include "utils_array.h"

using namespace ccgl::utils_array;

Measurement::Measurement(MongoClient* conn, const string& hydroDBName,
                         const string& sitesList, const string& siteType,
                         const time_t startTime, const time_t endTime) :
    m_conn(conn), m_hydroDBName(hydroDBName), m_type(siteType),
    m_startTime(startTime), m_endTime(endTime), pData(nullptr) {
    utils_string::SplitStringForValues(sitesList, ',', m_siteIDList);
    sort(m_siteIDList.begin(), m_siteIDList.end());
    Initialize1DArray(m_siteIDList.size(), pData, NODATA_VALUE);
}

Measurement::~Measurement() {
    if (pData != nullptr) Release1DArray(pData);
}
