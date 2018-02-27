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

/*!
 * \ingroup data
 * \class DataCenter
 * \brief Base class of Data center for SEIMS
 * \version 1.0-beta
 */
class DataCenter {
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
    DataCenter(string &modelPath, string &modulePath,
               LayeringMethod layeringMethod = UP_DOWN,
               int subBasinID = 0, int scenarioID = -1, int calibrationID = -1,
               int numThread = 1);
    //! Destructor
    virtual ~DataCenter();

public:
    /**** virtual functions dependent on database IO *****/

    /*! 
     * \brief Check project directory for the required input files
     *        file.in, file.out, config.fig
     */
    virtual bool checkConfigurationFiles();
    /*!
     * \brief create OUTPUT folder and clean if already existed
     */
    virtual bool createOutputFolder();
    /*!
     * \brief Make sure all the required data are presented
     */
    virtual bool checkModelPreparedData() = 0;
    /*!
     * \brief Read climate site data
     */
    virtual void readClimateSiteList() = 0;
    /*!
     * \brief Read initial and calibrated parameters
     */
    virtual bool readParametersInDB() = 0;
    /*!
     * \brief Output calibrated parameters to txt file
     */
    void dumpCaliParametersInDB();
    /*!
     * \brief Read raster data, both 1D and 2D, and insert to m_rsMap
     * \param[in] remoteFilename Raster file name.
     */
    virtual FloatRaster *readRasterData(const string &remoteFilename) = 0;
    /*!
     * \brief Read interpolated weight data and insert to m_weightDataMap
     * \param[in] remoteFilename \string data file name
     * \param[out] num \int&, data length
     * \param[out] data \float*&, returned data
     */
    virtual void readItpWeightData(string &remoteFilename, int &num, float *&data) = 0;
    /*!
     * \brief Read 1D array data
     * \param[in] paramName \string parameter name
     * \param[in] remoteFilename \string data file name
     * \param[out] num \int&, data length
     * \param[out] data \float*&, returned data
     */
    virtual void read1DArrayData(string &paramName, string &remoteFilename, int &num, float *&data) = 0;
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
    virtual void read2DArrayData(string &remoteFilename, int &rows, int &cols, float **&data) = 0;
    /*!
     * \brief Read IUH data and insert to m_2DArrayMap
     * \param[in] remoteFilename \string data file name
     * \param[out] n \int&, valid cell number
     * \param[out] data \float*&, returned data
     */
    virtual void readIUHData(string &remoteFilename, int &n, float **&data) = 0;
    /*!
     * \brief Make lapse 2D array data and insert to m_2DArrayMap
     * \param[in] remoteFilename \string data file name
     * \param[out] rows \int&, first dimension of the 2D Array, i.e., Rows
     * \param[out] cols \int&, second dimension of the 2D Array, i.e., Cols
     * \param[out] data \float**&, returned data
     */
    virtual void setLapseData(string &remoteFilename, int &rows, int &cols, float **&data);
    /*!
     * \brief Set Raster data for Scenario data
     * \return True if set successfully, otherwise false.
     */
    virtual bool setRasterForScenario() = 0;
public:
    /**** Accessors: Set and Get *****/

