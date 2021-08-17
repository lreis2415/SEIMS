/*!
 * \file ModuleFactory.h
 * \brief Constructor of ModuleFactory from config file
 *
 * Changelog:
 *   - 1. 2017-05-30 - lj - Refactor and DeCoupling with Database I/O.
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.0
 */
#ifndef SEIMS_MODULE_FACTORY_H
#define SEIMS_MODULE_FACTORY_H

#include "invoke.h"
#include "SEIMS_ModuleSetting.h"
#include "SimulationModule.h"
#include "ParamInfo.h"
#include "SettingsInput.h"

#include "tinyxml.h"

#ifdef WIN32
#define DLLINSTANCE HINSTANCE
#else
#define DLLINSTANCE void*
#endif
//! Simulation module instance
typedef SimulationModule*(*InstanceFunction)();
//! Simulation module metadata
typedef const char*(*MetadataFunction)();

using namespace bmps;

/*!
 * \class ModuleFactory
 * \ingroup module_setting
 * \brief Linking user-defined modules to create the modeling workflow.
 */
class ModuleFactory: NotCopyable {
public:
    /*!
    * \brief Constructor
    */
    ModuleFactory(string model_name,
                  vector<string>& moduleIDs,
                  map<string, SEIMSModuleSetting *>& moduleSettings,
                  vector<DLLINSTANCE>& dllHandles,
                  map<string, InstanceFunction>& instanceFuncs,
                  map<string, MetadataFunction>& metadataFuncs,
                  map<string, vector<ParamInfo *> >& moduleParameters,
                  map<string, vector<ParamInfo *> >& moduleInputs,
                  map<string, vector<ParamInfo *> >& moduleOutputs,
                  map<string, vector<ParamInfo *> >& moduleInOutputs,
                  vector<ParamInfo *>& tfValueInputs,
                  int mpi_rank = 0, int mpi_size = -1);
    /*!
     * \brief Initialization for exception-safe constructor
     */
    static ModuleFactory* Init(const string& module_path, InputArgs* input_args,
                               int mpi_rank = 0, int mpi_size = -1);

    //! Destructor
    ~ModuleFactory();

    //! Create a set of objects and set up the relationship among them. Return time-consuming.
    void CreateModuleList(vector<SimulationModule *>& modules, int nthread = 1);

    //! Get value from dependency modules
    void GetValueFromDependencyModule(int iModule, vector<SimulationModule *>& modules);

    //! Find outputID parameter's module. Return Module index iModule and its ParamInfo
    void FindOutputParameter(string& outputID, int& iModule, ParamInfo*& paraInfo);

    //! Get Module ID by index
    string GetModuleID(int i) const { return m_moduleIDs[i]; }

    //! Get unique module IDs
    vector<string>& GetModuleIDs() { return m_moduleIDs; }

    //! Get map of module settings
    map<string, SEIMSModuleSetting *>& GetModuleSettings() { return m_settings; }

    //! Get Parameters of modules
    map<string, vector<ParamInfo *> >& GetModuleParameters() { return m_moduleParameters; }

    //! Get Input of modules, from other modules
    map<string, vector<ParamInfo *> >& GetModuleInputs() { return m_moduleInputs; }

    //! Get Output of modules, out from current module
    map<string, vector<ParamInfo *> >& GetModuleOutputs() { return m_moduleOutputs; }

    //! Get InOutput of modules, in and out from current module
    map<string, vector<ParamInfo *> >& GetModuleInOutputs() { return m_moduleInOutputs; }

    //! Get transferred single value inputs across subbasins
    vector<ParamInfo*>& GetTransferredInputs() { return m_tfValueInputs; }

    //! Get the count of transferred single value inputs
    int GetTransferredInputsCount() { return CVT_INT(m_tfValueInputs.size()); }

    //! Load modules setting from file
    static bool LoadSettingsFromFile(const char* filename, vector<vector<string> >& settings);

