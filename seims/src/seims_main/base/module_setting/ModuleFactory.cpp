#include "ModuleFactory.h"

ModuleFactory::ModuleFactory(DataCenterMongoDB* dataCenter) :m_dataCenter(dataCenter),
                                                            m_parametersInDB(m_dataCenter->getInitParameters()),
                                                            m_rsMap(dataCenter->getRasterDataMap()),
                                                            m_1DArrayMap(m_dataCenter->get1DArrayMap()),
                                                            m_1DLenMap(m_dataCenter->get1DArrayLenMap()),
                                                            m_2DArrayMap(m_dataCenter->get2DArrayMap()),
                                                            m_2DRowsLenMap(m_dataCenter->get2DArrayRowsMap()),
                                                            m_2DColsLenMap(m_dataCenter->get2DArrayColsMap()),
                                                            m_weightDataMap(m_dataCenter->getItpWeightDataMap())
{
    m_dbName = m_dataCenter->getModelName();
    m_moduleCfgFile = m_dataCenter->getFileCfgFullPath();
    m_modulePath = m_dataCenter->getModulePath();
    m_subBasinID = m_dataCenter->getSubbasinID();
    m_layingMethod = m_dataCenter->getLayeringMethod();

    m_setingsInput = m_dataCenter->getSettingInput();
    m_scenario = m_dataCenter->getScenarioData();
    m_reaches = m_dataCenter->getReachesData();
    m_subbasins = m_dataCenter->getSubbasinData();
    m_climStation = m_dataCenter->getClimateStation();
    m_maskRaster = m_dataCenter->getMaskData();

    Init(m_dataCenter->getFileCfgFullPath());
}
ModuleFactory::~ModuleFactory() {
    StatusMessage("Start to release ModuleFactory ...");
    /// Improved by Liangjun, 2016-7-6
    /// First release memory, then erase map element. BE CAUTION WHEN USE ERASE!!!
    StatusMessage("---release map of SEIMSModuleSettings ...");
    for (auto it = m_settings.begin(); it != m_settings.end();) {
        if (nullptr!=it->second) {
            StatusMessage(("-----" + it->first + " ...").c_str());
            delete it->second;
            it->second = nullptr;
        }
        m_settings.erase(it++);
    }
    m_settings.clear();

    StatusMessage("---release map of metadata of modules ...");
    for (auto it = m_metadata.begin(); it != m_metadata.end();) {
        if (nullptr!=it->second) {
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
    StatusMessage("End to release ModuleFactory ...");
}

void ModuleFactory::Init(const string &configFileName) {
    ReadConfigFile(configFileName.c_str());

    size_t n = m_moduleIDs.size();
    // read all the .dll or .so and create objects
    for (size_t i = 0; i < n; i++) {
        string id = m_moduleIDs[i];
        string dllID = id;
        // for ITP modules, the input ids are ITP_T, ITP_P and ITP should be used as ID name
        // The same to TSD_RD.
        if (id.find(MID_ITP) != string::npos) {
#ifdef MSVC
            dllID = MID_ITP;
#else
            dllID = Tag_So + string(MID_ITP);

#ifndef NDEBUG
            dllID = dllID + "d";
#endif /* NDEBUG */
#endif /* MSVC */
        } else if (id.find(MID_TSD_RD) != string::npos) {
#ifdef MSVC
            dllID = MID_TSD_RD;
#else
            dllID = Tag_So + string(MID_TSD_RD);

#ifndef NDEBUG
            dllID = dllID + "d";
#endif /* NDEBUG */
#endif /* MSVC */
        }

        // load function pointers from DLL
        ReadDLL(id, dllID);

        // load metadata
        MetadataFunction metadataInfo = m_metadataFuncs[id];
        const char *metadata = metadataInfo();
        m_metadata[id] = metadata;
        // parse the metadata
        TiXmlDocument doc;
        doc.Parse(metadata);
        ReadParameterSetting(id, doc, m_settings[id]);
        ReadInputSetting(id, doc, m_settings[id]);
        ReadOutputSetting(id, doc, m_settings[id]);
    }
    map<string, vector<ParamInfo> >(m_moduleParameters).swap(m_moduleParameters);
    map<string, vector<ParamInfo> >(m_moduleInputs).swap(m_moduleInputs);
    map<string, vector<ParamInfo> >(m_moduleOutputs).swap(m_moduleOutputs);
    // set the connections among objects
    for (size_t i = 0; i < n; i++) {
        string id = m_moduleIDs[i];
        //cout << id << endl;
        vector <ParamInfo> &inputs = m_moduleInputs[id];
        for (size_t j = 0; j < inputs.size(); j++) {
            ParamInfo &param = inputs[j];
            if (StringMatch(param.Source, Source_Module) || StringMatch(param.Source, Source_Module_Optional)) {
                param.DependPara = FindDependentParam(param);
                //cout << "\t" << param.Name << "\t" << param.DependPara->ModuleID << ":" << param.DependPara->Name << endl;
            } else {
                continue;
            }
        }
    }
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

float ModuleFactory::CreateModuleList(vector<SimulationModule *> &modules) {
    size_t n = m_moduleIDs.size();
    for (size_t i = 0; i < n; i++) {
        SimulationModule *pModule = GetInstance(m_moduleIDs[i]);
        pModule->SetTheadNumber(m_dataCenter->getThreadNumber());
        modules.push_back(pModule);
    }
    double t1 = TimeCounting();
    /// initial parameter (reading parameter information from database)
    //cout << "reading parameter information from database...\n";
    for (size_t i = 0; i < n; i++) {
        string id = m_moduleIDs[i];
        vector<ParamInfo> &parameters = m_moduleParameters[id];

        bool verticalInterpolation = true;
        /// Special operation for ITP module
        if (id.find(MID_ITP) != string::npos) {
            modules[i]->SetClimateDataType(m_settings[id]->dataType());
            for (size_t j = 0; j < parameters.size(); j++) {
                ParamInfo &param = parameters[j];
                if (StringMatch(param.Name, Tag_VerticalInterpolation)) {
                    if (param.Value > 0.f) {
                        verticalInterpolation = true;
                    } else {
                        verticalInterpolation = false;
                    }
                    break;
                }
            }
        }
        for (size_t j = 0; j < parameters.size(); j++) {
            ParamInfo &param = parameters[j];
            SetData(m_dbName, m_subBasinID, m_settings[id], &param, m_maskRaster, modules[i],
                    verticalInterpolation);
        }
    }
    double t2 = TimeCounting();
    float timeconsume = float(t2 - t1);
    StatusMessage(("Reading parameter finished, TIMESPAN " + ValueToString(timeconsume) + " sec.").c_str());
    return float(t2 - t1);
}

ParamInfo *ModuleFactory::FindDependentParam(ParamInfo &paramInfo) {
    string paraName = GetComparableName(paramInfo.Name);
    dimensionTypes paraType = paramInfo.Dimension;

    size_t n = m_moduleIDs.size();
    for (size_t i = 0; i < n; i++) // module ID
    {
        string id = m_moduleIDs[i];
        vector <ParamInfo> &outputs = m_moduleOutputs[id];
        for (size_t j = 0; j < outputs.size(); j++) // output ID
        {
            ParamInfo &param = outputs[j];
            string compareName = GetComparableName(param.Name);
            if (StringMatch(paraName, compareName) && param.Dimension == paraType) {
                param.OutputToOthers = true;
                return &param;
            }
        }
    }
    if (!StringMatch(paramInfo.Source, Source_Module_Optional)) {
        throw ModelException("ModuleFactory", "FindDependentParam",
                             "Can not find input for " + paraName + " of " + paramInfo.ModuleID +
                                 " from other Modules.\n");
    }
    return nullptr;
}

void ModuleFactory::ReadDLL(string &id, string &dllID) {
    // the dll file is already read, return
    if (m_instanceFuncs.find(id) != m_instanceFuncs.end()) {
        return;
    }
    // check if the dll file exists
    string moduleFileName = trim(m_modulePath) + SEP + string(dllID) + string(Tag_DyLib);
    if (!FileExists(moduleFileName)) {
        throw ModelException("ModuleFactory", "ReadDLL", moduleFileName + " does not exist or has no read permission!");
    }

    //load library
#ifdef WIN32
    HINSTANCE handle = LoadLibrary(TEXT(moduleFileName.c_str()));
    if (handle == nullptr) throw ModelException("ModuleFactory", "ReadDLL", "Could not load " + moduleFileName);
    m_instanceFuncs[id] = InstanceFunction(GetProcAddress(HMODULE(handle), "GetInstance"));
    m_metadataFuncs[id] = MetadataFunction(GetProcAddress(HMODULE(handle), "MetadataInformation"));
#else
    void *handle = dlopen(moduleFileName.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
        cout << dlerror() << endl;
        throw ModelException("ModuleFactory", "ReadDLL", "Could not load " + moduleFileName);
    }
    m_instanceFuncs[id] = InstanceFunction(dlsym(handle, "GetInstance"));
    m_metadataFuncs[id] = MetadataFunction(dlsym(handle, "MetadataInformation"));
#endif /* WIN32 */
    m_dllHandles.push_back(handle);
    if (m_instanceFuncs[id] == nullptr) {
        throw ModelException("ModuleFactory", "ReadDLL",
                             moduleFileName + " does not implement API function: GetInstance");
    }
    if (m_metadataFuncs[id] == nullptr) {
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

void ModuleFactory::ReadParameterSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting) {
    m_moduleParameters.insert(make_pair(moduleID, vector<ParamInfo>()));
    vector<ParamInfo> &vecPara = m_moduleParameters.at(moduleID);

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
                            StringMatch(setting->dataTypeString(), DataType_MaximumTemperature)) 
                        {
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
            if (param->Name.size() <= 0) {
                delete param;
                throw ModelException("ModuleFactory", "ReadParameterSetting",
                                     "Some parameters have not name in metadata!");
            }

            if (param->Source.size() <= 0) {
                string name = param->Name;
                delete param;
                throw ModelException("ModuleFactory", "ReadParameterSetting",
                                     "parameter " + name + " does not have source!");
            }

            if (param->Dimension == DT_Unknown) {
                string name = param->Name;
                delete param;
                throw ModelException("ModuleFactory", "ReadParameterSetting",
                                     "parameter " + name + " does not have dimension!");
            }

            // add to the list
            //m_paramters[GetUpper(param->Name)] = param;
            vecPara.push_back(*param);
            delete param;

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

void ModuleFactory::ReadInputSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting) {
    m_moduleInputs.insert(make_pair(moduleID, vector<ParamInfo>()));
    vector <ParamInfo> &vecPara = m_moduleInputs.at(moduleID);

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
            if (param->Name.size() <= 0) {
                delete param;
                throw ModelException("SEIMSModule", "ReadInputSetting",
                                     "Some input variables have not name in metadata!");
            }

            if (param->Source.size() <= 0) {
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

            vecPara.push_back(*param);
            delete param;

            // get the next input if it exists
            elInput = elInput->NextSiblingElement();
        }
    }
}

void ModuleFactory::ReadOutputSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting) {
    m_moduleOutputs.insert(make_pair(moduleID, vector<ParamInfo>()));
    vector <ParamInfo> &vecPara = m_moduleOutputs.at(moduleID);

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
                    if (setting->dataTypeString().size() > 0) {
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
            if (param->Name.size() <= 0) {
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

            vecPara.push_back(*param);
            delete param;
            // get the next input if it exists
            elOutput = elOutput->NextSiblingElement();
        }
    }
}

bool ModuleFactory::LoadSettingsFromFile(const char* filename, vector<vector<string>>& settings) {
    vector<string> cfgStrs;
    if (!LoadPlainTextFile(filename, cfgStrs)) {
        return false;
    }
    string T_variables[7] = {DataType_Precipitation, DataType_MeanTemperature, DataType_MaximumTemperature,
                             DataType_MinimumTemperature, DataType_SolarRadiation, DataType_WindSpeed,
                             DataType_RelativeAirMoisture};
    for (vector<string>::iterator iter = cfgStrs.begin(); iter != cfgStrs.end(); ++iter) {
        // parse the line into separate item
        vector<string> tokens = SplitString(*iter, '|');
        // is there anything in the token list?
        if (tokens.size() > 0) {
            for (size_t i = 0; i < tokens.size(); i++) {
                //TrimSpaces(tokens[i]);
                tokens[i] = trim(tokens[i]);
            }
            // is there anything in the first item?
            if (tokens[0].size() > 0) {
                // there is something to add so resize the header list to append it
                int sz = settings.size(); // get the current number of rows
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
                                }
                                else {
                                    tokensTemp[1] += "_0";
                                }
                            }
                        }
                        settings[sz + j] = tokensTemp;
                    }
                }
                else {
                    settings.resize(sz + 1);        // resize with one more row
                    settings[sz] = tokens;
                }
            } // if there is nothing in the first item of the token list there is nothing to add to the header list
        }
    }
    return true;
}

