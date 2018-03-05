#include "DataCenter.h"

DataCenter::DataCenter(InputArgs* input_args, ModuleFactory* factory, int subBasinID /* = 0 */) :
    m_modelPath(input_args->m_model_path), m_modelName(input_args->m_model_name),
    m_lyrMethod(input_args->m_layer_mtd), m_subbasinID(subBasinID),
    m_scenarioID(input_args->m_scenario_id), m_useScenario(false),
    m_calibrationID(input_args->m_calibration_id),
    m_threadNum(input_args->m_thread_num), m_outputScene(DB_TAB_OUT_SPATIAL),
    m_factory(factory),
    m_outputPath(""), m_modelMode(""), m_nSubbasins(-1), m_outletID(-1),
    m_input(nullptr), m_output(nullptr), m_climStation(nullptr), m_scenario(nullptr),
    m_reaches(nullptr), m_subbasins(nullptr), m_maskRaster(nullptr) {
    /// Clean output folder
    if (m_scenarioID >= 0) { // -1 means no BMPs scenario will be simulated
        m_outputScene += ValueToString(m_scenarioID);
        /// Be aware, m_useScenario will be updated in checkModelPreparedData().
    }
    if (m_calibrationID >= 0) {  // -1 means no calibration setting will be used.
        m_outputScene += "-" + ValueToString(m_calibrationID);
    }
    m_outputPath = m_modelPath + SEP + m_outputScene + SEP;
    CleanDirectory(m_outputPath);
}

DataCenter::~DataCenter() {
    StatusMessage("Release DataCenter...");
    if (nullptr != m_input) {
        StatusMessage("---release setting input data ...");
        delete m_input;
        m_input = nullptr;
    }
    if (nullptr != m_output) {
        StatusMessage("---release setting output data ...");
        delete m_output;
        m_output = nullptr;
    }
    if (nullptr != m_climStation) {
        StatusMessage("---release climate station data ...");
        delete m_climStation;
        m_climStation = nullptr;
    }
    if (nullptr != m_scenario) {
        StatusMessage("---release bmps scenario data ...");
        delete m_scenario;
        m_scenario = nullptr;
    }
    if (nullptr != m_reaches) {
        StatusMessage("---release reaches data ...");
        delete m_reaches;
        m_reaches = nullptr;
    }
    if (nullptr != m_subbasins) {
        StatusMessage("---release subbasins data ...");
        delete m_subbasins;
        m_subbasins = nullptr;
    }
    StatusMessage("---release map of all 1D and 2D raster data ...");
    for (auto it = m_rsMap.begin(); it != m_rsMap.end();) {
        if (nullptr != it->second) {
            StatusMessage(("-----" + it->first + " ...").c_str());
            delete it->second;
            it->second = nullptr;
        }
        m_rsMap.erase(it++);
    }
    m_rsMap.clear();
    StatusMessage("---release map of parameters in MongoDB ...");
    for (auto it = m_initParameters.begin(); it != m_initParameters.end();) {
        if (nullptr != it->second) {
            //StatusMessage(("-----" + it->first + " ...").c_str());
            delete it->second;
            it->second = nullptr;
        }
        m_initParameters.erase(it++);
    }
    m_initParameters.clear();
    StatusMessage("---release map of 1D array data ...");
    for (auto it = m_1DArrayMap.begin(); it != m_1DArrayMap.end();) {
        if (nullptr != it->second) {
            StatusMessage(("-----" + it->first + " ...").c_str());
            Release1DArray(it->second);
        }
        m_1DArrayMap.erase(it++);
    }
    m_1DArrayMap.clear();
    StatusMessage("---release map of 2D array data ...");
    for (auto it = m_2DArrayMap.begin(); it != m_2DArrayMap.end();) {
        if (nullptr != it->second) {
            StatusMessage(("-----" + it->first + " ...").c_str());
            Release2DArray(m_2DRowsLenMap[it->first], it->second);
        }
        m_2DArrayMap.erase(it++);
    }
    m_2DArrayMap.clear();
    StatusMessage("---release Interpolation weight data ...");
    for (auto it = m_weightDataMap.begin(); it != m_weightDataMap.end();) {
        if (nullptr != it->second) {
            StatusMessage(("-----" + it->first + " ...").c_str());
            delete it->second;
            it->second = nullptr;
        }
        m_weightDataMap.erase(it++);
    }
    m_weightDataMap.clear();
}

