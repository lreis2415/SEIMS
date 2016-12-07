/*!
 * \brief Control the simulation of SEIMS
 * \author Junzhi Liu, LiangJun Zhu
 * \version 1.1
 * \date May 2016
 *
 * 
 */
#pragma once

#include <string>
#include <set>
#include <map>
#include "SettingsInput.h"
#include "SettingsOutput.h"
#include "mongoc.h"
#include "ModuleFactory.h"
#include "utils.h"
#include "clsRasterData.cpp"

using namespace std;

/*!
 * \class ModelMain
 * \ingroup seims_omp
 *
 * \brief SEIMS OpenMP version, Class to control the whole model
 *
 */
class ModelMain
{
public:
    /*!
     * \brief Constructor
     *
     * \param[in] conn \a mongoc_client_t, MongoDB client
     * \param[in] dbName model name, e.g., model_dianbu30m_longterm
     * \param[in] projectPath Path of the project, contains cofig.fig, file.in and file.out
     * \param[in] input \sa SettingInput, input setting information
     * \param[in] factory \sa ModuleFactory, module factory instance
     * \param[in] subBasinID subBasinID, default is 1
     * \param[in] scenarioID Scenario ID, default is 0
     * \param[in] numThread Thread number for OpenMP, default is 1
     * \param[in] layeringMethod Layering method, default is UP_DOWN
     */
    ModelMain(mongoc_client_t *conn, string dbName, string projectPath, SettingsInput *input,
              ModuleFactory *factory, int subBasinID = 1, int scenarioID = 0, int numThread = 1,
              LayeringMethod layeringMethod = UP_DOWN);
    //! Destructor
    ~ModelMain(void);

    //! Destroy the GridFS instance
    void CloseGridFS() { mongoc_gridfs_destroy(m_outputGfs); }

    //! Execute all the modules, create output, and write the total time-consuming.
    void Execute();

    //! Get hillslope time interval
    time_t getDtHillSlope() { return m_dtHs; }

    //! Get channel time interval
    time_t getDtChannel() { return m_dtCh; }

    //! Get daily time interval
    time_t getDtDaily() { return m_dtDaily; }

    //! Get start time of simulation
    time_t getStartTime() { return m_input->getStartTime(); }

    //! Get end time of simulation
    time_t getEndTime() { return m_input->getEndTime(); }

    //! Get module counts of current SEIMS
    int GetModuleCount() { return m_simulationModules.size(); }

    //! Get module ID by index in ModuleFactory
    string GetModuleID(int i) { return m_factory->GetModuleID(i); }

    //! Get module execute time by index in ModuleFactory
    float GetModuleExecuteTime(int i) { return (float)m_executeTime[i]; }

    //! Get time consuming of read files
    float GetReadDataTime() { return m_readFileTime; }

	//! 
    float GetQOutlet();

    //! Include channel processes or not?
    bool IncludeChannelProcesses() { return m_channelModules.size() != 0; }

    //void	Init(SettingsInput* input, int numThread);
    //! Has the model been initialized?
    bool IsInitialized() { return m_initialized; }

    //! Creating output files, e.g., QOutlet.txt
    void Output();

    //! Get output at the given time
    void Output(time_t time);

    void OutputExecuteTime();

    //! Set layering method
    void SetLayeringMethod(LayeringMethod method) { m_layeringMethod = method; }

    //! Execute channel modules in current step
    void StepChannel(time_t t, int yearIdx);

    //! Execute hillslope modules in current step
    void StepHillSlope(time_t t, int yearIdx, int subIndex);

    //! Execute overall modules in the entire simulation period, e.g., COST module.
    void StepOverall(time_t startT, time_t endT);
    //! Set Flow In Channel data for Channel-related module, e.g., CH_DW
    void SetChannelFlowIn(float value);
    //! Check module input data, date and execute module
    ///void	Step(time_t time);/// Deprecated. LJ
    //! Execute overland modules in current step
    ///void	StepOverland(time_t t);/// Deprecated. LJ
private:
    //! MongoDB Client
    mongoc_client_t *m_conn;
    //! output GridFS to store spatial data etc.
    mongoc_gridfs_t *m_outputGfs;
    //! Model name
    string m_dbName;
    //! Scenario ID
    int m_scenarioID;
    //! thread number for omp
    int m_threadNum;
    //! SubBasin ID
    int m_subBasinID;
    //! Parameters information map
    map<string, ParamInfo *> m_parameters;
    //! Modules factory
    ModuleFactory *m_factory;
    //! Modules list in current model run
    vector<SimulationModule *> m_simulationModules;
    //! Hillslope modules index list
    vector<int> m_hillslopeModules;
    //! Channel modules index list
    vector<int> m_channelModules;
    //! Ecology modules index list
    vector<int> m_ecoModules;
    //! Whole simulation scale modules index list
    vector<int> m_overallModules;
    //! Execute time list of each module
    vector<double> m_executeTime;
    //! Time consuming for read files
    float m_readFileTime;
    //! Layering method
    LayeringMethod m_layeringMethod;


    /*!
     * \brief Check whether the output file is valid
     * \TODO NEED TO BE UPDATED
     * 1. The output id should be valid for moduls in config files;
     * 2. The date range should be in the data range of file.in;
     * The method should be called after config, input and output is initialed.
     *
     * \param[in] gfs \a mongoc_gridfs_t
     */
    void CheckOutput(mongoc_gridfs_t *gfs);

    /*!
     * \brief Check whether the output file is valid
     * \TODO NEED TO BE UPDATED
     * 1. The output id should be valid for moduls in config files;
     * 2. The date range should be in the data range of file.in;
     * The method should be called after config, input and output is initialed.
     *
     * \param[in] gfs \a mongoc_gridfs_t
     */
    void CheckOutput();
	
    //! Check module input data, date and execute module
    void Step(time_t t, int yearIdx, vector<int> &moduleIndex, bool firstRun);

private:
    //! Is the firt run of SEIMS
    bool m_firstRun;
    //! Is the first run of overland
    bool m_firstRunOverland;
    //! Is the first run of channel
    bool m_firstRunChannel;
    //! Has the model been initialized
    bool m_initialized;
    //! Path of the project
    string m_projectPath;
    //! Path of database
    string m_databasePath;
    //! Path of the model
    string m_modulePath;
    //! The input setting of the model
    SettingsInput *m_input;
    //! The output setting of the model
    SettingsOutput *m_output;
    //! Template raster data
    clsRasterData<float> *m_templateRasterData;
    //! Daily time interval
    time_t m_dtDaily;
    //! Hillslope time interval
    time_t m_dtHs;
    //! Channel time interval
    time_t m_dtCh;
};