void ModuleFactory::ReadConfigFile(const char *configFileName) {
    vector<vector<string> > settings;
    LoadSettingsFromFile(configFileName, settings);

    try {
        for (size_t i = 0; i < settings.size(); i++) {
            if (settings[i].size() > 3) {
                string settingString = settings[i][1];
                string module = GetUpper(settings[i][3]);
#ifndef MSVC
                module = Tag_So + module;

#ifndef NDEBUG
                module = module + "d";
#endif /* NDEBUG */
#endif /* MSVC */

                SEIMSModuleSetting *moduleSetting = new SEIMSModuleSetting(module, settingString);
                if (moduleSetting->dataTypeString().length() > 0) {
                    module += "_" + moduleSetting->dataTypeString();
                } // make the module id unique

                m_moduleIDs.push_back(module);
                //m_settings[module] = moduleSetting;
                if (!m_settings.insert(make_pair(module, moduleSetting)).second) {
                    delete moduleSetting;
                }
            }
        }
    }
    catch (...) {
        cout <<"ReadConfigFile Failed!" << endl;
        throw;
    }
}

void ModuleFactory::SetData(string &dbName, int nSubbasin, SEIMSModuleSetting *setting, ParamInfo *param,
                            FloatRaster *templateRaster, SimulationModule *pModule, bool vertitalItp) {
#ifdef _DEBUG
    double stime = TimeCounting();
#endif
    string name = param->BasicName;
    if (setting->dataTypeString().size() == 0
        && !StringMatch(param->BasicName, CONS_IN_ELEV)
        && !StringMatch(param->BasicName, CONS_IN_LAT)
        && !StringMatch(param->BasicName, CONS_IN_XPR)
        && !StringMatch(param->BasicName, CONS_IN_YPR)) {
        name = param->Name;
        //cout << param->Name << " " << param->BasicName << endl;
    }
    ostringstream oss;
    int tmp = name.find("LOOKUP");
    if (tmp < 0) {
        oss << nSubbasin << "_" << name;
    } else {
        oss << name;
    }
    if (StringMatch(name, Tag_Weight)) {
        if (setting->dataTypeString() == DataType_Precipitation) {
            oss << "_P";
        } else {
            oss << "_M";
        }
    }
    string remoteFileName = oss.str();

    switch (param->Dimension) {
        case DT_Unknown:throw ModelException("ModuleFactory", "SetData", "Type of " + param->Name + " is unknown.");
            break;
        case DT_Single:SetValue(param, templateRaster, pModule);
            break;
        case DT_Array1D:Set1DData(dbName, name, remoteFileName, templateRaster, pModule, vertitalItp);
            break;
        case DT_Array2D:Set2DData(dbName, name, nSubbasin, remoteFileName, templateRaster, pModule);
            break;
        case DT_Array3D:break;
        case DT_Array1DDateValue:break;
        case DT_Raster1D:SetRaster(dbName, name, remoteFileName, templateRaster, pModule);
            break;
        case DT_Raster2D:SetRaster(dbName, name, remoteFileName, templateRaster, pModule);
            break;
        case DT_Scenario:SetScenario(pModule);
            break;
        case DT_Reach:SetReaches(pModule);
            break;
        case DT_Subbasin:SetSubbasins(pModule);
            break;
        default:break;
    }
#ifdef _DEBUG
    float timeconsume = float(TimeCounting() - stime);
    StatusMessage(("Set " + name + " data done, TIMESPAN " + ValueToString(timeconsume) + " sec.").c_str());
#endif
}