bool DataCenter::getFileInStringVector() {
    if (m_fileIn1DStrs.empty()) {
        if (!LoadPlainTextFile(m_fileInFile, m_fileIn1DStrs)) {
            throw ModelException("DataCenter", "getFileInStringVector", "");
            return false;
        }
    }
    return true;
}

void DataCenter::setLapseData(string &remoteFilename, int &rows, int &cols, float **&data) {
    int nRows = 12;
    int nCols = 5;
    data = new float *[nRows];
    for (int i = 0; i < nRows; i++) {
        data[i] = new float[nCols];
        data[i][0] = 4.f; /// element number
        data[i][1] = 0.03f; // P
        data[i][2] = -0.65f; // T
        data[i][3] = 0.f;    // PET
        data[i][4] = 0.f;    // other Meteorology variables
    }
    /// insert to corresponding maps
    m_2DArrayMap.insert(make_pair(remoteFilename, data));
    m_2DRowsLenMap.insert(make_pair(remoteFilename, nRows));
    m_2DColsLenMap.insert(make_pair(remoteFilename, nCols));
}

void DataCenter::dumpCaliParametersInDB() {
    if (m_initParameters.empty()) return;
    string fileName = m_outputPath + SEP + "param.cali";
    ofstream fs;
    fs.open(fileName.c_str(), ios::ate);
    if (nullptr == &fs) return;
    if (!fs.is_open()) return;

    if (m_calibrationID >= 0) fs << "# Calibration ID:" << m_calibrationID << endl;
    fs << "# All calibrated parameters" << endl;
    for (auto it = m_initParameters.begin(); it != m_initParameters.end(); it++) {
        if (nullptr == it->second) continue;
        ParamInfo* tmpParam = it->second;
        if ((StringMatch(tmpParam->Change, PARAM_CHANGE_RC) && FloatEqual(tmpParam->Impact, 1.f)) ||
            (StringMatch(tmpParam->Change, PARAM_CHANGE_AC) && FloatEqual(tmpParam->Impact, 0.f)) ||
            (StringMatch(tmpParam->Change, PARAM_CHANGE_VC) && FloatEqual(tmpParam->Impact, NODATA_VALUE)) ||
            (StringMatch(tmpParam->Change, PARAM_CHANGE_NC))) {
            continue;
        }
        fs << tmpParam->Name << "," << tmpParam->Impact << "," << tmpParam->Change << endl;
    }

    fs.close();
}

float DataCenter::LoadDataForModules(vector<SimulationModule *> &modules) {
    double t1 = TimeCounting();
    vector<string> moduleIDs = m_factory->GetModuleIDs();
    map<string, SEIMSModuleSetting *>& moduleSettings = m_factory->GetModuleSettings();
    map<string, vector<ParamInfo*> >& moduleParameters = m_factory->GetModuleParameters();
    for (size_t i = 0; i < moduleIDs.size(); i++) {
        string id = moduleIDs[i];
        vector<ParamInfo*> &parameters = moduleParameters[id];
        bool verticalInterpolation = true;
        /// Special operation for ITP module
        if (id.find(MID_ITP) != string::npos) {
            modules[i]->SetClimateDataType(moduleSettings[id]->dataType());
            for (size_t j = 0; j < parameters.size(); j++) {
                ParamInfo *param = parameters[j];
                if (StringMatch(param->Name, Tag_VerticalInterpolation)) {
                    if (param->Value > 0.f) {
                        verticalInterpolation = true;
                    }
                    else {
                        verticalInterpolation = false;
                    }
                    break;
                }
            }
        }
        for (size_t j = 0; j < parameters.size(); j++) {
            ParamInfo *param = parameters[j];
            SetData(moduleSettings[id], param, modules[i], verticalInterpolation);
        }
    }
    float timeconsume = float(TimeCounting() - t1);
    StatusMessage(("Loading data for modules, TIMESPAN " + ValueToString(timeconsume) + " sec.").c_str());
    return timeconsume;
}

