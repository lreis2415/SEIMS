/*!
 * \file DataCenterMongoDB.h
 * \brief Data center for running SEIMS based on MongoDB.
 *        including configuration, input data, output data, etc.
 *
 * Changelog:
 *   - 1. 2017-05-30 - lj - Initial implementation.
 *   - 2. 2021-04-06 - lj - Compatible with different flow direction algorithms.
 *
 *
 * \author Liangjun Zhu
 */
#ifndef SEIMS_DATA_CENTER_MONGODB_H
#define SEIMS_DATA_CENTER_MONGODB_H

#include "DataCenter.h"

/*!
 * \ingroup data
 * \class DataCenterMongoDB
 * \brief Class of Data center inherited from DataCenter based on MongoDB
 * \version 1.3
 */
class DataCenterMongoDB: public DataCenter {
public:
    /*!
     * \brief Constructor based on MongoDB
     * \param[in] input_args Input arguments of SEIMS
     * \param[in] client MongoDB connection client
     * \param[in] spatial_gfs_in MongoDB GridFS that stores input data
     * \param[in] spatial_gfs_out MongoDB GridFS that stores output data
     * \param[in] factory SEIMS modules factory
     * \param[in] subbasin_id Subbasin ID, 0 is the default for entire watershed
     */
    DataCenterMongoDB(InputArgs* input_args, MongoClient* client,
                      MongoGridFs* spatial_gfs_in, MongoGridFs* spatial_gfs_out,
                      ModuleFactory* factory, int subbasin_id = 0);
    //! Destructor
    ~DataCenterMongoDB();
    /*!
     * \brief Make sure all the required data are presented
     */
    bool CheckModelPreparedData() OVERRIDE;
    /*!
     * \brief Get file.in configuration from FILE_IN collection
     */
    bool GetFileInStringVector() OVERRIDE;
    /*!
     * \brief Get file.out configuration
     */
    bool GetFileOutVector() OVERRIDE;
    /*!
     * \brief Read climate site data from HydroClimate database
     */
    void ReadClimateSiteList() OVERRIDE;
    /*!
     * \brief Read initial and calibrated parameters
     *
     * Changlog:
     *   - 1. 2017-12-23 - lj - Read parameters (Impact value) according to calibration ID.
     */
    bool ReadParametersInDB() OVERRIDE;
    /*!
     * \brief Get subbasin number and outlet ID
     */
    int ReadIntParameterInDB(const char* param_name) OVERRIDE;
    /*!
     * \brief Read raster data, both 1D and 2D, and insert to m_rsMap
     * \param[in] remote_filename Raster file name.
     * \param[in] flt_rst Float raster data
     */
    bool ReadRasterData(const string& remote_filename, FloatRaster*& flt_rst) OVERRIDE;
    /*!
     * \brief Read raster data, both 1D and 2D, and insert to m_rsMap
     * \param[in] remote_filename Raster file name.
     * \param[in] int_rst Float raster data
     */
    bool ReadRasterData(const string& remote_filename, IntRaster*& int_rst) OVERRIDE;
    /*!
     * \brief Read interpolated weight data from MongoDB and insert to m_weightDataMap
     * \param[in] remote_filename \a string data file name
     * \param[out] num \a int&, data length
     * \param[out] stations \a int& number of stations
     * \param[out] data \a float*&, returned data
     */
    void ReadItpWeightData(const string& remote_filename, int& num, int& stations, FLTPT**& data) OVERRIDE;
    /*!
     * \brief Read 1D array data from MongoDB and insert to m_1DArrayMap
     *        CAUTION: Value data type stored in MongoDB MUST be float
     * \param[in] remote_filename \a string data file name
     * \param[out] num \a int&, data length
     * \param[out] data \a float*&, returned data
     */
    void Read1DArrayData(const string& remote_filename, int& num, FLTPT*& data) OVERRIDE;
    /*!
     * \brief Read 1D integer array data from MongoDB and insert to m_1DArrayMap
     *        CAUTION: Value data type stored in MongoDB MUST be float
     *        TODO: data stored in MongoDB should have an attribute to indicate data type!
     * \param[in] remote_filename \a string data file name
     * \param[out] num \a int&, data length
     * \param[out] data \a int*&, returned data
     */
    void Read1DArrayData(const string& remote_filename, int& num, int*& data) OVERRIDE;
    /*!
     * \brief Read 2D array data from MongoDB database
     * \param[in] remote_filename \a string data file name
     * \param[out] rows \a int&, first dimension of the 2D Array, i.e., Rows
     * \param[out] cols \a int&, second dimension of the 2D Array, i.e., Cols. If each col are different, set cols to 1.
     * \param[out] data \a float**&, returned data
     */
    void Read2DArrayData(const string& remote_filename, int& rows, int& cols, FLTPT**& data) OVERRIDE;
    // Read 2D integer array data
    void Read2DArrayData(const string& remote_filename, int& rows, int& cols, int**& data) OVERRIDE;
    /*!
     * \brief Read IUH data from MongoDB and insert to m_2DArrayMap
     * \param[in] remote_filename \a string data file name
     * \param[out] n \a int&, valid cell number
     * \param[out] data \a float*&, returned data
     */
    void ReadIuhData(const string& remote_filename, int& n, FLTPT**& data) OVERRIDE;
    /*!
     * \brief Set Raster data for Scenario data
     * \return True if set successfully, otherwise false.
    */
    bool SetRasterForScenario() OVERRIDE;

    /******* MongoDB specified functions *********/

    /*!
     * \brief Query database name
     */
    string QueryDatabaseName(bson_t* query, const char* tabname);
public:
    /**** Accessors: Set and Get *****/

    const char* GetHostIp() const { return mongodb_ip_; }
    uint16_t GetPort() const { return mongodb_port_; }
    string GetClimateDBName() const { return clim_dbname_; }
    string GetScenarioDBName() const { return scenario_dbname_; }
    MongoClient* GetMongoClient() const { return mongo_client_; }
    MongoDatabase* GetMainDatabase() const { return main_database_; }
    MongoGridFs* GetMongoGridFs() const { return spatial_gridfs_; }
    MongoGridFs* GetMongoGridFsOutput() const { return spatial_gfs_out_; }
private:
    const char* mongodb_ip_;       ///< Host IP address of MongoDB
    const uint16_t mongodb_port_;  ///< Port
    string clim_dbname_;           ///< Climate database name
    string scenario_dbname_;       ///< Scenario database name
    MongoClient* mongo_client_;    ///< MongoDB Client
    MongoDatabase* main_database_; ///< Main model database
    MongoGridFs* spatial_gridfs_;  ///< Spatial data handler
    MongoGridFs* spatial_gfs_out_; ///< Spatial data handler
};
#endif /* SEIMS_DATA_CENTER_MONGODB_H */
