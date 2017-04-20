/*!
 * \brief Setting Inputs for SEIMS
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date June 2010
 */
#pragma once
#include "utilities.h"
#include "Settings.h"
#include "InputStation.h"
#include "Scenario.h"
#include "MongoUtil.h"

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
class SettingsInput : public Settings {
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
    time_t getDtDaily(void) const;

    //! Get data of input HydroClimate stations
    InputStation *StationData(void);

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
    bool LoadSettingsFromFile(string, string);

    bool LoadSettingsInputFromMongoDB(void);

    //! Read start and end date, simulation mode and time interval
    bool readDate(void);

    ///bool readTimeSeriesData(void);///Deprecated
    //! Read HydroClimate site list
    void ReadSiteList(void);
};