void DataCenter::SetData(SEIMSModuleSetting *setting, ParamInfo *param, SimulationModule *pModule, bool vertitalItp) {
#ifdef _DEBUG
    double stime = TimeCounting();
#endif
    string name = param->BasicName;
    if (setting->dataTypeString().empty()
        && !StringMatch(param->BasicName, CONS_IN_ELEV)
        && !StringMatch(param->BasicName, CONS_IN_LAT)
        && !StringMatch(param->BasicName, CONS_IN_XPR)
        && !StringMatch(param->BasicName, CONS_IN_YPR)) {
        name = param->Name;
        //cout << param->Name << " " << param->BasicName << endl;
    }
    ostringstream oss;
    size_t tmp = name.find("LOOKUP");
    if (tmp == string::npos) {
        oss << m_subbasinID << "_" << name;
    }
    else {
        oss << name;
    }
    if (StringMatch(name, Tag_Weight)) {
        if (setting->dataTypeString() == DataType_Precipitation) {
            oss << "_P";
        }
        else {
            oss << "_M";
        }
    }
    string remoteFileName = oss.str();

    switch (param->Dimension) {
    case DT_Unknown:throw ModelException("ModuleFactory", "SetData", "Type of " + param->Name + " is unknown.");
        break;
    case DT_Single:SetValue(param, pModule);
        break;
    case DT_Array1D:Set1DData(name, remoteFileName, pModule, vertitalItp);
        break;
    case DT_Array2D:Set2DData(name, remoteFileName, pModule);
        break;
    case DT_Array3D:break;
    case DT_Array1DDateValue:break;
    case DT_Raster1D:SetRaster(name, remoteFileName, pModule);
        break;
    case DT_Raster2D:SetRaster(name, remoteFileName, pModule);
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
    StatusMessage(("Set " + name + ": " + remoteFileName + " done, TIMESPAN " +
        ValueToString(timeconsume) + " sec.").c_str());
#endif
}

void DataCenter::SetValue(ParamInfo *param, SimulationModule *pModule) {
    if (StringMatch(param->Name, Tag_DataType)) {
        // the data type is got from config.fig
        return;
    }
    else if (StringMatch(param->Name, Tag_SubbasinId)) {
        param->Value = float(m_subbasinID);
    }
    else if (StringMatch(param->Name, Tag_CellSize)) { // valid cells number, do not be confused with Tag_CellWidth
        param->Value = float(m_maskRaster->getCellNumber()); // old code is ->Size();  they have the same function
    }
    else if (StringMatch(param->Name, Tag_CellWidth)) { //cell size
        param->Value = float(m_maskRaster->getCellWidth());
    }
    else if (StringMatch(param->Name, Tag_TimeStep)) {
        param->Value = float(m_input->getDtDaily()); // return 86400 secs
    }
    else if (StringMatch(param->Name, Tag_HillSlopeTimeStep)) {
        param->Value = float(m_input->getDtHillslope());
    }
    else if (StringMatch(param->Name, Tag_ChannelTimeStep)) {
        param->Value = float(m_input->getDtChannel());
    }
    else if (StringMatch(param->Name, Tag_LayeringMethod)) {
        param->Value = (float)m_lyrMethod;
    }
    else {
        if (m_initParameters.find(GetUpper(param->Name)) != m_initParameters.end()) {
            param->Value = m_initParameters[GetUpper(param->Name)]->GetAdjustedValue();
        }
    }

    pModule->SetValue(param->Name.c_str(), param->Value);
}

void DataCenter::Set1DData(string &paraName, string &remoteFileName, SimulationModule *pModule,
                              bool vertitalItp) {
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
        read1DArrayData(paraName, remoteFileName, n, data);
        if (m_maskRaster->getCellNumber() != n) {
            throw ModelException("DataCenter", "Set1DData",
                                 "The data length derived from Read1DArray in " + remoteFileName + 
                                 " is not the same as the template.");
        }
    }
    /// 2. IF Weight data
    else if (StringMatch(paraName, Tag_Weight)) {
        readItpWeightData(remoteFileName, n, data);
    }
    /// 3. IF Meteorology sites data
    else if (StringMatch(paraName, Tag_Elevation_Meteorology)) {
        if (vertitalItp) {
            n = m_climStation->NumberOfSites(DataType_Meteorology);
            data = m_climStation->GetElevation(DataType_Meteorology);
        }
        else {
            return;
        }
    }
    /// 4. IF Precipitation sites data
    else if (StringMatch(paraName, Tag_Elevation_Precipitation)) {
        if (vertitalItp) {
            n = m_climStation->NumberOfSites(DataType_Precipitation);
            data = m_climStation->GetElevation(DataType_Precipitation);
        }
        else {
            return;
        }
    }
    /// 5. IF Latitude of sites
    else if (StringMatch(paraName, Tag_Latitude_Meteorology)) {
        if (vertitalItp) {
            n = m_climStation->NumberOfSites(DataType_Meteorology);
            data = m_climStation->GetLatitude(DataType_Meteorology);
        }
        else {
            return;
        }
    }
    /// 6. IF any other 1D arrays, such as Heat units of all simulation years (HUTOT)
    else {
        read1DArrayData(paraName, remoteFileName, n, data);
    }
    if (nullptr == data) {
        throw ModelException("ModuleFactory", "Set1DData", "Failed reading file " + remoteFileName);
    }
    pModule->Set1DData(paraName.c_str(), n, data);
}

