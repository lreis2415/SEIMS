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
                             map<string, vector<ParamInfo *> >& moduleParameters,
                             map<string, vector<ParamInfo *> >& moduleInputs,
                             map<string, vector<ParamInfo *> >& moduleOutputs,
                             map<string, vector<ParamInfo *> >& moduleInOutputs,
                             vector<ParamInfo *>& tfValueInputs,
                             const int mpi_rank /* = 0 */, const int mpi_size /* = -1 */) :
    m_mpi_rank(mpi_rank), m_mpi_size(mpi_size),
    m_dbName(std::move(model_name)), m_moduleIDs(moduleIDs),
    m_instanceFuncs(instanceFuncs), m_metadataFuncs(metadataFuncs), m_dllHandles(dllHandles),
    m_settings(moduleSettings), m_moduleParameters(moduleParameters),
    m_moduleInputs(moduleInputs), m_moduleOutputs(moduleOutputs), m_moduleInOutputs(moduleInOutputs),
    m_tfValueInputs(tfValueInputs) {
    // nothing to do
}

ModuleFactory* ModuleFactory::Init(const string& module_path, InputArgs* input_args,
                                   const int mpi_rank /* = 0 */, const int mpi_size /* = -1 */) {
    /// Check the existence of configuration files
    string file_in = input_args->model_path + SEP + File_Input;
    string file_out = input_args->model_path + SEP + File_Output;
    string file_cfg = input_args->model_path + SEP + File_Config;
    string cfgNames[] = {file_in, file_out, file_cfg};
    for (int i = 0; i < 3; ++i) {
        if (!FileExists(cfgNames[i])) {
            LOG(ERROR) << cfgNames[i] << " does not exist or has not the read permission!";
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
    map<string, vector<ParamInfo *> > moduleParameters; // Parameters of modules from MongoDB
    map<string, vector<ParamInfo *> > moduleInputs; // Inputs of modules from other modules
    map<string, vector<ParamInfo *> > moduleOutputs; // Output of current module
    map<string, vector<ParamInfo *> > moduleInOutputs; // InOutput of current module
    vector<ParamInfo*> tfValueInputs; // transferred single value across subbasins
    try {
        LoadParseLibrary(module_path, moduleIDs, moduleSettings, dllHandles, instanceFuncs,
                         metadataFuncs, moduleParameters, moduleInputs, moduleOutputs, moduleInOutputs, tfValueInputs);
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
                             moduleParameters, moduleInputs, moduleOutputs, moduleInOutputs, tfValueInputs,
                             mpi_rank, mpi_size);
}

ModuleFactory::~ModuleFactory() {
    CLOG(TRACE, LOG_RELEASE) << "Start to release ModuleFactory ...";
    /// Improved by Liangjun, 2016-7-6
    /// First release memory, then erase map element. BE CAUTION WHEN USE ERASE!!!
    CLOG(TRACE, LOG_RELEASE) << "---release map of SEIMSModuleSettings ...";
    for (auto it = m_settings.begin(); it != m_settings.end();) {
        if (nullptr != it->second) {
            CLOG(TRACE, LOG_RELEASE) << "-----" << it->first << " ...";
            delete it->second;
            it->second = nullptr;
        }
        m_settings.erase(it++);
    }
    m_settings.clear();
    CLOG(TRACE, LOG_RELEASE) << "---release module parameters ...";
    for (auto it = m_moduleParameters.begin(); it != m_moduleParameters.end();) {
        for (auto it2 = it->second.begin(); it2 != it->second.end();) {
            if (*it2 != nullptr) {
                delete *it2;
                *it2 = nullptr;
            }
            it2 = it->second.erase(it2);
        }
        m_moduleParameters.erase(it++);
    }
    CLOG(TRACE, LOG_RELEASE) << "---release module inputs ...";
    for (auto it = m_moduleInputs.begin(); it != m_moduleInputs.end();) {
        for (auto it2 = it->second.begin(); it2 != it->second.end();) {
            if (*it2 != nullptr) {
                delete *it2;
                *it2 = nullptr;
            }
            it2 = it->second.erase(it2);
        }
        m_moduleInputs.erase(it++);
    }
    CLOG(TRACE, LOG_RELEASE) << "---release module outputs ...";
    for (auto it = m_moduleOutputs.begin(); it != m_moduleOutputs.end();) {
        for (auto it2 = it->second.begin(); it2 != it->second.end();) {
            if (*it2 != nullptr) {
                delete *it2;
                *it2 = nullptr;
            }
            it2 = it->second.erase(it2);
        }
        m_moduleOutputs.erase(it++);
    }
    CLOG(TRACE, LOG_RELEASE) << "---release module in/outputs ...";
    for (auto it = m_moduleInOutputs.begin(); it != m_moduleInOutputs.end();) {
        for (auto it2 = it->second.begin(); it2 != it->second.end();) {
            if (*it2 != nullptr) {
                delete *it2;
                *it2 = nullptr;
            }
            it2 = it->second.erase(it2);
        }
        m_moduleInOutputs.erase(it++);
    }
    CLOG(TRACE, LOG_RELEASE) << "---release module transferred value inputs ...";
    for (auto it = m_tfValueInputs.begin(); it != m_tfValueInputs.end();) {
        if (*it != nullptr) {
            *it = nullptr;
        }
        it = m_tfValueInputs.erase(it);
    }
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
                                     map<string, vector<ParamInfo *> >& moduleParameters,
                                     map<string, vector<ParamInfo *> >& moduleInputs,
                                     map<string, vector<ParamInfo *> >& moduleOutputs,
                                     map<string, vector<ParamInfo *> >& moduleInOutputs,
                                     vector<ParamInfo*>& tfValueInputs) {
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
        ReadParameterSetting(id, doc, moduleSettings[id], moduleParameters);
        ReadIOSetting(id, doc, moduleSettings[id], TagInputs, TagInputVariable, moduleInputs);
        ReadIOSetting(id, doc, moduleSettings[id], TagOutputs, TagOutputVariable, moduleOutputs);
        ReadIOSetting(id, doc, moduleSettings[id], TagInOutputs, TagInOutputVariable, moduleInOutputs);
    }
    map<string, vector<ParamInfo *> >(moduleParameters).swap(moduleParameters);
    map<string, vector<ParamInfo *> >(moduleInputs).swap(moduleInputs);
    map<string, vector<ParamInfo *> >(moduleOutputs).swap(moduleOutputs);
    map<string, vector<ParamInfo *> >(moduleInOutputs).swap(moduleInOutputs);
    // set the connections among objects
    for (auto it = moduleIDs.begin(); it != moduleIDs.end(); ++it) {
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
        if (it->second.empty()) continue;
        for (auto itParam = it->second.begin(); itParam != it->second.end(); ++itParam) {
            tfValueInputs.emplace_back(*itParam);
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

void ModuleFactory::CreateModuleList(vector<SimulationModule *>& modules, int nthread /* = 1 */) {
    for (auto it = m_moduleIDs.begin(); it != m_moduleIDs.end(); ++it) {
        SimulationModule* pModule = GetInstance(*it);
        pModule->SetTheadNumber(nthread);
        modules.emplace_back(pModule);
    }
}

ParamInfo* ModuleFactory::FindDependentParam(ParamInfo* paramInfo, vector<string>& moduleIDs,
                                             map<string, vector<ParamInfo *> >& moduleOutputs) {
    string paraName = GetComparableName(paramInfo->Name);
    dimensionTypes paraType = paramInfo->Dimension;
    transferTypes tfType = paramInfo->Transfer;
    for (auto it = moduleIDs.rbegin(); it != moduleIDs.rend(); ++it) {
        // loop from the last module
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
                             " from other Modules.\n");
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
    string moduleFileName = module_path + SEP + dllID + LIBSUFFIX;
    if (!FileExists(moduleFileName)) {
        throw ModelException("ModuleFactory", "ReadDLL", moduleFileName + " does not exist or has no read permission!");
    }
    //load library
#ifdef WINDOWS
    // MSVC or MinGW64 in Windows
    HINSTANCE handle = LoadLibrary(TEXT(moduleFileName.c_str()));
    if (handle == nullptr) throw ModelException("ModuleFactory", "ReadDLL", "Could not load " + moduleFileName);
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

dimensionTypes ModuleFactory::MatchType(string strType) {
    // default
    dimensionTypes typ = DT_Unknown;
    if (StringMatch(strType, Type_Single)) typ = DT_Single;
    if (StringMatch(strType, Type_Array1D)) typ = DT_Array1D;
    if (StringMatch(strType, Type_Array2D)) typ = DT_Array2D;
    if (StringMatch(strType, Type_Array1DDateValue)) typ = DT_Array1DDateValue;
    if (StringMatch(strType, Type_Raster1D)) typ = DT_Raster1D;
    if (StringMatch(strType, Type_Raster2D)) typ = DT_Raster2D;
    if (StringMatch(strType, Type_Scenario)) typ = DT_Scenario;
    if (StringMatch(strType, Type_Reach)) typ = DT_Reach;
    if (StringMatch(strType, Type_Subbasin)) typ = DT_Subbasin;
    //if (StringMatch(strType, Type_SiteInformation)) typ = DT_SiteInformation;
    //if (StringMatch(strType, Type_LapseRateArray)) typ = DT_LapseRateArray;
    //if (StringMatch(strType, Type_LookupTable)) typ = DT_LookupTable;
    return typ;
}

transferTypes ModuleFactory::MatchTransferType(string tfType) {
    transferTypes typ = TF_None;
    if (StringMatch(tfType, TFType_Whole)) typ = TF_None;
    if (StringMatch(tfType, TFType_Single)) typ = TF_SingleValue;
    if (StringMatch(tfType, TFType_Array1D)) typ = TF_OneArray1D;
    return typ;
}

void ModuleFactory::ReadParameterSetting(string& moduleID, TiXmlDocument& doc, SEIMSModuleSetting* setting,
                                         map<string, vector<ParamInfo *> >& moduleParameters) {
#ifdef HAS_VARIADIC_TEMPLATES
    moduleParameters.emplace(moduleID, vector<ParamInfo *>());
#else
    moduleParameters.insert(make_pair(moduleID, vector<ParamInfo *>()));
#endif
    vector<ParamInfo *>& vecPara = moduleParameters.at(moduleID);
    TiXmlElement* eleMetadata = doc.FirstChildElement(TagMetadata.c_str());
    // start getting the parameters
    TiXmlElement* eleParams = eleMetadata->FirstChildElement(TagParameters.c_str());
    if (eleParams != nullptr) {
        TiXmlElement* eleParam = eleParams->FirstChildElement(TagParameter.c_str());
        while (eleParam != nullptr) {
            // clear the object
            ParamInfo* param = new ParamInfo();
            // set the module id
            param->ModuleID = moduleID;
            // get the parameter name
            TiXmlElement* elItm = eleParam->FirstChildElement(TagVariableName.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Name = GetUpper(string(elItm->GetText()));
                    param->BasicName = param->Name;
                    param->ClimateType = setting->dataTypeString();

                    //set climate data type got from config.fig
                    //this is used for interpolation module
                    if (StringMatch(param->Name, Tag_DataType)) param->Value = setting->dataType();

                    //special process for interpolation modules
                    if (StringMatch(param->Name, Tag_Weight[0])) {
                        if (setting->dataTypeString().length() == 0) {
                            throw ModelException("ModuleFactory", "ReadParameterSetting",
                                                 "The parameter " + string(Tag_Weight[0]) +
                                                 " should have corresponding data type in module " + moduleID);
                        }
                        if (StringMatch(setting->dataTypeString(), DataType_MeanTemperature) ||
                            StringMatch(setting->dataTypeString(), DataType_MinimumTemperature) ||
                            StringMatch(setting->dataTypeString(), DataType_MaximumTemperature)) {
                            //The weight coefficient file is same for TMEAN, TMIN and TMAX,
                            //  so just need to read one file named "Weight_M"
                            param->Name += "_M";

                        } else {
                            // Combine weight and data type. e.g. Weight + PET = Weight_PET,
                            //  this combined string must be the same with the parameter column
                            //  in the climate table of parameter database.
                            param->Name += "_" + setting->dataTypeString();
                        }
                    }

                    //special process for interpolation modules
                    if (StringMatch(param->Name, Tag_StationElevation)) {
                        if (setting->dataTypeString().length() == 0) {
                            throw ModelException("ModuleFactory", "readParameterSetting",
                                                 "The parameter " + string(Tag_StationElevation) +
                                                 " should have corresponding data type in module " + moduleID);
                        }
                        if (StringMatch(setting->dataTypeString(), DataType_Precipitation)) {
                            param->BasicName += "_P";
                            param->Name += "_P";
                        } else {
                            param->BasicName += "_M";
                            param->Name += "_M";
                        }
                    }
                    if (StringMatch(param->Name, Tag_VerticalInterpolation[0]))  {
                        param->Value = setting->needDoVerticalInterpolation() ? 1.0f : 0.0f; // Do vertical interpolation?
                    }
                }
            }
            // get the parameter description
            elItm = eleParam->FirstChildElement(TagVariableDescription.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Description = elItm->GetText();
                }
            }
            // get the parameter units
            elItm = eleParam->FirstChildElement(TagVariableUnits.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Units = elItm->GetText();
                }
            }
            // get the parameter source
            elItm = eleParam->FirstChildElement(TagVariableSource.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Source = elItm->GetText();
                }
            }
            // get the parameter dimension
            elItm = eleParam->FirstChildElement(TagVariableDimension.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Dimension = MatchType(string(elItm->GetText()));
                }
            }
            // cleanup
            elItm = nullptr;
            // parameter must have these values
            if (param->Name.empty()) {
                throw ModelException("ModuleFactory", "ReadParameterSetting",
                                     "Some parameters have not name in metadata!");
            }
            if (param->Source.empty()) {
                string name = param->Name;
                throw ModelException("ModuleFactory", "ReadParameterSetting",
                                     "parameter " + name + " does not have source!");
            }
            if (param->Dimension == DT_Unknown) {
                string name = param->Name;
                throw ModelException("ModuleFactory", "ReadParameterSetting",
                                     "parameter " + name + " does not have dimension!");
            }
            // add to the list
            vecPara.emplace_back(param);
            // get the next parameter if it exists
            eleParam = eleParam->NextSiblingElement();
        } // while
    }
}

