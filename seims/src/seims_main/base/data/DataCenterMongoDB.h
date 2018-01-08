/*!
 * \brief Data center for running SEIMS based on MongoDB.
 *        including configuration, input data, output data, etc.
 *        All interaction with database should be implemented here.
 * \author Liangjun Zhu
 * \date May 2017
 */
#ifndef SEIMS_DATA_CENTER_MONGODB_H
#define SEIMS_DATA_CENTER_MONGODB_H

#include "DataCenter.h"

using namespace std;

/*!
 * \ingroup data
 * \class DataCenterMongoDB
 * \brief Class of Data center inherited from \sa DataCenter based on MongoDB
 * \version 1.0
 */
class DataCenterMongoDB : public DataCenter {
public:
    /*!
     * \brief Constructor based on MongoDB
     * \param host IP address of MongoDB
     * \param port Unsigned integer
     * other parameters are the same as \sa DataCenter
     */
    DataCenterMongoDB(const char *host, uint16_t port, string &modelPath,
                      string &modulePath, LayeringMethod layeringMethod = UP_DOWN,
                      int subBasinID = 0, int scenarioID = -1, int calibrationID = -1,
                      int numThread = 1);
    //! Destructor
    virtual ~DataCenterMongoDB();
    /*!
     * \brief Make sure all the required data are presented
     */
    virtual bool checkModelPreparedData();
    /*!
     * \brief Get file.in configuration from FILE_IN collection
     */
    virtual bool getFileInStringVector();
    /*!
     * \brief Get file.out configuration
     * \param[in] originOutputs \sa OriginalOutputItem
     */
    virtual bool getFileOutVector();
    /*!
     * \brief Get subbasin number and outlet ID
     */
    virtual bool getSubbasinNumberAndOutletID();
    /*!
     * \brief Read climate site data from HydroClimate database
     */
    virtual void readClimateSiteList();
    /*!
     * \brief Read initial and calibrated parameters
     * \version 2017.12.23  lj - read parameters (Impact value) according to calibration ID
     */
    virtual bool readParametersInDB();
    /*!
     * \brief Read raster data, both 1D and 2D, and insert to m_rsMap
     * \param[in] remoteFilename Raster file name.
     */
    virtual FloatRaster *readRasterData(const string &remoteFilename);
    /*!
     * \brief Read interpolated weight data from MongoDB and insert to m_weightDataMap
     * \param[in] remoteFilename \string data file name
     * \param[out] num \int&, data length
     * \param[out] data \float*&, returned data
     */
    virtual void readItpWeightData(string &remoteFilename, int &num, float *&data);
    /*!
     * \brief Read 1D array data from MongoDB and insert to m_1DArrayMap
     *        CAUTION: Value data type stored in MongoDB MUST be float
     * \param[in] paramName \string parameter name
     * \param[in] remoteFilename \string data file name
     * \param[out] num \int&, data length
     * \param[out] data \float*&, returned data
     */
    virtual void read1DArrayData(string &paramName, string &remoteFilename, int &num, float *&data);
    /*!
     * \brief Read 2D array data from MongoDB database
     * \param[in] spatialData \a MongoGridFS
     * \param[in] remoteFilename \string data file name
     * \param[out] rows \int&, first dimension of the 2D Array, i.e., Rows
     * \param[out] cols \int&, second dimension of the 2D Array, i.e., Cols. If each col are different, set cols to 1.
     * \param[out] data \float**&, returned data
     */
    virtual void read2DArrayData(string &remoteFilename, int &rows, int &cols, float **&data);
    /*!
     * \brief Read IUH data from MongoDB and insert to m_2DArrayMap
     * \param[in] remoteFilename \string data file name
     * \param[out] n \int&, valid cell number
     * \param[out] data \float*&, returned data
     */
    virtual void readIUHData(string &remoteFilename, int &n, float **&data);
    /*!
     * \brief Set Raster data for Scenario data
     * \return True if set successfully, otherwise false.
    */
    virtual bool setRasterForScenario();
public:
    /******* MongoDB specified functions *********/

    /*!
     * \brief Query database name
     */
    string QueryDatabaseName(bson_t *query, const char *tabname);
public:
    /**** Accessors: Set and Get *****/

    const char *getHostIP() const { return m_mongodbIP; }
    uint16_t getPort() const { return m_mongodbPort; }
    string getClimateDBName() const { return m_climDBName; }
    string getScenarioDBName() const { return m_scenDBName; }
    MongoClient *getMongoClient() const { return m_mongoClient; }
    MongoDatabase *getMainDatabase() const { return m_mainDatabase; }
    MongoGridFS *getMongoGridFS() const { return m_spatialGridFS; }
private:
    const char *m_mongodbIP;     ///< Host IP address of MongoDB
    const uint16_t m_mongodbPort;   ///< Port
    string m_climDBName;    ///< Climate database name
    string m_scenDBName;    ///< Scenario database name
    MongoClient *m_mongoClient;   ///< MongoDB Client
    MongoDatabase *m_mainDatabase;  ///< Main model database
    MongoGridFS *m_spatialGridFS; ///< Spatial data handler
};
#endif /* SEIMS_DATA_CENTER_MONGODB_H */