    string getModelName() const { return m_modelName; }
    const string getProjectPath() const { return m_modelPath; }
    const string getModulePath() const { return m_modulePath; }
    string getFileInFullPath() const { return m_fileInFile; }
    string getFileOutFullPath() const { return m_fileOutFile; }
    string getFileCfgFullPath() const { return m_fileCfgFile; }
    LayeringMethod getLayeringMethod() const { return m_lyrMethod; }
    int getSubbasinID() const { return m_subbasinID; }
    int getScenarioID() const { return m_scenarioID; }
    int getCalibrationID() const { return m_calibrationID; }
    int getThreadNumber() const { return m_threadNum; }
    bool useScenario() const { return m_useScenario; }
    string getOutputSceneName() const { return m_outputScene; }
    string getOutputScenePath() const { return m_outputPath; }
    string getModelMode() const { return m_modelMode; }
    int getSubbasinsCount() const { return m_nSubbasins; }
    int getOutletID() const { return m_outletID; }
    SettingsInput *getSettingInput() { return m_input; }
    SettingsOutput *getSettingOutput() { return m_output; }
    InputStation *getClimateStation() { return m_climStation; }
    clsSubbasins *getSubbasinData() { return m_subbasins; }
    clsReaches *getReachesData() { return m_reaches; }
    Scenario *getScenarioData() { return m_useScenario ? m_scenario : nullptr; }
    FloatRaster *getMaskData() { return m_maskRaster; }
    map<string, FloatRaster *> &getRasterDataMap() { return m_rsMap; }
    map<string, ParamInfo *> &getInitParameters() { return m_initParameters; }
    map<string, float *> &get1DArrayMap() { return m_1DArrayMap; }
    map<string, int> &get1DArrayLenMap() { return m_1DLenMap; }
    map<string, float **> &get2DArrayMap() { return m_2DArrayMap; }
    map<string, int> &get2DArrayRowsMap() { return m_2DRowsLenMap; }
    map<string, int> &get2DArrayColsMap() { return m_2DColsLenMap; }
    map<string, clsITPWeightData *> &getItpWeightDataMap() { return m_weightDataMap; }
    /*!
    * \brief Get file.in configuration
    */
    virtual bool getFileInStringVector();
    /*!
    * \brief Get file.out configuration
    * \param[in] originOutputs \sa OriginalOutputItem
    */
    virtual bool getFileOutVector() = 0;
    /*!
    * \brief Get subbasin number and outlet ID
    */
    virtual bool getSubbasinNumberAndOutletID() = 0;

private:
    /**** Avoid usage of operator = and copy *****/

    /*!
     * \brief Operator= without implementation
     */
    DataCenter &operator=(const DataCenter &another);
    /*!
     * \brief Copy constructor without implementation
     */
    DataCenter(const DataCenter &another);

public:
    string m_modelName;     ///< Model name, e.g., model_dianbu30m_longterm
    const string m_modelPath;     ///< Model path
    const string m_modulePath;    ///< SEIMS module path
    string m_fileInFile;    ///< file.in full path
    string m_fileOutFile;   ///< file.out full path
    string m_fileCfgFile;   ///< config.fig full path
    const LayeringMethod m_lyrMethod;     ///< Layering method
    const int m_subbasinID;    ///< Subbasin ID
    const int m_scenarioID;    ///< Scenario ID
    const int m_calibrationID; ///< Calibration ID
    const int m_threadNum;     ///< Thread number for OpenMP
    bool m_useScenario;   ///< Model Scenario
    string m_outputScene;   ///< Output scenario identifier, e.g. output1 means scenario 1
    string m_outputPath;    ///< Output path (with / in the end) according to m_outputScene
    vector<string> m_fileIn1DStrs;  ///< file.in configuration
    vector<OrgOutItem> m_OriginOutItems;///< file.out configuration
    string m_modelMode;     ///< Storm or Longterm model
    int m_nSubbasins;    ///< Number of subbasins
    int m_outletID;      ///< Outlet subbasin ID
    SettingsInput *m_input;         ///< The basic input settings
    SettingsOutput *m_output;        ///< The user-defined outputs, Q, SED, etc
    InputStation *m_climStation;   ///< data of input HydroClimate stations
    Scenario *m_scenario;      ///< BMPs Scenario data
    clsReaches *m_reaches;       ///< Reaches information
    clsSubbasins *m_subbasins;     ///< Subbasins information
    FloatRaster *m_maskRaster;    ///< Mask data
    map<string, FloatRaster *> m_rsMap;        ///< Map of spatial data, both 1D and 2D
    map<string, ParamInfo *> m_initParameters;  ///< Store parameters from Database (PARAMETERS collection)
    map<string, float *> m_1DArrayMap;    ///< 1D array data map, e.g. FLOWOUT_INDEX_D8
    map<string, int> m_1DLenMap;      ///< 1D array data length map
    map<string, float **> m_2DArrayMap;    ///< 2D array data map, e.g. ROUTING_LAYERS
    map<string, int> m_2DRowsLenMap;  ///< Row number of 2D array data map
    map<string, int> m_2DColsLenMap;  ///< Col number of 2D array data map, CAUTION that nCols may not same for all rows
    map<string, clsITPWeightData *> m_weightDataMap; ///< Interpolation weight data map
};
#endif /* SEIMS_DATA_CENTER_H */
