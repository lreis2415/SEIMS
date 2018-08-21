/*!
 * \file SettingsInput.h
 * \brief Setting Inputs for SEIMS
 *
 * Changelog:
 *   - 1. 2017-05-30 - lj - Decoupling with Database IO.
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 */
#ifndef SEIMS_SETTING_INPUT_H
#define SEIMS_SETTING_INPUT_H

#include "Settings.h"
#include "Scenario.h"

using namespace bmps;

/*!
 * \ingroup data
 * \class SettingsInput
 * \brief Input settings for SEIMS
 */
class SettingsInput: public Settings {
public:
    //! Constructor
    explicit SettingsInput(vector<string>& stringvector);

    static SettingsInput* Init(vector<string>& stringvector);

    //! Output to log file
    void Dump(const string& filename) OVERRIDE;

    //! Get start time of simulation
    time_t getStartTime() const { return m_startDate; }

    //! Get end time of simulation
    time_t getEndTime() const { return m_endDate; }

    //! Get time interval for hillslope scale processes
    time_t getDtHillslope() const { return m_dtHs; }

    //! Get time interval for channel scale processes
    time_t getDtChannel() const { return m_dtCh; }

    //! Get daily time interval of simulation in sec
    time_t getDtDaily() const { return 86400; }

    //! Get model mode
    string& getModelMode() { return m_mode; }

    //! is storm model
    bool isStormMode() const { return m_isStormModel; }

private:
    //! Read start and end date, simulation mode and time interval
    bool readSimulationPeriodDate();

private:
    //! Start date of simulation
    time_t m_startDate;
    //! End date of simulation
    time_t m_endDate;
    //! Time interval for hillslope scale processes
    time_t m_dtHs;
    //! Time interval for channel scale processes
    time_t m_dtCh;
    //! Simulation mode, can be DAILY or HOURLY
    string m_mode;
    //! is storm model?
    bool m_isStormModel;
};
#endif /* SEIMS_SETTING_INPUT_H */