void ModuleFactory::SetValue(ParamInfo *param, FloatRaster *templateRaster, SimulationModule *pModule) {
    if (StringMatch(param->Name, Tag_DataType)) {
        // the data type is got from config.fig
        return;
    } else if (StringMatch(param->Name, Tag_CellSize))  // valid cells number, do not be confused with Tag_CellWidth
    {
        param->Value = float(templateRaster->getCellNumber()); // old code is ->Size();  they have the same function
    } else if (StringMatch(param->Name, Tag_CellWidth))  //cell size
    {
        param->Value = float(templateRaster->getCellWidth());
    } else if (StringMatch(param->Name, Tag_TimeStep)) {
        param->Value = float(m_setingsInput->getDtDaily()); // return 86400 secs
    } else if (StringMatch(param->Name, Tag_HillSlopeTimeStep)) {
        param->Value = float(m_setingsInput->getDtHillslope());
    } else if (StringMatch(param->Name, Tag_ChannelTimeStep)) {
        param->Value = float(m_setingsInput->getDtChannel());
    } else if (StringMatch(param->Name, Tag_LayeringMethod)) {
        param->Value = (float)m_layingMethod;
    } else {
        if (m_parametersInDB.find(GetUpper(param->Name)) != m_parametersInDB.end()) {
            param->Value = m_parametersInDB[GetUpper(param->Name)]->GetAdjustedValue();
        }
    }

    pModule->SetValue(param->Name.c_str(), param->Value);
}

