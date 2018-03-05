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
#include "invoke.h"
#include "SEIMS_ModuleSetting.h"
#include "MetadataInfo.h"
#include "SimulationModule.h"
#include "ParamInfo.h"
#include "clsInterpolationWeightData.h"
#include "SettingsInput.h"

#include "tinyxml.h"

#ifdef _DEBUG
#define POSTFIX "d"
#endif
#ifdef RELWITHDEBINFO
#define POSTFIX "rd"
#endif
#ifdef MINSIZEREL
#define POSTFIX "s"
#endif
#ifndef POSTFIX
#define POSTFIX ""
#endif

#ifdef WIN32
#define DLLINSTANCE HINSTANCE
#else
#define DLLINSTANCE void*
#endif
//! Simulation module instance
typedef SimulationModule *(*InstanceFunction)();
//! Simulation module metadata
typedef const char *(*MetadataFunction)();

using namespace std;
using namespace MainBMP;

class ModuleFactory {
public:
    /*!
     * \brief Constructor of ModuleFactory from \sa DataCenter
     * \param[in] dcenter
     */
    //explicit ModuleFactory(DataCenterMongoDB* dcenter);
    /*!
    * \brief Constructor
    */
    ModuleFactory(const string &model_name,
                  vector<string> &moduleIDs,
                  map<string, SEIMSModuleSetting *> &moduleSettings,
                  vector<DLLINSTANCE> &dllHandles,
                  map<string, InstanceFunction> &instanceFuncs,
                  map<string, MetadataFunction> &metadataFuncs,
                  map<string, const char *> &moduleMetadata,
                  map<string, vector<ParamInfo *> > &moduleParameters,
                  map<string, vector<ParamInfo *> > &moduleInputs,
                  map<string, vector<ParamInfo *> > &moduleOutputs) :
        m_dbName(model_name),
        m_moduleIDs(moduleIDs), m_settings(moduleSettings), m_dllHandles(dllHandles),
        m_instanceFuncs(instanceFuncs), m_metadataFuncs(metadataFuncs),
        m_metadata(moduleMetadata), m_moduleParameters(moduleParameters),
        m_moduleInputs(moduleInputs), m_moduleOutputs(moduleOutputs) {}
    /*!
     * \brief Initialization for exception-safe constructor
     */
    static ModuleFactory *Init(const string &module_path, InputArgs *input_args);
    //! Destructor
    ~ModuleFactory();

    //! Create a set of objects and set up the relationship among them. Return time-consuming.
    void CreateModuleList(vector<SimulationModule *> &modules, int nthread = 1);

    //! Get value from dependency modules
    void GetValueFromDependencyModule(int iModule, vector<SimulationModule *> &modules);

    //! Find outputID parameter's module. Return Module index iModule and its ParamInfo
    void FindOutputParameter(string &outputID, int &iModule, ParamInfo *&paraInfo);

    //! Get Module ID by index
    string GetModuleID(int i) const { return m_moduleIDs[i]; }

    //! Get unique module IDs
    vector<string> GetModuleIDs() const { return m_moduleIDs; }

    //! Get map of module settings
    map<string, SEIMSModuleSetting *> &GetModuleSettings() { return m_settings; }
    //! Get Metadata of modules
    map<string, const char *> &GetModuleMetadata() { return m_metadata; }
    //! Get Parameters of modules
    map<string, vector<ParamInfo *> > &GetModuleParameters() { return m_moduleParameters; }
    //! Get Input of modules, from other modules
    map<string, vector<ParamInfo *> > &GetModuleInputs() { return m_moduleInputs; }
    //! Get Output of modules, out from current module
    map<string, vector<ParamInfo *> > &GetModuleOutputs() { return m_moduleOutputs; }

    //! Load modules setting from file
    static bool LoadSettingsFromFile(const char *filename, vector<vector<string> > &settings);

    /*!
     * \brief Read configuration file
     * \param[in] configFileName Configuration full file path
     * \param[out] moduleIDs Unique module IDs (name)
     * \param[out] moduleSettings Map of \sa SEIMSModuleSetting
     * \return True if succeed.
     */
    static bool ReadConfigFile(const char *configFileName, vector<string> &moduleIDs,
                               map<string, SEIMSModuleSetting *> &moduleSettings);

    /*!
     * \brief Load and parse module libraries
     * \param module_path
     * \param moduleIDs
     * \param dllHandles
     * \param instanceFuncs
     * \param metadataFuncs
     * \return True if succeed, else throw exception and return false.
     */
    static bool LoadParseLibrary(const string &module_path, vector<string> &moduleIDs,
                                 map<string, SEIMSModuleSetting *> &moduleSettings,
                                 vector<DLLINSTANCE> &dllHandles,
                                 map<string, InstanceFunction> &instanceFuncs,
                                 map<string, MetadataFunction> &metadataFuncs,
                                 map<string, const char *> &moduleMetadata,
                                 map<string, vector<ParamInfo *> > &moduleParameters,
                                 map<string, vector<ParamInfo *> > &moduleInputs,
                                 map<string, vector<ParamInfo *> > &moduleOutputs);
    //! Load function pointers from .DLL or .so
    static void ReadDLL(const string &module_path, const string &id, const string &dllID,
                        vector<DLLINSTANCE> &dllHandles,
                        map<string, InstanceFunction> &instanceFuncs,
                        map<string, MetadataFunction> &metadataFuncs);

    //! Get module instance by moduleID
    SimulationModule *GetInstance(string &moduleID) { return m_instanceFuncs[moduleID](); }

    //! Match data type, e.g., 1D array
    static dimensionTypes MatchType(string strType);

    //! Is constant input?
    static bool IsConstantInputFromName(string &name);

    //! Read module's parameters setting from XML string
    static void ReadParameterSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting,
                                     map<string, vector<ParamInfo *> > &moduleParameters);

    //! Read module's input setting from XML string
    static void ReadInputSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting,
                                 map<string, vector<ParamInfo *> > &moduleInputs);

    //! Read module's output setting from XML string
    static void ReadOutputSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting,
                                  map<string, vector<ParamInfo *> > &moduleOutputs);

    //! Get comparable name after underscore if necessary, e.g., T_PET => use PET
    static string GetComparableName(string &paraName);

    //! Find dependent parameters
    static ParamInfo *FindDependentParam(ParamInfo *paramInfo, vector<string> &moduleIDs,
                                         map<string, vector<ParamInfo *> > &moduleOutputs);

private:
    //! Database name of the simulation model
    string m_dbName;
    //! Module IDs
    vector<string> m_moduleIDs;
    //! instance map of modules
    map<string, InstanceFunction> m_instanceFuncs;
    //! Metadata map of modules
    map<string, MetadataFunction> m_metadataFuncs;
    //! dynamic library handles (.dll in Windows, .so in Linux, and .dylib in macOS)
    vector<DLLINSTANCE> m_dllHandles;
    //! Module settings
    map<string, SEIMSModuleSetting *> m_settings;
    //! Metadata of modules
    map<string, const char *> m_metadata;
    //! Parameters of modules, from \sa m_parametersInDB
    map<string, vector<ParamInfo *> > m_moduleParameters;
    //! Input of modules, from other modules
    map<string, vector<ParamInfo *> > m_moduleInputs;
    //! Output of modules, out from current module
    map<string, vector<ParamInfo *> > m_moduleOutputs;
};
#endif /* SEIMS_MODULE_FACTORY_H */
