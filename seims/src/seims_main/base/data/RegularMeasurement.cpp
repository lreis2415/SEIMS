#include "RegularMeasurement.h"

#include <sstream>
#include <memory>

#include "text.h"
#include "utils_time.h"
#include "utils_array.h"
#include "utils_string.h"

using namespace utils_time;
using namespace utils_string;

RegularMeasurement::RegularMeasurement(MongoClient* conn, const string& hydroDBName,
                                       const string& sitesList, const string& siteType,
                                       const time_t startTime, const time_t endTime, const time_t interval):
    Measurement(conn, hydroDBName, sitesList, siteType, startTime, endTime), m_interval(interval) {
    int nSites = CVT_INT(m_siteIDList.size());
    int nRecords = CVT_INT((m_endTime - m_startTime) / m_interval + 1);
    m_siteData.reserve(nRecords);

    // Query command likes:
    // { "$query" : { "STATIONID" : { "$in" : [58911] },
    //                "TYPE" : "TMEAN",
    //                "UTCDATETIME" : { "$gte" : { "$date" : 1325376000000 },
    //                                  "$lte" : { "$date" : 1451606399000 } }
    //               },
    //   "$orderby" : { "STATIONID" : 1,
    //                  "UTCDATETIME" : 1 }
    // }
    bson_t* q;
    bson_t* site_array = bson_new();
    for (int i = 0; i < nSites; i++) {
        BSON_APPEND_INT32(site_array, ValueToString(i).c_str(), m_siteIDList[i]);
    }
    // StatusMessage(bson_as_json(site_array, NULL));
    vint st = CVT_VINT(startTime) * 1000;
    vint et = CVT_VINT(endTime) * 1000;

    q = BCON_NEW("$query", "{", MONG_HYDRO_DATA_SITEID, "{", "$in", BCON_ARRAY(site_array), "}",
                                MONG_HYDRO_SITE_TYPE, BCON_UTF8(siteType.c_str()),
                                MONG_HYDRO_DATA_UTC, "{", "$gte", BCON_DATE_TIME(st),
                                                          "$lte", BCON_DATE_TIME(et),
                                                     "}",
                           "}",
                 "$orderby", "{", MONG_HYDRO_DATA_SITEID, BCON_INT32(1),
                                  MONG_HYDRO_DATA_UTC, BCON_INT32(1),
                             "}");

    // StatusMessage(bson_as_json(q, NULL));

    // perform query and read measurement data
    std::unique_ptr<MongoCollection>
            collection(new MongoCollection(m_conn->GetCollection(hydroDBName, DB_TAB_DATAVALUES)));
    mongoc_cursor_t* cursor = collection->ExecuteQuery(q);
    bson_error_t err;
    if (mongoc_cursor_error(cursor, &err)) {
        StatusMessage(err.message);
        bson_destroy(q);
        mongoc_cursor_destroy(cursor);
        throw ModelException("RegularMeasurement", "Constructor", "Query error! ");
    }
    FLTPT value;
    int stationIDLast = -1;
    int stationID = -1;
    int iSite = -1;
    vector<int>::size_type index = 0;
    const bson_t* doc;
    while (mongoc_cursor_next(cursor, &doc)) {
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

        if (m_siteData.size() < index + 1) {
            FLTPT* tmpData = nullptr;
            Initialize1DArray(nSites, tmpData, 0.f);
            m_siteData.emplace_back(tmpData);
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
    if (CVT_INT(index) < nRecords) {
        std::ostringstream oss;
        oss << "There are no adequate data of " << siteType << " for sites:[" << sitesList << "] in database:" <<
                hydroDBName << " during " << ConvertToString2(m_startTime) << " to " << ConvertToString2(m_endTime) <<
                ". You may want to check the database or the input simulation period!";
        throw ModelException("RegularMeasurement", "Constructor", oss.str());
    }
    if (iSite + 1 != nSites) {
        std::ostringstream oss;
        oss << "The number of sites should be " << nSites << " while the query result is " << iSite + 1 <<
                " for sites:[" << sitesList << "] in database:" << hydroDBName
                << " during " << ConvertToString2(m_startTime) << " to " << ConvertToString2(m_endTime);
        throw ModelException("RegularMeasurement", "Constructor", oss.str());
    }
    bson_destroy(site_array);
    bson_destroy(q);
    mongoc_cursor_destroy(cursor);
}

RegularMeasurement::~RegularMeasurement() {
    for (auto it = m_siteData.begin(); it != m_siteData.end(); ++it) {
        if (*it != nullptr) {
            utils_array::Release1DArray(*it);
        }
        // it = m_siteData.erase(it);
    }
    m_siteData.clear();
}

FLTPT* RegularMeasurement::GetSiteDataByTime(const time_t t) {
    int index = CVT_INT((t - m_startTime) / m_interval);
    if (index < 0) {
        index = 0;
    }
    size_t nSites = m_siteIDList.size();
    for (size_t i = 0; i < nSites; i++) {
        pData[i] = m_siteData[index][i];
    }
    return pData;
}
