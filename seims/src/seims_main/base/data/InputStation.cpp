/*!
 * \brief
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date May 2016
 */
#include <fstream>
#include <sstream>
#include <ctime>
#include "InputStation.h"
#include "util.h"
#include "utils.h"
#include "ModelException.h"
#include "RegularMeasurement.h"
#include "NotRegularMeasurement.h"

using namespace std;

InputStation::InputStation(mongoc_client_t *conn, time_t dtHillslope, time_t dtChannel) : m_conn(conn),
                                                                                          m_dtHs(dtHillslope),
                                                                                          m_dtCh(dtChannel)
{
}

InputStation::~InputStation(void)
{
    for (map<string, Measurement *>::iterator it = m_measurement.begin();
         it != m_measurement.end();)
    {
        if (it->second != NULL)
        {
            delete it->second;
            it->second = NULL;
        }
        it = m_measurement.erase(it);
    }
    m_measurement.clear();

    for (map<string, float *>::iterator it = m_latitude.begin(); it != m_latitude.end();)
    {
        if (it->second != NULL)
        {
            delete[] it->second;
            it->second = NULL;
        }
        it = m_latitude.erase(it);
    }
    m_latitude.clear();
    for (map<string, float *>::iterator it = m_elevation.begin(); it != m_elevation.end();)
    {
        if (it->second != NULL)
        {
            delete[] it->second;
            it->second = NULL;
        }
        it = m_elevation.erase(it);
    }
    m_elevation.clear();
}

void InputStation::build_query_bson(int nSites, vector<int> &siteIDList, string &siteType, bson_t *query)
{
    /// build query statement
    //query = bson_new();  query has been initalized before
    bson_t *child = bson_new();
    bson_t *child2 = bson_new();
    bson_t *child3 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(query, "$query", child);
    BSON_APPEND_DOCUMENT_BEGIN(child, MONG_HYDRO_DATA_SITEID, child2);
    BSON_APPEND_ARRAY_BEGIN(child2, "$in", child3);
    ostringstream ossIndex;
    for (int iSite = 0; iSite < nSites; iSite++)
    {
        ossIndex.str("");
        ossIndex << iSite;
        BSON_APPEND_INT32(child3, ossIndex.str().c_str(), siteIDList[iSite]);
    }
    bson_append_array_end(child2, child3);
    bson_append_document_end(child, child2);
    bson_destroy(child2);
    bson_destroy(child3);

    if (siteType == DataType_Precipitation)
    {
        BSON_APPEND_UTF8(child, MONG_HYDRO_SITE_TYPE, siteType.c_str());//"Type"
        child2 = bson_new();
        child3 = bson_new();
        BSON_APPEND_DOCUMENT_BEGIN(child, MONG_HYDRO_SITE_TYPE, child2);//"Type"
        BSON_APPEND_ARRAY_BEGIN(child2, "$in", child3);
        BSON_APPEND_UTF8(child3, "0", DataType_Precipitation);  //"P"
        BSON_APPEND_UTF8(child3, "1", DataType_Meteorology);    //"M"
        bson_append_array_end(child2, child3);
        bson_append_document_end(child, child2);
        bson_destroy(child2);
        bson_destroy(child3);
    }
    else
        BSON_APPEND_UTF8(child, MONG_HYDRO_SITE_TYPE, siteType.c_str());//"Type"
    bson_append_document_end(query, child);
    bson_destroy(child);
    child2 = bson_new();
    BSON_APPEND_DOCUMENT_BEGIN(query, "$orderby", child2);
    /// sort by site ID
    BSON_APPEND_INT32(child2, MONG_HYDRO_DATA_SITEID, 1);
    bson_append_document_end(query, child2);
    bson_destroy(child2);
}

