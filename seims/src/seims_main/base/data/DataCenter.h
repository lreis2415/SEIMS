/*!
 * \brief Data center for running SEIMS
 *        including configuration, input data, output data, etc.
 *        All interaction with database should be implemented here.
 * \author Liangjun Zhu
 * \date May 2017
 */
#ifndef SEIMS_DATA_CENTER_H
#define SEIMS_DATA_CENTER_H

#include "seims.h"
#include "MongoUtil.h"

#include "InputStation.h"
#include "SettingsInput.h"
#include "SettingsOutput.h"
#include "clsReach.h"
#include "clsSubbasin.h"
#include "Scenario.h"
#include "clsInterpolationWeightData.h"

using namespace std;

const int    MAIN_DB_TABS_REQ_NUM = 6;
const string MAIN_DB_TABS_REQ[MAIN_DB_TABS_REQ_NUM] = { DB_TAB_FILE_IN, DB_TAB_FILE_OUT, DB_TAB_SITELIST, 
                                                        DB_TAB_PARAMETERS, DB_TAB_REACH, DB_TAB_SPATIAL };
const int    METEO_VARS_NUM = 6;
const string METEO_VARS[METEO_VARS_NUM] = { DataType_MeanTemperature, DataType_MaximumTemperature,
                                            DataType_MinimumTemperature, DataType_SolarRadiation,
                                            DataType_WindSpeed, DataType_RelativeAirMoisture };
typedef clsRasterData<float>*   FloatRaster;
/*!
 * \ingroup data
 * \class DataCenter
 * \brief Base class of Data center for SEIMS
 * \version 1.0-beta
 */
class DataCenter
{
public:
    /*!
     * \brief Constructor
     * \param modelPath Path of the project, contains config.fig, file.in and file.out
     * \param modulePath Path of SEIMS modules
     * \param layeringMethod Layering method, default is UP_DOWN
     * \param subBasinID Subbasin ID, default is 0, which means the whole basin
     * \param scenarioID Scenario ID, default is -1, which means do not use Scenario
     * \param numThread Thread number for OpenMP, default is 1
     */
    DataCenter(const string modelPath, const string modulePath,
               const LayeringMethod layeringMethod = UP_DOWN,
               const int subBasinID = 0, const int scenarioID = -1, const int numThread = 1);
    //! Destructor
    virtual ~DataCenter(void);

public:
    /**** virtual functions dependent on database IO *****/

    /*! 
     * \brief Check project directory for the required input files
     *        file.in, file.out, config.fig
     */
    virtual bool checkConfigurationFiles(void);
    /*!
     * \brief create OUTPUT folder and clean if already existed
     */
    virtual bool createOutputFolder(void);
    /*!
     * \brief Make sure all the required data are presented
     */
    virtual bool checkModelPreparedData(void) = 0;
    /*!
     * \brief Read climate site data
     */
    virtual void readClimateSiteList(void) = 0;
    /*!
     * \brief Read initial and calibrated parameters
     */
    virtual bool readParametersInDB(void) = 0;
    /*!
    * \brief Read raster data, both 1D and 2D, and insert to m_rsMap
    * \param[in]
    */
    virtual FloatRaster readRasterData(string& remoteFilename) = 0;
    /*!
     * \brief Read interpolated weight data and insert to m_weightDataMap
     * \param[in] remoteFilename \string data file name
     * \param[out] num \int&, data length
     * \param[out] data \float*&, returned data
     */
    virtual void readItpWeightData(string& remoteFilename, int& num, float*& data) = 0;
    /*!
     * \brief Read 1D array data
     * \param[in] paramName \string parameter name
     * \param[in] remoteFilename \string data file name
     * \param[out] num \int&, data length
     * \param[out] data \float*&, returned data
     */
    virtual void read1DArrayData(string& paramName, string& remoteFilename, int& num, float*& data) = 0;
    /*!
     * \brief Read 2D array data and insert to m_2DArrayMap
     * The matrix format is as follows:
     *                           5  (Row number)
     *          RowIdx\ColIdx    0  1  2  3  4
     *                  0        1  9.
     *                  1        2  8. 1.
     *                  2        2  5. 2.
     *                  3        1  2.
     *                  4        4  2. 5. 1. 8.
     * i.e., the first element in each row is the valid number of the current row.
     * \param[in] remoteFilename \string data file name
     * \param[out] rows \int&, first dimension of the 2D Array, i.e., Rows
     * \param[out] cols \int&, second dimension of the 2D Array, i.e., Cols. If each col are different, set cols to 1.
     * \param[out] data \float**&, returned data
     */
    virtual void read2DArrayData(string& remoteFilename, int& rows, int& cols, float**& data) = 0;
    /*!
     * \brief Read IUH data and insert to m_2DArrayMap
     * \param[in] remoteFilename \string data file name
     * \param[out] n \int&, valid cell number
     * \param[out] data \float*&, returned data
     */
    virtual void readIUHData(string& remoteFilename, int& n, float**& data) = 0;
public:
    /**** Accessors: Set and Get *****/

