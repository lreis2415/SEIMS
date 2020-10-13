#include "invoke.h"

#include <text.h>

void Usage(const string& appname, const string& error_msg = "") {
    if (!error_msg.empty()) {
        cout << "FAILURE: " << error_msg << endl;
    }
    string corename = GetCoreFileName(appname);
    bool mpi_version = corename.find("mpi") != string::npos;
    cout << "Simple Usage:\n    " << appname <<
            // Common arguments
            " <modelPath> [<threadsNum> <layeringMethod> <IP> <port> <scenarioID> <calibrationID>"
            // For MPI version or Field version
            " <subbasinID>"
            // MPI version arguments,
            // " <groupMethod> <scheduleMethod> <timeSlices>"
            "]" << endl;
    cout << "\t<modelPath> is the path of the SEIMS-based watershed model." << endl;
    cout << "\t<threadsNum> is the number of thread used by OpenMP, which must be greater or equal than 1 (default)." << endl;
    cout << "\t<layeringMethod> can be 0 and 1, which means UP_DOWN (default) and DOWN_UP, respectively." << endl;
    cout << "\t<IP> is the address of MongoDB database, and <port> is its port number." << endl;
    cout << "\t\tBy default, MongoDB IP is 127.0.0.1 (i.e., localhost), and the port is 27017." << endl;
    cout << "\t<scenarioID> is the ID of BMPs Scenario which has been defined in BMPs database." << endl;
    cout << "\t\tBy default, the Scenario ID is -1, which means no scenarios will be simulated." << endl;
    cout << "\t<calibrationID> is the ID of Calibration which has been defined in PARAMETERS table." << endl;
    cout << "\t\tBy default, the Calibration ID is -1, which means no calibration will be applied." << endl;
    cout << "\t<subbasinID> is the subbasin that will be executed. "
            "0 means the whole watershed. 9999 is reserved for Field version." << endl;
    // cout << "\t<groupMethod> can be 0 and 1, which means KMETIS (default) and PMETIS, respectively." << endl;
    // cout << "\t<scheduleMethod> can be 0 and 1, which means "
    //         "SPATIAL (default) and TEMPOROSPATIAL, respectively." << endl;
    // cout << "\t<timeSlices> should be greater than 1, required when <scheduleMethod> is 1." << endl;
    cout << "\t<logLevel> is the logging level: Trace, Debug, Info (default), Warning, Error, and Fatal" << endl;
    cout << endl;
    cout << "Complete and recommended Usage:\n    ";
    if (mpi_version) {
        cout << "<executable of MPI (e.g., mpiexec and mpirun)> -hosts(or machinefile, configfile, etc) "
                "<hosts_list_file> -n <process numbers> " << appname;
    } else {
        cout << appname;
    }
    cout << " -wp <modelPath> [-thread <threadsNum> -lyr <layeringMethod> -host <IP> -port <port>"
            " -sce <scenarioID> -cali <calibrationID>"
            " -id <subbasinID>"
            // " -grp <groupMethod> -skd <scheduleMethdo> -ts <timeSlices>"
            " -ll <logLevel>"
            "]" << endl;
    exit(1);
}

InputArgs* InputArgs::Init(const int argc, const char** argv) {
    string model_path;
    int num_thread = 1;
    LayeringMethod layering_method = UP_DOWN;
    string mongodb_ip = "127.0.0.1";
    int port = 27017;
    int scenario_id = -1;    /// By default, no BMPs Scenario is used, in case of lack of BMPs database.
    int calibration_id = -1; /// By default, no calibration ID is needed.
    /// MPI version specific arguments
    int subbasin_id = 0;     /// By default, the whole basin will be executed.
    GroupMethod group_method = KMETIS;
    ScheduleMethod schedule_method = SPATIAL;
    int time_slices = -1;
    string log_level = "Info";
    /// Parse input arguments.
    int i = 0;
    char* strend = nullptr;
    errno = 0;
    if (argc < 2) {
        Usage(argv[0], "To run the program, use either the Simple Usage option or "
              "the Complete Usage option as below.");
        return nullptr;
    }

    if (argc >= 2 && argv[1][0] != '-') {
        // old style, i.e., arguments arranged in a fixed order
        model_path = argv[1];
        if (argc >= 3) num_thread = strtol(argv[2], &strend, 10);
        if (argc >= 4) layering_method = LayeringMethod(strtol(argv[3], &strend, 10));
        if (argc >= 5) mongodb_ip = argv[4];
        if (argc >= 6) port = strtol(argv[5], &strend, 10);
        if (argc >= 7) scenario_id = strtol(argv[6], &strend, 10);
        if (argc >= 8) calibration_id = strtol(argv[7], &strend, 10);
        if (argc >= 9) subbasin_id = strtol(argv[8], &strend, 10);
        if (argc >= 10) group_method = GroupMethod(strtol(argv[9], &strend, 10));
        if (argc >= 11) schedule_method = ScheduleMethod(strtol(argv[10], &strend, 10));
        if (argc >= 12) time_slices = strtol(argv[11], &strend, 10);
        i = 9999; // avoid to run the while-statement
    } else {
        i = 1;
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
                layering_method = LayeringMethod(strtol(argv[i], &strend, 10));
                i++;
            } else {
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
                port = strtol(argv[i], &strend, 10);
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
            }
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
            }
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
    if (num_thread < 1) {
        Usage(argv[0], "Thread number must greater or equal than 1.");
        return nullptr;
    }
    if (!IsIpAddress(mongodb_ip.c_str())) {
        Usage(argv[0], "MongoDB Hostname " + mongodb_ip + " is not a valid IP address!");
        return nullptr;
    }
    if (port < 0) {
        Usage(argv[0], "Port number must greater than 0.");
        return nullptr;
    }
    return new InputArgs(model_path, num_thread, layering_method, mongodb_ip, port,
                         scenario_id, calibration_id,
                         subbasin_id, group_method, schedule_method, time_slices, log_level);
}

InputArgs::InputArgs(const string& model_path, const int thread_num, const LayeringMethod lyr_mtd,
                     const string& host, const uint16_t port,
                     const int scenario_id, const int calibration_id,
                     const int subbasin_id, const GroupMethod grp_mtd,
                     const ScheduleMethod skd_mtd, const int time_slices,
                     const string& log_level)
    : model_path(model_path), model_name(""), output_scene(DB_TAB_OUT_SPATIAL),
      thread_num(thread_num), lyr_mtd(lyr_mtd),
      host(host), port(port), scenario_id(scenario_id), calibration_id(calibration_id),
      subbasin_id(subbasin_id), grp_mtd(grp_mtd), skd_mtd(skd_mtd), time_slices(time_slices),
      log_level(log_level) {
    /// Get model name
    size_t name_idx = model_path.rfind(SEP);
    model_name = model_path.substr(name_idx + 1);
    /// Clean output folder
    if (scenario_id >= 0) {
        // -1 means no BMPs scenario will be simulated
        output_scene += ValueToString(scenario_id);
    }
    if (calibration_id >= 0) {
        // -1 means no calibration setting will be used.
        output_scene += "-" + ValueToString(calibration_id);
    }
    output_path = model_path + SEP + output_scene + SEP;
    if (subbasin_id <= 1) CleanDirectory(output_path); // avoid repeat operation in mpi version
}
