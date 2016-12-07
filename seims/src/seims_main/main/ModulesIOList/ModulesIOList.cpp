/*!
 * \brief 
 * \author Liangjun Zhu
 * \version 1.0
 * \date July 2015
 */
#ifndef linux
#include <WinSock2.h>
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif


#include <time.h>
#include <string.h>
#include <fstream>
#include "util.h"
#include "utils.h"
#include "ModulesIOList.h"
#include "ModelException.h"
#include "MetadataInfoConst.h"
#include "MetadataInfo.h"

#define BUFSIZE 255

// gets the root path to the current executable.
string _GetApplicationPath()
{
    string RootPath;

#ifndef linux
    TCHAR buffer[BUFSIZE];
    GetModuleFileName(NULL, buffer, BUFSIZE);
    RootPath = string((char *) buffer);
#else
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
    static char buf[PATH_MAX];
    int rslt = readlink("/proc/self/exe", buf, PATH_MAX);
    if(rslt < 0 || rslt >= PATH_MAX)
        buf[0] = '\0';
    else
        buf[rslt] = '\0';
    RootPath = buf;
#endif
    basic_string<char>::size_type idx = RootPath.find_last_of(SEP);
    RootPath = RootPath.substr(0, idx + 1);

    return RootPath;
}

//! Constructor
ModulesIOList::ModulesIOList(const string &configFileName, const string &dllPath) : m_dllPath(dllPath)
{
    Init(configFileName);
}

//! Destructor
ModulesIOList::~ModulesIOList(void)
{
    for (map<string, SEIMSModuleSetting *>::iterator it = m_settings.begin(); it != m_settings.end(); ++it)
        delete it->second;

    for (map<string, const char *>::iterator it = m_metadata.begin(); it != m_metadata.end(); ++it)
        delete it->second;

    for (size_t i = 0; i < m_dllHandles.size(); i++)
    {
#ifndef linux
        FreeLibrary(m_dllHandles[i]);
#else
        dlclose(m_dllHandles[i]);
#endif
    }
}

//! Initialization
void ModulesIOList::Init(const string &configFileName)
{
    ReadModulesListFile(configFileName.c_str());
    size_t n = m_moduleIDs.size();
    // read all the .dll or .so and create objects
    for (size_t i = 0; i < n; i++)
    {
        string id = m_moduleIDs[i];
        string dllID = id;
        // for ITP modules, the input ids are ITP_T, ITP_P and ITP should be used as ID name
        if (id.find(MID_ITP) != string::npos)
#ifndef linux
            dllID = MID_ITP;
#else
            dllID = Tag_So + string(MID_ITP);
#endif
        else if (id.find(MID_TSD_RD) != string::npos)
#ifndef linux
            dllID = MID_TSD_RD;
#else
        dllID = Tag_So + string(MID_TSD_RD);
#endif
        // load function pointers from DLL
        ReadDLL(id, dllID);

        // load metadata
        MetadataFunction metadataInfo = m_metadataFuncs.at(id);
        const char *metadata = metadataInfo();
        m_metadata[id] = metadata;
        // parse the metadata
        TiXmlDocument doc;
        doc.Parse(metadata);
        ReadParameterSetting(id, doc, m_settings[id]);
        ReadInputSetting(id, doc, m_settings[id]);
        ReadOutputSetting(id, doc, m_settings[id]);
    }

    // set the connections among objects
    for (size_t i = 0; i < n; i++)
    {
        string id = m_moduleIDs[i];
        //cout << id << endl;
        vector<ParamInfo> &inputs = m_inputs[id];
        for (size_t j = 0; j < inputs.size(); j++)
        {
            ParamInfo &param = inputs[j];
            if (!StringMatch(param.Source, "module"))
                continue;

            param.DependPara = FindDependentParam(param);
            //cout << "\t" << param.Name << "\t" << param.DependPara->ModuleID << ":" << param.DependPara->Name << endl;
        }
    }
    GenerateModulesInfoList();
    GenerateParamsInfoList();
    GenerateModuleParamsInfoList();
}