    string getModelName(void) const { return m_modelName; }
    const string getProjectPath(void) const { return m_modelPath; }
    const string getModulePath(void) const { return m_modulePath; }
    string getFileInFullPath(void) const { return m_fileInFile; }
    string getFileOutFullPath(void) const { return m_fileOutFile; }
    string getFileCfgFullPath(void) const { return m_fileCfgFile; }
    const LayeringMethod getLayeringMethod(void) const { return m_lyrMethod; }
    const int getSubbasinID(void) const { return m_subbasinID; }
    const int getScenarioID(void) const { return m_scenarioID; }
    const int getThreadNumber(void) const { return m_threadNum; }
    bool useScenario(void) const { return m_useScenario; }
    string getOutputSceneName(void) const { return m_outputScene; }
    string getOutputScenePath(void) const { return m_outputPath; }
    string getModelMode(void) const { return m_modelMode; }
    int getSubbasinsCount(void) const { return m_nSubbasins; }
    int getOutletID(void) const { return m_outletID; }
    SettingsInput* getSettingInput(void) { return m_input; }
    SettingsOutput* getSettingOutput(void) { return m_output; }
    InputStation* getClimateStation(void) { return m_climStation; }
    clsSubbasins* getSubbasinData(void) { return m_subbasins; }
    clsReaches* getReachesData(void) { return m_reaches; }
    Scenario* getScenarioData(void) { return m_useScenario ? m_scenario : NULL; }
    FloatRaster getMaskData(void) { return m_maskRaster; }
    map<string, FloatRaster>& getRasterDataMap(void) { return m_rsMap; }
    map<string, ParamInfo*>& getInitParameters(void) { return m_initParameters; }
    map<string, float *>& get1DArrayMap(void) { return m_1DArrayMap; }
    map<string, int>& get1DArrayLenMap(void) { return m_1DLenMap; }
    map<string, float **>& get2DArrayMap(void) { return m_2DArrayMap; }
    map<string, int>& get2DArrayRowsMap(void) { return m_2DRowsLenMap; }
    map<string, int>& get2DArrayColsMap(void) { return m_2DColsLenMap; }
    map<string, clsITPWeightData *>& getItpWeightDataMap(void) { return m_weightDataMap; }
    /*!
    * \brief Get file.in configuration
    */
    virtual vector<string>& getFileInStringVector(void);
    /*!
    * \brief Get file.out configuration
    */
    virtual vector<OriginalOutputItem>& getFileOutVector(void) = 0;
    /*!
    * \brief Get subbasin number and outlet ID
    */
    virtual bool getSubbasinNumberAndOutletID(void) = 0;

private:
    /**** Avoid usage of operator = and copy *****/

    /*!
     * \brief Operator= without implementation
     */
    DataCenter& operator=(const DataCenter &another);
    /*!
     * \brief Copy constructor without implementation
     */
    DataCenter(const DataCenter &another);

public:
    string                   m_modelName;     ///< Model name, e.g., model_dianbu30m_longterm
    const string             m_modelPath;     ///< Model path
    const string             m_modulePath;    ///< SEIMS module path
    string                   m_fileInFile;    ///< file.in full path
    string                   m_fileOutFile;   ///< file.out full path
    string                   m_fileCfgFile;   ///< config.fig full path
    const LayeringMethod     m_lyrMethod;     ///< Layering method
    const int                m_subbasinID;    ///< Subbasin ID
    const int                m_scenarioID;    ///< Scenario ID
    const int                m_threadNum;     ///< Thread number for OpenMP
    bool                     m_useScenario;   ///< Model Scenario
    string                   m_outputScene;   ///< Output scenario identifier, e.g. output1 means scenario 1
    string                   m_outputPath;    ///< Output path according to m_outputScene
    vector<string>           m_fileIn1DStrs;  ///< file.in configuration
    string                   m_modelMode;     ///< Storm or Longterm model
    int                      m_nSubbasins;    ///< Number of subbasins
    int                      m_outletID;      ///< Outlet subbasin ID
    SettingsInput*           m_input;         ///< The basic input settings
    SettingsOutput*          m_output;        ///< The user-defined outputs, Q, SED, etc
    InputStation*            m_climStation;   ///< data of input HydroClimate stations
    Scenario*                m_scenario;      ///< BMPs Scenario data
    clsReaches*              m_reaches;       ///< Reaches information
    clsSubbasins*            m_subbasins;     ///< Subbasins information
    FloatRaster              m_maskRaster;    ///< Mask data
    map<string, FloatRaster> m_rsMap;         ///< Map of spatial data, both 1D and 2D
    map<string, ParamInfo*>  m_initParameters; ///< Store parameters from Database
    map<string, float* >     m_1DArrayMap;    ///< 1D array data map, e.g. FLOWOUT_INDEX_D8
    map<string, int>         m_1DLenMap;      ///< 1D array data length map
    map<string, float** >    m_2DArrayMap;    ///< 2D array data map, e.g. ROUTING_LAYERS
    map<string, int>         m_2DRowsLenMap;  ///< Row number of 2D array data map
    map<string, int>         m_2DColsLenMap;  ///< Col number of 2D array data map, CAUTION that nCols may not same for all rows
    map<string, clsITPWeightData *> m_weightDataMap;///< Interpolation weight data map
};
/*!
 * \ingroup data
 * \class DataCenterMongoDB
 * \brief Class of Data center inherited from DataCenter based on MongoDB
 * \version 1.0-beta
 */