bool ModuleFactory::IsConstantInputFromName(string& name) {
    return StringMatch(name, CONS_IN_ELEV) || StringMatch(name, CONS_IN_LAT) ||
            StringMatch(name, CONS_IN_XPR) || StringMatch(name, CONS_IN_YPR);
}

void ModuleFactory::ReadIOSetting(string& moduleID, TiXmlDocument& doc, SEIMSModuleSetting* setting,
                                  const string& header,
                                  const string& title, map<string, vector<ParamInfo *> >& variables) {
    TiXmlElement* eleMetadata = doc.FirstChildElement(TagMetadata.c_str());
    TiXmlElement* eleVariables = eleMetadata->FirstChildElement(header.c_str());
    if (nullptr == eleVariables) return;
#ifdef HAS_VARIADIC_TEMPLATES
    variables.emplace(moduleID, vector<ParamInfo *>());
#else
    variables.insert(make_pair(moduleID, vector<ParamInfo *>()));
#endif
    vector<ParamInfo *>& vecPara = variables.at(moduleID);
    TiXmlElement* eleVar = eleVariables->FirstChildElement(title.c_str());
    while (eleVar != nullptr) {
        ParamInfo* param = new ParamInfo();
        // set the module id
        param->ModuleID = moduleID;
        param->ClimateType = setting->dataTypeString();
        // get the input variable name
        TiXmlElement* elItm = eleVar->FirstChildElement(TagVariableName.c_str());
        if (elItm != nullptr && elItm->GetText() != nullptr) {
            param->Name = GetUpper(string(elItm->GetText()));
            param->BasicName = param->Name;
            param->IsConstant = IsConstantInputFromName(param->Name);
            if (setting->dataTypeString().length() > 0) {
                param->Name = param->Name + "_" + setting->dataTypeString();
            }
        }
        // get the input variable units(
        elItm = eleVar->FirstChildElement(TagVariableUnits.c_str());
        if (elItm != nullptr && elItm->GetText() != nullptr) {
            param->Units = elItm->GetText();
        }
        // get the input variable description
        elItm = eleVar->FirstChildElement(TagVariableDescription.c_str());
        if (elItm != nullptr && elItm->GetText() != nullptr) {
            param->Description = elItm->GetText();
        }
        // get the input variable source
        elItm = eleVar->FirstChildElement(TagVariableSource.c_str());
        if (elItm != nullptr && elItm->GetText() != nullptr) {
            param->Source = elItm->GetText();
        }
        // get the input variable dimension
        elItm = eleVar->FirstChildElement(TagVariableDimension.c_str());
        if (elItm != nullptr && elItm->GetText() != nullptr) {
            param->Dimension = MatchType(string(elItm->GetText()));
        }
        // get the input variable transfer type
        elItm = eleVar->FirstChildElement(TagVariableTransfer.c_str());
        if (elItm != nullptr && elItm->GetText() != nullptr) {
            param->Transfer = MatchTransferType(string(elItm->GetText()));
        }
        elItm = nullptr;
        if (header.find("output", 0) != string::npos) {
            param->IsOutput = true;
        }
        // input must have these values
        if (param->Name.empty()) {
            delete param;
            throw ModelException("SEIMSModule", "ReadIOSetting", "Some variables have no name in metadata!");
        }
        if (param->Source.empty() && !param->IsOutput) {
            string name = param->Name;
            delete param;
            throw ModelException("SEIMSModule", "ReadIOSetting", "Variable " + name + " does not have source!");
        }
        if (param->Dimension == DT_Unknown) {
            string name = param->Name;
            delete param;
            throw ModelException("SEIMSModule", "ReadIOSetting", "Variable " + name + " does not have dimension!");
        }
        vecPara.emplace_back(param);
        // get the next input if it exists
        eleVar = eleVar->NextSiblingElement();
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
    };
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
                    //   0 | TimeSeries_P | | TSD_RD, etc.
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
    if (!LoadSettingsFromFile(configFileName, settings)) return false;
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
                moduleIDs.emplace_back(module);
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
void ModuleFactory::GetValueFromDependencyModule(int iModule, vector<SimulationModule *>& modules) {
    int n = CVT_INT(m_moduleIDs.size());
    string id = m_moduleIDs[iModule];
    vector<ParamInfo *>& inputs = m_moduleInputs[id];
    /// if there are no inputs from other modules for current module
    bool noInputsFromOthers = true;
    for (auto it = inputs.begin(); it != inputs.end(); ++it) {
        ParamInfo* param = *it;
        if (StringMatch(param->Source, Source_Module) ||
            (StringMatch(param->Source, Source_Module_Optional) && param->DependPara != nullptr)) {
            noInputsFromOthers = false;
        }
    }
    if (noInputsFromOthers) {
        modules[iModule]->SetInputsDone(true);
        return;
    }

    for (size_t j = 0; j < inputs.size(); j++) {
        ParamInfo* dependParam = inputs[j]->DependPara;
        if (dependParam == nullptr) continue;

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
            float* data;
            modules[k]->Get1DData(compareName.c_str(), &dataLen, &data);
            modules[iModule]->Set1DData(inputs[j]->Name.c_str(), dataLen, data);
            dependParam->initialized = true;
            modules[iModule]->SetInputsDone(true);
        } else if (dependParam->Dimension == DT_Array2D || dependParam->Dimension == DT_Raster2D) {
            int nCol;
            float** data;
            modules[k]->Get2DData(compareName.c_str(), &dataLen, &nCol, &data);
            modules[iModule]->Set2DData(inputs[j]->Name.c_str(), dataLen, nCol, data);
            dependParam->initialized = true;
            modules[iModule]->SetInputsDone(true);
        } else if (dependParam->Dimension == DT_Single) {
            float value;
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
}

void ModuleFactory::FindOutputParameter(string& outputID, int& iModule, ParamInfo*& paraInfo) {
    size_t n = m_moduleIDs.size();
    for (size_t i = 0; i < n; i++) {
        string id = m_moduleIDs[i];
        vector<ParamInfo *>& vecPara = m_moduleOutputs[id];
        for (size_t j = 0; j < vecPara.size(); j++) {
            if (StringMatch(outputID, vecPara[j]->Name)) {
                iModule = CVT_INT(i);
                paraInfo = vecPara[j];
                return;
            }
        }
    }
}
