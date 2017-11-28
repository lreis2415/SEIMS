/*!
 * \brief Measurement class to store HydroClimate site data
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date May 2016
 * \revised LJ - Replace mongoc_client_t by MongoClient interface
 */
#ifndef SEIMS_MEASUREMENT_H
#define SEIMS_MEASUREMENT_H

#include "utilities.h"
#include "MongoUtil.h"

using namespace std;

/*!
 * \ingroup data
 * \class Measurement
 * \brief Get HydroClimate measurement data from MongoDB
 */
class Measurement {
public:
    /*!
     * \brief Constructor
     *
     * Initialize Measurement instance from MongoDB
     *
     * \param[in] conn \a MongoClient, MongoDB client
     * \param[in] hydroDBName \a string, HydroClimate database name
     * \param[in] siteType \a string, site type
     * \param[in] startDate \a time_t, start date time
     * \param[in] endDate \a time_t, end date time
     */
    Measurement(MongoClient *conn, string& hydroDBName, string& sitesList, string& siteType,
                time_t startDate, time_t endDate);

    //! Destructor
    virtual ~Measurement();

    //! Get site data by time
    virtual float *GetSiteDataByTime(time_t t) = 0;

    //! Get Number of site
    int NumberOfSites() const { return (int) m_siteIDList.size(); }

    //! Get HydroClimate site type, "M" or "P"
    string Type() const { return m_type; }

    //! start time
    time_t StartTime() const { return m_startTime; }

    //! end time
    time_t EndTime() const { return m_endTime; }

protected:
    //! MongoDB client object
    MongoClient* m_conn;
    //! HydroClimate database name
    string m_hydroDBName;
    //! Site IDs list
    vector<int> m_siteIDList;
    //! Site type, M means meteorology, and P means precipitation
    string m_type;
    //! Start time
    time_t m_startTime;
    //! End time
    time_t m_endTime;
    //!	Measurement data of all sites in given date
    float* pData;
};
#endif /* SEIMS_MEASUREMENT_H */
