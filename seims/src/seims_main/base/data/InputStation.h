/*!
 * \file InputStation.h
 * \brief HydroClimate site information
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date May 2016
 */
#ifndef SEIMS_CLIMATE_STATION_H
#define SEIMS_CLIMATE_STATION_H

#include <map>

#include "basic.h"
#include "db_mongoc.h"

#include "Measurement.h"

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
    InputStation(MongoClient* conn, time_t dtHillslope, time_t dtChannel);

    //! Destructor
    ~InputStation();

    //! Get site number of given site type
    int NumberOfSites(const char* site_type) const { return m_numSites.at(site_type); }

    //! Get elevations of given site type
    float* GetElevation(const char* type) const { return m_elevation.at(type); }

    //! Get latitudes of given site type
    float* GetLatitude(const char* type) const { return m_latitude.at(type); }

    /*!
     * \brief Get time series data
     *
     * \param[in] time data time
     * \param[in] type data type
     * \param[out] nRow data item number
     * \param[out] data time series data
     */
    void GetTimeSeriesData(time_t time, const string& type, int* nRow, float** data);

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
                       time_t startDate, time_t endDate, bool stormMode = false);

private:
    /*!
     * \brief build BSON query sentences for MongoDB
     *
     * \param[in] nSites site number
     * \param[in] siteIDList site ID list
     * \param[in] siteType site type, "P" or "M"
     * \param[in] query \a bson_t
     */
    void build_query_bson(int nSites, vector<int>& siteIDList, const string& siteType, bson_t* query);

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
    map<string, float *> m_elevation;
    //! Site ID: latitude
    map<string, float *> m_latitude;
    //! site numbers of each site type
    map<string, int> m_numSites;
};
#endif /* SEIMS_CLIMATE_STATION_H */