void DataCenter::Set2DData(string &paraName, string &remoteFileName, SimulationModule *pModule) {
    int nRows = 0;
    int nCols = 1;
    float **data;
    /// Get ROUTING_LAYERS real file name
    if (StringMatch(paraName, Tag_ROUTING_LAYERS)) {
        ostringstream oss;
        if (m_lyrMethod == UP_DOWN) {
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
    if (StringMatch(paraName, Tag_ROUTING_LAYERS) || StringMatch(paraName, Tag_ROUTING_LAYERS_DINF)) {
        // Routing layering data based on different flow model
        read2DArrayData(remoteFileName, nRows, nCols, data);
    }
    else if (StringMatch(paraName, Tag_FLOWIN_INDEX_D8) || StringMatch(paraName, Tag_FLOWIN_INDEX_DINF)
        || StringMatch(paraName, Tag_FLOWIN_PERCENTAGE_DINF) || StringMatch(paraName, Tag_FLOWOUT_INDEX_DINF)) {
        // Flow in and flow out data based on different flow model
        read2DArrayData(remoteFileName, nRows, nCols, data);
    }
    else if (StringMatch(paraName, TAG_OUT_OL_IUH)) { // Overland flow IUH
        readIUHData(remoteFileName, nRows, data);
    }
    else if (StringMatch(paraName, Tag_LapseRate)) /// Match to the format of DT_Array2D, By LJ.
    {
        setLapseData(remoteFileName, nRows, nCols, data);
    }
    else {
        read2DArrayData(remoteFileName, nRows, nCols, data);
    }

    pModule->Set2DData(paraName.c_str(), nRows, nCols, data);
}

void DataCenter::SetRaster(string &paraName, string &remoteFileName, SimulationModule *pModule) {
    int n, lyrs;
    float *data = nullptr;
    float **data2D = nullptr;
    FloatRaster *raster = nullptr;
    if (m_rsMap.find(remoteFileName) == m_rsMap.end()) {
        raster = readRasterData(remoteFileName);
        if (nullptr == raster) {
            throw ModelException("DataCenter", "SetRaster", "Load " + remoteFileName + " failed!");
        }
        string upperName = GetUpper(paraName);
        auto find_iter = m_initParameters.find(upperName);
        bool adjust_data = false;
        if (find_iter != m_initParameters.end()) {
            ParamInfo *tmpParam = find_iter->second;
            if ((StringMatch(tmpParam->Change, PARAM_CHANGE_RC) && !FloatEqual(tmpParam->Impact, 1.f)) ||
                (StringMatch(tmpParam->Change, PARAM_CHANGE_AC) && !FloatEqual(tmpParam->Impact, 0.f)) ||
                (StringMatch(tmpParam->Change, PARAM_CHANGE_VC) && !FloatEqual(tmpParam->Impact, NODATA_VALUE)) ||
                (StringMatch(tmpParam->Change, PARAM_CHANGE_NC))) {
                adjust_data = true;
            }
        }
        /// 1D or 2D raster data
        if (raster->is2DRaster()) {
            if (!raster->get2DRasterData(&n, &lyrs, &data2D)) {
                throw ModelException("DataCenter", "SetRaster", "Load " + remoteFileName + " failed!");
            }
            if (nullptr != data2D && adjust_data) {
                m_initParameters[upperName]->Adjust2DRaster(n, raster->getLayers(), data2D);
            }
        }
        else {
            if (!raster->getRasterData(&n, &data)) {
                throw ModelException("DataCenter", "SetRaster", "Load " + remoteFileName + " failed!");
            }
            if (nullptr != data && adjust_data) {
                m_initParameters[upperName]->Adjust1DRaster(n, data);
            }
        }
    }
    else {
        raster = m_rsMap.at(remoteFileName);
        //cout << remoteFileName << endl;
    }
    if (raster->is2DRaster()) {
        if (!raster->get2DRasterData(&n, &lyrs, &data2D)) {
            throw ModelException("DataCenter", "SetRaster", "Load " + remoteFileName + " failed!");
        }
        string upperName = GetUpper(paraName);
        pModule->Set2DData(paraName.c_str(), n, lyrs, data2D);
    }
    else {
        if (!raster->getRasterData(&n, &data)) {
            throw ModelException("DataCenter", "SetRaster", "Load " + remoteFileName + " failed!");
        }
        string upperName = GetUpper(paraName);
        pModule->Set1DData(paraName.c_str(), n, data);
    }
}

/// Added by Liang-Jun Zhu, 2016-6-22
void DataCenter::SetScenario(SimulationModule *pModule) {
    if (nullptr == m_scenario && nullptr == getScenarioData()) {
        throw ModelException("DataCenter", "SetScenario", "Scenarios has not been set!");;
    }
    else {
        pModule->SetScenario(m_scenario);
    }
}

/// Added by Liang-Jun Zhu, 2016-7-2
void DataCenter::SetReaches(SimulationModule *pModule) {
    if (nullptr == m_reaches && nullptr == getReachesData()) {
        throw ModelException("DataCenter", "SetReaches", "Reaches has not been set!");
    }
    pModule->SetReaches(m_reaches);
}

/// Added by Liang-Jun Zhu, 2016-7-28
void DataCenter::SetSubbasins(SimulationModule *pModule) {
    if (nullptr == m_subbasins && nullptr == getSubbasinData()) {
        throw ModelException("DataCenter", "SetSubbasins", "Subbasins data has not been initialized!");
    }
    pModule->SetSubbasins(m_subbasins);
}

void DataCenter::UpdateInput(vector<SimulationModule *> &modules, time_t t) {
    vector<string> moduleIDs = m_factory->GetModuleIDs();
    map<string, SEIMSModuleSetting *>& moduleSettings = m_factory->GetModuleSettings();
    map<string, vector<ParamInfo*> >& moduleInputs = m_factory->GetModuleInputs();
    size_t n = moduleIDs.size();
    string id;
    SimulationModule *pModule;
    for (size_t i = 0; i < n; i++) {
        id = moduleIDs[i];
        pModule = modules[i];
        vector<ParamInfo*> &inputs = moduleInputs[id];
        string dataType = moduleSettings[id]->dataTypeString();
        for (size_t j = 0; j < inputs.size(); j++) {
            ParamInfo *param = inputs[j];
            if (param->DependPara != nullptr) {
                continue;
            }    //the input which comes from other modules will not change when the date is change.
            if (StringMatch(param->Name.c_str(), CONS_IN_ELEV)
                || StringMatch(param->Name.c_str(), CONS_IN_LAT)
                || StringMatch(param->Name.c_str(), CONS_IN_XPR)
                || StringMatch(param->Name.c_str(), CONS_IN_YPR))
                continue;
            if (dataType.length() > 0) {
                int datalen;
                float *data;
                m_climStation->GetTimeSeriesData(t, dataType, &datalen, &data);
                if (StringMatch(param->Name.c_str(), DataType_PotentialEvapotranspiration)) {
                    for (int iData = 0; iData < datalen; iData++) {
                        data[iData] *= m_initParameters[VAR_K_PET]->GetAdjustedValue();
                    }
                }
                pModule->Set1DData(DataType_Prefix_TS, datalen, data);
            }
        }
    }
}

/// added by Huiran GAO, Feb. 2017
/// redesigned by Liangjun Zhu, 08/16/17
void DataCenter::updateParametersByScenario(int subbsnID) {
    if (nullptr == m_scenario) {
        return;
    }
    map<int, BMPFactory *> bmpFactories = m_scenario->GetBMPFactories();
    for (auto iter = bmpFactories.begin(); iter != bmpFactories.end(); iter++) {
        /// Key is uniqueBMPID, which is calculated by BMP_ID * 100000 + subScenario;
        if (iter->first / 100000 != BMP_TYPE_AREALSTRUCT) {
            continue;
        }
        cout << "Update parameters by Scenario settings." << endl;
        map<int, BMPArealStruct *> arealbmps = ((BMPArealStructFactory *)iter->second)->getBMPsSettings();
        float *mgtunits = ((BMPArealStructFactory *)iter->second)->getRasterData();
        vector<int> selIDs = ((BMPArealStructFactory *)iter->second)->getUnitIDs();
        /// Get landuse data of current subbasin ("0_" for the whole basin)
        string lur = GetUpper(ValueToString(subbsnID) + "_" + VAR_LANDUSE);
        int nsize = -1;
        float *ludata = nullptr;
        m_rsMap[lur]->getRasterData(&nsize, &ludata);

        for (auto iter2 = arealbmps.begin(); iter2 != arealbmps.end(); iter2++) {
            cout << "  - SubScenario ID: " << iter->second->GetSubScenarioId() << ", BMP name: "
                << iter2->second->getBMPName() << endl;
            vector<int> suitablelu = iter2->second->getSuitableLanduse();
            map<string, ParamInfo *> updateparams = iter2->second->getParameters();
            for (auto iter3 = updateparams.begin(); iter3 != updateparams.end(); iter3++) {
                string paraname = iter3->second->Name;
                cout << "   -- Parameter ID: " << paraname << endl;
                /// Check whether the parameter is existed in m_parametersInDB.
                ///   If existed, update the missing values, otherwise, print warning message and continue.
                if (m_initParameters.find(paraname) == m_initParameters.end()) {
                    cout << "      Warning: the parameter is not defined in PARAMETER table, and "
                        " will not work as expected." << endl;
                    continue;
                }
                ParamInfo *tmpparam = m_initParameters[paraname];
                if (iter3->second->Change.empty()) {
                    iter3->second->Change = tmpparam->Change;
                }
                iter3->second->Maximum = tmpparam->Maximum;
                iter3->second->Minimun = tmpparam->Minimun;
                // Perform update
                string remoteFileName = GetUpper(ValueToString(subbsnID) + "_" + paraname);
                if (m_rsMap.find(remoteFileName) == m_rsMap.end()) {
                    cout << "      Warning: the parameter name: " << remoteFileName <<
                        " is not loaded as 1D or 2D raster, and "
                        " will not work as expected." << endl;
                    continue;
                }
                if (m_rsMap[remoteFileName]->is2DRaster()) {
                    int lyr = -1;
                    float **data2D = nullptr;
                    m_rsMap[remoteFileName]->get2DRasterData(&nsize, &lyr, &data2D);
                    iter3->second->Adjust2DRaster(nsize, lyr, data2D, mgtunits, selIDs, ludata, suitablelu);
                }
                else {
                    float *data = nullptr;
                    m_rsMap[remoteFileName]->getRasterData(&nsize, &data);
                    iter3->second->Adjust1DRaster(nsize, data, mgtunits, selIDs, ludata, suitablelu);
                }
            }
        }
    }
}
