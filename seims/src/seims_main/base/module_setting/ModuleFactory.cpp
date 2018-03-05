#include "ModuleFactory.h"

ModuleFactory *ModuleFactory::Init(const string &module_path, InputArgs *input_args) {
    /// Check the existence of configuration files
    string file_in = input_args->m_model_path + SEP + File_Input;
    string file_out = input_args->m_model_path + SEP + File_Output;
    string file_cfg = input_args->m_model_path + SEP + File_Config;
    string cfgNames[] = {file_in, file_out, file_cfg};
    for (int i = 0; i < 3; ++i) {
        if (!FileExists(cfgNames[i])) {
            cout << cfgNames[i] << " does not exist or has not the read permission!" << endl;
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

    map<string, const char *> moduleMetadata; // Metadata of modules
    map<string, vector<ParamInfo *> > moduleParameters; // Parameters of modules from MongoDB
    map<string, vector<ParamInfo *> > moduleInputs; // Inputs of modules from other modules
    map<string, vector<ParamInfo *> > moduleOutputs; // Output of current module
    try {
        LoadParseLibrary(module_path, moduleIDs, moduleSettings, dllHandles, instanceFuncs,
                         metadataFuncs, moduleMetadata,
                         moduleParameters, moduleInputs, moduleOutputs);
    }
    catch (ModelException &e) {
        cout << e.toString() << endl;
        return nullptr;
    }
    catch (exception &e) {
        cout << e.what() << endl;
        return nullptr;
    }
    catch (...) {
        cout << "Unknown exception occurred when loading module library!" << endl;
        return nullptr;
    }
    return new ModuleFactory(input_args->m_model_name, moduleIDs, moduleSettings, dllHandles,
                             instanceFuncs, metadataFuncs,
                             moduleMetadata, moduleParameters,
                             moduleInputs, moduleOutputs);
}

ModuleFactory::~ModuleFactory() {
    StatusMessage("Start to release ModuleFactory ...");
    /// Improved by Liangjun, 2016-7-6
    /// First release memory, then erase map element. BE CAUTION WHEN USE ERASE!!!
    StatusMessage("---release map of SEIMSModuleSettings ...");
    for (auto it = m_settings.begin(); it != m_settings.end();) {
        if (nullptr != it->second) {
            StatusMessage(("-----" + it->first + " ...").c_str());
            delete it->second;
            it->second = nullptr;
        }
        m_settings.erase(it++);
    }
    m_settings.clear();

    StatusMessage("---release map of metadata of modules ...");
    for (auto it = m_metadata.begin(); it != m_metadata.end();) {
        if (nullptr != it->second) {
            StatusMessage(("-----" + it->first + " ...").c_str());
            delete it->second;
            it->second = nullptr;
        }
        m_metadata.erase(it++);
    }
    m_metadata.clear();

    StatusMessage("---release dynamic library handles ...");
    for (size_t i = 0; i < m_dllHandles.size(); i++) {
#ifdef WIN32
        FreeLibrary(m_dllHandles[i]);
#else
        dlclose(m_dllHandles[i]);
#endif /* WIN32 */
    }
    StatusMessage("---release module parameters ...");
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
    StatusMessage("---release module inputs ...");
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
    StatusMessage("---release module outputs ...");
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
    StatusMessage("End to release ModuleFactory ...");
}

bool ModuleFactory::LoadParseLibrary(const string &module_path, vector<string> &moduleIDs,
                                     map<string, SEIMSModuleSetting *> &moduleSettings,
                                     vector<DLLINSTANCE> &dllHandles,
                                     map<string, InstanceFunction> &instanceFuncs,
                                     map<string, MetadataFunction> &metadataFuncs,
                                     map<string, const char *> &moduleMetadata,
                                     map<string, vector<ParamInfo *> > &moduleParameters,
                                     map<string, vector<ParamInfo *> > &moduleInputs,
                                     map<string, vector<ParamInfo *> > &moduleOutputs) {
    size_t n = moduleIDs.size();
    // read all the .dll or .so and create objects
    for (size_t i = 0; i < n; i++) {
        string id = moduleIDs[i];
        string dllID = id;
        // for ITP modules, the input ids are ITP_T, ITP_P and ITP should be used as ID name
        // The same to TSD_RD.
        if (id.find(MID_ITP) != string::npos) {
#ifdef MSVC
            dllID = MID_ITP;
#else
            dllID = Tag_So + string(MID_ITP);
#endif /* MSVC */
            dllID += POSTFIX;
        } else if (id.find(MID_TSD_RD) != string::npos) {
#ifdef MSVC
            dllID = MID_TSD_RD;
#else
            dllID = Tag_So + string(MID_TSD_RD);
#endif /* MSVC */
            dllID += POSTFIX;
        }

        // load function pointers from DLL
        ReadDLL(module_path, id, dllID, dllHandles, instanceFuncs, metadataFuncs);

        // load metadata
        MetadataFunction metadataInfo = metadataFuncs[id];
        moduleMetadata.insert(make_pair(id, metadataInfo()));
        // parse the metadata
        TiXmlDocument doc;
        doc.Parse(moduleMetadata[id]);
        ReadParameterSetting(id, doc, moduleSettings[id], moduleParameters);
        ReadInputSetting(id, doc, moduleSettings[id], moduleInputs);
        ReadOutputSetting(id, doc, moduleSettings[id], moduleOutputs);
    }
    map<string, vector<ParamInfo *> >(moduleParameters).swap(moduleParameters);
    map<string, vector<ParamInfo *> >(moduleInputs).swap(moduleInputs);
    map<string, vector<ParamInfo *> >(moduleOutputs).swap(moduleOutputs);
    // set the connections among objects
    for (size_t i = 0; i < n; i++) {
        string id = moduleIDs[i];
        //cout << id << endl;
        vector<ParamInfo *> &inputs = moduleInputs[id];
        for (size_t j = 0; j < inputs.size(); j++) {
            ParamInfo *param = inputs[j];
            if (StringMatch(param->Source, Source_Module) ||
                StringMatch(param->Source, Source_Module_Optional)) {
                param->DependPara = FindDependentParam(param, moduleIDs, moduleOutputs);
            } else {
                continue;
            }
        }
    }
    return true;
}

string ModuleFactory::GetComparableName(string &paraName) {
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

void ModuleFactory::CreateModuleList(vector<SimulationModule *> &modules, int nthread /* = 1 */) {
    for (auto it = m_moduleIDs.begin(); it != m_moduleIDs.end(); it++) {
        SimulationModule *pModule = GetInstance(*it);
        pModule->SetTheadNumber(nthread);
        modules.push_back(pModule);
    }
}

ParamInfo *ModuleFactory::FindDependentParam(ParamInfo *paramInfo, vector<string> &moduleIDs,
                                             map<string, vector<ParamInfo *> > &moduleOutputs) {
    string paraName = GetComparableName(paramInfo->Name);
    dimensionTypes paraType = paramInfo->Dimension;

    size_t n = moduleIDs.size();
    for (size_t i = 0; i < n; i++) // module ID
    {
        string id = moduleIDs[i];
        vector<ParamInfo *> &outputs = moduleOutputs[id];
        for (size_t j = 0; j < outputs.size(); j++) // output ID
        {
            ParamInfo *param = outputs[j];
            string compareName = GetComparableName(param->Name);
            if (StringMatch(paraName, compareName) && param->Dimension == paraType) {
                param->OutputToOthers = true;
                return param;
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

void ModuleFactory::ReadDLL(const string &module_path, const string &id, const string &dllID,
                            vector<DLLINSTANCE> &dllHandles,
                            map<string, InstanceFunction> &instanceFuncs,
                            map<string, MetadataFunction> &metadataFuncs) {
    // the dll file is already read, return
    if (instanceFuncs.find(id) != instanceFuncs.end()) {
        return;
    }
    // check if the dll file exists
    string moduleFileName = module_path + SEP + string(dllID) + string(Tag_DyLib);
    if (!FileExists(moduleFileName)) {
        throw ModelException("ModuleFactory", "ReadDLL", moduleFileName + " does not exist or has no read permission!");
    }
    //load library
#ifdef WIN32
    HINSTANCE handle = LoadLibrary(TEXT(moduleFileName.c_str()));
    if (handle == nullptr) throw ModelException("ModuleFactory", "ReadDLL", "Could not load " + moduleFileName);
    instanceFuncs[id] = InstanceFunction(GetProcAddress(HMODULE(handle), "GetInstance"));
    metadataFuncs[id] = MetadataFunction(GetProcAddress(HMODULE(handle), "MetadataInformation"));
#else
    void *handle = dlopen(moduleFileName.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        cout << dlerror() << endl;
        throw ModelException("ModuleFactory", "ReadDLL", "Could not load " + moduleFileName);
    }
    instanceFuncs[id] = InstanceFunction(dlsym(handle, "GetInstance"));
    metadataFuncs[id] = MetadataFunction(dlsym(handle, "MetadataInformation"));
#endif /* WIN32 */
    dllHandles.push_back(handle);
    if (instanceFuncs[id] == nullptr) {
        throw ModelException("ModuleFactory", "ReadDLL",
                             moduleFileName + " does not implement API function: GetInstance");
    }
    if (metadataFuncs[id] == nullptr) {
        throw ModelException("ModuleFactory", "ReadDLL",
                             moduleFileName + " does not implement API function: MetadataInformation");
    }
    StatusMessage(("Read DLL: " + moduleFileName).c_str());
}

dimensionTypes ModuleFactory::MatchType(string strType) {
    // default
    dimensionTypes typ = DT_Unknown;
    if (StringMatch(strType, Type_Single)) typ = DT_Single;
    if (StringMatch(strType, Type_Array1D)) typ = DT_Array1D;
    if (StringMatch(strType, Type_Array2D)) typ = DT_Array2D;
    if (StringMatch(strType, Type_Array3D)) typ = DT_Array3D;
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

void ModuleFactory::ReadParameterSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting,
                                         map<string, vector<ParamInfo *> > &moduleParameters) {
    moduleParameters.insert(make_pair(moduleID, vector<ParamInfo *>()));
    vector<ParamInfo *> &vecPara = moduleParameters.at(moduleID);
    TiXmlElement *eleMetadata = doc.FirstChildElement(TagMetadata.c_str());
    // start getting the parameters
    TiXmlElement *eleParams = eleMetadata->FirstChildElement(TagParameters.c_str());
    if (eleParams != nullptr) {
        TiXmlElement *eleParam = eleParams->FirstChildElement(TagParameter.c_str());
        while (eleParam != nullptr) {
            // clear the object
            ParamInfo *param = new ParamInfo();
            // set the module id
            param->ModuleID = moduleID;
            // get the parameter name
            TiXmlElement *elItm = eleParam->FirstChildElement(TagParameterName.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Name = GetUpper(string(elItm->GetText()));
                    param->BasicName = param->Name;
                    param->ClimateType = setting->dataTypeString();

                    //set climate data type got from config.fig
                    //this is used for interpolation module
                    if (StringMatch(param->Name, Tag_DataType)) param->Value = setting->dataType();

                    //special process for interpolation modules
                    if (StringMatch(param->Name, Tag_Weight)) {
                        if (setting->dataTypeString().length() == 0) {
                            throw ModelException("ModuleFactory", "ReadParameterSetting",
                                                 "The parameter " + string(Tag_Weight) +
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

                    if (StringMatch(param->Name, Tag_VerticalInterpolation)) //if do the vertical interpolation
                    {
                        param->Value = (setting->needDoVerticalInterpolation() ? 1.0f : 0.0f);
                    }
                }
            }
            // get the parameter description
            elItm = eleParam->FirstChildElement(TagParameterDescription.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Description = elItm->GetText();
                }
            }
            // get the parameter units
            elItm = eleParam->FirstChildElement(TagParameterUnits.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Units = elItm->GetText();
                }
            }
            // get the parameter source
            elItm = eleParam->FirstChildElement(TagParameterSource.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Source = elItm->GetText();
                }
            }
            // get the parameter dimension
            elItm = eleParam->FirstChildElement(TagParameterDimension.c_str());
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
            vecPara.push_back(param);
            // get the next parameter if it exists
            eleParam = eleParam->NextSiblingElement();
        } // while
    }
}

bool ModuleFactory::IsConstantInputFromName(string &name) {
    if (StringMatch(name, CONS_IN_ELEV) ||
        StringMatch(name, CONS_IN_LAT) ||
        StringMatch(name, CONS_IN_XPR) ||
        StringMatch(name, CONS_IN_YPR)) {
        return true;
    }
    return false;
}

void ModuleFactory::ReadInputSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting,
                                     map<string, vector<ParamInfo *> > &moduleInputs) {
    moduleInputs.insert(make_pair(moduleID, vector<ParamInfo *>()));
    vector<ParamInfo *> &vecPara = moduleInputs.at(moduleID);
    TiXmlElement *eleMetadata = doc.FirstChildElement(TagMetadata.c_str());
    TiXmlElement *eleInputs = eleMetadata->FirstChildElement(TagInputs.c_str());
    if (eleInputs != nullptr) {
        TiXmlElement *elInput = eleInputs->FirstChildElement(TagInputVariable.c_str());
        while (elInput != nullptr) {
            ParamInfo *param = new ParamInfo();
            // set the module id
            param->ModuleID = moduleID;
            param->ClimateType = setting->dataTypeString();
            // get the input variable name
            TiXmlElement *elItm = elInput->FirstChildElement(TagInputVariableName.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Name = GetUpper(string(elItm->GetText()));
                    param->BasicName = param->Name;
                    param->IsConstant = IsConstantInputFromName(param->Name);
                    if (setting->dataTypeString().length() > 0) {
                        param->Name = param->Name + "_" + setting->dataTypeString();
                    }
                }
            }
            // get the input variable units(
            elItm = elInput->FirstChildElement(TagInputVariableUnits.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Units = elItm->GetText();
                }
            }
            // get the input variable description
            elItm = elInput->FirstChildElement(TagInputVariableDescription.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Description = elItm->GetText();
                }
            }
            // get the input variable source
            elItm = elInput->FirstChildElement(TagInputVariableSource.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Source = elItm->GetText();
                }
            }
            // get the input variable dimension
            elItm = elInput->FirstChildElement(TagInputVariableDimension.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Dimension = MatchType(string(elItm->GetText()));
                }
            }
            elItm = nullptr;
            // input must have these values
            if (param->Name.empty()) {
                delete param;
                throw ModelException("SEIMSModule", "ReadInputSetting",
                                     "Some input variables have not name in metadata!");
            }
            if (param->Source.empty()) {
                string name = param->Name;
                delete param;
                throw ModelException("SEIMSModule", "ReadInputSetting",
                                     "Input variable " + name + " does not have source!");
            }
            if (param->Dimension == DT_Unknown) {
                string name = param->Name;
                delete param;
                throw ModelException("SEIMSModule", "ReadInputSetting",
                                     "Input variable " + name + " does not have dimension!");
            }
            vecPara.push_back(param);
            // get the next input if it exists
            elInput = elInput->NextSiblingElement();
        }
    }
}

