/*!
 * \brief Control the simulation of SEIMS
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date May 2017
 * \revised LJ - Refactoring, May 2017
 *               The ModelMain class mainly focuses on the entire workflow.
 */

#ifndef SEIMS_MODEL_MAIN_H
#define SEIMS_MODEL_MAIN_H

/// include utility classes and const definition of SEIMS
#include "seims.h"

/// include module_setting related
#include "SettingsInput.h"
#include "SettingsOutput.h"
#include "PrintInfo.h"
#include "ModuleFactory.h"

/// include data related
#include "DataCenter.h"
#include "ClimateParams.h"

#include "MongoUtil.h"
#include "clsRasterData.h"

/// include build-in libs
#include <string>
#include <set>
#include <map>
#include <ctime>
#include <sstream>
#include <memory>

using namespace std;

/*!
 * \class ModelMain
 * \ingroup seims_omp
 * \brief SEIMS OpenMP version, Class to control the whole model
 */
class ModelMain {
public:
    /*!
     * \brief Constructor independent to any database IO, instead of the \sa DataCenter object
     * \param[in] dcenter \sa DataCenter, \sa DataCenterMongoDB, or others in future
     * \param[in] mfactory \sa ModuleFactory, assemble the module workspace
     */
    ModelMain(DataCenterMongoDB* dcenter, ModuleFactory* mfactory);
    //! Destructor
    ~ModelMain();
    //! Execute all the modules, aggregate output data, and write the total time-consuming, etc.
    void Execute();
    //! Write output files, e.g., Q.txt
    void Output();

    /*!
    * \brief Check whether the validation of outputs
    * 1. The output id should be valid for modules in config files;
    * 2. The date range should be in the data range of file.in;
    */
    void CheckAvailableOutput();
    /*!
     * \brief Append output data to Output Item by the corresponding aggregation type
     * \param[in] time Current simulation time
     */
    void AppendOutputData(time_t time);
    /*!
     * \brief Print execution time on the screen
     */
    void OutputExecuteTime();
    /*!
     * \brief Execute hillslope modules in current time
     * \param[in] t Current time
     * \param[in] yearIdx Year index of the entire simulation period
     * \param[in] subIndex Time step index of the entire simulation period
     */
    void StepHillSlope(time_t t, int yearIdx, int subIndex);
    /*!
     * \brief Execute channel modules in current time
     * \param[in] t Current time
     * \param[in] yearIdx Year index of the entire simulation period
     */
    void StepChannel(time_t t, int yearIdx);
    /*!
     * \brief Execute overall modules in the entire simulation period, e.g., COST module.
     * \param[in] startT Start time period
     * \param[in] endT End time period
     */
    void StepOverall(time_t startT, time_t endT);
    
    // temporary debug code
    float GetQOutlet() { return 0.f; }
    void SetChannelFlowIn(float value) {
        m_simulationModules[m_channelModules[0]]->SetValue(VAR_QUPREACH, value);
    }
    // temporary debug code

public:
    /************************************************************************/
    /*            Get functions for MPI version                             */
    /************************************************************************/

    //! Get module counts of current SEIMS
    int GetModuleCount() const { return (int) m_simulationModules.size(); }
    //! Get module ID by index in ModuleFactory
    string GetModuleID(int i) const { return m_factory->GetModuleID(i); }
    //! Get module execute time by index in ModuleFactory
    float GetModuleExecuteTime(int i) const { return (float) m_executeTime[i]; }
    //! Get time consuming of read data
    float GetReadDataTime() const { return m_readFileTime; }
    //! Include channel processes or not?
    bool IncludeChannelProcesses() { return !m_channelModules.empty(); }

private:
    /************************************************************************/
    /*             Input parameters                                         */
    /************************************************************************/

    DataCenterMongoDB*          m_dataCenter;         ///< inherited DataCenter
    ModuleFactory*              m_factory;            ///< Modules factory
private:
    /************************************************************************/
    /*   Pointer or reference of object and data derived from input params  */
    /************************************************************************/

    SettingsInput*              m_input;              ///< The basic input settings
    SettingsOutput*             m_output;             ///< The user-defined outputs, Q, SED, etc
    FloatRaster*                m_maskRaster;         ///< Mask raster data
    string                      m_outputPath;         ///< Path of output scenario
    time_t                      m_dtDaily;            ///< Daily time interval, seconds
    time_t                      m_dtHs;               ///< Hillslope time interval, seconds
    time_t                      m_dtCh;               ///< Channel time interval, seconds
private:
    /************************************************************************/
    /*   Variables newly allocated in this class                            */
    /************************************************************************/

    float                       m_readFileTime;       ///< Time consuming for read data
    vector<SimulationModule *>  m_simulationModules;  ///< Modules list in the model run
    vector<int>                 m_hillslopeModules;   ///< Hillslope modules index list
    vector<int>                 m_channelModules;     ///< Channel modules index list
    vector<int>                 m_ecoModules;         ///< Ecology modules index list
    vector<int>                 m_overallModules;     ///< Whole simulation scale modules index list
    vector<double>              m_executeTime;        ///< Execute time list of each module
   
    bool                        m_firstRunOverland;   ///< Is the first run of overland
    bool                        m_firstRunChannel;    ///< Is the first run of channel
};
#endif /* SEIMS_MODEL_MAIN_H */
