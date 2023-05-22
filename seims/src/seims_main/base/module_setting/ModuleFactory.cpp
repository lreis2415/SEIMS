#include "ModuleFactory.h"

#ifndef WINDOWS
#include <dlfcn.h> // dlopen()
#endif

#include "Logging.h"
#include "basic.h"
#include "MetadataInfo.h"
#include "text.h"

ModuleFactory::ModuleFactory(string model_name, vector<string>& moduleIDs,
                             map<string, SEIMSModuleSetting *>& moduleSettings,
                             vector<DLLINSTANCE>& dllHandles, map<string, InstanceFunction>& instanceFuncs,
                             map<string, MetadataFunction>& metadataFuncs,
                             map<string, vector<ParamInfo<FLTPT> *> >& moduleParams,
                             map<string, vector<ParamInfo<int>*> >& moduleParamsInt,
                             map<string, vector<ParamInfo<FLTPT> *> >& moduleInputs,
                             map<string, vector<ParamInfo<int>*> >& moduleInputsInt,
                             map<string, vector<ParamInfo<FLTPT> *> >& moduleOutputs,
                             map<string, vector<ParamInfo<int>*> >& moduleOutputsInt,
                             map<string, vector<ParamInfo<FLTPT> *> >& moduleInOutputs,
                             map<string, vector<ParamInfo<int>*> >& moduleInOutputsInt,
                             vector<ParamInfo<FLTPT> *>& tfValueInputs,
                             vector<ParamInfo<int>*>& tfValueInputsInt,
                             const int mpi_rank /* = 0 */, const int mpi_size /* = -1 */) :
    m_mpi_rank(mpi_rank), m_mpi_size(mpi_size),
    m_dbName(std::move(model_name)), m_moduleIDs(moduleIDs),
    m_instanceFuncs(instanceFuncs), m_metadataFuncs(metadataFuncs), m_dllHandles(dllHandles),
    m_settings(moduleSettings), m_moduleParams(moduleParams), m_moduleParamsInt(moduleParamsInt),
    m_moduleInputs(moduleInputs), m_moduleInputsInt(moduleInputsInt),
    m_moduleOutputs(moduleOutputs), m_moduleOutputsInt(moduleOutputsInt),
    m_moduleInOutputs(moduleInOutputs), m_moduleInOutputsInt(moduleInOutputsInt),
    m_tfValueInputs(tfValueInputs), m_tfValueInputsInt(tfValueInputsInt) {
    // nothing to do
}

ModuleFactory* ModuleFactory::Init(const string& module_path, InputArgs* input_args,
                                   const int mpi_rank /* = 0 */, const int mpi_size /* = -1 */) {
    string model_cfgpath = input_args->model_path;
    if (!input_args->model_cfgname.empty()) { model_cfgpath += SEP + input_args->model_cfgname; }
    /// Check the existence of configuration files
    /// Currently, file_in and file_out are not necessarily checked, since the FILE_IN and FILE_OUT in MongoDB are used.
    //string file_in = model_cfgpath + SEP + File_Input;
    //string file_out = model_cfgpath + SEP + File_Output;
    // The specific configuration file of the subbasin is prior.
    string file_cfg = model_cfgpath + SEP + "subbsn." + ValueToString(input_args->subbasin_id) + "." + File_Config;
    //string cfgNames[] = {file_in, file_out, file_cfg};
    if (!FileExists(file_cfg)) {
        file_cfg = model_cfgpath + SEP + File_Config;
        if (!FileExists(file_cfg)) {
            LOG(ERROR) << file_cfg << " does not exist or has not the read permission!";
            return nullptr;
        }
    }
    /// Read module configuration file
    vector<string> moduleIDs; // Unique module IDs (name)
    map<string, SEIMSModuleSetting *> moduleSettings; // basic module settings from cfg file
    if (!ReadConfigFile(file_cfg.c_str(), moduleIDs, moduleSettings)) return nullptr;
    /// Load module libraries and parse metadata
    vector<DLLINSTANCE> dllHandles; // dynamic library handles (.dll in Windows, .so in Linux, and .dylib in macOS)
    map<string, InstanceFunction> instanceFuncs; // map of modules instance
    map<string, MetadataFunction> metadataFuncs; // Metadata map of modules

    //map<string, const char *> moduleMetadata; // Metadata of modules
    map<string, vector<ParamInfo<FLTPT>*> > moduleParams; // Parameters of modules from MongoDB
    map<string, vector<ParamInfo<int>*> > moduleParamsInt; // Integer parameters of modules from MongoDB
    map<string, vector<ParamInfo<FLTPT>*> > moduleInputs; // Inputs of modules from other modules
    map<string, vector<ParamInfo<int>*> > moduleInputsInt; // Integer inputs of modules from other modules
    map<string, vector<ParamInfo<FLTPT>*> > moduleOutputs; // Output of current module
    map<string, vector<ParamInfo<int>*> > moduleOutputsInt; // Integer output of current module
    map<string, vector<ParamInfo<FLTPT>*> > moduleInOutputs; // InOutput of current module
    map<string, vector<ParamInfo<int>*> > moduleInOutputsInt; // Integer InOutput of current module
    vector<ParamInfo<FLTPT>*> tfValueInputs; // transferred single value across subbasins
    vector<ParamInfo<int>*> tfValueInputsInt; // transferred single value across subbasins
    try {
        LoadParseLibrary(module_path, moduleIDs, moduleSettings, dllHandles, instanceFuncs, metadataFuncs,
                         moduleParams, moduleParamsInt, moduleInputs, moduleInputsInt,
                         moduleOutputs,moduleOutputsInt, moduleInOutputs, moduleInOutputsInt,
                         tfValueInputs, tfValueInputsInt);
    } catch (ModelException& e) {
        LOG(ERROR) << e.ToString();
        return nullptr;
    }
    catch (std::exception& e) {
        LOG(ERROR) << e.what();
        return nullptr;
    }
    catch (...) {
        LOG(ERROR) << "Unknown exception occurred when loading module library!";
        return nullptr;
    }
    return new ModuleFactory(input_args->model_name, moduleIDs, moduleSettings, dllHandles,
                             instanceFuncs, metadataFuncs,
                             moduleParams, moduleParamsInt, moduleInputs, moduleInputsInt,
                             moduleOutputs, moduleOutputsInt, moduleInOutputs, moduleInOutputsInt,
                             tfValueInputs, tfValueInputsInt,
                             mpi_rank, mpi_size);
}

