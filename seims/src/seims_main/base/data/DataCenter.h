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

using namespace std;
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
     * \param dbName Model name, e.g., model_dianbu30m_longterm
     * \param projectPath Path of the project, contains config.fig, file.in and file.out
     * \param modulePath Path of SEIMS modules
     * \param layeringMethod Layering method, default is UP_DOWN
     * \param subBasinID Subbasin ID, default is 0, which means the whole basin
     * \param scenarioID Scenario ID, default is -1, which means do not use Scenario
     * \param numThread Thread number for OpenMP, default is 1
     */
    DataCenter(const string dbName, const string projectPath, const string modulePath,
               const LayeringMethod layeringMethod = UP_DOWN,
               const int subBasinID = 0, const int scenarioID = -1, const int numThread = 1);
    //! Destructor
    virtual ~DataCenter(void);

public:
    /**** virtual functions dependent on database IO *****/

    /*!
     * \brief Make sure all the required data are presented
     */
    virtual bool CheckProjectData(void) = 0;

public:
    /**** Accessors: Set and Get *****/

    const string getModelName(void) const { return m_modelName; }
    const string getProjectPath(void) const { return m_projPath; }
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

private:
    const string             m_modelName; ///< Model name
    const string             m_projPath; ///< Project path
    const string             m_modulePath; ///< SEIMS module path
    const LayeringMethod     m_lyrMethod; ///< Layering method
    const int                m_subbasinID; ///< Subbasin ID
    const int                m_scenarioID; ///< Scenario ID
    const int                m_threadNum; ///< Thread number for OpenMP
};
/*!
 * \ingroup data
 * \class DataCenterMongoDB
 * \brief Class of Data center inherited from DataCenter based on MongoDB
 * \version 1.0-beta
 */
class DataCenterMongoDB :public DataCenter
{
public:
    /*!
     * \brief Constructor based on MongoDB
     * \param host IP address of MongoDB
     * \param port Unsigned integer
     * other parameters are the same as \sa DataCenter
     */
    DataCenterMongoDB(const char *host, const uint16_t port, const string dbName, const string projectPath,
                      const string modulePath, const LayeringMethod layeringMethod = UP_DOWN,
                      const int subBasinID = 0, const int scenarioID = -1, const int numThread = 1);
    //! Destructor
    virtual ~DataCenterMongoDB(void);
    /*!
     * \brief Make sure all the required data are presented
     */
    virtual bool CheckProjectData(void);

public:
    /**** Accessors: Set and Get *****/

    const char* getHostIP(void) const { return m_mongodbIP; }
    const uint16_t getPort(void) const { return m_mongodbPort; }

private:
    const char*              m_mongodbIP; ///< Host IP address of MongoDB
    const uint16_t           m_mongodbPort; ///< Port
};

#endif /* SEIMS_DATA_CENTER_H */
