#include "RegularMeasurement.h"

//! Constructor
RegularMeasurement::RegularMeasurement(MongoClient* conn, string hydroDBName, string sitesList, string siteType,
                                       time_t startTime, time_t endTime, time_t interval)
    : Measurement(conn, hydroDBName, sitesList, siteType, startTime, endTime), m_interval(interval) {
    int nSites = (int) m_siteIDList.size();

    /// build query statement
    bson_t *query = bson_new();
    bson_t *child = bson_new();
    bson_t *child2 = bson_new();
    bson_t *child3 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(query, "$query", child);
    BSON_APPEND_DOCUMENT_BEGIN(child, MONG_HYDRO_DATA_SITEID, child2);
    BSON_APPEND_ARRAY_BEGIN(child2, "$in", child3);
    ostringstream ossIndex;
    for (int iSite = 0; iSite < nSites; iSite++) {
        ossIndex.str("");
        ossIndex << iSite;
        BSON_APPEND_INT32(child3, ossIndex.str().c_str(), m_siteIDList[iSite]);
    }
    bson_append_array_end(child2, child3);
    bson_append_document_end(child, child2);
    bson_destroy(child2);
    bson_destroy(child3);
    BSON_APPEND_UTF8(child, MONG_HYDRO_SITE_TYPE, siteType.c_str());
    child2 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(child, MONG_HYDRO_DATA_UTC, child2);
    /// startTime <= t <= endTime,
    BSON_APPEND_TIME_T(child2, "$gte", startTime);
    BSON_APPEND_TIME_T(child2, "$lte", endTime);
    bson_append_document_end(child, child2);
    bson_append_document_end(query, child);
    bson_destroy(child2);
    bson_destroy(child);
    child = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(query, "$orderby", child);
    /// sort by stationID and time
    BSON_APPEND_INT32(child, MONG_HYDRO_DATA_SITEID, 1);
    BSON_APPEND_INT32(child, MONG_HYDRO_DATA_UTC, 1);
    bson_append_document_end(query, child);
    bson_destroy(child);
    //printf("%s\n", bson_as_json(query,NULL));

    // perform query and read measurement data
    unique_ptr<MongoCollection> collection(new MongoCollection(m_conn->getCollection(hydroDBName, DB_TAB_DATAVALUES)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(query);

    float value;
    int stationIDLast = -1;
    int stationID = -1;
    int iSite = -1;
    vector<int>::size_type index = 0;
    const bson_t *doc;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &doc)) {
        bson_iter_t iter;
        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, MONG_HYDRO_DATA_SITEID)) {
            GetNumericFromBsonIterator(&iter, stationID);
        } else {
            throw ModelException("RegularMeasurement", "Constructor", 
                "The Value field: " + string(MONG_HYDRO_DATA_SITEID) + 
                " does not exist in DataValues table.");
        }
        if (stationID != stationIDLast) {
            iSite++;
            index = 0;
            stationIDLast = stationID;
        }

        if (m_siteData.size() < (index + 1)) {
            float* pData = new float[nSites];
            for (int i = 0; i < nSites; i++) {
                pData[i] = 0.f;
            }
            m_siteData.push_back(pData);
        }
        if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, MONG_HYDRO_DATA_VALUE)) {
            GetNumericFromBsonIterator(&iter, value);
        } else {
            throw ModelException("RegularMeasurement", "Constructor",
                "The Value field: " + string(MONG_HYDRO_DATA_VALUE) +
                " does not exist in DataValues table.");
        }
        m_siteData[index][iSite] = value;
        index++;
    }
    if (index <= 0) {
        ostringstream oss;
        oss << "There are no " << siteType << " data available for sites:[" << sitesList << "] in database:" <<
            hydroDBName
            << " during " << ConvertToString2(&m_startTime) << " to " << ConvertToString2(&m_endTime);
        throw ModelException("RegularMeasurement", "Constructor", oss.str());
    } else if (iSite + 1 != nSites) {
        ostringstream oss;
        oss << "The number of sites should be " << nSites << " while the query result is " << iSite + 1 <<
            " for sites:[" << sitesList << "] in database:" << hydroDBName
            << " during " << ConvertToString2(&m_startTime) << " to " << ConvertToString2(&m_endTime);
        throw ModelException("RegularMeasurement", "Constructor", oss.str());
    }
    bson_destroy(query);
    mongoc_cursor_destroy(cursor);
}

RegularMeasurement::~RegularMeasurement() {
    for (auto it = m_siteData.begin(); it != m_siteData.end();) {
        if (*it != nullptr) {
            delete[] *it;
            *it = nullptr;
        }
        it = m_siteData.erase(it);
    }
    m_siteData.clear();
}

float *RegularMeasurement::GetSiteDataByTime(time_t t) {
    int index = int((t - m_startTime) / m_interval);

    if (index < 0) {
        index = 0;
    }

    size_t nSites = m_siteIDList.size();
    for (size_t i = 0; i < nSites; i++) {
        pData[i] = m_siteData[index][i];
    }
    return pData;
}
