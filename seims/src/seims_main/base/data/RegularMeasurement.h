/*!
 * \file RegularMeasurement.h
 * \brief Regular hydroclimate data, e.g., daily data
 *
 * Changelog:
 *   - 1. 2016-05-30 - lj - Replace mongoc_client_t by MongoClient interface.
 *
 * \author Junzhi Liu, Liangjun Zhu
 * \version 2.0
 */
#ifndef SEIMS_REGULAR_MEASUREMENT_H
#define SEIMS_REGULAR_MEASUREMENT_H

#include "db_mongoc.h"
#include "Measurement.h"

/*!
 * \ingroup data
 * \class RegularMeasurement
 * \brief Regular hydroclimate data, e.g., daily data
 */
class RegularMeasurement: public Measurement {
public:
    /*!
     * \brief Initialize NotRegular Measurement instance from MongoDB
     *
     * \param[in] conn \a MongoClient, MongoDB client
     * \param[in] hydroDBName \a string, HydroClimate database name
     * \param[in] sitesList \a string, site list
     * \param[in] siteType \a string, site type
     * \param[in] startTime \a time_t, start date time
     * \param[in] endTime \a time_t, end date time
     * \param[in] interval \a time_t, time interval
     */
    RegularMeasurement(MongoClient* conn,
                       const string& hydroDBName, const string& sitesList, const string& siteType,
                       time_t startTime, time_t endTime, time_t interval);

    //! Destructor
    ~RegularMeasurement();

    //! Get site date by time \a pData
    float* GetSiteDataByTime(time_t t) OVERRIDE;

private:
    vector<float *> m_siteData; ///< data array ordered by sites
    time_t m_interval;          ///< data record interval
};
#endif /* SEIMS_REGULAR_MEASUREMENT_H */