ModuleFactory::~ModuleFactory() {
    CLOG(TRACE, LOG_RELEASE) << "Start to release ModuleFactory ...";
    CLOG(TRACE, LOG_RELEASE) << "---release map of SEIMSModuleSettings ...";
    for (auto it = m_settings.begin(); it != m_settings.end(); ++it) {
        if (nullptr != it->second) {
            CLOG(TRACE, LOG_RELEASE) << "-----" << it->first << " ...";
            delete it->second;
            it->second = nullptr;
        }
    }
    m_settings.clear();

    CLOG(TRACE, LOG_RELEASE) << "---release module parameters ...";
    for (auto it = m_moduleParams.begin(); it != m_moduleParams.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            if (*it2 != nullptr) {
                delete *it2;
                *it2 = nullptr;
            }
        }
        it->second.clear();
    }
    m_moduleParams.clear();

    CLOG(TRACE, LOG_RELEASE) << "---release module parameters in integer ...";
    for (auto it = m_moduleParamsInt.begin(); it != m_moduleParamsInt.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            if (*it2 != nullptr) {
                delete* it2;
                *it2 = nullptr;
            }
        }
        it->second.clear();
    }
    m_moduleParamsInt.clear();

    CLOG(TRACE, LOG_RELEASE) << "---release module inputs ...";
    for (auto it = m_moduleInputs.begin(); it != m_moduleInputs.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            if (*it2 != nullptr) {
                delete *it2;
                *it2 = nullptr;
            }
        }
        it->second.clear();
    }
    m_moduleInputs.clear();

    CLOG(TRACE, LOG_RELEASE) << "---release module inputs in integer ...";
    for (auto it = m_moduleInputsInt.begin(); it != m_moduleInputsInt.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            if (*it2 != nullptr) {
                delete* it2;
                *it2 = nullptr;
            }
        }
        it->second.clear();
    }
    m_moduleInputsInt.clear();

    CLOG(TRACE, LOG_RELEASE) << "---release module outputs ...";
    for (auto it = m_moduleOutputs.begin(); it != m_moduleOutputs.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            if (*it2 != nullptr) {
                delete *it2;
                *it2 = nullptr;
            }
        }
        it->second.clear();
    }
    m_moduleOutputs.clear();

    CLOG(TRACE, LOG_RELEASE) << "---release module outputs in integer ...";
    for (auto it = m_moduleOutputsInt.begin(); it != m_moduleOutputsInt.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            if (*it2 != nullptr) {
                delete* it2;
                *it2 = nullptr;
            }
        }
        it->second.clear();
    }
    m_moduleOutputsInt.clear();

    CLOG(TRACE, LOG_RELEASE) << "---release module in/outputs ...";
    for (auto it = m_moduleInOutputs.begin(); it != m_moduleInOutputs.end(); ++it) {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            if (*it2 != nullptr) {
                delete *it2;
                *it2 = nullptr;
            }
        }
        it->second.clear();
    }
    m_moduleInOutputs.clear();

    CLOG(TRACE, LOG_RELEASE) << "---release module transferred value inputs ...";
    for (auto it = m_tfValueInputs.begin(); it != m_tfValueInputs.end(); ++it) {
        if (*it != nullptr) {
            *it = nullptr;
        }
    }
    m_tfValueInputs.clear();

    CLOG(TRACE, LOG_RELEASE) << "---release dynamic library handles ...";
    for (vector<DLLINSTANCE>::iterator dllit = m_dllHandles.begin(); dllit != m_dllHandles.end(); ) {
#ifdef WIN32
        FreeLibrary(*dllit);
#else
        dlclose(*dllit);
#endif /* WIN32 */
        dllit = m_dllHandles.erase(dllit);
    }
    CLOG(TRACE, LOG_RELEASE) << "End to release ModuleFactory.";
}

