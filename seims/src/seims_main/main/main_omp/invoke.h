/*!
 * \ingroup main
 * \file invoke.h
 * \brief Main header file for the application
 * \author Junzhi Liu
 * \date May 2010
 *
 * 
 */
#pragma once

#include <string>
#include "util.h"
#include "tinyxml.h"
//#include <io.h>
#include "ParamInfo.h"
#include "PrintInfo.h"
//#include "DBManager.h"
//#include "SiteInfo.h"
#include "Settings.h"
#include "SettingsInput.h"
#include "SettingsOutput.h"
#include "StringList.h"
#include "ModelMain.h"
#include "ModelException.h"

using namespace std;

//! Get the root path of the current executable (i.e., seims_omp or seims).
string _GetApplicationPath();

//! Check database to make sure the collections (tables) are provided
void checkDatabase(mongoc_client_t *conn, string dbName);

//! Check project directory for the required input files
void checkProject(string projectPath);

//! Check table exists or not
void checkTable(vector<string> &tableNameList, string dbName, const char *tableName);

//! Is valid IP address?
bool isIPAddress(const char *ip);

//! Is file path existed?
bool isPathExists(const char *path);

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


//void	testMainSQLite(string,int,int,LayeringMethod); // Deprecated
//void	testBMP();
//void	testModule();
//void	testSettingInput();
//void	testSettingOutput();
//void	testRaster();