void ModuleFactory::Set1DData(string &dbName, string &paraName, string &remoteFileName, 
                              FloatRaster *templateRaster, SimulationModule *pModule, bool vertitalItp) {
    int n;
    float *data = nullptr;
    /// the data has been read before, which stored in m_1DArrayMap
    if (m_1DArrayMap.find(remoteFileName) != m_1DArrayMap.end()) {
        data = m_1DArrayMap.at(remoteFileName);
        pModule->Set1DData(paraName.c_str(), m_1DLenMap.at(remoteFileName), data);
        return;
    }
    else if (m_weightDataMap.find(remoteFileName) != m_weightDataMap.end()) {
        clsITPWeightData *weightData = m_weightDataMap.at(remoteFileName);
        weightData->getWeightData(&n, &data);
        pModule->Set1DData(paraName.c_str(), n, data);
        return;
    }
    /// the data has not been read yet.
    /// 1. IF FLOWOUT_INDEX_D8
    if (StringMatch(paraName, Tag_FLOWOUT_INDEX_D8)) {
        m_dataCenter->read1DArrayData(paraName, remoteFileName, n, data);
        if (templateRaster->getCellNumber() != n) {
            throw ModelException("ModuleFactory", "Set1DArray",
                "The data length derived from Read1DArray in " + remoteFileName
                + " is not the same as the template.");
        }
    }
    /// 2. IF Weight data
    else if (StringMatch(paraName, Tag_Weight)) {
        m_dataCenter->readItpWeightData(remoteFileName, n, data);
    }
    /// 3. IF Meteorology sites data
    else if (StringMatch(paraName, Tag_Elevation_Meteorology)) {
        if (vertitalItp) {
            n = m_climStation->NumberOfSites(DataType_Meteorology);
            data = m_climStation->GetElevation(DataType_Meteorology);
        } else {
            return;
        }
    }
    /// 4. IF Precipitation sites data
    else if (StringMatch(paraName, Tag_Elevation_Precipitation)) {
        if (vertitalItp) {
            n = m_climStation->NumberOfSites(DataType_Precipitation);
            data = m_climStation->GetElevation(DataType_Precipitation);
        } else {
            return;
        }
    }
    /// 5. IF Latitude of sites
    else if (StringMatch(paraName, Tag_Latitude_Meteorology)) {
        if (vertitalItp) {
            n = m_climStation->NumberOfSites(DataType_Meteorology);
            data = m_climStation->GetLatitude(DataType_Meteorology);
        } else {
            return;
        }
    }
    /// 6. IF any other 1D arrays, such as Heat units of all simulation years (HUTOT)
    else {
        m_dataCenter->read1DArrayData(paraName, remoteFileName, n, data);
    }

    if (nullptr == data) {
        throw ModelException("ModuleFactory", "Set1DData", "Failed reading file " + remoteFileName);
    }

    pModule->Set1DData(paraName.c_str(), n, data);
}

