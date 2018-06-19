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
    InputArgs(const string& model_path, int thread_num, LayeringMethod lyr_mtd,
              const string& host, uint16_t port,
              int scenario_id, int calibration_id,
              int subbasin_id, GroupMethod grp_mtd,
              ScheduleMethod skd_mtd, int time_slices);

    static InputArgs* Init(int argc, const char** argv);

public:
    string model_path;      ///< file path which contains the model input files
    string model_name;      ///< model_name
    int thread_num;         ///< thread number for OpenMP
    LayeringMethod lyr_mtd; ///< Layering method for sequencing computing, default is 0
    string host;            ///< Host IP address of MongoDB database
    uint16_t port;          ///< port of MongoDB, 27017 is default
    int scenario_id;        ///< scenario ID defined in Database, -1 for no use.
    int calibration_id;     ///< calibration ID defined in Database (PARAMETERS), -1 for no use.
    int subbasin_id;        ///< Subbasin ID, which will be executed, 0 for whole basin, 9999 for field-version
    GroupMethod grp_mtd;    ///< Group method for parallel task scheduling, default is 0
    ScheduleMethod skd_mtd; ///< Parallel task scheduling strategy at subbasin level by MPI
    int time_slices;        ///< Time slices for Temporal-Spatial discretization method, Wang et al. (2013)
};

#endif /* SEIMS_INPUT_ARGUMENTS_H */
