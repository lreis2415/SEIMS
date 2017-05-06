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

using namespace std;

const int    MAIN_DB_TABS_REQ_NUM = 6;
const string MAIN_DB_TABS_REQ[MAIN_DB_TABS_REQ_NUM] = { DB_TAB_FILE_IN, DB_TAB_FILE_OUT, DB_TAB_SITELIST, 
                                                        DB_TAB_PARAMETERS, DB_TAB_REACH, DB_TAB_SPATIAL };

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
     * \brief Make sure all the required data are presented
     */
    virtual bool CheckModelPreparedData(void) = 0;
    /*!
    * \brief Get file.in configuration
    */
    virtual vector<string>& getFileInStringVector(void);

public:
    /**** Accessors: Set and Get *****/

    string getModelName(void) const { return m_modelName; }
    const string getProjectPath(void) const { return m_modelPath; }
    const string getModulePath(void) const { return m_modulePath; }
    const LayeringMethod getLayeringMethod(void) const { return m_lyrMethod; }
    const int getSubbasinID(void) const { return m_subbasinID; }
    const int getScenarioID(void) const { return m_scenarioID; }
    const int getThreadNumber(void) const { return m_threadNum; }

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
    const string             m_fileInFile;    ///< file.in full path
    const string             m_modulePath;    ///< SEIMS module path
    const LayeringMethod     m_lyrMethod;     ///< Layering method
    const int                m_subbasinID;    ///< Subbasin ID
    const int                m_scenarioID;    ///< Scenario ID
    const int                m_threadNum;     ///< Thread number for OpenMP
    bool                     m_useScenario;   ///< Model Scenario
    vector<string>           m_fileIn1DStrs;  ///< file.in configuration
    string                   m_modelMode;     ///< Storm or Longterm model
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
    virtual bool CheckModelPreparedData(void);
    /*!
     * \brief Get file.in configuration from FILE_IN collection
     */
    virtual vector<string>& getFileInStringVector(void);
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

private:
    const char*              m_mongodbIP;     ///< Host IP address of MongoDB
    const uint16_t           m_mongodbPort;   ///< Port
    string                   m_climDBName;    ///< Climate database name
    string                   m_scenDBName;    ///< Scenario database name
    MongoClient*             m_mongoClient;   ///< MongoDB Client
    MongoDatabase*           m_mainDatabase;  ///< Main model database
    MongoDatabase*           m_climDatabase;  ///< Climate database
    MongoDatabase*           m_scenDatabase;  ///< Scenario database
};

#endif /* SEIMS_DATA_CENTER_H */
