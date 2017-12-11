/*!
 * \brief Constructor of ModuleFactory from config file
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date May 2017
 * \revised LJ - Refactor and DeCoupling with Database I/O
 */
#ifndef SEIMS_MODULE_FACTORY_H
#define SEIMS_MODULE_FACTORY_H

#include "seims.h"
#include "SEIMS_ModuleSetting.h"
#include "MetadataInfo.h"
#include "SimulationModule.h"
#include "ParamInfo.h"
#include "clsInterpolationWeightData.h"
#include "SettingsInput.h"
#include "DataCenter.h"

#include "tinyxml.h"


using namespace std;
using namespace MainBMP;

class ModuleFactory {
public:
    /*!
     * \brief Constructor of ModuleFactory from \sa DataCenter
     * \param[in] dcenter
     */
    explicit ModuleFactory(DataCenterMongoDB* dcenter);
    //! Destructor
    ~ModuleFactory();

    //! Create a set of objects and set up the relationship among them. Return time-consuming.
    float CreateModuleList(vector<SimulationModule *>& modules);

    //! Update inputs, such climate data.
    void UpdateInput(vector<SimulationModule *>& modules, time_t t);

    //! Get value from dependency modules
    void GetValueFromDependencyModule(int iModule, vector<SimulationModule *>& modules);

    //! Find outputID parameter's module. Return Module index iModule and its ParamInfo
    void FindOutputParameter(string& outputID, int& iModule, ParamInfo*& paraInfo);

    //! Get Module ID by index
    string GetModuleID(int i) { return m_moduleIDs[i]; }

    /*!
     *\brief Update model parameters (value, 1D raster, and 2D raster, etc.) by Scenario, e.g., areal BMPs.
     * \sa BMPArealStructFactory, and \sa BMPArealStruct
     */
    void updateParametersByScenario(int subbsnID);

private:
    //! Initialization, read the config.fig file and initialize
    void Init(const string &configFileName);

    //! Load modules setting from file
    bool LoadSettingsFromFile(const char *filename, vector<vector<string> > &settings);

    //! Read configuration file
    void ReadConfigFile(const char *configFileName);

    //! Load function pointers from .DLL or .so
    void ReadDLL(string &moduleID, string &dllID);

    //! Get module instance by moduleID
    SimulationModule *GetInstance(string &moduleID) { return m_instanceFuncs[moduleID](); }

    //! Match data type, e.g., 1D array
    dimensionTypes MatchType(string strType);

    //! Is constant input?
    bool IsConstantInputFromName(string &name);

    //! Read module's parameters setting from XML string
    void ReadParameterSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting);

    //! Read module's input setting from XML string
    void ReadInputSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting);

    //! Read module's output setting from XML string
    void ReadOutputSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting);

    //! Get comparable name after underscore if necessary, e.g., T_PET => use PET
    string GetComparableName(string &paraName);

    //! Find dependent parameters
    ParamInfo* FindDependentParam(ParamInfo &paramInfo);

    //! Set data for modules, include all datatype
    void SetData(string &dbName, int nSubbasin, SEIMSModuleSetting *setting, ParamInfo *param,
                 FloatRaster *templateRaster, SimulationModule *pModule, bool vertitalItp);

    //! Set single Value
    void SetValue(ParamInfo *param, FloatRaster *templateRaster, SimulationModule *pModule);

    //! Set 1D Data
    void Set1DData(string &dbName, string &paraName, string &remoteFileName, FloatRaster *templateRaster,
                   SimulationModule *pModule, bool vertitalItp);

    //! Set 2D Data
    void Set2DData(string &dbName, string &paraName, int nSubbasin, string &remoteFileName,
                   FloatRaster *templateRaster, SimulationModule *pModule);

    //! Set raster data
    void SetRaster(string &dbName, string &paraName, string &remoteFileName, FloatRaster *templateRaster,
                   SimulationModule *pModule);

    //! Set BMPs Scenario data
    void SetScenario(SimulationModule *pModule);

    //! Set Reaches information
    void SetReaches(SimulationModule *pModule);

    //! Set Subbasins information
    void SetSubbasins(SimulationModule *pModule);