void ModuleFactory::ReadOutputSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting,
                                      map<string, vector<ParamInfo *> > &moduleOutputs) {
    moduleOutputs.insert(make_pair(moduleID, vector<ParamInfo *>()));
    vector<ParamInfo *> &vecPara = moduleOutputs.at(moduleID);
    TiXmlElement *eleMetadata = doc.FirstChildElement(TagMetadata.c_str());
    TiXmlElement *eleOutputs = eleMetadata->FirstChildElement(TagOutputs.c_str());
    if (eleOutputs != nullptr) {
        TiXmlElement *elOutput = eleOutputs->FirstChildElement(TagOutputVariable.c_str());
        while (elOutput != nullptr) {
            ParamInfo *param = new ParamInfo();
            // set the module id
            param->ModuleID = moduleID;
            param->ClimateType = setting->dataTypeString();
            // get the output variable name
            TiXmlElement *elItm = elOutput->FirstChildElement(TagOutputVariableName.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Name = GetUpper(string(elItm->GetText()));
                    param->BasicName = param->Name;
                    if (!setting->dataTypeString().empty()) {
                        param->Name = param->Name + "_" + setting->dataTypeString();
                    }
                }
            }
            // get the output variable units
            elItm = elOutput->FirstChildElement(TagOutputVariableUnits.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Units = elItm->GetText();
                }
            }
            // get the output variable description
            elItm = elOutput->FirstChildElement(TagOutputVariableDescription.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Description = elItm->GetText();
                }
            }
            param->Source = "";
            // get the output variable dimension
            elItm = elOutput->FirstChildElement(TagOutputVariableDimension.c_str());
            if (elItm != nullptr) {
                if (elItm->GetText() != nullptr) {
                    param->Dimension = MatchType(string(elItm->GetText()));
                }
            }
            elItm = nullptr;
            // add to the list
            param->IsOutput = true;
            // output variable must have these values
            if (param->Name.empty()) {
                delete param;
                throw ModelException("SEIMSModule", "readOutputSetting",
                                     "Some output variables have not name in metadata!");
            }
            if (param->Dimension == DT_Unknown) {
                string name = param->Name;
                delete param;
                throw ModelException("SEIMSModule", "readInputSetting",
                                     "Input variable " + name + " does not have dimension!");
            }
            vecPara.push_back(param);
            // get the next input if it exists
            elOutput = elOutput->NextSiblingElement();
        }
    }
}

