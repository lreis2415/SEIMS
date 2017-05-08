/*!
 * \brief Constructor of ModuleFactory from config file
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 * \date May 2017
 * \revised LJ - Refactor
 */
#ifndef SEIMS_MODULE_FACTORY_H
#define SEIMS_MODULE_FACTORY_H

#include "tinyxml.h"

#include "seims.h"
#include "SEIMS_ModuleSetting.h"
#include "MetadataInfo.h"
#include "SimulationModule.h"
#include "ParamInfo.h"
#include "SpecifiedData.h"
#include "clsInterpolationWeightData.h"
#include "SettingsInput.h"
#include "DataCenter.h"

using namespace std;
using namespace MainBMP;

class ModuleFactory {
public:
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

    ModuleFactory(unique_ptr<DataCenter>& dcenter);
    //! Destructor
    ~ModuleFactory(void);

    //! Create a set of objects and set up the relationship among them. Return time-consuming.
    float CreateModuleList(vector<SimulationModule *> &modules);

    //! Update inputs, such climate data.
    void UpdateInput(vector<SimulationModule *> &modules, time_t t);

    //! Get value from dependency modules
    void GetValueFromDependencyModule(int iModule, vector<SimulationModule *> &modules);

    //! Find outputID parameter's module. Return Module index iModule and its ParamInfo
    void FindOutputParameter(string &outputID, int &iModule, ParamInfo *&paraInfo);

    //! Get Module ID by index
    string GetModuleID(int i) { return m_moduleIDs[i]; }

    //! Add mask raster to m_rsMap
    void AddMaskRaster(string, clsRasterData<float> *);

private:
    unique_ptr<DataCenter>  m_dataCenter;
    typedef SimulationModule *(*InstanceFunction)(void);
    typedef const char *(*MetadataFunction)(void);
    //! Modules' instance map
    map <string, InstanceFunction> m_instanceFuncs;
    //! Metadata map of modules
    map <string, MetadataFunction> m_metadataFuncs;
    //! Module path
    string m_modulePath;
    //! SubBasin ID
    int m_subBasinID;
    //! Layering method, can be UP_DOWN or DOWN_UP
    LayeringMethod m_layingMethod;
    //! Module IDs
    vector <string> m_moduleIDs;
#ifdef windows
    vector<HINSTANCE>                  m_dllHandles; ///< .DLL handles
#else
    vector<void *>                     m_dllHandles; ///< .so or .dylib handles
#endif
    //! Module settings, \a map<string, SEIMSModuleSetting*>
    map<string, SEIMSModuleSetting *>  m_settings;
    //! Metadata of modules
    map<string, const char *> m_metadata;
    //! Parameters of modules
    map<string, vector<ParamInfo>> m_moduleParameters;
    //! Input of modules
    map<string, vector<ParamInfo>> m_moduleInputs;
    //! Output of modules
    map<string, vector<ParamInfo>> m_moduleOutputs;
    //! Input settings, \sa SettingInput
    SettingsInput *m_setingsInput;
    ////! IP address of MongoDB
    //string m_host;
    ////! MongoDB port
    //int m_port;
    //! Database name of the simulation model
    string m_dbName;
//    //! BMPs Scenario database name
//    string m_dbScenario;
//    //! BMPs Scenario ID
//    int m_scenarioID;

    //! MongoDB Client
    MongoClient* m_conn;
    //! Mongo GridFS to store spatial data
    MongoGridFS* m_spatialData;
    //! Store parameters from Database \sa m_dbName
    map<string, ParamInfo *> m_parametersInDB;

    //! 1D array data map
    map<string, float *> m_1DArrayMap;
    //! 1D array data length map
    map<string, int> m_1DLenMap;
    //! 2D array data map
    map<string, float **> m_2DArrayMap;
    //! Row number of 2D array data map
    map<string, int> m_2DRowsLenMap;
    //! Col number of 2D array data map /// Since the first element of each row is the ColsLen, Is this m_2DColsLenMap necessary? By LJ.
    map<string, int> m_2DColsLenMap;
    //! Interpolation weight data map
    map<string, clsInterpolationWeightData *> m_weightDataMap;
    //! Mask data
    clsRasterData<float>* m_maskRaster; 
    //! Raster data (include 1D and/or 2D) map
    map<string, clsRasterData<float> *> m_rsMap;
    //! BMPs Scenario data
    Scenario* m_scenario;
    //! Reaches information
    clsReaches* m_reaches;
    //! Subbasins information
    clsSubbasins* m_subbasins;
    //! Climate input stations
    InputStation* m_climStation;

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
    SimulationModule *GetInstance(string &moduleID);

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

    ////! Query and get BMP scenario database name
    //void GetBMPScenarioDBName(void);

    //! Get comparable name after underscore if necessary, e.g., T_PET => use PET
    string GetComparableName(string &paraName);

    //! Find dependent parameters
    ParamInfo *FindDependentParam(ParamInfo &paramInfo);

    //! Read parameter settings from MongoDB
    void ReadParametersFromMongoDB(void);

    //! Set data for modules, include all datatype
    void SetData(string &dbName, int nSubbasin, SEIMSModuleSetting *setting, ParamInfo *param,
                 clsRasterData<float> *templateRaster, SimulationModule *pModule, bool vertitalItp);

    //! Set single Value
    void SetValue(ParamInfo *param, clsRasterData<float> *templateRaster, SimulationModule *pModule);

    //! Set 1D Data
    void Set1DData(string &dbName, string &paraName, string &remoteFileName, clsRasterData<float> *templateRaster,
                   SimulationModule *pModule, bool vertitalItp);

    //! Set 2D Data
    void Set2DData(string &dbName, string &paraName, int nSubbasin, string &remoteFileName,
                   clsRasterData<float> *templateRaster, SimulationModule *pModule);

    //! Set raster data
    void SetRaster(string &dbName, string &paraName, string &remoteFileName, clsRasterData<float> *templateRaster,
                   SimulationModule *pModule);

    //! Set BMPs Scenario data
    void SetScenario(SimulationModule *pModule);

    //! Set Reaches information
    void SetReaches(SimulationModule *pModule);

    //! Set Subbasins information
    void SetSubbasins(SimulationModule *pModule);

    //! Read multiply reach information from file
    //void ReadMultiReachInfo(const string &filename, LayeringMethod layeringMethod, int& nRows, int& nCols, float**& data);
    //! Read single reach information
    //void ReadSingleReachInfo(int nSubbasin, const string &filename, LayeringMethod layeringMethod, int& nAttr, int& nReaches, float**& data);
};
#endif /* SEIMS_MODULE_FACTORY_H */
