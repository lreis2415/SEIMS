/*!
 * \file NotRegularMeasurement.h
 * \brief Not Regular hydroclimate data, i.e., for storm model
 *
 * Changelog:
 *   - 1. 2016-05-30 - lj - Replace mongoc_client_t by MongoClient interface.
 *   - 2. 2022-08-18 - lj - Change float to FLTPT.
 *
 * \author Junzhi Liu, Liangjun Zhu
 * \version 2.1
 */
#ifndef SEIMS_NOTREGULAR_MEASUREMENT_H
#define SEIMS_NOTREGULAR_MEASUREMENT_H

#include "db_mongoc.h"
#include "Measurement.h"
#include <seims.h>

/*!
 * \ingroup data
 * \class NotRegularMeasurement
 * \brief Not Regular hydroclimate data
 */
class NotRegularMeasurement: public Measurement {
public:
    /*!
     * \brief Constructor
     *
     * Initialize NotRegular Measurement instance from MongoDB
     *
     * \param[in] conn \a MongoClient, MongoDB client
     * \param[in] hydroDBName \a string, HydroClimate database name
     * \param[in] sitesList \a string, site list
     * \param[in] siteType \a string, site type
     * \param[in] startTime \a time_t, start date time
     * \param[in] endTime \a time_t, end date time
     */
    NotRegularMeasurement(MongoClient* conn, const string& hydroDBName,
                          const string& sitesList, const string& siteType,
                          time_t startTime, time_t endTime);

    //! Get site date by time \a pData
    FLTPT* GetSiteDataByTime(time_t t) OVERRIDE;

private:
    vector<vector<time_t> > m_timeList; ///< time list of site data
    vector<vector<FLTPT> > m_valueList; ///< site data corresponding to m_timeList
    vector<int> m_curIndexList;         ///< index
};
#endif /* SEIMS_NOTREGULAR_MEASUREMENT_H */