bool ModuleFactory::LoadSettingsFromFile(const char *filename, vector<vector<string> > &settings) {
    vector<string> cfgStrs;
    if (!LoadPlainTextFile(filename, cfgStrs)) {
        return false;
    }
    string T_variables[7] = {DataType_Precipitation, DataType_MeanTemperature, DataType_MaximumTemperature,
                             DataType_MinimumTemperature, DataType_SolarRadiation, DataType_WindSpeed,
                             DataType_RelativeAirMoisture};
    for (auto iter = cfgStrs.begin(); iter != cfgStrs.end(); iter++) {
        // parse the line into separate item
        vector<string> tokens = SplitString(*iter, '|');
        // is there anything in the token list?
        if (tokens.empty()) continue;
        for (size_t i = 0; i < tokens.size(); i++) {
            //TrimSpaces(tokens[i]);
            tokens[i] = trim(tokens[i]);
        }
        if (tokens.empty()) continue;
        // is there anything in the first item? Or, the size of tokens greater than 4?
        if (tokens[0].empty() || tokens.size() < 4) continue;
        // there is something to add so resize the header list to append it
        size_t sz = settings.size(); // get the current number of rows
        if (tokens[3].find(MID_ITP) != string::npos ||
            tokens[3].find(MID_TSD_RD) != string::npos) {
            settings.resize(sz + 7);
            for (size_t j = 0; j < 7; j++) {
                vector<string> tokensTemp(tokens);
                tokensTemp[1] += "_" + T_variables[j];
                if (tokens[3].find(MID_ITP) != string::npos) {
                    vector<string> ITPProperty = SplitString(*iter, '_');
                    if (ITPProperty.size() == 2) {
                        int isVertical = atoi(ITPProperty[1].c_str());
                        if (isVertical) {
                            tokensTemp[1] += "_1";
                        } else {
                            tokensTemp[1] += "_0";
                        }
                    }
                }
                settings[sz + j] = tokensTemp;
            }
        } else {
            settings.resize(sz + 1);        // resize with one more row
            settings[sz] = tokens;
        }
    }
    return true;
}