bool ModuleFactory::LoadParseLibrary(const string& module_path, vector<string>& moduleIDs,
                                     map<string, SEIMSModuleSetting *>& moduleSettings,
                                     vector<DLLINSTANCE>& dllHandles,
                                     map<string, InstanceFunction>& instanceFuncs,
                                     map<string, MetadataFunction>& metadataFuncs,
                                     map<string, vector<ParamInfo<FLTPT>*> >& moduleParams,
                                     map<string, vector<ParamInfo<int>*> >& moduleParamsInt,
                                     map<string, vector<ParamInfo<FLTPT>*> >& moduleInputs,
                                     map<string, vector<ParamInfo<int>*> >& moduleInputsInt,
                                     map<string, vector<ParamInfo<FLTPT>*> >& moduleOutputs,
                                     map<string, vector<ParamInfo<int>*> >& moduleOutputsInt,
                                     map<string, vector<ParamInfo<FLTPT>*> >& moduleInOutputs,
                                     map<string, vector<ParamInfo<int>*> >& moduleInOutputsInt,
                                     vector<ParamInfo<FLTPT>*>& tfValueInputs,
                                     vector<ParamInfo<int>*>& tfValueInputsInt) {
    size_t n = moduleIDs.size();
    // read all the .dll or .so and create objects
    for (size_t i = 0; i < n; i++) {
        string id = moduleIDs[i];
        string dllID = id;
        // for ITP modules, the input ids are ITP_T, ITP_P and ITP should be used as ID name
        // The same to TSD_RD.
        if (id.find(M_ITP[0]) != string::npos) {
#ifdef MSVC
            dllID = M_ITP[0];
#else
            dllID = LIBPREFIX + string(M_ITP[0]);
#endif /* MSVC */
            dllID += POSTFIX;
        } else if (id.find(M_TSD_RD[0]) != string::npos) {
#ifdef MSVC
            dllID = M_TSD_RD[0];
#else
            dllID = LIBPREFIX + string(M_TSD_RD[0]);
#endif /* MSVC */
            dllID += POSTFIX;
        }

        // load function pointers from DLL
        ReadDLL(module_path, id, dllID, dllHandles, instanceFuncs, metadataFuncs);

        // load metadata
        MetadataFunction metadataInfo = metadataFuncs[id];
        const char* current_metadata = metadataInfo();
        // parse the metadata
        TiXmlDocument doc;
        doc.Parse(current_metadata);
        ReadParameterSetting(id, doc, moduleSettings[id], moduleParams, moduleParamsInt);
        ReadIOSetting(id, doc, moduleSettings[id], TagInputs, TagInputVariable, moduleInputs, moduleInputsInt);
        ReadIOSetting(id, doc, moduleSettings[id], TagOutputs, TagOutputVariable, moduleOutputs, moduleOutputsInt);
        ReadIOSetting(id, doc, moduleSettings[id], TagInOutputs, TagInOutputVariable, moduleInOutputs, moduleInOutputsInt);
    }
    map<string, vector<ParamInfo<FLTPT>*> >(moduleParams).swap(moduleParams);
    map<string, vector<ParamInfo<int>*> >(moduleParamsInt).swap(moduleParamsInt);
    map<string, vector<ParamInfo<FLTPT>*> >(moduleInputs).swap(moduleInputs);
    map<string, vector<ParamInfo<int>*> >(moduleInputsInt).swap(moduleInputsInt);
    map<string, vector<ParamInfo<FLTPT>*> >(moduleOutputs).swap(moduleOutputs);
    map<string, vector<ParamInfo<int>*> >(moduleOutputsInt).swap(moduleOutputsInt);
    map<string, vector<ParamInfo<FLTPT>*> >(moduleInOutputs).swap(moduleInOutputs);
    map<string, vector<ParamInfo<int>*> >(moduleInOutputsInt).swap(moduleInOutputsInt);
    // set the connections among objects
    for (auto it = moduleIDs.begin(); it != moduleIDs.end(); ++it) {
        if (moduleInputs.find(*it) == moduleInputs.end()) { continue; }
        for (auto itInput = moduleInputs[*it].begin(); itInput != moduleInputs[*it].end(); ++itInput) {
            if (StringMatch((*itInput)->Source, Source_Module) ||
                StringMatch((*itInput)->Source, Source_Module_Optional)) {
                (*itInput)->DependPara = FindDependentParam(*itInput, moduleIDs, moduleOutputs);
                if ((*itInput)->Transfer == TF_SingleValue) {
                    // Check and append Inputs need to be transferred
                    if ((*itInput)->DependPara != nullptr) {
                        tfValueInputs.emplace_back((*itInput));
                    } else {
                        throw ModelException("ModelFactory", "LoadParseLibrary",
                                             "Couldn't find dependent output for " + (*itInput)->Name);
                    }
                }
            }
        }
    }
    for (auto it = moduleInOutputs.begin(); it != moduleInOutputs.end(); ++it) {
        if (it->second.empty()) { continue; }
        for (auto itParam = it->second.begin(); itParam != it->second.end(); ++itParam) {
            tfValueInputs.emplace_back(*itParam); // todo, is there possible that InOutput has duplication?
        }
    }
    for (auto it = moduleIDs.begin(); it != moduleIDs.end(); ++it) {
        if (moduleInputsInt.find(*it) == moduleInputsInt.end()) { continue; }
        for (auto itInput = moduleInputsInt[*it].begin(); itInput != moduleInputsInt[*it].end(); ++itInput) {
            if (StringMatch((*itInput)->Source, Source_Module) ||
                StringMatch((*itInput)->Source, Source_Module_Optional)) {
                (*itInput)->DependPara = FindDependentParam(*itInput, moduleIDs, moduleOutputsInt);
                if ((*itInput)->Transfer == TF_SingleValue) {
                    // Check and append Inputs need to be transferred
                    if ((*itInput)->DependPara != nullptr) {
                        tfValueInputsInt.emplace_back((*itInput));
                    }
                    else {
                        throw ModelException("ModelFactory", "LoadParseLibrary",
                                             "Couldn't find dependent output for integer " + (*itInput)->Name);
                    }
                }
            }
        }
    }
    for (auto it = moduleInOutputsInt.begin(); it != moduleInOutputsInt.end(); ++it) {
        if (it->second.empty()) { continue; }
        for (auto itParam = it->second.begin(); itParam != it->second.end(); ++itParam) {
            tfValueInputsInt.emplace_back(*itParam); // todo, is there possible that InOutput has duplication?
        }
    }
    return true;
}

string ModuleFactory::GetComparableName(string& paraName) {
    if (paraName.length() <= 2) {
        return paraName;
    }
    string compareName;
    string prefix = paraName.substr(0, 2);
    if (prefix == "D_" || prefix == "T_" || prefix == "d_" || prefix == "t_") {
        compareName = paraName.substr(2); //use the string after the underscore, T_PET, use PET
    } else {
        compareName = paraName;
    }
    return compareName;
}

