/*!
 * \brief Parse the input arguments as a class.
 * \author Liangjun Zhu
 * \date Feb 2018
 */
#ifndef SEIMS_INVOKE_H
#define SEIMS_INVOKE_H

#include "seims.h"

using namespace std;

/*!
 * \brief Parse the input arguments of SEIMS.
 */
class InputArgs: NotCopyable {
public:
    InputArgs(const string& modelPath, char* host, uint16_t port,
              int scenarioID, int calibrationID,
              int numThread, LayeringMethod lyrMethod);

    static InputArgs* Init(int argc, const char** argv);

    ~InputArgs() {
    };

public:
    string m_model_path; ///< file path which contains the model input files
    string m_model_name; ///< model_name
    char m_host_ip[16]; ///< Host IP address of MongoDB database
    uint16_t m_port; ///< port of MongoDB, 27017 is default
    int m_thread_num; ///< thread number for OpenMP
    LayeringMethod m_layer_mtd; ///< Layering method for sequencing computing
    int m_scenario_id; ///< scenario ID defined in Database, -1 for no use.
    int m_calibration_id; ///< calibration ID defined in Database (PARAMETERS), -1 for no use.
};

#endif /* SEIMS_INVOKE_H */
