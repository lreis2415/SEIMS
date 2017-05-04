#include "DataCenter.h"

/******* DataCenter ********/
DataCenter::DataCenter(const string dbName, const string projectPath, const string modulePath,
                       const LayeringMethod layeringMethod /* = UP_DOWN */,
                       const int subBasinID /* = 0 */, const int scenarioID /* = -1 */,
                       const int numThread /* = 1 */):
        m_modelName(dbName), m_projPath(projectPath), m_modulePath(modulePath),
        m_lyrMethod(layeringMethod), m_subbasinID(subBasinID), m_scenarioID(scenarioID),
        m_threadNum(numThread){

}

DataCenter::~DataCenter() {

}

/******* DataCenterMongoDB ********/
DataCenterMongoDB::DataCenterMongoDB(const char *host, const uint16_t port, const string dbName, const string projectPath,
                                     const string modulePath, const LayeringMethod layeringMethod /* = UP_DOWN */,
                                     const int subBasinID /* = 0 */, const int scenarioID /* = -1 */,
                                     const int numThread /* = 1 */):
        m_mongodbIP(host), m_mongodbPort(port),
        DataCenter(dbName, projectPath, modulePath, layeringMethod, subBasinID, scenarioID, numThread) {

}

DataCenterMongoDB::~DataCenterMongoDB() {

}

bool DataCenterMongoDB::CheckProjectData() {
    /// TODO
    return true;
}