void ModuleFactory::CreateModuleList(vector<SimulationModule *>& modules, const int nthread /* = 1 */) {
    for (auto it = m_moduleIDs.begin(); it != m_moduleIDs.end(); ++it) {
        SimulationModule* pModule = GetInstance(*it);
        pModule->SetTheadNumber(nthread);
        modules.emplace_back(pModule);
    }
}

ParamInfo<FLTPT>* ModuleFactory::FindDependentParam(ParamInfo<FLTPT>* paramInfo, vector<string>& moduleIDs,
                                                    map<string, vector<ParamInfo<FLTPT> *> >& moduleOutputs) {
    string paraName = GetComparableName(paramInfo->Name);
    dimensionTypes paraType = paramInfo->Dimension;
    transferTypes tfType = paramInfo->Transfer;
    for (auto it = moduleIDs.rbegin(); it != moduleIDs.rend(); ++it) {
        // loop from the last module
        if (moduleOutputs.find(*it) == moduleOutputs.end()) { continue; }
        for (auto itOut = moduleOutputs[*it].begin(); itOut != moduleOutputs[*it].end(); ++itOut) {
            string compareName = GetComparableName((*itOut)->Name);
            if (!StringMatch(paraName, compareName)) continue;
            if ((*itOut)->Dimension == paraType && // normal
                (tfType == (*itOut)->Transfer || tfType == TF_None) && // specified handling for mpi version
                !StringMatch(*it, paramInfo->ModuleID)) {
                // Avoid to dependent on the module itself
                (*itOut)->OutputToOthers = true;
                return *itOut;
            }
        }
    }
    if (!StringMatch(paramInfo->Source, Source_Module_Optional)) {
        throw ModelException("ModuleFactory", "FindDependentParam",
                             "Can not find input for " + paraName + " of " + paramInfo->ModuleID +
                             " from other Modules.");
    }
    return nullptr;
}

ParamInfo<int>* ModuleFactory::FindDependentParam(ParamInfo<int>* paramInfo, vector<string>& moduleIDs,
                                                  map<string, vector<ParamInfo<int>*> >& moduleOutputs) {
    string paraName = GetComparableName(paramInfo->Name);
    dimensionTypes paraType = paramInfo->Dimension;
    transferTypes tfType = paramInfo->Transfer;
    for (auto it = moduleIDs.rbegin(); it != moduleIDs.rend(); ++it) {
        // loop from the last module
        if (moduleOutputs.find(*it) == moduleOutputs.end()) { continue; }
        for (auto itOut = moduleOutputs[*it].begin(); itOut != moduleOutputs[*it].end(); ++itOut) {
            string compareName = GetComparableName((*itOut)->Name);
            if (!StringMatch(paraName, compareName)) continue;
            if ((*itOut)->Dimension == paraType && // normal
                (tfType == (*itOut)->Transfer || tfType == TF_None) && // specified handling for mpi version
                !StringMatch(*it, paramInfo->ModuleID)) {
                // Avoid to dependent on the module itself
                (*itOut)->OutputToOthers = true;
                return *itOut;
            }
        }
    }
    if (!StringMatch(paramInfo->Source, Source_Module_Optional)) {
        throw ModelException("ModuleFactory", "FindDependentParam",
                             "Can not find integer input for " + paraName + " of " + paramInfo->ModuleID +
                             " from other Modules.");
    }
    return nullptr;
}

void ModuleFactory::ReadDLL(const string& module_path, const string& id, const string& dllID,
                            vector<DLLINSTANCE>& dllHandles,
                            map<string, InstanceFunction>& instanceFuncs,
                            map<string, MetadataFunction>& metadataFuncs) {
    // the dll file is already read, return
    if (instanceFuncs.find(id) != instanceFuncs.end()) {
        return;
    }
    // check if the dll file exists
#ifndef WINDOWS
    string moduleFileName = module_path + "../lib/" + dllID + LIBSUFFIX;
#else
    string moduleFileName = module_path + SEP + dllID + LIBSUFFIX;
#endif
    //string moduleFileName = module_path + SEP + dllID + LIBSUFFIX;
    if (!FileExists(moduleFileName)) {
#ifndef WINDOWS
        moduleFileName = module_path + SEP + dllID + LIBSUFFIX;
#else
        moduleFileName = module_path + "../lib/" + dllID + LIBSUFFIX;
#endif
        if (!FileExists(moduleFileName)) {
            throw ModelException("ModuleFactory", "ReadDLL",
                                 moduleFileName + " does not exist or has no read permission!");
        }
    }
    //load library
#ifdef WINDOWS
    // MSVC or MinGW64 in Windows
    HINSTANCE handle = LoadLibrary(TEXT(moduleFileName.c_str()));
    if (handle == nullptr) throw ModelException("ModuleFactory", "ReadDLL",
                                                "Could not load " + moduleFileName);
    instanceFuncs[id] = InstanceFunction(GetProcAddress(HMODULE(handle), "GetInstance"));
    metadataFuncs[id] = MetadataFunction(GetProcAddress(HMODULE(handle), "MetadataInformation"));
#else
    void *handle = dlopen(moduleFileName.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        LOG(ERROR) << dlerror();
        throw ModelException("ModuleFactory", "ReadDLL", "Could not load " + dllID);
    }
    instanceFuncs[id] = InstanceFunction(dlsym(handle, "GetInstance"));
    metadataFuncs[id] = MetadataFunction(dlsym(handle, "MetadataInformation"));
#endif /* WINDOWS */
    dllHandles.emplace_back(handle);
    if (instanceFuncs[id] == nullptr) {
        throw ModelException("ModuleFactory", "ReadDLL",
                             moduleFileName + " does not implement API function: GetInstance");
    }
    if (metadataFuncs[id] == nullptr) {
        throw ModelException("ModuleFactory", "ReadDLL",
                             moduleFileName + " does not implement API function: MetadataInformation");
    }
    CLOG(TRACE, LOG_INIT) << "Read DLL: " << dllID;
}

