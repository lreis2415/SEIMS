/*!
 * \brief Setting Inputs for SEIMS
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date May 2017
 * \revised LJ - Decoupling with Database IO
 */
#ifndef SEIMS_SETTING_INPUT_H
#define SEIMS_SETTING_INPUT_H

#include "Settings.h"
#include "utilities.h"

#include "InputStation.h"
#include "Scenario.h"
#include "MongoUtil.h"

using namespace MainBMP;

/*!
 * \ingroup module_setting
 * \class SettingsInput
 * \brief Input settings for SEIMS
 */
class SettingsInput : public Settings {
public:
    ////! Constructor
    //SettingsInput(string fileName, mongoc_client_t *conn, string dbName, int nSubbasin = 0);

    ////! Constructor, read from MongoDB
    //SettingsInput(mongoc_client_t *conn, string dbName, int nSubbasin = 0);

    //! Constructor
    SettingsInput(vector<string>& stringvector);

    //! Destructor
    ~SettingsInput(void);

    //! Output to log file
    virtual void Dump(string);

    //! Get start time of simulation
    time_t getStartTime(void) const { return m_startDate; }

    //! Get end time of simulation
    time_t getEndTime(void) const { return m_endDate; }

    //! Get time interval for hillslope scale processes
    time_t getDtHillslope(void) const { return m_dtHs; }

    //! Get time interval for channel scale processes
    time_t getDtChannel(void) const { return m_dtCh; }

    //! Get daily time interval of simulation in sec
    time_t getDtDaily(void) const { return 86400; }

    //! Get model mode
    string& getModelMode(void) { return m_mode; }

    //! is storm model
    bool isStormMode(void) const { return m_isStormModel; }

    //! Get data of input HydroClimate stations
    //InputStation* StationData(void) { return m_inputStation; }

private:
    //bool LoadSettingsFromFile(string, string);

    //bool LoadSettingsInputFromMongoDB(void);

    //! Read start and end date, simulation mode and time interval
    bool readSimulationPeriodDate(void);

    ///bool readTimeSeriesData(void);///Deprecated
    //! Read HydroClimate site list, move to DataCenter
    //void ReadSiteList(void);
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
    //InputStation *m_inputStation;

    //! Parameter database name
    //string m_dbName;
    //! HydroClimate database name
    //string m_dbHydro;
    //! MongoDB client
    //mongoc_client_t *m_conn;
    //! HydroClimate site list <siteType, siteIDList>
    //map<string, vector<int> > m_siteListMap;
    //! Subbasin ID
    //int m_subbasinID;
    //! Simulation mode, can be DAILY or HOURLY
    string m_mode;
    //! is storm model?
    bool m_isStormModel;
};
#endif /* SEIMS_SETTING_INPUT_H */
