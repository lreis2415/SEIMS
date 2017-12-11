/*!
 * \brief Not Regular hydroclimate data, i.e., for storm model
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date May 2016
 * \revised LJ - Replace mongoc_client_t by MongoClient interface
 */
#ifndef SEIMS_NOTREGULAR_MEASUREMENT_H
#define SEIMS_NOTREGULAR_MEASUREMENT_H

#include "Measurement.h"
#include "text.h"
#include "utilities.h"
#include "MongoUtil.h"

/*!
 * \ingroup data
 * \class NotRegularMeasurement
 * \brief Not Regular hydroclimate data
 */
class NotRegularMeasurement : public Measurement {
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
     */
    NotRegularMeasurement(MongoClient *conn, string& hydroDBName, string& sitesList, string& siteType,
                          time_t startTime, time_t endTime);

    //! Destructor
    virtual ~NotRegularMeasurement() = default;

    //! Get site date by time \a pData
    virtual float *GetSiteDataByTime(time_t t);

private:
    //! time list of site data
    vector<vector<time_t>> m_timeList;
    //! site data corresponding to m_timeList
    vector<vector<float>> m_valueList;
    //! index
    vector<int> m_curIndexList;
};
#endif /* SEIMS_NOTREGULAR_MEASUREMENT_H */
