/*!
 * \file invoke.h
 * \brief Parse the input arguments as a class which can be easily extended.
 *
 * Changelog:
 *   - 1. 2018-02-01 - lj - Initial implementation.
 *   - 2. 2018-06-06 - lj - Add parameters related to MPI version, e.g., group method.
 *   - 3. 2021-04-06 - lj - Add flow direction algorithm as an input argument
 *
 * \author Liangjun Zhu
 */
#ifndef SEIMS_INPUT_ARGUMENTS_H
#define SEIMS_INPUT_ARGUMENTS_H

#include "basic.h"

#include "seims.h"

using namespace ccgl;

/// Full usage information
void Usage(const string& appname, const string& error_msg = std::string());

/*!
 * \class InputArgs
 * \ingroup module_setting
 * \brief Parse the input arguments of SEIMS.
 */
class InputArgs: Interface {
public:
    /*!
     * \brief Constructor by detailed parameters
     * \param[in] model_path path of the configuration of the Model
     * \param[in] model_cfgname config name of specific model, the default is "", it corresponds to a subfolder
     * \param[in] thread_num thread or processor number, which must be greater or equal than 1 (default)
     * \param[in] fdir_mtd can be 0, 1, and 2, which means D8 (default), Dinf, and MFDmd, respectively
     * \param[in] lyr_mtd can be 0 and 1, which means UP_DOWN (default) and DOWN_UP, respectively
     * \param[in] host the address of MongoDB database, by default, MongoDB IP is 127.0.0.1 (i.e., localhost)
     * \param[in] port port number, default is 27017
     * \param[in] scenario_id the ID of BMPs Scenario which has been defined in BMPs database
     * \param[in] calibration_id the ID of Calibration which has been defined in PARAMETERS table
     * \param[in] subbasin_id the subbasin that will be executed, default is 0 which means the whole watershed
     * \param[in] grp_mtd can be 0 and 1, which means KMETIS (default) and PMETIS, respectively
     * \param[in] skd_mtd (TESTED) can be 0 and 1, which means SPATIAL (default) and TEMPOROSPATIAL, respectively
     * \param[in] time_slices (TESTED) should be greater than 1, required when <skd_mtd> is 1
     * \param[in] log_level logging level, the default is Info
     * \param[in] mpi_version Optional, is running the MPI version?
     */
    InputArgs(const string& model_path, const string& model_cfgname,
              int thread_num, FlowDirMethod fdir_mtd, LayeringMethod lyr_mtd, 
              const string& host, uint16_t port,
              int scenario_id, int calibration_id,
              int subbasin_id, GroupMethod grp_mtd,
              ScheduleMethod skd_mtd, int time_slices,
              const string& log_level, bool mpi_version = false);

    /*!
     * \brief Initializer.
     * \param[in] argc Number of arguments
     * \param[in] argv \a char* Arguments
     * \param[in] mpi_version Optional, is running the MPI version?
     */
    static InputArgs* Init(int argc, const char** argv, bool mpi_version = false);

public:
    string model_path;      ///< full path of model folder which contains all inputs and outputs of all models
    string model_cfgname;   ///< config name of specific model, the default is "", it corresponds to a subfolder
    string model_name;      ///< model_name
    string output_scene;    ///< output identifier concatenated with scenario_id, calibration_id, lyr_mtd, fdir_mtd
    string output_path;     ///< full path of model outputs
    int thread_num;         ///< thread number for OpenMP
    FlowDirMethod fdir_mtd; ///< Flow direction method for flow routing, default is 0 (D8)
    LayeringMethod lyr_mtd; ///< Layering method for sequencing computing, default is 0
    string host;            ///< Host IP address or name of MongoDB database
    uint16_t port;          ///< port of MongoDB, 27017 is default
    int scenario_id;        ///< scenario ID defined in Database, -1 for no use.
    int calibration_id;     ///< calibration ID defined in Database (PARAMETERS), -1 for no use.
    int subbasin_id;        ///< Subbasin ID, which will be executed, 0 for whole basin, 9999 for field-version
    GroupMethod grp_mtd;    ///< Group method for parallel task scheduling, default is 0
    ScheduleMethod skd_mtd; ///< Parallel task scheduling strategy at subbasin level by MPI
    int time_slices;        ///< Time slices for Temporal-Spatial discretization method, Wang et al. (2013). Unfinished!
    string log_level;       ///< logging level, i.e., Trace, Debug, Info (default), Warning, Error, and Fatal
    bool mpi_version;       ///< is running the MPI version?
};

#endif /* SEIMS_INPUT_ARGUMENTS_H */