bool ModuleFactory::ReadConfigFile(const char *configFileName, vector<string> &moduleIDs,
                                   map<string, SEIMSModuleSetting *> &moduleSettings) {
    vector<vector<string> > settings;
    if (!LoadSettingsFromFile(configFileName, settings)) return false;
    try {
        for (size_t i = 0; i < settings.size(); i++) {
            if (settings[i].size() > 3) {
                string settingString = settings[i][1];
                string module = GetUpper(settings[i][3]);
#ifndef MSVC
                module.insert(0, Tag_So);
#endif /* MSVC */
                module += POSTFIX;

                SEIMSModuleSetting *moduleSetting = new SEIMSModuleSetting(module, settingString);
                if (moduleSetting->dataTypeString().length() > 0) {
                    module += "_" + moduleSetting->dataTypeString();
                } // make the module id unique
                if (!moduleSettings.insert(make_pair(module, moduleSetting)).second) {
                    delete moduleSetting;
                    continue;
                }
                moduleIDs.push_back(module);
            }
        }
    }
    catch (...) {
        cout << "ReadConfigFile failed, please contact the developers!" << endl;
        return false;
    }
    return true;
}

/// Revised LiangJun Zhu
/// 1. Fix code of DT_Raster2D related, 2016-5-27
/// 2. Bugs fixed in continuous dependency, 2016-9-6
void ModuleFactory::GetValueFromDependencyModule(int iModule, vector<SimulationModule *> &modules) {
    size_t n = m_moduleIDs.size();
    string id = m_moduleIDs[iModule];
    vector<ParamInfo *> &inputs = m_moduleInputs[id];
    /// if there are no inputs from other modules for current module
    for (auto it = inputs.begin(); it != inputs.end(); it++) {
        ParamInfo *param = *it;
        if (StringMatch(param->Source, Source_Module) ||
            (StringMatch(param->Source, Source_Module_Optional) && param->DependPara != nullptr)) {
            break;
        }
        modules[iModule]->SetInputsDone(true);
        return;
    }

    for (size_t j = 0; j < inputs.size(); j++) {
        ParamInfo *dependParam = inputs[j]->DependPara;
        if (dependParam == nullptr) {
            continue;
        }

        size_t k = 0;
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
            float *data;
            modules[k]->Get1DData(compareName.c_str(), &dataLen, &data);
            modules[iModule]->Set1DData(inputs[j]->Name.c_str(), dataLen, data);
            dependParam->initialized = true;
            modules[iModule]->SetInputsDone(true);
        } else if (dependParam->Dimension == DT_Array2D || dependParam->Dimension == DT_Raster2D) {
            int nCol;
            float **data;
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
            ostringstream oss;
            oss << "Dimension type: " << dependParam->Dimension << " is currently not supported.";
            throw ModelException("ModuleFactory", "GetValueFromDependencyModule", oss.str());
        }
    }
}

void ModuleFactory::FindOutputParameter(string &outputID, int &iModule, ParamInfo *&paraInfo) {
    size_t n = m_moduleIDs.size();
    for (size_t i = 0; i < n; i++) {
        string id = m_moduleIDs[i];
        vector<ParamInfo *> &vecPara = m_moduleOutputs[id];
        for (size_t j = 0; j < vecPara.size(); j++) {
            if (StringMatch(outputID, vecPara[j]->Name)) {
                iModule = (int) i;
                paraInfo = vecPara[j];
                return;
            }
        }
    }
}
