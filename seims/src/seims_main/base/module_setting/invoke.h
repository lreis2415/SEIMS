/*!
 * \brief Parse the input arguments as a class.
 * \author Liangjun Zhu
 * \date Feb 2018
 */
#ifndef SEIMS_INVOKE_H
#define SEIMS_INVOKE_H

#include "basic.h"

#include "seims.h"

using namespace ccgl;

/*!
 * \brief Parse the input arguments of SEIMS.
 */
class InputArgs: Interface {
public:
    InputArgs(const string& model_path, char* host, uint16_t port,
              int scenario_id, int calibration_id,
              int thread_num, LayeringMethod lyr_mtd);

    static InputArgs* Init(int argc, const char** argv);

public:
    string model_path;      ///< file path which contains the model input files
    string model_name;      ///< model_name
    char host_ip[16];       ///< Host IP address of MongoDB database
    uint16_t port;          ///< port of MongoDB, 27017 is default
    int thread_num;         ///< thread number for OpenMP
    LayeringMethod lyr_mtd; ///< Layering method for sequencing computing
    int scenario_id;        ///< scenario ID defined in Database, -1 for no use.
    int calibration_id;     ///< calibration ID defined in Database (PARAMETERS), -1 for no use.
};

#endif /* SEIMS_INVOKE_H */