dimensionTypes ModuleFactory::MatchType(const string& strType) {
    dimensionTypes typ = DT_Unknown;
    if (StringMatch(strType, Type_Single)) typ = DT_Single;
    if (StringMatch(strType, Type_SingleInt)) typ = DT_SingleInt;
    if (StringMatch(strType, Type_Array1D)) typ = DT_Array1D;
    if (StringMatch(strType, Type_Array1DInt)) typ = DT_Array1DInt;
    if (StringMatch(strType, Type_Array2D)) typ = DT_Array2D;
    if (StringMatch(strType, Type_Array2DInt)) typ = DT_Array2DInt;
    if (StringMatch(strType, Type_Array1DDateValue)) typ = DT_Array1DDateValue;
    if (StringMatch(strType, Type_Raster1D)) typ = DT_Raster1D;
    if (StringMatch(strType, Type_Raster1DInt)) typ = DT_Raster1DInt;
    if (StringMatch(strType, Type_Raster2D)) typ = DT_Raster2D;
    if (StringMatch(strType, Type_Raster2DInt)) typ = DT_Raster2DInt;
    if (StringMatch(strType, Type_Scenario)) typ = DT_Scenario;
    if (StringMatch(strType, Type_Reach)) typ = DT_Reach;
    if (StringMatch(strType, Type_Subbasin)) typ = DT_Subbasin;
    return typ;
}

transferTypes ModuleFactory::MatchTransferType(const string& tfType) {
    transferTypes typ = TF_None;
    if (StringMatch(tfType, TFType_Whole)) typ = TF_None;
    if (StringMatch(tfType, TFType_Single)) typ = TF_SingleValue;
    if (StringMatch(tfType, TFType_Array1D)) typ = TF_OneArray1D;
    return typ;
}

void ModuleFactory::ReadParameterSetting(string& moduleID, TiXmlDocument& doc,
                                         SEIMSModuleSetting* setting,
                                         map<string, vector<ParamInfo<FLTPT>*> >& moduleParams,
                                         map<string, vector<ParamInfo<int>*> >& moduleParamsInt) {
#ifdef HAS_VARIADIC_TEMPLATES
    moduleParams.emplace(moduleID, vector<ParamInfo<FLTPT>*>());
    moduleParamsInt.emplace(moduleID, vector<ParamInfo<int>*>());
#else
    moduleParams.insert(make_pair(moduleID, vector<ParamInfo<FLTPT>*>()));
    moduleParamsInt.insert(make_pair(moduleID, vector<ParamInfo<int>*>()));
#endif
    vector<ParamInfo<FLTPT>*>& vecPara = moduleParams.at(moduleID);
    vector<ParamInfo<int>*>& vecParaInt = moduleParamsInt.at(moduleID);
    // begin to parse parameters from metadata xml
    TiXmlElement* eleMetadata = doc.FirstChildElement(TagMetadata.c_str()); // "metadata"
    TiXmlElement* eleParams = eleMetadata->FirstChildElement(TagParameters.c_str()); // "parameters"
    if (nullptr == eleParams) {
        return;
    }
    TiXmlElement* eleParam = eleParams->FirstChildElement(TagParameter.c_str()); // "parameter"
    while (eleParam != nullptr) {
        TiXmlElement* elItm = eleParam->FirstChildElement(TagVariableName.c_str()); // "name"
        if (nullptr == elItm || nullptr == elItm->GetText()) { // "name" is required
            throw ModelException("ModuleFactory", "ReadParameterSetting",
                                 "Some parameters have no name in metadata!");
        }
        string name = GetUpper(string(elItm->GetText()));
        string desc;
        elItm = eleParam->FirstChildElement(TagVariableDescription.c_str()); // "description"
        if (elItm != nullptr && elItm->GetText() != nullptr) {
            desc = elItm->GetText();
        }
        string unit;
        elItm = eleParam->FirstChildElement(TagVariableUnits.c_str()); // "units"
        if (elItm != nullptr && elItm->GetText() != nullptr) {
            unit = elItm->GetText();
        }
        elItm = eleParam->FirstChildElement(TagVariableSource.c_str()); // "source"
        if (nullptr == elItm || nullptr == elItm->GetText()) {
            throw ModelException("ModuleFactory", "ReadParameterSetting",
                                 "parameter " + name + " does not have source!");
        }
        string src = elItm->GetText();
        dimensionTypes dim = DT_Unknown;
        elItm = eleParam->FirstChildElement(TagVariableDimension.c_str()); // "dimension"
        if (elItm != nullptr && elItm->GetText() != nullptr) {
            dim = MatchType(string(elItm->GetText()));
        }
        if (dim == DT_Unknown) {
            throw ModelException("ModuleFactory", "ReadParameterSetting",
                                 "parameter " + name + " does not have dimension!");
        }
        // handle parameter name
        string basicname = name;
        string datatype = setting->dataTypeString();
        int value = -1;
        // set climate data type got from config.fig. Used for interpolation module
        if (StringMatch(basicname, Tag_DataType)) { value = setting->dataType(); }

        //special process for interpolation modules
        if (StringMatch(name, Tag_Weight[0])) {
            if (setting->dataTypeString().length() == 0) {
                throw ModelException("ModuleFactory", "ReadParameterSetting",
                                     "The parameter " + name +
                                     " should have corresponding data type in module " + moduleID);
            }
            if (StringMatch(setting->dataTypeString(), DataType_MeanTemperature) ||
                StringMatch(setting->dataTypeString(), DataType_MinimumTemperature) ||
                StringMatch(setting->dataTypeString(), DataType_MaximumTemperature)) {
                //The weight coefficient file is same for TMEAN, TMIN and TMAX,
                //  so just need to read one file named "Weight_M"
                name += "_M";
            } else {
                // Combine weight and data type. e.g. Weight + PET = Weight_PET,
                //  this combined string must be the same with the parameter column
                //  in the climate table of parameter database.
                name += "_" + setting->dataTypeString();
            }
        }

        //special process for interpolation modules
        if (StringMatch(name, Tag_StationElevation)) {
            if (setting->dataTypeString().length() == 0) {
                throw ModelException("ModuleFactory", "readParameterSetting",
                                     "The parameter " + name +
                                     " should have corresponding data type in module " + moduleID);
            }
            if (StringMatch(setting->dataTypeString(), DataType_Precipitation)) {
                basicname += "_P";
                name += "_P";
            } else {
                basicname += "_M";
                name += "_M";
            }
        }
        if (StringMatch(name, Tag_VerticalInterpolation[0])) {
            value = setting->needDoVerticalInterpolation() ? 1 : 0; // Do vertical interpolation?
        }
        string climtype = setting->dataTypeString();
        if (dim == DT_SingleInt || dim == DT_Array1DInt || dim == DT_Raster1DInt
            || dim == DT_Array2DInt || dim == DT_Raster2DInt) {
            vecParaInt.emplace_back(new ParamInfo<int>(name, basicname, desc, unit, src,
                                                       moduleID, dim, climtype, value));
        } else {
            vecPara.emplace_back(new ParamInfo<FLTPT>(name, basicname, desc, unit, src,
                                                      moduleID, dim, climtype, value));
        }
        elItm = nullptr; // cleanup
        eleParam = eleParam->NextSiblingElement(); // get the next parameter if it exists
    } // while
}

