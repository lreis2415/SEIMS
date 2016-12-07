/*!
 * \brief Setting Inputs for SEIMS
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date June 2010
 */
#pragma once

#include "Settings.h"
#include <map>
#include "InputStation.h"
#include "Scenario.h"
#include "mongoc.h"
#include "MongoUtil.h"
#include <set>

using namespace MainBMP;

/*!
 * \ingroup module_setting
 * \class SettingsInput
 *
 * \brief Input settings for SEIMS
 *
 *
 *
 */
class SettingsInput : public Settings
{
public:
    //! Constructor
    SettingsInput(string fileName, mongoc_client_t *conn, string dbName, int nSubbasin = 1);

    //! Constructor, read from MongoDB
    SettingsInput(mongoc_client_t *conn, string dbName, int nSubbasin = 1);

    //! Destructor
    ~SettingsInput(void);

    //! Output to log file
    void Dump(string);

    //! Get start time of simulation
    time_t getStartTime(void) const;

    //! Get end time of simulation
    time_t getEndTime(void) const;

    //! Get time interval for hillslope scale processes
    time_t getDtHillslope(void) const;

    //! Get time interval for channel scale processes
    time_t getDtChannel(void) const;

    //! Get daily time interval of simulation in sec
    time_t getDtDaily() const;

    //! Get data of input HydroClimate stations
    InputStation *StationData();

private:
    //! Start date of simulation
    time_t m_startDate;
    //! End date of simulation
    time_t m_endDate;
    //! Time interval for hillslope scale processes
    time_t m_dtHs;
    //! Time interval for channel scale processes
    time_t m_dtCh;

    //! data of input HydroClimate stations
    InputStation *m_inputStation;

    //! Parameter database name
    string m_dbName;
    //! HydroClimate database name
    string m_dbHydro;
    //! MongoDB client
    mongoc_client_t *m_conn;
    //! HydroClimate site list <siteType, siteIDList>
    map<string, vector<int> > m_siteListMap;
    //! Subbasin ID
    int m_subbasinID;
    //! Simulation mode, can be DAILY or HOURLY
    string m_mode;
private:

    //void buildTimeQuery(time_t startTime, time_t endTime, bson_t* query);/// Deprecated
    //void buildQuery(const set<int>& idSet, const string& type, bson_t* query);/// Deprecated
    bool LoadSettingsFromFile(string, string);

    bool LoadSettingsInputFromMongoDB();

    //! Read start and end date, simulation mode and time interval
    bool readDate(void);

    ///bool readTimeSeriesData(void);///Deprecated
    //! Read HydroClimate site list
    void ReadSiteList();
};