void ModuleFactory::Set2DData(string &dbName, string &paraName, int nSubbasin, string &remoteFileName,
                              FloatRaster *templateRaster, SimulationModule *pModule) {
    int nRows = 0;
    int nCols = 1;
    float **data;
    /// Get ROUTING_LAYERS real file name
    if (StringMatch(paraName, Tag_ROUTING_LAYERS)) {
        ostringstream oss;
        if (m_layingMethod == UP_DOWN) {
            oss << remoteFileName << "_UP_DOWN";
            remoteFileName = oss.str();
        }
        else {
            oss << remoteFileName << "_DOWN_UP";
            remoteFileName = oss.str();
        }
    }
    /// Check if the data is already loaded
    if (m_2DArrayMap.find(remoteFileName) != m_2DArrayMap.end()) {
        data = m_2DArrayMap.at(remoteFileName);
        nRows = m_2DRowsLenMap.at(remoteFileName);
        nCols = m_2DColsLenMap.at(remoteFileName);

        pModule->Set2DData(paraName.c_str(), nRows, nCols, data);
        return;
    }
    /// Load data from DataCenter
    if (StringMatch(paraName, Tag_ROUTING_LAYERS) || StringMatch(paraName, Tag_ROUTING_LAYERS_DINF)){
        // Routing layering data based on different flow model
        m_dataCenter->read2DArrayData(remoteFileName, nRows, nCols, data);
    } else if (StringMatch(paraName, Tag_FLOWIN_INDEX_D8) || StringMatch(paraName, Tag_FLOWIN_INDEX_DINF)
        || StringMatch(paraName, Tag_FLOWIN_PERCENTAGE_DINF) || StringMatch(paraName, Tag_FLOWOUT_INDEX_DINF)) {
        // Flow in and flow out data based on different flow model
        m_dataCenter->read2DArrayData(remoteFileName, nRows, nCols, data);
    } else if (StringMatch(paraName, TAG_OUT_OL_IUH)) { // Overland flow IUH
        m_dataCenter->readIUHData(remoteFileName, nRows, data);
    } else if (StringMatch(paraName, Tag_LapseRate)) /// Match to the format of DT_Array2D, By LJ.
    {
        m_dataCenter->setLapseData(remoteFileName, nRows, nCols, data);
    } else {
        m_dataCenter->read2DArrayData(remoteFileName, nRows, nCols, data);
    }

    pModule->Set2DData(paraName.c_str(), nRows, nCols, data);
}

