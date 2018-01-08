#include "DataCenter.h"

DataCenter::DataCenter(string &modelPath, string &modulePath,
                       LayeringMethod layeringMethod /* = UP_DOWN */,
                       int subBasinID /* = 0 */, int scenarioID /* = -1 */,
                       int calibrationID /* = -1 */,
                       int numThread /* = 1 */) :
    m_modelPath(modelPath), m_modulePath(modulePath),
    m_lyrMethod(layeringMethod), m_subbasinID(subBasinID), m_scenarioID(scenarioID),
    m_calibrationID(calibrationID),
    m_threadNum(numThread), m_useScenario(false), m_outputScene(DB_TAB_OUT_SPATIAL),
    m_outputPath(""), m_modelMode(""), m_nSubbasins(-1), m_outletID(-1),
    m_input(nullptr), m_output(nullptr), m_climStation(nullptr), m_scenario(nullptr),
    m_reaches(nullptr), m_subbasins(nullptr), m_maskRaster(nullptr) {
    /// Get model name
    size_t nameIdx = modelPath.rfind(SEP);
    m_modelName = modelPath.substr(nameIdx + 1);
    /// Check configuration files
    m_fileInFile = modelPath + SEP + File_Input;
    m_fileOutFile = modelPath + SEP + File_Output;
    m_fileCfgFile = modelPath + SEP + File_Config;
    checkConfigurationFiles();
    /// Clean output folder
    if (m_scenarioID >= 0) { // -1 means no BMPs scenario will be simulated
        m_outputScene += ValueToString(m_scenarioID);
        /// Be aware, m_useScenario will be updated in checkModelPreparedData().
    }
    if (m_calibrationID >= 0) {  // -1 means no calibration setting will be used.
        m_outputScene += "-" + ValueToString(m_calibrationID);
    }
    m_outputPath = m_modelPath + SEP + m_outputScene + SEP;
    createOutputFolder();
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
            StatusMessage(("-----" + it->first + " ...").c_str());
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

bool DataCenter::checkConfigurationFiles() {
    string cfgNames[] = {m_fileInFile, m_fileOutFile, m_fileCfgFile};
    for (int i = 0; i < 3; ++i) {
        if (!FileExists(cfgNames[i])) {
            throw ModelException("DataCenter", "checkConfigurationFiles", cfgNames[i] +
                " does not exist or has not the read permission!");
            return false;
        }
    }
    return true;
}

bool DataCenter::createOutputFolder() {
    return CleanDirectory(m_outputPath);
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

}