class DataCenterMongoDB : public DataCenter
{
public:
    /*!
     * \brief Constructor based on MongoDB
     * \param host IP address of MongoDB
     * \param port Unsigned integer
     * other parameters are the same as \sa DataCenter
     */
    DataCenterMongoDB(const char *host, const uint16_t port, const string modelPath,
                      const string modulePath, const LayeringMethod layeringMethod = UP_DOWN,
                      const int subBasinID = 0, const int scenarioID = -1, const int numThread = 1);
    //! Destructor
    virtual ~DataCenterMongoDB(void);
    /*!
     * \brief Make sure all the required data are presented
     */
    virtual bool checkModelPreparedData(void);
    /*!
     * \brief Get file.in configuration from FILE_IN collection
     */
    virtual vector<string>& getFileInStringVector(void);
    /*!
     * \brief Get file.out configuration from FILE_OUT collection
     */
    virtual vector<OriginalOutputItem>& getFileOutVector(void);
    /*!
     * \brief Get subbasin number and outlet ID
     */
    virtual bool getSubbasinNumberAndOutletID(void);
    /*!
     * \brief Read climate site data from HydroClimate database
     */
    virtual void readClimateSiteList(void);
    /*!
     * \brief Read initial and calibrated parameters
     */
    virtual bool readParametersInDB(void);
    /*!
     * \brief Read raster data, both 1D and 2D, and insert to m_rsMap
     * \param[in]
     */
    virtual FloatRaster readRasterData(string& remoteFilename);
    /*!
     * \brief Read interpolated weight data from MongoDB and insert to m_weightDataMap
     * \param[in] remoteFilename \string data file name
     * \param[out] num \int&, data length
     * \param[out] data \float*&, returned data
     */
    virtual void readItpWeightData(string& remoteFilename, int& num, float*& data);
    /*!
     * \brief Read 1D array data from MongoDB and insert to m_1DArrayMap
     *        CAUTION: Value data type stored in MongoDB MUST be float
     * \param[in] paramName \string parameter name
     * \param[in] remoteFilename \string data file name
     * \param[out] num \int&, data length
     * \param[out] data \float*&, returned data
     */
    virtual void read1DArrayData(string& paramName, string& remoteFilename, int& num, float*& data);
    /*!
     * \brief Read 2D array data from MongoDB database
     * \param[in] spatialData \a MongoGridFS
     * \param[in] remoteFilename \string data file name
     * \param[out] rows \int&, first dimension of the 2D Array, i.e., Rows
     * \param[out] cols \int&, second dimension of the 2D Array, i.e., Cols. If each col are different, set cols to 1.
     * \param[out] data \float**&, returned data
     */
    virtual void read2DArrayData(string& remoteFilename, int& rows, int& cols, float**& data);
    /*!
     * \brief Read IUH data from MongoDB and insert to m_2DArrayMap
     * \param[in] remoteFilename \string data file name
     * \param[out] n \int&, valid cell number
     * \param[out] data \float*&, returned data
     */
    virtual void readIUHData(string& remoteFilename, int& n, float**& data);
public:
    /******* MongoDB specified functions *********/

    /*!
     * \brief Query database name
     */
    string QueryDatabaseName(bson_t* query, const char* tabname);
public:
    /**** Accessors: Set and Get *****/

    const char* getHostIP(void) const { return m_mongodbIP; }
    const uint16_t getPort(void) const { return m_mongodbPort; }
    string getClimateDBName(void) const { return m_climDBName; }
    string getScenarioDBName(void) const { return m_scenDBName; }
    MongoClient* getMongoClient(void) const { return m_mongoClient; }
    MongoDatabase* getMainDatabase(void) const { return m_mainDatabase; }
    MongoGridFS* getMongoGridFS(void) const { return m_spatialGridFS; }
private:
    const char*              m_mongodbIP;     ///< Host IP address of MongoDB
    const uint16_t           m_mongodbPort;   ///< Port
    string                   m_climDBName;    ///< Climate database name
    string                   m_scenDBName;    ///< Scenario database name
    MongoClient*             m_mongoClient;   ///< MongoDB Client
    MongoDatabase*           m_mainDatabase;  ///< Main model database
    MongoGridFS*             m_spatialGridFS; ///< Spatial data handler
};
#endif /* SEIMS_DATA_CENTER_H */