void ModuleFactory::SetRaster(string &dbName, string &paraName, string &remoteFileName,
                              FloatRaster *templateRaster, SimulationModule *pModule) {
    int n, lyrs;
    float *data = nullptr;
    float **data2D = nullptr;
    FloatRaster *raster = nullptr;
    if (m_rsMap.find(remoteFileName) == m_rsMap.end()) {
        raster = m_dataCenter->readRasterData(remoteFileName);
        if (nullptr == raster) {
            throw ModelException("ModuleFactory", "SetRaster", "Load " + remoteFileName + " failed!");
        }
        string upperName = GetUpper(paraName);
        auto find_iter = m_parametersInDB.find(upperName);
        bool adjust_data = false;
        if (find_iter != m_parametersInDB.end()) {
            ParamInfo* tmpParam = find_iter->second;
            if ((StringMatch(tmpParam->Change, PARAM_CHANGE_RC) && !FloatEqual(tmpParam->Impact, 1.f)) ||
                (StringMatch(tmpParam->Change, PARAM_CHANGE_AC) && !FloatEqual(tmpParam->Impact, 0.f)) ||
                (StringMatch(tmpParam->Change, PARAM_CHANGE_VC) && !FloatEqual(tmpParam->Impact, NODATA_VALUE))) {
                adjust_data = true;
            }
        }
        /// 1D or 2D raster data
        if (raster->is2DRaster())
        {
            if (!raster->get2DRasterData(&n, &lyrs, &data2D)) {
                throw ModelException("ModuleFactory", "SetRaster", "Load " + remoteFileName + " failed!");
            }
            if (nullptr != data2D && adjust_data) {
                m_parametersInDB[upperName]->Adjust2DRaster(n, raster->getLayers(), data2D);
            }
        }
        else
        {
            if (!raster->getRasterData(&n, &data)) {
                throw ModelException("ModuleFactory", "SetRaster", "Load " + remoteFileName + " failed!");
            }
            if (nullptr != data && adjust_data) {
                m_parametersInDB[upperName]->Adjust1DRaster(n, data);
            }
        }
    } else {
        raster = m_rsMap.at(remoteFileName);
        //cout << remoteFileName << endl;
    }
    if (raster->is2DRaster()) {
        if (!raster->get2DRasterData(&n, &lyrs, &data2D)) {
            throw ModelException("ModuleFactory", "SetRaster", "Load " + remoteFileName + " failed!");
        }
        string upperName = GetUpper(paraName);
        pModule->Set2DData(paraName.c_str(), n, lyrs, data2D);
    } else {
        if (!raster->getRasterData(&n, &data)) {
            throw ModelException("ModuleFactory", "SetRaster", "Load " + remoteFileName + " failed!");
        }
        string upperName = GetUpper(paraName);
        pModule->Set1DData(paraName.c_str(), n, data);
    }
}

