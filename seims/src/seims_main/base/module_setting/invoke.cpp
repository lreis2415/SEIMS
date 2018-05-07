#include "invoke.h"

InputArgs* InputArgs::Init(const int argc, const char** argv) {
    string model_path;
    int num_thread = 1;
    LayeringMethod layering_method = UP_DOWN;
    char mongodb_ip[16];
    stringcpy(mongodb_ip, "127.0.0.1");
    int port = 27017;
    int scenario_id = -1;    /// By default, no BMPs Scenario is used, in case of lack of BMPs database.
    int calibration_id = -1; /// By default, no calibration ID is needed.
    /// Parse input arguments.
    int i = 0;
    char* strend = nullptr;
    errno = 0;
    if (argc < 2) {
        cout << "Error: To run the program, use either the Simple Usage option or Usage option as below." << endl;
        goto errexit;
    }

    if (argc <= 8 && argv[1][0] != '-') {
        // old style, i.e., arguments arranged in a fixed order
        model_path = argv[1];
        if (argc >= 3) num_thread = strtol(argv[2], &strend, 10);
        if (argc >= 4) layering_method = LayeringMethod(strtol(argv[3], &strend, 10));
        if (argc >= 5) stringcpy(mongodb_ip, argv[4]);
        if (argc >= 6) port = strtol(argv[5], &strend, 10);
        if (argc >= 7) scenario_id = strtol(argv[6], &strend, 10);
        if (argc >= 8) calibration_id = strtol(argv[7], &strend, 10);
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
            } else { goto errexit; }
        } else if (StringMatch(argv[i], "-thread")) {
            i++;
            if (argc > i) {
                num_thread = strtol(argv[i], &strend, 10);
                i++;
            } else { goto errexit; }
        } else if (StringMatch(argv[i], "-lyr")) {
            i++;
            if (argc > i) {
                layering_method = LayeringMethod(strtol(argv[i], &strend, 10));
                i++;
            } else { goto errexit; }
        } else if (StringMatch(argv[i], "-host")) {
            i++;
            if (argc > i) {
                stringcpy(mongodb_ip, argv[i]);
                i++;
            } else { goto errexit; }
        } else if (StringMatch(argv[i], "-port")) {
            i++;
            if (argc > i) {
                port = strtol(argv[i], &strend, 10);
                i++;
            } else { goto errexit; }
        } else if (StringMatch(argv[i], "-sce")) {
            i++;
            if (argc > i) {
                scenario_id = strtol(argv[i], &strend, 10);
                i++;
            } else { goto errexit; }
        } else if (StringMatch(argv[i], "-cali")) {
            i++;
            if (argc > i) {
                calibration_id = strtol(argv[i], &strend, 10);
                i++;
            } else { goto errexit; }
        }
    }
    /// Check the validation of input arguments
    if (!PathExists(model_path)) {
        cout << "Model folder " << model_path << " is not existed!" << endl;
        goto errexit;
    }
    if (num_thread < 1) {
        cout << "Thread number must greater or equal than 1." << endl;
        goto errexit;
    }
    if (!IsIpAddress(mongodb_ip)) {
        cout << "MongoDB Hostname " << mongodb_ip << " is not a valid IP address!" << endl;
        goto errexit;
    }
    if (port < 0) {
        cout << "Port number must greater than 0." << endl;
        goto errexit;
    }
    return new InputArgs(model_path, mongodb_ip, port, scenario_id,
                         calibration_id, num_thread, layering_method);

errexit:
    cout << "Simple Usage:\n    " << argv[0] <<
            " <ModelPath> [<threadsNum> <layeringMethod> <IP> <port> <ScenarioID> <CalibrationID>]" << endl;
    cout << "\t<ModelPath> is the path of the configuration of the Model." << endl;
    cout << "\t<threadsNum> is thread or processor number, which must be greater or equal than 1 (default)." << endl;
    cout << "\t<layeringMethod> can be 0 and 1, which means UP_DOWN (default) and DOWN_UP respectively." << endl;
    cout << "\t<IP> is the address of MongoDB database, and <port> is its port number." << endl;
    cout << "\t\tBy default, MongoDB IP is 127.0.0.1 (i.e., localhost), and the port is 27017." << endl;
    cout << "\t<ScenarioID> is the ID of BMPs Scenario which has been defined in BMPs database." << endl;
    cout << "\t\tBy default, the Scenario ID is -1, which means not used." << endl << endl;
    cout << "\t<CalibrationID> is the ID of Calibration which has been defined in PARAMETERS table." << endl;
    cout << "\t\tBy default, the Calibration ID is -1, which means not used." << endl;
    cout << endl;
    cout << "Complete and recommended Usage:\n    " << argv[0] <<
            " -wp <ModelPath> [-thread <threadsNum> -lyr <layeringMethod>"
            " -host <IP> -port <port> -sce <ScenarioID> -cali <CalibrationID>]" << endl;
    return nullptr;
}

InputArgs::InputArgs(const string& model_path, char* host, const uint16_t port, const int scenario_id,
                     const int calibration_id, const int thread_num, const LayeringMethod lyr_mtd)
    : model_path(model_path), model_name(""), port(port), thread_num(thread_num),
      lyr_mtd(lyr_mtd), scenario_id(scenario_id), calibration_id(calibration_id) {
    stringcpy(host_ip, host);
    /// Get model name
    size_t name_idx = model_path.rfind(SEP);
    model_name = model_path.substr(name_idx + 1);
}