//! Get comparable name after underscore if necessary
string ModulesIOList::GetComparableName(string &paraName)
{
    if (paraName.length() <= 2)
        return paraName;

    string compareName;

    string prefix = paraName.substr(0, 2);
    if (prefix == "D_" || prefix == "T_" || prefix == "d_" || prefix == "t_")
        compareName = paraName.substr(2); //use the string after the underscore, T_PET, use PET
    else
        compareName = paraName;

    return compareName;
}

//! Match type
dimensionTypes ModulesIOList::MatchType(string strType)
{
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

//! is constant input?
bool ModulesIOList::IsConstantInputFromName(string &name)
{
    if (StringMatch(name, CONS_IN_ELEV) ||
        StringMatch(name, CONS_IN_LAT) ||
        StringMatch(name, CONS_IN_XPR) ||
        StringMatch(name, CONS_IN_YPR))
        //StringMatch(name,Contant_Input_FlowdiversionProperty) ||
        //StringMatch(name,Contant_Input_PointsourceProperty) ||
        //StringMatch(name,Contant_Input_ReservoirProperty) ||
        //StringMatch(name,Contant_Input_ReservoirRatingCurve) ||
        //StringMatch(name,Contant_Input_ReservoirOperationSchedual))
        return true;
    return false;
}

//! Load function pointers from .DLL or .so
void ModulesIOList::ReadDLL(string &id, string &dllID)
{
    // the dll file is already read, return
    if (m_instanceFuncs.find(id) != m_instanceFuncs.end())
        return;

    // check if the dll file exists
    string moduleFileName = trim(m_dllPath) + string(dllID) + string(Tag_ModuleExt);
    utils util;
    if (!util.FileExists(moduleFileName))
        throw ModelException("ModulesIOList", "ReadDLL", moduleFileName + " does not exist or has no read permission!");

    //load library
#ifndef linux
    HINSTANCE handle = LoadLibrary(TEXT(moduleFileName.c_str()));
    if (handle == NULL) throw ModelException("ModulesIOList", "ReadDLL", "Could not load " + moduleFileName);
    m_instanceFuncs[id] = InstanceFunction(GetProcAddress(HMODULE(handle), "GetInstance"));
    m_metadataFuncs[id] = MetadataFunction(GetProcAddress(HMODULE(handle), "MetadataInformation"));
#else
    void* handle = dlopen(moduleFileName.c_str(), RTLD_LAZY);
    if (handle == NULL)
    {
        cout << dlerror() << endl;
        throw ModelException("ModulesIOList", "ReadDLL", "Could not load " + moduleFileName);
    }
    m_instanceFuncs[id] = InstanceFunction(dlsym(handle, "GetInstance"));
    m_metadataFuncs[id] = MetadataFunction(dlsym(handle, "MetadataInformation"));
#endif
    m_dllHandles.push_back(handle);
    if (m_instanceFuncs[id] == NULL)
        throw ModelException("ModulesIOList", "ReadDLL",
                             moduleFileName + " does not implement API function: GetInstance");
    if (m_metadataFuncs[id] == NULL)
        throw ModelException("ModulesIOList", "ReadDLL",
                             moduleFileName + " does not implement API function: MetadataInformation");

}

//! Get instance
SimulationModule *ModulesIOList::GetInstance(string &moduleID)
{
    return m_instanceFuncs[moduleID]();
}

//! Read module's parameters setting from XML string
void ModulesIOList::ReadParameterSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting)
{
    m_parameters.insert(map<string, vector<ParamInfo> >::value_type(moduleID, vector<ParamInfo>()));
    vector<ParamInfo> &vecPara = m_parameters[moduleID];

    TiXmlElement *eleMetadata = doc.FirstChildElement(TagMetadata.c_str());

    // start getting the parameters
    TiXmlElement *eleParams = eleMetadata->FirstChildElement(TagParameters.c_str());
    if (eleParams != NULL)
    {
        TiXmlElement *eleParam = eleParams->FirstChildElement(TagParameter.c_str());
        while (eleParam != NULL)
        {
            // clear the object
            ParamInfo *param = new ParamInfo();

            // set the module id
            param->ModuleID = moduleID;

            // get the parameter name
            TiXmlElement *elItm = eleParam->FirstChildElement(TagParameterName.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Name = GetUpper(elItm->GetText());
                    param->BasicName = param->Name;
                    param->ClimateType = setting->dataTypeString();

                    //set climate data type got from config.fig
                    //this is used for interpolation module
                    if (StringMatch(param->Name, Tag_DataType)) param->Value = setting->dataType();
                }
            }

            // get the parameter description
            elItm = eleParam->FirstChildElement(TagParameterDescription.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Description = elItm->GetText();
                }
            }

            // get the parameter units
            elItm = eleParam->FirstChildElement(TagParameterUnits.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Units = elItm->GetText();
                }
            }

            // get the parameter source
            elItm = eleParam->FirstChildElement(TagParameterSource.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Source = elItm->GetText();
                }
            }

            // get the parameter dimension
            elItm = eleParam->FirstChildElement(TagParameterDimension.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Dimension = MatchType(string(elItm->GetText()));
                }
            }

            // cleanup
            elItm = NULL;

            // parameter must have these values
            if (param->Name.size() <= 0)
            {
                delete param;
                throw ModelException("ModulesIOList", "ReadParameterSetting",
                                     "Some parameters have not name in metadata!");
            }

            if (param->Source.size() <= 0)
            {
                string name = param->Name;
                delete param;
                throw ModelException("ModulesIOList", "ReadParameterSetting",
                                     "parameter " + name + " does not have source!");
            }

            if (param->Dimension == DT_Unknown)
            {
                string name = param->Name;
                delete param;
                throw ModelException("ModulesIOList", "ReadParameterSetting",
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

//! Read module's input setting from XML string
void ModulesIOList::ReadInputSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting)
{
    m_inputs.insert(map<string, vector<ParamInfo> >::value_type(moduleID, vector<ParamInfo>()));
    vector<ParamInfo> &vecPara = m_inputs[moduleID];

    TiXmlElement *eleMetadata = doc.FirstChildElement(TagMetadata.c_str());

    TiXmlElement *eleInputs = eleMetadata->FirstChildElement(TagInputs.c_str());
    if (eleInputs != NULL)
    {
        TiXmlElement *elInput = eleInputs->FirstChildElement(TagInputVariable.c_str());
        while (elInput != NULL)
        {
            ParamInfo *param = new ParamInfo();

            // set the module id
            param->ModuleID = moduleID;
            param->ClimateType = setting->dataTypeString();

            // get the input variable name
            TiXmlElement *elItm = elInput->FirstChildElement(TagInputVariableName.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Name = GetUpper(elItm->GetText());
                    param->BasicName = param->Name;
                    param->IsConstant = IsConstantInputFromName(param->Name);
                    if (setting->dataTypeString().length() > 0)
                        param->Name = param->Name + "_" + setting->dataTypeString();
                }
            }

            // get the input variable units(
            elItm = elInput->FirstChildElement(TagInputVariableUnits.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Units = elItm->GetText();
                }
            }

            // get the input variable description
            elItm = elInput->FirstChildElement(TagInputVariableDescription.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Description = elItm->GetText();
                }
            }

            // get the input variable source
            elItm = elInput->FirstChildElement(TagInputVariableSource.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Source = elItm->GetText();
                }
            }

            // get the input variable dimension
            elItm = elInput->FirstChildElement(TagInputVariableDimension.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Dimension = MatchType(string(elItm->GetText()));
                }
            }

            elItm = NULL;

            // input must have these values
            if (param->Name.size() <= 0)
            {
                delete param;
                throw ModelException("SEIMSModule", "readInputSetting",
                                     "Some input variables have not name in metadata!");
            }

            if (param->Source.size() <= 0)
            {
                string name = param->Name;
                delete param;
                throw ModelException("SEIMSModule", "readInputSetting",
                                     "Input variable " + name + " does not have source!");
            }

            if (param->Dimension == DT_Unknown)
            {
                string name = param->Name;
                delete param;
                throw ModelException("SEIMSModule", "readInputSetting",
                                     "Input variable " + name + " does not have dimension!");
            }

            vecPara.push_back(*param);
            delete param;

            // get the next input if it exists
            elInput = elInput->NextSiblingElement();
        }
    }
}