/// Added by Liang-Jun Zhu, 2016-6-22
void ModuleFactory::SetScenario(SimulationModule *pModule) {
    if (nullptr == m_scenario && nullptr == m_dataCenter->getScenarioData()) {
        throw ModelException("ModuleFactory", "SetScenario", "Scenarios has not been set!");;
    } else {
        pModule->SetScenario(m_scenario);
    }
}

/// Added by Liang-Jun Zhu, 2016-7-2
void ModuleFactory::SetReaches(SimulationModule *pModule) {
    if (nullptr == m_reaches && nullptr == m_dataCenter->getReachesData()) {
        throw ModelException("ModuleFactory", "SetReaches", "Reaches has not been set!");
    }
    pModule->SetReaches(m_reaches);
}


/// Added by Liang-Jun Zhu, 2016-7-28
void ModuleFactory::SetSubbasins(SimulationModule *pModule) {
    if (nullptr == m_subbasins && nullptr == m_dataCenter->getSubbasinData()) {
        throw ModelException("ModuleFactory", "SetSubbasins", "Subbasins data has not been initialized!");
    }
    pModule->SetSubbasins(m_subbasins);
}

void ModuleFactory::UpdateInput(vector<SimulationModule * > &modules, time_t t) {
    size_t n = m_moduleIDs.size();
    string id;
    SimulationModule* pModule;
    for (size_t i = 0; i < n; i++) {
        id = m_moduleIDs[i];
        pModule = modules[i];
        vector<ParamInfo>& inputs = m_moduleInputs[id];
        string dataType = m_settings[id]->dataTypeString();
        for (size_t j = 0; j < inputs.size(); j++) {
            ParamInfo& param = inputs[j];
            if (param.DependPara != nullptr)
                continue;    //the input which comes from other modules will not change when the date is change.
            if (StringMatch(param.Name.c_str(), CONS_IN_ELEV)
                || StringMatch(param.Name.c_str(), CONS_IN_LAT)
                || StringMatch(param.Name.c_str(), CONS_IN_XPR)
                || StringMatch(param.Name.c_str(), CONS_IN_YPR))
                continue;
            if (dataType.length() > 0) {
                int datalen;
                float *data;
                m_climStation->GetTimeSeriesData(t, dataType, &datalen, &data);
                if (StringMatch(param.Name.c_str(), DataType_PotentialEvapotranspiration)) {
                    for (int iData = 0; iData < datalen; iData++) {
                        data[iData] *= m_parametersInDB[VAR_K_PET]->GetAdjustedValue();
                    }
                }
                pModule->Set1DData(DataType_Prefix_TS, datalen, data);
            }
        }
    }
}

