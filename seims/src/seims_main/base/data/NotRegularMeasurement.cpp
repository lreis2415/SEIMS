/*!
 * \brief Not Regular hydroclimate data
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date May 2016
 *
 * 
 */
#include "NotRegularMeasurement.h"
#include "utils.h"
#include "ModelException.h"

//! Constructor
NotRegularMeasurement::NotRegularMeasurement(mongoc_client_t *conn, string hydroDBName, string sitesList,
                                             string siteType, time_t startTime, time_t endTime)
        : Measurement(conn, hydroDBName, sitesList, siteType, startTime, endTime)
{
    int nSites = m_siteIDList.size();
    m_valueList.resize(nSites);
    m_timeList.resize(nSites);
    m_curIndexList.resize(nSites, 0);
    for (int iSite = 0; iSite < nSites; iSite++)
    {
        /// build query statement
        bson_t *query = bson_new();
        bson_t *child = bson_new();
        bson_t *child2 = bson_new();
        bson_t *child3 = bson_new();
        BSON_APPEND_DOCUMENT_BEGIN(query, "$query", child);
        BSON_APPEND_DOCUMENT_BEGIN(child, MONG_HYDRO_DATA_SITEID, child2);
        BSON_APPEND_ARRAY_BEGIN(child2, "$in", child3);
        ostringstream ossIndex;
        ossIndex.str("");
        ossIndex << iSite;
        BSON_APPEND_INT32(child3, ossIndex.str().c_str(), m_siteIDList[iSite]);
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
        mongoc_cursor_t *cursor;
        mongoc_collection_t *collection;
        const bson_t *doc;
//		char *record;
        collection = mongoc_client_get_collection(m_conn, hydroDBName.c_str(), DB_TAB_DATAVALUES);
        cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
        float value;
        time_t dt;
        bool hasData = false;
        while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &doc))
        {
            hasData = true;
//			record = bson_as_json(doc,NULL);
            bson_iter_t iter;
            if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, MONG_HYDRO_DATA_VALUE))
            {
                value = GetFloatFromBSONITER(&iter);
            }
            else
                throw ModelException("NotRegularMeasurement", "NotRegularMeasurement",
                                     "The Value field does not exist in DataValues table.");

            if (bson_iter_init(&iter, doc) && bson_iter_find(&iter, MONG_HYDRO_DATA_UTC))
            {
                dt = GetDateTimeFromBSONITER(&iter) / 1000.f;
            }
            else
                throw ModelException("NotRegularMeasurement", "NotRegularMeasurement",
                                     "The UTCDateTime field does not exist in DataValues table.");

            m_timeList[iSite].push_back(dt);
            m_valueList[iSite].push_back(value);
        }
        bson_destroy(query);
        mongoc_cursor_destroy(cursor);
        mongoc_collection_destroy(collection);

        if (!hasData)
        {
            ostringstream oss;
            utils util;
            oss << "There are no " << siteType << " data available for sites:[" << m_siteIDList[iSite] <<
            "] in database:" << hydroDBName
            << " during " << util.ConvertToString2(&m_startTime) << " to " << util.ConvertToString2(&m_endTime);
            throw ModelException("NotRegularMeasurement", "Constructor", oss.str());
        }
    }
}

//! Destructor
NotRegularMeasurement::~NotRegularMeasurement(void)
{
    if (pData != NULL) Release1DArray(pData);
    for (vector<vector<float> >::iterator it = m_valueList.begin(); it != m_valueList.end();)
    {
        it = m_valueList.erase(it);
    }
    m_valueList.clear();
}

//! Get site data by time
float *NotRegularMeasurement::GetSiteDataByTime(time_t t)
{
    for (vector<int>::size_type iSite = 0; iSite < m_siteIDList.size(); iSite++)
    {
        vector<time_t> &tlist = m_timeList[iSite];
        vector<float> &vlist = m_valueList[iSite];
        size_t curIndex = m_curIndexList[iSite];

        // find the index for current time
        // the nearest record before t
        while (curIndex < tlist.size() && tlist[curIndex] <= t)
            curIndex++;
        curIndex--;

        //if (curIndex < 0)
        //{
        //    pData[iSite] = 0.f;
        //    m_curIndexList[iSite] = 0;
        //}
        //else
        //{
            pData[iSite] = vlist[curIndex];
            m_curIndexList[iSite] = curIndex;
        //}
    }
    return pData;
}