void InputStation::ReadSitesInfo(string siteType, string hydroDBName, string sitesList)
{
    vector<string> vecSites = utils::SplitString(sitesList, ',');
    int nSites = vecSites.size();
    //convert from string to int, the IDList is in order in MongoDB
    vector<int> siteIDList;
    for (int iSite = 0; iSite < nSites; iSite++)
        siteIDList.push_back(atoi(vecSites[iSite].c_str()));
    //sort(siteIDList.begin(), siteIDList.end());

    bson_t *query = bson_new();
    build_query_bson(nSites, siteIDList, siteType, query);
    //printf("%s\n",bson_as_json(query,NULL));

    mongoc_cursor_t *cursor;
    mongoc_collection_t *collection;
    collection = mongoc_client_get_collection(m_conn, hydroDBName.c_str(), DB_TAB_SITES);
    cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, query, NULL, NULL);
    const bson_t *record = NULL;
    float *pEle = new float[nSites];
    float *pLat = new float[nSites];
    float ele, lat;
    bool hasData = false;
    vector<int>::iterator siteIDIter;
    while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &record))
    {
        hasData = true;
        bson_iter_t iter;
        int siteIndex = -1;
        int curSiteID = -1;
        if (bson_iter_init(&iter, record) && bson_iter_find(&iter, MONG_HYDRO_DATA_SITEID))
        {
            curSiteID = GetIntFromBSONITER(&iter);
        }
        else
            throw ModelException("Measurement", "Measurement",
                                 "The Site ID field does not exist in Sites table, Please check and retry.");
        siteIDIter = find(siteIDList.begin(), siteIDList.end(), curSiteID);
        siteIndex = distance(siteIDList.begin(), siteIDIter);
        if (bson_iter_init(&iter, record) && bson_iter_find(&iter, MONG_HYDRO_SITE_LAT))
        {
            lat = GetFloatFromBSONITER(&iter);
        }
        else
            throw ModelException("InputStation", "ReadSitesInfo", "The Lat field does not exist in Sites table.");

        if (bson_iter_init(&iter, record) && bson_iter_find(&iter, MONG_HYDRO_SITE_ELEV))
        {
            ele = GetFloatFromBSONITER(&iter);
        }
        else
            throw ModelException("InputStation", "ReadSitesInfo", "The Lat field does not exist in Sites table.");
        pLat[siteIndex] = lat;
        pEle[siteIndex] = ele;
    }
    if (!hasData)
        throw ModelException("InputStation", "ReadSitesInfo", "Query failed.");

    mongoc_cursor_destroy(cursor);
    mongoc_collection_destroy(collection);
    bson_destroy(query);
    m_elevation[siteType] = pEle;
    m_latitude[siteType] = pLat;
    m_numSites[siteType] = nSites;
}

void InputStation::ReadSitesData(string hydroDBName, string sitesList, string siteType, time_t startDate,
                                 time_t endDate, bool stormMode)
{
    siteType = GetUpper(siteType);

    //clock_t start = clock();
    if (stormMode)
    {
        m_measurement[siteType] = new NotRegularMeasurement(m_conn, hydroDBName, sitesList, siteType, startDate,
                                                            endDate);
    }
    else
    {
        m_measurement[siteType] = new RegularMeasurement(m_conn, hydroDBName, sitesList, siteType, startDate, endDate,
                                                         m_dtHs);
    }
    //clock_t end = clock();
    //cout << "Read measurement " << siteType << " " << end - start << endl;

    //start = clock();
    if (StringMatch(siteType, DataType_Precipitation))  // "P"
    {
        ReadSitesInfo(DataType_Precipitation, hydroDBName, sitesList);
    }
    else if (m_elevation.find(DataType_Meteorology) == m_elevation.end()) // "M"
    {
        ReadSitesInfo(DataType_Meteorology, hydroDBName, sitesList);
    }
    //end = clock();
    //cout << "ReadSitesInfo " << siteType << " " << end - start << endl;
}

void InputStation::GetTimeSeriesData(time_t time, string type, int *nRow, float **data)
{
    Measurement *m = m_measurement[type];
    *nRow = m->NumberOfSites();
    //cout << type << "\t" << *nRow << endl;
    *data = m->GetSiteDataByTime(time);
}
