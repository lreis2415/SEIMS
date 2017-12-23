/*!
 * \brief Implementation of invoking SEIMS, the main entrance.
 * \author Junzhi Liu, Liangjun Zhu
 * \date May 2010
 * \revised Feb 2017
 */
#ifndef SEIMS_INVOKE_H
#define SEIMS_INVOKE_H

#include "DataCenter.h"
#include "ModelMain.h"
/// include module_setting related
#include "tinyxml.h"
#include "ParamInfo.h"
#include "Settings.h"

using namespace std;

/*!
 * \brief SEIMS main invoke entrance using MongoDB
 *
 * \param[in] modelPath file path which contains the model input files
 * \param[in] host \a char*, Host IP address of MongoDB database
 * \param[in] port \a int, port of MongoDB, 27017 is default
 * \param[in] scenarioID \a int, scenario ID defined in Database, -1 for no use.
 * \param[in] calibrationID \a int, calibration ID defined in Database (PARAMETERS), -1 for no use.
 * \param[in] numThread \a int, thread number for OpenMP
 * \param[in] lyrMethod \sa LayeringMethod, method for sequencing Grid
 */
int MainMongoDB(string modelPath, char* host, uint16_t port, int scenarioID, int calibrationID,
                int numThread, LayeringMethod lyrMethod);

#endif /* SEIMS_INVOKE_H */
