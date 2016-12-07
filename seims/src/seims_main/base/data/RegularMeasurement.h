/*!
 * \brief Regular hydroclimate data, e.g., daily data
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date May 2016
 *
 * 
 */
#pragma once

#include "Measurement.h"
#include "text.h"
#include "MongoUtil.h"

/*!
 * \ingroup data
 * \class RegularMeasurement
 *
 * \brief Regular hydroclimate data, e.g., daily data
 *
 * 
 *
 */
class RegularMeasurement : public Measurement
{
public:
    /*!
     * \brief Constructor
     *
     * Initialize NotRegular Measurement instance from MongoDB
     *
     * \param[in] conn \a mongoc_client_t, MongoDB client
     * \param[in] hydroDBName \a string, HydroClimate database name
     * \param[in] sitesList \a string, site list
     * \param[in] siteType \a string, site type
     * \param[in] startDate \a time_t, start date time
     * \param[in] endDate \a time_t, end date time
     * \param[in] interval \a time_t, time interval
     */
    RegularMeasurement(mongoc_client_t *conn, string hydroDBName, string sitesList, string siteType, time_t startTime,
                       time_t endTime, time_t interval);

    //! Destructor
    ~RegularMeasurement(void);

    //! Get site date by time \a pData
    virtual float *GetSiteDataByTime(time_t t);

private:
    //! site data : the first dimension is time, and the second dimension is data by sites
    vector<float *> m_siteData;
    //! data record interval
    time_t m_interval;
};

