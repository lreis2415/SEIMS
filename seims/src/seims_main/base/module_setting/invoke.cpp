#include "invoke.h"

#include <text.h>

void Usage(const string& appname, const string& error_msg) {
    if (!error_msg.empty()) {
        cout << "FAILURE: " << error_msg << endl;
    }
    string corename = GetCoreFileName(appname);
    bool mpi_version = corename.find("mpi") != string::npos;
    cout << "Complete and recommended Usage:\n";
    if (mpi_version) {
        cout << "<executable of MPI (e.g., mpiexec and mpirun)> -hosts(or machinefile, configfile, etc) "
                "<hosts_list_file> -n <process numbers> ";
    }
    cout << appname << " -wp <modelPath> [-cfg <configName>"
            " -thread <threadsNum> -lyr <layeringMethod> -fdir <flowDirMethod>"
            " -host <IP> -port <port>"
            " -sce <scenarioID> -cali <calibrationID>"
            " -id <subbasinID>" // For MPI version or testing execution of a single subbasin
            // " -grp <groupMethod> -skd <scheduleMethdo> -ts <timeSlices>"
            " -ll <logLevel>"
            "]\n";
    cout << "\t<modelPath> is the path of the SEIMS-based watershed model.\n";
    cout << "\t<configName> is the config name of specific model.\n";
    cout << "\t<threadsNum> is the number of thread used by OpenMP, which must be greater or equal than 1 (default).\n";
    cout << "\t<layeringMethod> can be 0 and 1, which means UP_DOWN (default) and DOWN_UP, respectively.\n";
    cout << "\t<flowDirMethod> can be 0, 1, and 2, which means D8 (default), Dinf, and MFDmd, respectively.\n";
    cout << "\t<IP> is the address of MongoDB database, and <port> is its port number.\n";
    cout << "\t\tBy default, MongoDB IP is 127.0.0.1 (i.e., localhost), and the port is 27017.\n";
    cout << "\t<scenarioID> is the ID of BMPs Scenario which has been defined in BMPs database.\n";
    cout << "\t\tBy default, the Scenario ID is -1, which means no scenarios will be simulated.\n";
    cout << "\t<calibrationID> is the ID of Calibration which has been defined in PARAMETERS table.\n";
    cout << "\t\tBy default, the Calibration ID is -1, which means no calibration will be applied.\n";
    cout << "\t<subbasinID> is the subbasin that will be executed. "
            "0 means the whole watershed. 9999 is reserved for Field version.\n";
    // cout << "\t<groupMethod> can be 0 and 1, which means KMETIS (default) and PMETIS, respectively.\n";
    // cout << "\t<scheduleMethod> can be 0 and 1, which means "
    //         "SPATIAL (default) and TEMPOROSPATIAL, respectively.\n";
    // cout << "\t<timeSlices> should be greater than 1, required when <scheduleMethod> is 1.\n";
    cout << "\t<logLevel> is the logging level: Trace, Debug, Info (default), Warning, Error, and Fatal.\n\n";
    exit(1);
}

