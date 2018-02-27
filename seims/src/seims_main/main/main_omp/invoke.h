/*!
 * \brief Implementation of invoking SEIMS, the main entrance.
 * \author Junzhi Liu, Liangjun Zhu
 * \date May 2010
 * \revised Feb 2017
 */
#ifndef SEIMS_INVOKE_H
#define SEIMS_INVOKE_H

#ifdef USE_MONGODB

#include "DataCenterMongoDB.h"

#endif /* USE_MONGODB */

#include "ModelMain.h"
/// include module_setting related
#include "tinyxml.h"
#include "ParamInfo.h"
#include "Settings.h"

using namespace std;

/*!
 * \brief Parse the input arguments of SEIMS.
 */
class InputArgs : private NotCopyable {
public:
    InputArgs(string modelPath, char *host, uint16_t port,
              int scenarioID, int calibrationID,
              int numThread, LayeringMethod lyrMethod);

    static InputArgs *Init(int argc, const char **argv);

    ~InputArgs() {};

public:
    string m_model_path; ///< file path which contains the model input files
    string m_model_name; ///< model_name
    char m_host_ip[16];  ///< Host IP address of MongoDB database
    uint16_t m_port;  ///< port of MongoDB, 27017 is default
    int m_thread_num;  ///< thread number for OpenMP
    LayeringMethod m_layer_mtd;  ///< Layering method for sequencing computing
    int m_scenario_id;  ///< scenario ID defined in Database, -1 for no use.
    int m_calibration_id;  ///< calibration ID defined in Database (PARAMETERS), -1 for no use.
};

/*!
 * \brief SEIMS main invoke entrance using MongoDB
 *
 * \param[in] in_args \sa InputArgs
 * \param[in] subbasin_id 0 means the whole basin which is default for omp version.
 */
int MainMongoDB(InputArgs *in_args, int subbasin_id = 0);

#endif /* SEIMS_INVOKE_H */
