/*!
 * \file SettingsOutput.h
 * \brief Setting Outputs for SEIMS
 *
 * Changelog:
 *   - 1. 2017-05-20 - lj - Refactor, decoupling with database IO.
 *
 * \author Junzhi Liu, Liangjun Zhu
 * \version 2.0
 */
#ifndef SEIMS_SETTING_OUTPUT_H
#define SEIMS_SETTING_OUTPUT_H

#include "Settings.h"
#include "PrintInfo.h"

/*!
 * \ingroup data
 * \struct OrgOutItem
 * \brief Original output item
 */
struct OrgOutItem {
    OrgOutItem() : modCls(""), outputID(""), descprition(""), outFileName(""),
                   aggType(""), unit(""), subBsn(""), intervalUnit(""),
                   sTimet(0), eTimet(0), interval(-1), use(-1) {
    }

    string modCls;
    string outputID;
    string descprition;
    string outFileName;
    string aggType;
    string unit;
    string subBsn;
    string intervalUnit;
    time_t sTimet;
    time_t eTimet;
    int interval;
    int use;
};

/*!
 * \ingroup data
 * \class SettingsOutput
 * \brief Setting outputs
 * \sa Settings
 */
class SettingsOutput: public Settings {
public:
    /*!
     * \brief Constructor
     * \param[in] subbasinNum Subbasin number of the entire watershed
     * \param[in] outletID The subbasin ID of outlet
     * \param[in] subbasinID Current subbasin ID, 0 for OMP version
     * \param[in] outputItems Vector of original output items read from FILE_OUT file (or table)
     * \param[in] scenarioID Scenario ID, -1 means no scenario is used, default is 0
     * \param[in] calibrationID Calibration ID, -1 means no calibration from calibration sequence is used
     * \param[in] mpi_rank Rank ID for MPI, 0 is the default, also for OMP version
     * \param[in] mpi_size Rank size for MPI, -1 is the default for compatible with OMP version
     */
    SettingsOutput(int subbasinNum, int outletID, int subbasinID, vector<OrgOutItem>& outputItems,
                   int scenarioID = 0, int calibrationID = -1,
                   int mpi_rank = 0, int mpi_size = -1);

    //! Destructor
    ~SettingsOutput();

    //! Init function
    static SettingsOutput* Init(int subbasinNum, int outletID, int subbasinID,
                                vector<OrgOutItem>& outputItems,
                                int scenarioID = 0, int calibrationID = -1,
                                int mpi_rank = 0, int mpi_size = -1);

    //! Write output information to log file
    void Dump(const string& filename) OVERRIDE;

    // This function has been deprecated and replaced by DataCenter::UpdateOutputDate(). -LJ.
    //! Check date of output settings
    //void checkDate(time_t, time_t);

public:
    //! All the print settings
    vector<PrintInfo *> m_printInfos;
    /*!
     * \brief All the output settings
     * key: OutputID
     * value: PrintInfo instance
     * \sa PrintInfo
     */
    map<string, PrintInfo *> m_printInfosMap;

private:
    //! number of subbasins
    int m_nSubbasins;
    //! subbasin ID which outlet located
    int m_outletID;
    //! current subbasin ID, 0 for OMP version
    int m_subbasinID;
    //! Scenario ID, -1 means no scenario is used
    int m_scenarioID;
    //! Calibration ID, -1 means no calibration from calibration sequence is used
    int m_calibrationID;
    //! Rank ID for MPI, starts from 0 to mpi_size_ - 1
    int m_mpi_rank;
    //! Rank size for MPI
    int m_mpi_size;
};
#endif /* SEIMS_SETTING_OUTPUT_H */