//! Read module's output from XML string
void ModulesIOList::ReadOutputSetting(string &moduleID, TiXmlDocument &doc, SEIMSModuleSetting *setting)
{
    m_outputs.insert(map<string, vector<ParamInfo> >::value_type(moduleID, vector<ParamInfo>()));
    vector<ParamInfo> &vecPara = m_outputs[moduleID];

    TiXmlElement *eleMetadata = doc.FirstChildElement(TagMetadata.c_str());

    TiXmlElement *eleOutputs = eleMetadata->FirstChildElement(TagOutputs.c_str());
    if (eleOutputs != NULL)
    {
        TiXmlElement *elOutput = eleOutputs->FirstChildElement(TagOutputVariable.c_str());
        while (elOutput != NULL)
        {
            ParamInfo *param = new ParamInfo();

            // set the module id
            param->ModuleID = moduleID;
            param->ClimateType = setting->dataTypeString();
            // get the output variable name
            TiXmlElement *elItm = elOutput->FirstChildElement(TagOutputVariableName.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Name = GetUpper(elItm->GetText());
                    param->BasicName = param->Name;
                    if (setting->dataTypeString().size() > 0)
                        param->Name = param->Name + "_" + setting->dataTypeString();
                }
            }

            // get the output variable units
            elItm = elOutput->FirstChildElement(TagOutputVariableUnits.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Units = elItm->GetText();
                }
            }

            // get the output variable description
            elItm = elOutput->FirstChildElement(TagOutputVariableDescription.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Description = elItm->GetText();
                }
            }

            param->Source = "";

            // get the output variable dimension
            elItm = elOutput->FirstChildElement(TagOutputVariableDimension.c_str());
            if (elItm != NULL)
            {
                if (elItm->GetText() != NULL)
                {
                    param->Dimension = MatchType(string(elItm->GetText()));
                }
            }

            elItm = NULL;

            // add to the list
            param->IsOutput = true;

            // output variable must have these values
            if (param->Name.size() <= 0)
            {
                delete param;
                throw ModelException("SEIMSModule", "readOutputSetting",
                                     "Some output variables have not name in metadata!");
            }

            if (param->Dimension == DT_Unknown)
            {
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

//! Load Modules information from file
bool ModulesIOList::LoadModuleInfoFromFile(const char *filename, vector<vector<string> > &infos)
{
    bool bStatus = false;
    ifstream myfile;
    string line;
    utils utl;
    string T_variables[8] = {DataType_Precipitation, DataType_MeanTemperature, DataType_MaximumTemperature,
                             DataType_PotentialEvapotranspiration, DataType_SolarRadiation, DataType_WindSpeed,
                             DataType_RelativeAirMoisture};
    try
    {
        // open the file
        myfile.open(filename, ios::in);
        if (myfile.is_open())
        {
            while (!myfile.eof())
            {
                if (myfile.good())
                {
                    getline(myfile, line);
                    line = trim(line);
                    if ((line.size() > 0) && (line[0] != '#')) // ignore comments and empty lines
                    {
                        // parse the line into separate item
                        vector<string> tokens = utl.SplitString(line, '|');
                        // is there anything in the token list?
                        if (tokens.size() > 0)
                        {
                            for (size_t i = 0; i < tokens.size(); i++)
                            {
                                tokens[i] = trim(tokens[i]);
                            }
                            if (tokens[0].size() > 0)
                            {
                                // there is something to add so resize the header list to append it
                                int sz = infos.size(); // get the current number of rows
                                if (tokens[2].find(MID_ITP) != string::npos ||
                                    tokens[2].find(MID_TSD_RD) != string::npos)
                                {
                                    infos.resize(sz + 7);

                                    for (size_t j = 0; j < 7; j++)
                                    {
                                        vector<string> tokensTemp(tokens);
                                        tokensTemp[2] += "_" + T_variables[j];
                                        if (tokens[2].find(MID_ITP) != string::npos)
                                        {
                                            tokensTemp[3] += "_" + T_variables[j];
                                        }
                                        else
                                        {
                                            tokensTemp[3] += "_" + T_variables[j] + "_0";
                                        }
                                        infos[sz + j] = tokensTemp;
                                    }
                                }
                                else
                                {
                                    infos.resize(sz + 1);        // resize with one more row
                                    infos[sz] = tokens;
                                }
                                bStatus = true; // consider this a success
                            } // if there is nothing in the first item of the token list there is nothing to add to the header list
                        }
                    }
                }
            }
            bStatus = true;
            myfile.close();
        }
    }
    catch (...)
    {
        myfile.close();
    }
    return bStatus;
}

//! Read ModulesList file
void ModulesIOList::ReadModulesListFile(const char *configFileName)
{
    vector<vector<string> > moduleInfos;
    LoadModuleInfoFromFile(configFileName, moduleInfos);
    try
    {
        for (size_t i = 0; i < moduleInfos.size(); i++)
        {
            if (moduleInfos[i].size() > 2)
            {
//                string moduleCategory = moduleInfos[i][1];
                string moduleID = GetUpper(moduleInfos[i][2]);
                string moduleSettingPrefix = moduleInfos[i][3];
#ifdef linux
                moduleID = Tag_So + moduleID;
#endif
                SEIMSModuleSetting *moduleSetting = new SEIMSModuleSetting(moduleID, moduleSettingPrefix);
                m_moduleIDs.push_back(moduleID);
                m_settings[moduleID] = moduleSetting;
            }
        }
    }
    catch (...)
    {
        throw;
    }

}

///
void ModulesIOList::GenerateModulesInfoList()
{
    size_t n = m_moduleIDs.size();
    vector<string> header;
    header.push_back("ModuleName");
    header.push_back("Parameters");
    header.push_back("Inputs");
    header.push_back("Outputs");
    moduleInfoList.push_back(header);
    for (size_t i = 0; i < n; i++)
    {
        vector<string> tempModuleInfo;
        string id = m_moduleIDs[i];
        tempModuleInfo.push_back(id);
        vector<ParamInfo> &curParams = m_parameters[id];
        vector<ParamInfo> &curInputs = m_inputs[id];
        vector<ParamInfo> &curOutputs = m_outputs[id];
        vector<ParamInfo>::iterator iParam;
        vector<ParamInfo>::iterator iInput;
        vector<ParamInfo>::iterator iOutput;
        string curParamsStr = "", curInputsStr = "", curOutputsStr = "";
        for (iParam = curParams.begin(); iParam != curParams.end(); iParam++)
        {
            curParamsStr += iParam->Name + ",";
        }
        for (iInput = curInputs.begin(); iInput != curInputs.end(); iInput++)
        {
            curInputsStr += iInput->Name + ",";
        }
        for (iOutput = curOutputs.begin(); iOutput != curOutputs.end(); iOutput++)
        {
            curOutputsStr += iOutput->Name + ",";
        }
        tempModuleInfo.push_back(curParamsStr.substr(0, curParamsStr.find_last_of(",")));
        tempModuleInfo.push_back(curInputsStr.substr(0, curInputsStr.find_last_of(",")));
        tempModuleInfo.push_back(curOutputsStr.substr(0, curOutputsStr.find_last_of(",")));
        moduleInfoList.push_back(tempModuleInfo);
    }

}

//! Find outputID parameter's module. Return Module index iModule and its ParamInfo
void ModulesIOList::FindOutputParameter(string &outputID, int &iModule, ParamInfo *&paraInfo)
{
    string compareName = outputID;
    size_t n = m_moduleIDs.size();
    for (size_t i = 0; i < n; i++)
    {
        string id = m_moduleIDs[i];
        vector<ParamInfo> &vecPara = m_outputs[id];
        for (size_t j = 0; j < vecPara.size(); j++)
        {
            if (StringMatch(compareName, vecPara[j].Name))
            {
                iModule = i;
                paraInfo = &vecPara[j];
                return;
            }
        }
    }
}

//! Find dependent parameters
ParamInfo *ModulesIOList::FindDependentParam(ParamInfo &paramInfo)
{
    string paraName = GetComparableName(paramInfo.Name);
    dimensionTypes paraType = paramInfo.Dimension;

    size_t n = m_moduleIDs.size();
    for (size_t i = 0; i < n; i++)
    {
        string id = m_moduleIDs[i];
        vector<ParamInfo> &outputs = m_outputs[id];
        for (size_t j = 0; j < outputs.size(); j++)
        {
            ParamInfo &param = outputs[j];
            string compareName = GetComparableName(param.Name);

            if (StringMatch(paraName, compareName) && param.Dimension == paraType)
            {
                param.OutputToOthers = true;
                return &param;
            }
        }
    }
    //throw ModelException("ModulesIOList", "FindDependentParam", "Can not find input for " + paraName + ".\n");
    return NULL;
}

//! find the modules who invoke the parameter, and return the modules' ID separated by comma
string ModulesIOList::GetInvokeModulesIDs(ParamInfo &paramIns)
{
    string paraName = GetComparableName(paramIns.Name);
    dimensionTypes paraType = paramIns.Dimension;
    vector<string> invokeModuleIDs;
    string invokeModuleIDsStr = "";
    size_t n = m_moduleIDs.size();
    for (size_t i = 0; i < n; i++)
    {
        string id = m_moduleIDs[i];
        vector<ParamInfo> &inputs = m_inputs[id];
        for (size_t j = 0; j < inputs.size(); j++)
        {
            ParamInfo &param = inputs[j];
            string compareName = GetComparableName(param.Name);

            if (StringMatch(paraName, compareName) && param.Dimension == paraType)
            {
                invokeModuleIDs.push_back(id);
                break;
            }
        }
    }
    for (size_t i = 0; i < invokeModuleIDs.size(); i++)
    {
        invokeModuleIDsStr += invokeModuleIDs[i] + ",";
    }
    int idx = invokeModuleIDsStr.find_last_of(",");
    if (idx != -1)
    {
        return invokeModuleIDsStr.substr(0, idx);
    }
    else
        return "None";
}

//! find the modules who invoke the parameter, and return the modules' ID separated by comma
string ModulesIOList::GetDBParamsInvokeModulesIDs(ParamInfo &paramIns)
{
    string paraName = GetComparableName(paramIns.Name);
    dimensionTypes paraType = paramIns.Dimension;
    vector<string> invokeModuleIDs;
    string invokeModuleIDsStr = "";
    size_t n = m_moduleIDs.size();
    for (size_t i = 0; i < n; i++)
    {
        string id = m_moduleIDs[i];
        vector<ParamInfo> &params = m_parameters[id];
        for (size_t j = 0; j < params.size(); j++)
        {
            ParamInfo &param = params[j];
            string compareName = GetComparableName(param.Name);

            if (StringMatch(paraName, compareName) && param.Dimension == paraType)
            {
                invokeModuleIDs.push_back(id);
                break;
            }
        }
    }
    for (size_t i = 0; i < invokeModuleIDs.size(); i++)
    {
        invokeModuleIDsStr += invokeModuleIDs[i] + ",";
    }
    int idx = invokeModuleIDsStr.find_last_of(",");
    if (idx != -1)
    {
        return invokeModuleIDsStr.substr(0, idx);
    }
    else
        return "None";
}

//! find the modules who output the parameter, and return the modules' ID separated by comma
string ModulesIOList::GetOutputModulesIDs(ParamInfo &paramIns)
{
    string paraName = GetComparableName(paramIns.Name);
    dimensionTypes paraType = paramIns.Dimension;
    vector<string> outputModuleIDs;
    string outputModuleIDsStr = "";
    size_t n = m_moduleIDs.size();
    for (size_t i = 0; i < n; i++)
    {
        string id = m_moduleIDs[i];
        vector<ParamInfo> &outputs = m_outputs[id];
        for (size_t j = 0; j < outputs.size(); j++)
        {
            ParamInfo &param = outputs[j];
            string compareName = GetComparableName(param.Name);

            if (StringMatch(paraName, compareName) && param.Dimension == paraType)
            {
                outputModuleIDs.push_back(id);
                break;
            }
        }
    }
    for (size_t i = 0; i < outputModuleIDs.size(); i++)
    {
        outputModuleIDsStr += outputModuleIDs[i] + ",";
    }
    int idx = outputModuleIDsStr.find_last_of(",");
    if (idx != -1)
    {
        return outputModuleIDsStr.substr(0, idx);
    }
    else
        return "None";
}

void ModulesIOList::GenerateModuleParamsInfoList()
{
    size_t n = m_moduleIDs.size();
    for (size_t i = 0; i < n; i++)
    {
        string id = m_moduleIDs[i];
        vector<ParamInfo> &curInputPara = m_inputs[id];
        for (size_t j = 0; j < curInputPara.size(); j++)
        {
            string curInputParaID = curInputPara[j].Name;
            map<string, vector<string> >::iterator it;
            it = moduleParamsInfoList.find(curInputParaID);
            if (it == moduleParamsInfoList.end())  /// curInputParaID is not in paramsInfoList
            {
                vector<string> curInputParaDescription;
                curInputParaDescription.push_back(curInputPara[j].Units);
                curInputParaDescription.push_back(curInputPara[j].Description);
                curInputParaDescription.push_back(DimentionType2String(curInputPara[j].Dimension));
                curInputParaDescription.push_back(GetInvokeModulesIDs(curInputPara[j]));
                curInputParaDescription.push_back(GetOutputModulesIDs(curInputPara[j]));
                moduleParamsInfoList.insert(
                        map<string, vector<string> >::value_type(curInputParaID, curInputParaDescription));
            }
        }
        vector<ParamInfo> &curOutputPara = m_outputs[id];
        for (size_t j = 0; j < curOutputPara.size(); j++)
        {
            string curOutputParaID = curOutputPara[j].Name;
            map<string, vector<string> >::iterator it;
            it = moduleParamsInfoList.find(curOutputParaID);
            if (it == moduleParamsInfoList.end())  /// curOutputParaID is not in paramsInfoList
            {
                vector<string> curOutputParaDescription;
                curOutputParaDescription.push_back(curOutputPara[j].Units);
                curOutputParaDescription.push_back(curOutputPara[j].Description);
                curOutputParaDescription.push_back(DimentionType2String(curOutputPara[j].Dimension));
                curOutputParaDescription.push_back(GetInvokeModulesIDs(curOutputPara[j]));
                curOutputParaDescription.push_back(GetOutputModulesIDs(curOutputPara[j]));
                moduleParamsInfoList.insert(
                        map<string, vector<string> >::value_type(curOutputParaID, curOutputParaDescription));
            }
        }
    }
}

void ModulesIOList::GenerateParamsInfoList()
{
    size_t n = m_moduleIDs.size();
    for (size_t i = 0; i < n; i++)
    {
        string id = m_moduleIDs[i];
        vector<ParamInfo> &curPara = m_parameters[id];
        for (size_t j = 0; j < curPara.size(); j++)
        {
            string curParaID = curPara[j].Name;
            map<string, vector<string> >::iterator it;
            it = paramsInfoList.find(curParaID);
            if (it == paramsInfoList.end())  /// curInputParaID is not in paramsInfoList
            {
                vector<string> curParaDescription;
                curParaDescription.push_back(curPara[j].Units);
                curParaDescription.push_back(curPara[j].Description);
                curParaDescription.push_back(DimentionType2String(curPara[j].Dimension));
                curParaDescription.push_back(GetDBParamsInvokeModulesIDs(curPara[j]));
                paramsInfoList.insert(map<string, vector<string> >::value_type(curParaID, curParaDescription));
            }
        }
    }
}

void writeText(string &filename, vector<vector<string> > &data)
{
    ofstream fs;
    fs.open(filename.c_str(), ios::out);
    if (fs.is_open())
    {
        vector<vector<string> >::iterator it;
        for (it = data.begin(); it != data.end(); it++)
        {
            vector<string>::iterator it2;
            for (it2 = it->begin(); it2 != it->end(); it2++)
            {
                fs << *it2 << "\t";
            }
            fs << "\n";
        }
    }
    fs.close();
    StatusMessage(("Create " + filename + " successfully!").c_str());
    return;
}

void writeText(string &filename, vector<string> &data)
{
    ofstream fs;
    fs.open(filename.c_str(), ios::out);
    if (fs.is_open())
    {
        vector<string>::iterator it;
        for (it = data.begin(); it != data.end(); it++)
        {
            fs << *it << "\t" << endl;
        }
    }
    fs.close();
    StatusMessage(("Create " + filename + " successfully!").c_str());
    return;
}

void writeText(string &filename, map<string, vector<string> > &data)
{
    ofstream fs;
    fs.open(filename.c_str(), ios::out);
    if (fs.is_open())
    {
        map<string, vector<string> >::iterator it;
        for (it = data.begin(); it != data.end(); it++)
        {
            fs << it->first << "\t";
            vector<string>::iterator it2;
            for (it2 = it->second.begin(); it2 != it->second.end(); it2++)
            {
                fs << *it2 << "\t";
            }
            fs << endl;
        }
    }
    fs.close();
    StatusMessage(("Create " + filename + " successfully!").c_str());
    return;
}

string DimentionType2String(dimensionTypes dimType)
{
    string strTmp = "";

    switch (dimType)
    {
        case DT_Array1D:
            strTmp = Type_Array1D;
            break;
        case DT_Array1DDateValue:
            strTmp = Type_Array1DDateValue;
            break;
        case DT_Array2D:
            strTmp = Type_Array2D;
            break;
        case DT_Array3D:
            strTmp = Type_Array3D;
            break;
        case DT_Raster1D:
            strTmp = Type_Raster1D;
            break;
        case DT_Raster2D:
            strTmp = Type_Raster2D;
            break;
        case DT_Single:
            strTmp = Type_Single;
            break;
		case DT_Scenario:
			strTmp = Type_Scenario;
			break;
		case DT_Reach:
			strTmp = Type_Reach;
			break;
		case DT_Subbasin:
			strTmp = Type_Subbasin;
			break;
		//case DT_LapseRateArray:
		//	strTmp = Type_LapseRateArray;
		//	break;
		//case DT_LookupTable:
		//	strTmp = Type_LookupTable;
		//	break;
        //case DT_SiteInformation:
        //    strTmp = Type_SiteInformation;
        //    break;
        default:
            break;
    }
    return strTmp;
}