InputArgs* InputArgs::Init(const int argc, const char** argv, bool mpi_version/* = false*/) {
    string model_path;
    string model_cfgname = "";
    int num_thread = 1;
    FlowDirMethod flowdir_method = D8;
    LayeringMethod layering_method = UP_DOWN;
    string mongodb_ip = "127.0.0.1";
    vuint16_t port = 27017;
    int scenario_id = -1;    /// By default, no BMPs Scenario is used, in case of lack of BMPs database.
    int calibration_id = -1; /// By default, no calibration ID is needed.
    /// MPI version specific arguments
    int subbasin_id = 0;     /// By default, the whole basin will be executed.
    GroupMethod group_method = KMETIS;
    ScheduleMethod schedule_method = SPATIAL;
    int time_slices = -1;
    string log_level = "Info";
    /// Parse input arguments.
    int i = 1;
    char* strend = nullptr;
    errno = 0;
    if (argc < 3) {
        Usage(argv[0], "To run the program, please use "
              "the Complete Usage option as below.");
        return nullptr;
    }
    while (argc > i) {
        if (StringMatch(argv[i], "-wp")) {
            i++;
            if (argc > i) {
                model_path = argv[i];
                i++;
            } else {
                Usage(argv[0]);
                return nullptr;
            }
        } else if (StringMatch(argv[i], "-cfg")) {
            i++;
            if (argc > i) {
                model_cfgname = argv[i];
                i++;
            } else {
                Usage(argv[0]);
                return nullptr;
            }
        } else if (StringMatch(argv[i], "-thread")) {
            i++;
            if (argc > i) {
                num_thread = strtol(argv[i], &strend, 10);
                i++;
            } else {
                Usage(argv[0]);
                return nullptr;
            }
        } else if (StringMatch(argv[i], "-lyr")) {
            i++;
            if (argc > i) {
                layering_method = static_cast<LayeringMethod>(strtol(argv[i], &strend, 10));
                i++;
            } else {
                Usage(argv[0]);
                return nullptr;
            }
        } else if (StringMatch(argv[i], "-fdir")) {
            i++;
            if (argc > i) {
                flowdir_method = static_cast<FlowDirMethod>(strtol(argv[i], &strend, 10));
                i++;
            }
            else {
                Usage(argv[0]);
                return nullptr;
            }
        } else if (StringMatch(argv[i], "-host")) {
            i++;
            if (argc > i) {
                mongodb_ip = argv[i];
                i++;
            } else {
                Usage(argv[0]);
                return nullptr;
            }
        } else if (StringMatch(argv[i], "-port")) {
            i++;
            if (argc > i) {
                port = static_cast<vuint16_t>(strtol(argv[i], &strend, 10));
                i++;
            } else {
                Usage(argv[0]);
                return nullptr;
            }
        } else if (StringMatch(argv[i], "-sce")) {
            i++;
            if (argc > i) {
                scenario_id = strtol(argv[i], &strend, 10);
                i++;
            } else {
                Usage(argv[0]);
                return nullptr;
            }
        } else if (StringMatch(argv[i], "-cali")) {
            i++;
            if (argc > i) {
                calibration_id = strtol(argv[i], &strend, 10);
                i++;
            } else {
                Usage(argv[0]);
                return nullptr;
            }
        } else if (StringMatch(argv[i], "-id")) {
            i++;
            if (argc > i) {
                subbasin_id = strtol(argv[i], &strend, 10);
                i++;
            } else {
                Usage(argv[0]);
                return nullptr;
            }/*
        } else if (StringMatch(argv[i], "-grp")) {
            i++;
            if (argc > i) {
                group_method = GroupMethod(strtol(argv[i], &strend, 10));
                i++;
            } else {
                Usage(argv[0]);
                return nullptr;
            }
        } else if (StringMatch(argv[i], "-skd")) {
            i++;
            if (argc > i) {
                schedule_method = ScheduleMethod(strtol(argv[i], &strend, 10));
                i++;
            } else {
                Usage(argv[0]);
                return nullptr;
            }
        } else if (StringMatch(argv[i], "-ts")) {
            i++;
            if (argc > i) {
                time_slices = strtol(argv[i], &strend, 10);
                i++;
            } else {
                Usage(argv[0]);
                return nullptr;
            }*/
        } else if (StringMatch(argv[i], "-ll")) {
            i++;
            if (argc > i) {
                log_level = argv[i];
                i++;
            } else {
                Usage(argv[0]);
                return nullptr;
            }
        }
    }
    /// Check the validation of input arguments
    if (!PathExists(model_path)) {
        Usage(argv[0], "Model folder " + model_path + " is not existed!");
        return nullptr;
    }
    if (!model_cfgname.empty()) {
        string model_path2 = model_path + SEP + model_cfgname;
        if (!PathExists(model_path2)) {
            Usage(argv[0], "Specific model folder " + model_path2 + " is not existed!");
            return nullptr;
        }
    }
    if (num_thread < 1) {
        Usage(argv[0], "Thread number must greater or equal than 1.");
        return nullptr;
    }
    if (!IsIpAddress(mongodb_ip.c_str())) {
        Usage(argv[0], "MongoDB Hostname " + mongodb_ip + " is not a valid IP address!");
        return nullptr;
    }

    return new InputArgs(model_path, model_cfgname, num_thread,
                         flowdir_method, layering_method, mongodb_ip, port,
                         scenario_id, calibration_id,
                         subbasin_id,
                         group_method, schedule_method, time_slices,
                         log_level, mpi_version);
}

InputArgs::InputArgs(const string& model_path, const string& model_cfgname,
                     const int thread_num, 
                     const FlowDirMethod fdir_mtd, const LayeringMethod lyr_mtd, 
                     const string& host, const uint16_t port,
                     const int scenario_id, const int calibration_id,
                     const int subbasin_id, const GroupMethod grp_mtd,
                     const ScheduleMethod skd_mtd, const int time_slices,
                     const string& log_level, bool mpi_version/* = false*/)
    : model_path(model_path), model_cfgname(model_cfgname), output_scene(DB_TAB_OUT_SPATIAL),
      thread_num(thread_num), fdir_mtd(fdir_mtd), lyr_mtd(lyr_mtd),
      host(host), port(port), scenario_id(scenario_id), calibration_id(calibration_id),
      subbasin_id(subbasin_id), grp_mtd(grp_mtd), skd_mtd(skd_mtd), time_slices(time_slices),
      log_level(log_level), mpi_version(mpi_version) {
    /// Get model name
    size_t name_idx = model_path.rfind(SEP);
    model_name = model_path.substr(name_idx + 1);
    /// Create output folder
    /// This code should be simultaneously updated with `MainSEIMS.UpdateScenarioID` function in Python
    if (mpi_version) output_scene += "_MPI";
    output_scene += FlowDirMethodString[fdir_mtd];
    output_scene += LayeringMethodString[lyr_mtd];
    output_scene += "-";
    if (scenario_id >= 0) {
        // -1 means no BMPs scenario will be simulated
        output_scene += ValueToString(scenario_id);
    }
    output_scene += "-";
    if (calibration_id >= 0) {
        // -1 means no calibration setting will be used.
        output_scene += ValueToString(calibration_id);
    }
    if (!model_cfgname.empty()) {
        output_path = model_path + SEP + model_cfgname + SEP + output_scene + SEP;
    } else {
        output_path = model_path + SEP + output_scene + SEP;
    }
    if (!DirectoryExists(output_path)) MakeDirectory(output_path);
    // Do not clean output directory here, delete output files when generate new ones
}
