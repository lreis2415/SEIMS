/*!
 * \brief Parse the input arguments as a class which can be easily extended.
 * \author Liangjun Zhu
 * \changelog 2018-02-01 - lj - Initial implementation.\n
 *            2018-06-06 - lj - Add parameters related to MPI version, e.g., group method.\n
 */
#ifndef SEIMS_INPUT_ARGUMENTS_H
#define SEIMS_INPUT_ARGUMENTS_H

#include "basic.h"

#include "seims.h"

using namespace ccgl;

/*!
 * \brief Parse the input arguments of SEIMS.
 */
class InputArgs: Interface {
public:
    InputArgs(const string& model_path, const string& host, uint16_t port, int subbasin_id,
              int scenario_id, int calibration_id,
              int thread_num, LayeringMethod lyr_mtd,
              GroupMethod grp_mtd, ScheduleMethod skd_mtd);

    static InputArgs* Init(int argc, const char** argv);

public:
    string model_path;      ///< file path which contains the model input files
    string model_name;      ///< model_name
    string host;            ///< Host IP address of MongoDB database
    uint16_t port;          ///< port of MongoDB, 27017 is default
    int subbasin_id;        ///< Subbasin ID, which will be executed, 0 for whole basin, 9999 for field-version
    int thread_num;         ///< thread number for OpenMP
    LayeringMethod lyr_mtd; ///< Layering method for sequencing computing, default is 0
    GroupMethod grp_mtd;    ///< Group method for parallel task scheduling, default is 0
    ScheduleMethod skd_mtd; ///< Parallel task scheduling strategy at subbasin level by MPI
    int scenario_id;        ///< scenario ID defined in Database, -1 for no use.
    int calibration_id;     ///< calibration ID defined in Database (PARAMETERS), -1 for no use.
};

#endif /* SEIMS_INPUT_ARGUMENTS_H */
