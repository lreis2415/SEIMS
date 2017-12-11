/*!
 * \brief Regular hydroclimate data, e.g., daily data
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date May 2016
 * \revised LJ - Replace mongoc_client_t by MongoClient interface
 */
#ifndef SEIMS_REGULAR_MEASUREMENT_H
#define SEIMS_REGULAR_MEASUREMENT_H

#include "text.h"
#include "utilities.h"
#include "MongoUtil.h"
#include "Measurement.h"

/*!
 * \ingroup data
 * \class RegularMeasurement
 * \brief Regular hydroclimate data, e.g., daily data
 */
class RegularMeasurement : public Measurement {
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
     * \param[in] startDate \a time_t, start date time
     * \param[in] endDate \a time_t, end date time
     * \param[in] interval \a time_t, time interval
     */
    RegularMeasurement(MongoClient *conn, string hydroDBName, string sitesList, string siteType, time_t startTime,
                       time_t endTime, time_t interval);

    //! Destructor
    virtual ~RegularMeasurement();

    //! Get site date by time \a pData
    virtual float *GetSiteDataByTime(time_t t);

private:
    //! site data : the first dimension is time, and the second dimension is data by sites
    vector<float *> m_siteData;
    //! data record interval
    time_t m_interval;
};
#endif /* SEIMS_REGULAR_MEASUREMENT_H */