bool ModuleFactory::IsConstantInputFromName(const string& name) {
    return StringMatch(name, CONS_IN_ELEV) || StringMatch(name, CONS_IN_LAT) ||
            StringMatch(name, CONS_IN_XPR) || StringMatch(name, CONS_IN_YPR);
}

void ModuleFactory::ReadIOSetting(string& moduleID, TiXmlDocument& doc, SEIMSModuleSetting* setting,
                                  const string& header, const string& title,
                                  map<string, vector<ParamInfo<FLTPT>*> >& vars,
                                  map<string, vector<ParamInfo<int>*> >& varsInt) {
    TiXmlElement* eleMetadata = doc.FirstChildElement(TagMetadata.c_str());
    TiXmlElement* eleVariables = eleMetadata->FirstChildElement(header.c_str()); // inputs, outputs, inoutputs
    if (nullptr == eleVariables) { return; }
#ifdef HAS_VARIADIC_TEMPLATES
    vars.emplace(moduleID, vector<ParamInfo<FLTPT>*>());
    varsInt.emplace(moduleID, vector<ParamInfo<int>*>());
#else
    vars.insert(make_pair(moduleID, vector<ParamInfo<FLTPT>*>()));
    varsInt.insert(make_pair(moduleID, vector<ParamInfo<int>*>()));
#endif
    vector<ParamInfo<FLTPT>* >& vecPara = vars.at(moduleID);
    vector<ParamInfo<int>* >& vecParaInt = varsInt.at(moduleID);
    TiXmlElement* eleVar = eleVariables->FirstChildElement(title.c_str());
    while (eleVar != nullptr) {
        // get the input variable name
        TiXmlElement* elItm = eleVar->FirstChildElement(TagVariableName.c_str());
        if (nullptr == elItm || nullptr == elItm->GetText()) {
            throw ModelException("ModuleFactory", "ReadIOSetting",
                                 "Some variables have empty name in metadata!");
        }
        string name = GetUpper(string(elItm->GetText()));
        string basicname = name;
        bool is_const = IsConstantInputFromName(name);
        if (setting->dataTypeString().length() > 0) {
            name += "_" + setting->dataTypeString();
        }
        // get the input variable unit
        string unit;
        elItm = eleVar->FirstChildElement(TagVariableUnits.c_str());
        if (elItm != nullptr && elItm->GetText() != nullptr) {
            unit = elItm->GetText();
        }
        // get the input variable description
        string desc;
        elItm = eleVar->FirstChildElement(TagVariableDescription.c_str());
        if (elItm != nullptr && elItm->GetText() != nullptr) {
            desc = elItm->GetText();
        }
        // get the input variable source
        bool is_output = false;
        if (header.find("output", 0) != string::npos) {
            is_output = true;
        }
        elItm = eleVar->FirstChildElement(TagVariableSource.c_str());
        if ((nullptr == elItm || nullptr == elItm->GetText()) && !is_output) {
            throw ModelException("ModuleFactory", "ReadIOSetting",
                                 "Variable " + name + " does not have source!");
        }
        string source = is_output ? "" : elItm->GetText();
        // get the input variable dimension
        elItm = eleVar->FirstChildElement(TagVariableDimension.c_str());
        if (nullptr == elItm || nullptr == elItm->GetText()) {
            throw ModelException("SEIMSModule", "ReadIOSetting",
                                 "Variable " + name + " does not have dimension!");
        }
        dimensionTypes dim = MatchType(string(elItm->GetText()));
        // get the input variable transfer type
        transferTypes tftype = TF_None;
        elItm = eleVar->FirstChildElement(TagVariableTransfer.c_str());
        if (elItm != nullptr && elItm->GetText() != nullptr) {
            tftype = MatchTransferType(string(elItm->GetText()));
        }
        string climtype = setting->dataTypeString();
        if (dim == DT_SingleInt || dim == DT_Array1DInt || dim == DT_Raster1DInt
            || dim == DT_Array2DInt || dim == DT_Raster2DInt) {
            vecParaInt.emplace_back(new ParamInfo<int>(name, basicname, desc, unit, source,
                                                       moduleID, dim, tftype, climtype,
                                                       is_const, is_output));
        } else {
            vecPara.emplace_back(new ParamInfo<FLTPT>(name, basicname, desc, unit, source,
                                                      moduleID, dim, tftype, climtype,
                                                      is_const, is_output));
        }

        elItm = nullptr;
        eleVar = eleVar->NextSiblingElement(); // get the next input if it exists
    }
}

