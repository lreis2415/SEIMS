/*!
 * \file ModuleFactory.h
 * \brief Constructor of ModuleFactory from config file
 *
 * Changelog:
 *   - 1. 2017-05-30 - lj - Refactor and DeCoupling with Database I/O.
 *   - 2. 2022-08-19 - lj - Separate integer and floating point of parameter, input, output, and inoutput.
 *
 * \author Junzhi Liu, LiangJun Zhu
 * \version 2.1
 */
#ifndef SEIMS_MODULE_FACTORY_H
#define SEIMS_MODULE_FACTORY_H

#include "invoke.h"
#include "MetadataInfo.h"
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
                  map<string, Information>& moduleInformations,
                  map<string, vector<ParamInfo<FLTPT>*> >& moduleParams,
                  map<string, vector<ParamInfo<int>*> >& moduleParamsInt,
                  map<string, vector<ParamInfo<FLTPT>*> >& moduleInputs,
                  map<string, vector<ParamInfo<int>*> >& moduleInputsInt,
                  map<string, vector<ParamInfo<FLTPT>*> >& moduleOutputs,
                  map<string, vector<ParamInfo<int>*> >& moduleOutputsInt,
                  map<string, vector<ParamInfo<FLTPT>*> >& moduleInOutputs,
                  map<string, vector<ParamInfo<int>*> >& moduleInOutputsInt,
                  vector<ParamInfo<FLTPT> *>& tfValueInputs,
                  vector<ParamInfo<int>*>& tfValueInputsInt,
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

    //! Find outputID parameter's module. Return Module index iModule and its ParamInfo<FLTPT>
    bool FindOutputParameter(string& outputID, int& iModule, ParamInfo<FLTPT>*& paraInfo);

    //! Get Module ID by index
    string GetModuleID(int i) const { return m_moduleIDs[i]; }

    //! Get unique module IDs
    vector<string>& GetModuleIDs() { return m_moduleIDs; }

    //! Get map of module settings
    map<string, SEIMSModuleSetting *>& GetModuleSettings() { return m_settings; }

    map<string, Information>& GetModuleInformations() { return m_moduleInformations; }

    //! Get Parameters of modules
    map<string, vector<ParamInfo<FLTPT> *> >& GetModuleParams() { return m_moduleParams; }

    //! Get integer parameters of modules
    map<string, vector<ParamInfo<int>*> >& GetModuleParamsInt() { return m_moduleParamsInt; }

    //! Get Input of modules, from other modules
    map<string, vector<ParamInfo<FLTPT> *> >& GetModuleInputs() { return m_moduleInputs; }

    //! Get integer input of modules, from other modules
    map<string, vector<ParamInfo<int>*> >& GetModuleInputsInt() { return m_moduleInputsInt; }

    //! Get Output of modules, out from current module
    map<string, vector<ParamInfo<FLTPT>*> >& GetModuleOutputs() { return m_moduleOutputs; }

    //! Get integer Output of modules, out from current module
    map<string, vector<ParamInfo<int>*> >& GetModuleOutputsInt() { return m_moduleOutputsInt; }

    //! Get InOutput of modules, in and out from current module
    map<string, vector<ParamInfo<FLTPT> *> >& GetModuleInOutputs() { return m_moduleInOutputs; }

    //! Get integer InOutput of modules, in and out from current module
    map<string, vector<ParamInfo<int>*> >& GetModuleInOutputsInt() { return m_moduleInOutputsInt; }

    //! Get transferred single value inputs across subbasins
    vector<ParamInfo<FLTPT>*>& GetTransferredInputs() { return m_tfValueInputs; }

    //! Get transferred single integer value inputs across subbasins
    vector<ParamInfo<int>*>& GetTransferredInputsInt() { return m_tfValueInputsInt; }

    //! Get the count of transferred single value inputs
    int GetTransferredInputsCount() { return CVT_INT(m_tfValueInputs.size()); }

    //! Get the count of transferred single integer value inputs
    int GetTransferredInputsIntCount() { return CVT_INT(m_tfValueInputsInt.size()); }

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
     * \param moduleParams  
     * \param moduleParamsInt 
     * \param moduleInputs 
     * \param moduleInputsInt 
     * \param moduleOutputs 
     * \param moduleOutputsInt 
     * \param moduleInOutputs 
     * \param tfValueInputs 
     * \return True if succeed, else throw exception and return false.
     */
    static bool LoadParseLibrary(const string& module_path, vector<string>& moduleIDs,
                                 map<string, SEIMSModuleSetting *>& moduleSettings,
                                 vector<DLLINSTANCE>& dllHandles,
                                 map<string, InstanceFunction>& instanceFuncs,
                                 map<string, MetadataFunction>& metadataFuncs,
                                 map<string, Information>& moduleInformations,
                                 map<string, vector<ParamInfo<FLTPT>*> >& moduleParams,
                                 map<string, vector<ParamInfo<int>*> >& moduleParamsInt,
                                 map<string, vector<ParamInfo<FLTPT>*> >& moduleInputs,
                                 map<string, vector<ParamInfo<int>*> >& moduleInputsInt,
                                 map<string, vector<ParamInfo<FLTPT> *> >& moduleOutputs,
                                 map<string, vector<ParamInfo<int>*> >& moduleOutputsInt,
                                 map<string, vector<ParamInfo<FLTPT>*> >& moduleInOutputs,
                                 map<string, vector<ParamInfo<int>*> >& moduleInOutputsInt,
                                 vector<ParamInfo<FLTPT>*>& tfValueInputs,
                                 vector<ParamInfo<int>*>& tfValueInputsInt);

    //! Load function pointers from .DLL or .so
    static void ReadDLL(const string& module_path, const string& id, const string& dllID,
                        vector<DLLINSTANCE>& dllHandles,
                        map<string, InstanceFunction>& instanceFuncs,
                        map<string, MetadataFunction>& metadataFuncs);

    //! Get module instance by moduleID
    SimulationModule* GetInstance(const string& moduleID) { return m_instanceFuncs[moduleID](); }

    //! Match data type, e.g., 1D array
    static dimensionTypes MatchType(const string &strType);

    //! Match data transfer type, e.g., TF_SingleValue
    static transferTypes MatchTransferType(const string& tfType);

    static void ReadInformation(string& moduleID, TiXmlDocument& doc, map<string, Information>& information);

    //! Is constant input?
    static bool IsConstantInputFromName(const string& name);

    //! Read module's parameters setting from XML string
    static void ReadParameterSetting(string& moduleID, TiXmlDocument& doc, SEIMSModuleSetting* setting,
                                     map<string, vector<ParamInfo<FLTPT> *> >& moduleParams,
                                     map<string, vector<ParamInfo<int>*> >& moduleParamsInt);

    //! Read module's input, output, and in/output setting from XML string
    static void ReadIOSetting(string& moduleID, TiXmlDocument& doc, SEIMSModuleSetting* setting,
                              const string& header, const string& title,
                              map<string, vector<ParamInfo<FLTPT>*> >& vars,
                              map<string, vector<ParamInfo<int>*> >& varsInt);

    //! Get comparable name after underscore if necessary, e.g., T_PET => use PET
    static string GetComparableName(string& paraName);

    //! Find dependent parameters
    static ParamInfo<FLTPT>* FindDependentParam(ParamInfo<FLTPT>* paramInfo, vector<string>& moduleIDs,
                                                map<string, vector<ParamInfo<FLTPT> *> >& moduleOutputs);

    //! Find dependent parameters
    static ParamInfo<int>* FindDependentParam(ParamInfo<int>* paramInfo, vector<string>& moduleIDs,
                                              map<string, vector<ParamInfo<int>*> >& moduleOutputs);

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

    map<string, Information> m_moduleInformations;
    //! Parameters of modules, from database
    map<string, vector<ParamInfo<FLTPT>*> > m_moduleParams;
    //! Integer parameters of modules, from database
    map<string, vector<ParamInfo<int>*> > m_moduleParamsInt;
    //! Input of modules, from other modules
    map<string, vector<ParamInfo<FLTPT>*> > m_moduleInputs;
    //! Integer input of modules, from other modules
    map<string, vector<ParamInfo<int>*> > m_moduleInputsInt;
    //! Output of modules, out from current module
    map<string, vector<ParamInfo<FLTPT>*> > m_moduleOutputs;
    //! Integer output of modules, out from current module
    map<string, vector<ParamInfo<int>*> > m_moduleOutputsInt;
    //! InOutput of modules, out from current module, and from current module(i.e., other instance) meanwhile
    map<string, vector<ParamInfo<FLTPT>*> > m_moduleInOutputs;
    //! Integer InOutput of modules, out from current module, and from current module meanwhile
    map<string, vector<ParamInfo<int>*> > m_moduleInOutputsInt;
    //! transferred single value across subbasins
    vector<ParamInfo<FLTPT> *> m_tfValueInputs;
    //! transferred single integer value across subbasins
    vector<ParamInfo<int>*> m_tfValueInputsInt;
};
#endif /* SEIMS_MODULE_FACTORY_H */
