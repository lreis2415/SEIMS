/*!
 * \ingroup main
 * \file invoke.h
 * \brief Main header file for the application
 * \author Junzhi Liu
 * \date May 2010
 *
 *
 */
#ifndef SEIMS_INVOKE
#define SEIMS_INVOKE

#include "ModelMain.h"
/// include module_setting related
#include "tinyxml.h"
#include "ParamInfo.h"
#include "Settings.h"

using namespace std;

////! Get the root path of the current executable (i.e., seims_omp or seims).
//string _GetApplicationPath();

//! Check database to make sure the collections (tables) are provided
void checkDatabase(mongoc_client_t *conn, string dbName);

//! Check project directory for the required input files
void checkProject(string projectPath);

//! Check table exists or not
void checkTable(vector<string> &tableNameList, string dbName, const char *tableName);

////! Is file path existed?
//bool isPathExists(const char *path);

/*!
 * \brief SEIMS main invoke entrance using MongoDB
 *
 * \param[in] modelPath file path which contains the model input files
 * \param[in] host \a char*, Host IP address of MongoDB database
 * \param[in] port \a int, port of MongoDB, 27017 is default
 * \param[in] scenarioID \a int,
 * \param[in] numThread \a int, thread number for OpenMP
 * \param[in] layingMethod \sa LayeringMethod, method for sequencing Grid
 */
void MainMongoDB(string, char *, int, int, int, LayeringMethod);

#endif