bool ModuleFactory::LoadSettingsFromFile(const char* filename, vector<vector<string> >& settings) {
    vector<string> cfgStrs;
    if (!LoadPlainTextFile(filename, cfgStrs)) {
        return false;
    }
    string T_variables[7] = {
        DataType_Precipitation, DataType_MeanTemperature, DataType_MaximumTemperature,
        DataType_MinimumTemperature, DataType_SolarRadiation, DataType_WindSpeed,
        DataType_RelativeAirMoisture
    };// �����������ͣ�����P���¶�T��
    for (auto iter = cfgStrs.begin(); iter != cfgStrs.end(); ++iter) {
        // parse the line into separate item
        vector<string> tokens = SplitString(*iter, '|');
        // is there anything in the token list?
        if (tokens.empty()) continue;
        for (size_t i = 0; i < tokens.size(); i++) {
            //TrimSpaces(tokens[i]);
            tokens[i] = Trim(tokens[i]);
        }
        if (tokens.empty()) continue;
        // is there anything in the first item? Or, the size of tokens greater than 4?
        if (tokens[0].empty() || tokens.size() < 4) continue;
        // there is something to add so resize the header list to append it
        size_t sz = settings.size(); // get the current number of rows
        if (tokens[3].find(M_ITP[0]) != string::npos ||
            tokens[3].find(M_TSD_RD[0]) != string::npos) {
            settings.resize(sz + 7);
            for (size_t j = 0; j < 7; j++) {
                vector<string> tokensTemp(tokens);
                if (tokens[3].find(M_ITP[0]) != string::npos) {
                    bool useVerticalItp = false;  // Default
                    vector<string> ITPProperty = SplitString(tokensTemp[1], '_');
                    if (ITPProperty.size() == 2) {
                        char* strend = nullptr;
                        errno = 0;
                        useVerticalItp = strtol(ITPProperty[1].c_str(), &strend, 10) > 0;
                    }
                    // For interpolation modules, e.g.:
                    //   0 | Interpolation_0 | Thiessen | ITP
                    // will be updated as:
                    //  0 | Interpolation_P_0 | Thiessen | ITP, etc.
                    if (useVerticalItp) {
                        tokensTemp[1] = ITPProperty[0] + "_" + T_variables[j] + "_1";
                    } else {
                        tokensTemp[1] = ITPProperty[0] + "_" + T_variables[j] + "_0";
                    }
                } else {
                    // For time series data reading modules, e.g.:
                    //   0 | TimeSeries | | TSD_RD
                    // will be updated as:
                    //   0 | TimeSeries_P | | TSD_RD, etc. ����ģ����ƴ�������������ͣ����硰P��
                    tokensTemp[1] += "_" + T_variables[j];  // PROCESS NAME
                }
                settings[sz + j] = tokensTemp;
            }
        } else {
            settings.resize(sz + 1); // resize with one more row
            settings[sz] = tokens;
        }
    }
    return true;
}

