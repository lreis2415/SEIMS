/*!
 * \file InputStation.h
 * \brief HydroClimate site information
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.2
 * \date Aug., 2022
 */
#ifndef SEIMS_CLIMATE_STATION_H
#define SEIMS_CLIMATE_STATION_H

#include <map>

#include "basic.h"
#include "db_mongoc.h"

#include "Measurement.h"
#include <seims.h>

using namespace ccgl;
using namespace db_mongoc;
using std::map;

/*!
 * \ingroup data
 * \class InputStation
 * \brief HydroClimate sites information
 */
class InputStation: Interface {
public:
    //! Constructor
    InputStation(MongoClient* conn, time_t dtHillslope, time_t dtChannel, string dataValueDirectory="");

    //! Destructor
    ~InputStation();

    //! Get site number of given site type
    bool NumberOfSites(const char* site_type, int& site_count);

    //! Get elevations of given site type
    bool GetElevation(const char* site_type, FLTPT*& site_elevs);

    //! Get latitudes of given site type
    bool GetLatitude(const char* site_type, FLTPT*& site_lats);

    /*!
     * \brief Get time series data
     *
     * \param[in] time data time
     * \param[in] type data type
     * \param[out] nRow data item number
     * \param[out] data time series data
     */
    void GetTimeSeriesData(time_t time, const string& type, int* nRow, FLTPT** data);

    /*!
     * \brief Read data of each site type
     *
     * \param[in] hydroDBName HydroClimate database which contains "DataValues" collection
     * \param[in] sitesList  \a string, site ID string derived from model database (e.g., model_dianbu30m_longterm)
     * \param[in] siteType site type
     * \param[in] startDate start date
     * \param[in] endDate end date
     * \param[in] stormMode \a bool, false by default
     */
    void ReadSitesData(const string& hydroDBName, const string& sitesList, const string& siteType,
                       time_t startDate, time_t endDate, bool stormMode = false, bool fileDB = false);

private:
    /*!
     * \brief build BSON query sentences for MongoDB
     *
     * \param[in] nSites site number
     * \param[in] siteIDList site ID list
     * \param[in] siteType site type, "P" or "M"
     * \param[in] query \a bson_t
     */
    void build_query_bson(int nSites, const vector<int>& siteIDList, const string& siteType, bson_t* query);

    /*!
     * \brief Read HydroClimate sites information from HydroClimateDB (MongoDB)
     *
     * \param[in] siteType site type, "P", "M" or others
     * \param[in] hydroDBName HydroClimate database which contains "Sites" collection
     * \param[in] sitesList \a string, site ID string derived from model database (e.g., model_dianbu30m_longterm)
     */
    void ReadSitesInfo(const string& siteType, const string& hydroDBName, const string& sitesList);

private:
    //! MongoDB client object
    MongoClient* m_conn;
    //! Channel scale time interval
    time_t m_dtCh;
    //! Hillslope scale time interval
    time_t m_dtHs;
    //! Measurement object of each data type
    map<string, Measurement *> m_measurement;
    //! Site ID: elevation
    map<string, FLTPT*> m_elevation;
    //! Site ID: latitude
    map<string, FLTPT*> m_latitude;
    //! site numbers of each site type
    map<string, int> m_numSites;

    string m_dataValueDirectory;
};
#endif /* SEIMS_CLIMATE_STATION_H */