    /*!
     * \brief Read configuration file
     * \param[in] configFileName Configuration full file path
     * \param[out] moduleIDs Unique module IDs (name)
     * \param[out] moduleSettings Map of SEIMSModuleSetting
     * \return True if succeed.
     */
    static bool ReadConfigFile(const char* configFileName, vector<string>& moduleIDs,
                               map<string, SEIMSModuleSetting *>& moduleSettings);

    /*!
     * \brief Load and parse module libraries
     * \param module_path
     * \param moduleIDs
     * \param moduleSettings
     * \param dllHandles
     * \param instanceFuncs
     * \param metadataFuncs
     * \param moduleParameters
     * \param moduleInputs
     * \param moduleOutputs
     * \param moduleInOutputs
     * \param tfValueInputs
     * \return True if succeed, else throw exception and return false.
     */
    static bool LoadParseLibrary(const string& module_path, vector<string>& moduleIDs,
                                 map<string, SEIMSModuleSetting *>& moduleSettings,
                                 vector<DLLINSTANCE>& dllHandles,
                                 map<string, InstanceFunction>& instanceFuncs,
                                 map<string, MetadataFunction>& metadataFuncs,
                                 map<string, vector<ParamInfo *> >& moduleParameters,
                                 map<string, vector<ParamInfo *> >& moduleInputs,
                                 map<string, vector<ParamInfo *> >& moduleOutputs,
                                 map<string, vector<ParamInfo *> >& moduleInOutputs,
                                 vector<ParamInfo*>& tfValueInputs);

    //! Load function pointers from .DLL or .so
    static void ReadDLL(const string& module_path, const string& id, const string& dllID,
                        vector<DLLINSTANCE>& dllHandles,
                        map<string, InstanceFunction>& instanceFuncs,
                        map<string, MetadataFunction>& metadataFuncs);

    //! Get module instance by moduleID
    SimulationModule* GetInstance(string& moduleID) { return m_instanceFuncs[moduleID](); }

    //! Match data type, e.g., 1D array
    static dimensionTypes MatchType(string strType);

    //! Match data transfer type, e.g., TF_SingleValue
    static transferTypes MatchTransferType(string tfType);

    //! Is constant input?
    static bool IsConstantInputFromName(string& name);

    //! Read module's parameters setting from XML string
    static void ReadParameterSetting(string& moduleID, TiXmlDocument& doc, SEIMSModuleSetting* setting,
                                     map<string, vector<ParamInfo *> >& moduleParameters);

    //! Read module's input, output, and in/output setting from XML string
    static void ReadIOSetting(string& moduleID, TiXmlDocument& doc, SEIMSModuleSetting* setting,
                              const string& header, const string& title, map<string, vector<ParamInfo *> >& variables);

    //! Get comparable name after underscore if necessary, e.g., T_PET => use PET
    static string GetComparableName(string& paraName);

    //! Find dependent parameters
    static ParamInfo* FindDependentParam(ParamInfo* paramInfo, vector<string>& moduleIDs,
                                         map<string, vector<ParamInfo *> >& moduleOutputs);

public:
    //! Rank ID for MPI, starts from 0 to mpi_size_ - 1
    int m_mpi_rank;                   
    //! Rank size for MPI
    int m_mpi_size;
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
    //! Parameters of modules, from #m_parametersInDB
    map<string, vector<ParamInfo *> > m_moduleParameters;
    //! Input of modules, from other modules
    map<string, vector<ParamInfo *> > m_moduleInputs;
    //! Output of modules, out from current module
    map<string, vector<ParamInfo *> > m_moduleOutputs;
    //! InOutput of modules, out from current module, and from current module(i.e., other instance) meanwhile
    map<string, vector<ParamInfo *> > m_moduleInOutputs;
    //! transferred single value across subbasins
    vector<ParamInfo *> m_tfValueInputs;
};
#endif /* SEIMS_MODULE_FACTORY_H */