bool ModuleFactory::ReadConfigFile(const char* configFileName, vector<string>& moduleIDs,
                                   map<string, SEIMSModuleSetting *>& moduleSettings) {
    vector<vector<string> > settings;
    if (!LoadSettingsFromFile(configFileName, settings)) { return false; }
    try {
        for (size_t i = 0; i < settings.size(); i++) {
            if (settings[i].size() > 3) {
                // PROCESS NAME with suffix, e.g., Interpolation_P_0 and TimeSeries_M
                string settingString = settings[i][1];
                string module = GetUpper(settings[i][3]);
#ifndef MSVC
                module.insert(0, LIBPREFIX);
#endif /* MSVC */
                module += POSTFIX;

                SEIMSModuleSetting* moduleSetting = new SEIMSModuleSetting(module, settingString);
                if (moduleSetting->dataTypeString().length() > 0) {
                    module += "_" + moduleSetting->dataTypeString();
                } // make the module ID unique for TimeSeries read module and interpolation module
#ifdef HAS_VARIADIC_TEMPLATES
                if (!moduleSettings.emplace(module, moduleSetting).second) {
#else
                if (!moduleSettings.insert(make_pair(module, moduleSetting)).second) {
#endif
                    delete moduleSetting;
                    continue;
                }
                moduleIDs.emplace_back(module);// ģ��id = .cfg�ļ��е�MODULE ID + "d"  + "_" + ��������"P", eg."TSD_RDd_P" ��"TSD_RDd_TMEAN"
            }
        }
    } catch (...) {
        cout << "ReadConfigFile failed, please contact the developers!" << endl;
        return false;
    }
    return true;
}

/// Revised LiangJun Zhu
/// 1. Fix code of DT_Raster2D related, 2016-5-27
/// 2. Bugs fixed in continuous dependency, 2016-9-6
void ModuleFactory::GetValueFromDependencyModule(const int iModule, vector<SimulationModule *>& modules) {
    int n = CVT_INT(m_moduleIDs.size());
    string id = m_moduleIDs[iModule];
    vector<ParamInfo<FLTPT>* >& inputs = m_moduleInputs[id];
    vector<ParamInfo<int>* >& inputsInt = m_moduleInputsInt[id];
    /// if there are no inputs from other modules for current module
    bool noInputsFromOthers = true;
    for (auto it = inputs.begin(); it != inputs.end(); ++it) {
        ParamInfo<FLTPT>* param = *it;
        if (StringMatch(param->Source, Source_Module) ||
            (StringMatch(param->Source, Source_Module_Optional) && param->DependPara != nullptr)) {
            noInputsFromOthers = false;
        }
    }
    bool noInputsIntFromOthers = true;
    for (auto it = inputsInt.begin(); it != inputsInt.end(); ++it) {
        ParamInfo<int>* param = *it;
        if (StringMatch(param->Source, Source_Module) ||
            (StringMatch(param->Source, Source_Module_Optional) && param->DependPara != nullptr)) {
            noInputsIntFromOthers = false;
        }
    }
    if (noInputsFromOthers && noInputsIntFromOthers) {
        modules[iModule]->SetInputsDone(true);
        return;
    }
    // for floating point variables
    for (size_t j = 0; j < inputs.size(); j++) {
        ParamInfo<FLTPT>* dependParam = inputs[j]->DependPara;
        if (dependParam == nullptr) { continue; }
        int k = 0;
        for (k = 0; k < n; k++) {
            if (m_moduleIDs[k] == dependParam->ModuleID) {
                break;
            }
        }
        //cout<<iModule<<", "<<k<<", "<<dependParam->ModuleID<<", "<<dependParam->Name<<endl;
        if (!modules[k]->IsInputsSetDone()) {
            GetValueFromDependencyModule(k, modules);
        }
        string compareName = GetComparableName(dependParam->Name);
        int dataLen;
        if (dependParam->Dimension == DT_Array1D || dependParam->Dimension == DT_Raster1D) {
            FLTPT* data;
            modules[k]->Get1DData(compareName.c_str(), &dataLen, &data);
            modules[iModule]->Set1DData(inputs[j]->Name.c_str(), dataLen, data);
            dependParam->initialized = true;
            modules[iModule]->SetInputsDone(true);
        } else if (dependParam->Dimension == DT_Array2D || dependParam->Dimension == DT_Raster2D) {
            int nCol;
            FLTPT** data;
            modules[k]->Get2DData(compareName.c_str(), &dataLen, &nCol, &data);
            modules[iModule]->Set2DData(inputs[j]->Name.c_str(), dataLen, nCol, data);
            dependParam->initialized = true;
            modules[iModule]->SetInputsDone(true);
        } else if (dependParam->Dimension == DT_Single) {
            FLTPT value;
            modules[k]->GetValue(compareName.c_str(), &value);
            modules[iModule]->SetValue(inputs[j]->Name.c_str(), value);
            dependParam->initialized = true;
            modules[iModule]->SetInputsDone(true);
        } else {
            std::ostringstream oss;
            oss << "Dimension type: " << dependParam->Dimension << " is currently not supported.";
            throw ModelException("ModuleFactory", "GetValueFromDependencyModule", oss.str());
        }
    }
    // for integer variables
    for (size_t j = 0; j < inputsInt.size(); j++) {
        ParamInfo<int>* dependParam = inputsInt[j]->DependPara;
        if (dependParam == nullptr) { continue; }
        int k = 0;
        for (k = 0; k < n; k++) {
            if (m_moduleIDs[k] == dependParam->ModuleID) {
                break;
            }
        }
        //cout<<iModule<<", "<<k<<", "<<dependParam->ModuleID<<", "<<dependParam->Name<<endl;
        if (!modules[k]->IsInputsSetDone()) {
            GetValueFromDependencyModule(k, modules);
        }
        string compareName = GetComparableName(dependParam->Name);
        int dataLen;
        if (dependParam->Dimension == DT_Array1DInt || dependParam->Dimension == DT_Raster1DInt) {
            int* data;
            modules[k]->Get1DData(compareName.c_str(), &dataLen, &data);
            modules[iModule]->Set1DData(inputsInt[j]->Name.c_str(), dataLen, data);
            dependParam->initialized = true;
            modules[iModule]->SetInputsDone(true);
        }
        else if (dependParam->Dimension == DT_Array2DInt || dependParam->Dimension == DT_Raster2DInt) {
            int nCol;
            int** data;
            modules[k]->Get2DData(compareName.c_str(), &dataLen, &nCol, &data);
            modules[iModule]->Set2DData(inputsInt[j]->Name.c_str(), dataLen, nCol, data);
            dependParam->initialized = true;
            modules[iModule]->SetInputsDone(true);
        }
        else if (dependParam->Dimension == DT_SingleInt) {
            int value;
            modules[k]->GetValue(compareName.c_str(), &value);
            modules[iModule]->SetValue(inputsInt[j]->Name.c_str(), value);
            dependParam->initialized = true;
            modules[iModule]->SetInputsDone(true);
        }
        else {
            std::ostringstream oss;
            oss << "Dimension type: " << dependParam->Dimension << " is currently not supported.";
            throw ModelException("ModuleFactory", "GetValueFromDependencyModule", oss.str());
        }
    }
}

bool ModuleFactory::FindOutputParameter(string& outputID, int& iModule, ParamInfo<FLTPT>*& paraInfo) {
    size_t n = m_moduleIDs.size();
    paraInfo = nullptr;
    for (size_t i = 0; i < n; i++) {
        string id = m_moduleIDs[i];
        vector<ParamInfo<FLTPT>*>& vecPara = m_moduleOutputs[id];
        for (size_t j = 0; j < vecPara.size(); j++) {
            if (StringMatch(outputID, vecPara[j]->Name)) {
                iModule = CVT_INT(i);
                paraInfo = vecPara[j];
                return true;
            }
        }
    }
    return false;
}
