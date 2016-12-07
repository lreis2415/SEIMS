/*!
 * \brief Not Regular hydroclimate data
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date May 2016
 *
 * 
 */
#pragma once

#include "Measurement.h"
#include "MongoUtil.h"
#include <map>
#include "text.h"

/*!
 * \ingroup data
 * \class NotRegularMeasurement
 *
 * \brief Not Regular hydroclimate data
 *
 *
 *
 */
class NotRegularMeasurement : public Measurement
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
     */
    NotRegularMeasurement(mongoc_client_t *conn, string hydroDBName, string sitesList, string siteType,
                          time_t startTime, time_t endTime);

    //! Destructor
    ~NotRegularMeasurement(void);

    //! Get site date by time \a pData
    virtual float *GetSiteDataByTime(time_t t);

private:
    //! time list of site data
    vector<vector<time_t> > m_timeList;
    //! site data corresponding to m_timeList
    vector<vector<float> > m_valueList;
    //! index
    vector<int> m_curIndexList;
};