private:
    //! input parameter, DataCenter
    DataCenterMongoDB*                 m_dataCenter;
private:
    /************************************************************************/
    /*           Derived parameters                                         */
    /************************************************************************/
    //! Database name of the simulation model
    string                             m_dbName;
    //! Module configuration file
    string                              m_moduleCfgFile;
    //! Module path, without SEP(/ or \) at the end
    string                              m_modulePath;
    //! SubBasin ID
    int                                 m_subBasinID;
    //! Layering method, can only be UP_DOWN or DOWN_UP
    LayeringMethod                      m_layingMethod;
    //! Initial parameters from Database
    map<string, ParamInfo *>&           m_parametersInDB;
    //! Input settings, \sa SettingInput
    SettingsInput*                      m_setingsInput;
    //! BMPs Scenario data
    Scenario*                           m_scenario;
    //! Reaches information
    clsReaches*                         m_reaches;
    //! Subbasins information
    clsSubbasins*                       m_subbasins;
    //! Climate input stations
    InputStation*                       m_climStation;
    //! Mask data
    clsRasterData<float>*               m_maskRaster;
    //! Raster data (include 1D and/or 2D) map
    map<string, clsRasterData<float>*>& m_rsMap;
    //! 1D array data map, e.g. FLOWOUT_INDEX_D8
    map<string, float *>&               m_1DArrayMap;
    //! 1D array data length map
    map<string, int>&                   m_1DLenMap;
    //! 2D array data map, e.g. ROUTING_LAYERS
    map<string, float **>&              m_2DArrayMap;
    //! Row number of 2D array data map
    map<string, int>&                   m_2DRowsLenMap;
    //! Col number of 2D array data map, CAUTION that nCols may not same for all rows
    map<string, int>&                   m_2DColsLenMap;
    //! Interpolation weight data map
    map<string, clsITPWeightData *>&    m_weightDataMap;
private:
    /************************************************************************/
    /*           Parameters created during constructor                      */
    /************************************************************************/
    //! Simulation module instance
    typedef SimulationModule *(*InstanceFunction)();
    //! Simulation module metadata
    typedef const char *(*MetadataFunction)();
    //! Module IDs
    vector<string>                     m_moduleIDs;
    //! instance map of modules
    map<string, InstanceFunction>      m_instanceFuncs;
    //! Metadata map of modules
    map<string, MetadataFunction>      m_metadataFuncs;
    //! dynamic library handles (.dll in Windows, .so in Linux, and .dylib in macOS)
#ifdef WIN32
    vector<HINSTANCE>                  m_dllHandles;
#else
    vector<void *>                     m_dllHandles;
#endif
    //! Module settings
    map<string, SEIMSModuleSetting *>  m_settings;
    //! Metadata of modules
    map<string, const char *>          m_metadata;
    //! Parameters of modules, from \sa m_parametersInDB
    map<string, vector<ParamInfo> >    m_moduleParameters;
    //! Input of modules, from other modules
    map<string, vector<ParamInfo> >    m_moduleInputs;
    //! Output of modules, out from current module
    map<string, vector<ParamInfo> >    m_moduleOutputs;
};
#endif /* SEIMS_MODULE_FACTORY_H */


///*!
// * \brief Constructor of ModuleFactory from config file
// * By default, the layering method is UP_DOWN
// * \param[in] configFileName config.fig file which contains modules list for Simulation
// * \param[in] modelPath the path of your model
// * \param[in] conn \a mongoc_client_t
// * \param[in] dbName name of your model
// * \param[in] subBasinID subBasinID
// * \param[in] scenarioID
// */
//ModuleFactory(string &configFileName, string &modelPath, mongoc_client_t *conn, string &dbName,
//    int subBasinID, LayeringMethod layingMethod, int scenarioID, SettingsInput*& input);

//! Read multiply reach information from file
//void ReadMultiReachInfo(const string &filename, LayeringMethod layeringMethod, int& nRows, int& nCols, float**& data);
//! Read single reach information
//void ReadSingleReachInfo(int nSubbasin, const string &filename, LayeringMethod layeringMethod, int& nAttr, int& nReaches, float**& data);