/// Revised LiangJun Zhu
/// 1. Fix code of DT_Raster2D related, 2016-5-27
/// 2. Bugs fixed in continuous dependency, 2016-9-6
void ModuleFactory::GetValueFromDependencyModule(int iModule, vector<SimulationModule *> &modules) {
    size_t n = m_moduleIDs.size();
    string id = m_moduleIDs[iModule];
    vector <ParamInfo> &inputs = m_moduleInputs[id];
    /// if there are no inputs from other modules for current module
    for (vector<ParamInfo>::iterator it = inputs.begin(); it != inputs.end(); it++) {
        ParamInfo &param = *it;
        if (StringMatch(param.Source, Source_Module) ||
            (StringMatch(param.Source, Source_Module_Optional) && param.DependPara != nullptr)) {
            break;
        }
        modules[iModule]->SetInputsDone(true);
        return;
    }

    for (size_t j = 0; j < inputs.size(); j++) {
        ParamInfo *dependParam = inputs[j].DependPara;
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
            modules[iModule]->Set1DData(inputs[j].Name.c_str(), dataLen, data);
            dependParam->initialized = true;
            modules[iModule]->SetInputsDone(true);
        } else if (dependParam->Dimension == DT_Array2D || dependParam->Dimension == DT_Raster2D) {
            int nCol;
            float **data;
            modules[k]->Get2DData(compareName.c_str(), &dataLen, &nCol, &data);
            modules[iModule]->Set2DData(inputs[j].Name.c_str(), dataLen, nCol, data);
            dependParam->initialized = true;
            modules[iModule]->SetInputsDone(true);
        } else if (dependParam->Dimension == DT_Single) {
            float value;
            modules[k]->GetValue(compareName.c_str(), &value);
            modules[iModule]->SetValue(inputs[j].Name.c_str(), value);
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
    string compareName = outputID;
    size_t n = m_moduleIDs.size();
    for (size_t i = 0; i < n; i++) {
        string id = m_moduleIDs[i];
        vector <ParamInfo> &vecPara = m_moduleOutputs[id];
        for (size_t j = 0; j < vecPara.size(); j++) {
            if (StringMatch(compareName, vecPara[j].Name)) {
                iModule = i;
                paraInfo = &vecPara[j];
                return;
            }
        }
    }
}
/// added by Huiran GAO, Feb. 2017
/// redesigned by Liangjun Zhu, 08/16/17
void ModuleFactory::updateParametersByScenario(int subbsnID)
{
    if (nullptr == m_scenario) {
        return;
    }
    map<int, BMPFactory *> bmpFactories = m_scenario->GetBMPFactories();
    for (auto iter = bmpFactories.begin(); iter != bmpFactories.end(); iter++)
    {
        /// Key is uniqueBMPID, which is calculated by BMP_ID * 100000 + subScenario;
        if (iter->first / 100000 != BMP_TYPE_AREALSTRUCT) {
            continue;
        }
        cout << "Update parameters by Scenario settings." << endl;
        map<int, BMPArealStruct*> arealbmps = ((BMPArealStructFactory*)iter->second)->getBMPsSettings();
        float* mgtunits = ((BMPArealStructFactory*)iter->second)->getRasterData();
        vector<int> selIDs = ((BMPArealStructFactory*)iter->second)->getUnitIDs();
        /// Get landuse data of current subbasin ("0_" for the whole basin)
        string lur = GetUpper(ValueToString(subbsnID) + "_" + VAR_LANDUSE);
        int nsize = -1;
        float* ludata = nullptr;
        m_rsMap[lur]->getRasterData(&nsize, &ludata);

        map<int, BMPArealStruct*>::iterator iter2;
        for (iter2 = arealbmps.begin();iter2 != arealbmps.end(); iter2++) {
            cout << "  - SubScenario ID: "<< iter->second->GetSubScenarioId() << ", BMP name: " 
                << iter2->second->getBMPName() << endl;
            vector<int> suitablelu = iter2->second->getSuitableLanduse();
            map<string, ParamInfo*> updateparams = iter2->second->getParameters();
            map<string, ParamInfo*>::iterator iter3;
            for (iter3 = updateparams.begin(); iter3 != updateparams.end(); iter3++) {
                string paraname = iter3->second->Name;
                cout << "   -- Parameter ID: " << paraname << endl;
                /// Check whether the parameter is existed in m_parametersInDB.
                ///   If existed, update the missing values, otherwise, print warning message and continue.
                if (m_parametersInDB.find(paraname) == m_parametersInDB.end()) {
                    cout << "      Warning: the parameter is not defined in PARAMETER table, and "
                        " will not work as expected." << endl;
                    continue;
                }
                ParamInfo* tmpparam = m_parametersInDB[paraname];
                if (iter3->second->Change == "") {
                    iter3->second->Change = tmpparam->Change;
                }
                iter3->second->Maximum = tmpparam->Maximum;
                iter3->second->Minimun = tmpparam->Minimun;
                // Perform update
                string remoteFileName = GetUpper(ValueToString(subbsnID) + "_" + paraname);
                if (m_rsMap.find(remoteFileName) == m_rsMap.end()) {
                    cout << "      Warning: the parameter name: "<< remoteFileName << 
                        " is not loaded as 1D or 2D raster, and "
                        " will not work as expected." << endl;
                    continue;
                }
                if (m_rsMap[remoteFileName]->is2DRaster()) {
                    int lyr = -1;
                    float** data2D = nullptr;
                    m_rsMap[remoteFileName]->get2DRasterData(&nsize, &lyr, &data2D);
                    iter3->second->Adjust2DRaster(nsize, lyr, data2D, mgtunits, selIDs, ludata, suitablelu);
                }
                else {
                    float* data = nullptr;
                    m_rsMap[remoteFileName]->getRasterData(&nsize, &data);
                    iter3->second->Adjust1DRaster(nsize, data, mgtunits, selIDs, ludata, suitablelu);
                }
            }
        }
    }
}
