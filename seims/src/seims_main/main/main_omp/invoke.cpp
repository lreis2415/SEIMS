#include "invoke.h"

InputArgs *InputArgs::Init(int argc, const char **argv) {
    string modelPath;
    int numThread = 1;
    LayeringMethod layeringMethod = UP_DOWN;
    char mongodbIP[16];
    stringcpy(mongodbIP, "127.0.0.1");
    int port = 27017;
    int scenarioID = -1;  /// By default, no BMPs Scenario is used, in case of lack of BMPs database.
    int calibrationID = -1; /// By default, no calibration ID is needed.
    /// Parse input arguments.
    int i = 0;
    if (argc < 2) {
        cout << "Error: To run the program, use either the Simple Usage option or Usage option as below." << endl;
        goto errexit;
    } else if (argc <= 8 && argv[1][0] != '-') {  // old style, i.e., arguments arranged in a fixed order
        modelPath = argv[1];
        if (argc >= 3) numThread = atoi(argv[2]);
        if (argc >= 4) layeringMethod = (LayeringMethod) atoi(argv[3]);
        if (argc >= 5) stringcpy(mongodbIP, argv[4]);
        if (argc >= 6) port = atoi(argv[5]);
        if (argc >= 7) scenarioID = atoi(argv[6]);
        if (argc >= 8) calibrationID = atoi(argv[7]);
        i = 9999;  // avoid to run the while-statement
    } else {
        i = 1;
    }
    while (argc > i) {
        if (StringMatch(argv[i], "-wp")) {
            i++;
            if (argc > i) {
                modelPath = argv[i];
                i++;
            } else { goto errexit; }
        } else if (StringMatch(argv[i], "-thread")) {
            i++;
            if (argc > i) {
                numThread = atoi(argv[i]);
                i++;
            } else { goto errexit; }
        } else if (StringMatch(argv[i], "-lyr")) {
            i++;
            if (argc > i) {
                layeringMethod = (LayeringMethod) atoi(argv[3]);
                i++;
            } else { goto errexit; }
        } else if (StringMatch(argv[i], "-host")) {
            i++;
            if (argc > i) {
                stringcpy(mongodbIP, argv[i]);
                i++;
            } else { goto errexit; }
        } else if (StringMatch(argv[i], "-port")) {
            i++;
            if (argc > i) {
                port = atoi(argv[i]);
                i++;
            } else { goto errexit; }
        } else if (StringMatch(argv[i], "-sce")) {
            i++;
            if (argc > i) {
                scenarioID = atoi(argv[i]);
                i++;
            } else { goto errexit; }
        } else if (StringMatch(argv[i], "-cali")) {
            i++;
            if (argc > i) {
                calibrationID = atoi(argv[i]);
                i++;
            } else { goto errexit; }
        }
    }
    /// Check the validation of input arguments
    if (!PathExists(modelPath)) {
        cout << "Model folder " << modelPath << " is not existed!" << endl;
        goto errexit;
    }
    if (numThread < 1) {
        cout << "Thread number must greater or equal than 1." << endl;
        goto errexit;
    }
    if (!isIPAddress(mongodbIP)) {
        cout << "MongoDB Hostname " << mongodbIP << " is not a valid IP address!" << endl;
        goto errexit;
    }
    if (port < 0) {
        cout << "Port number must greater than 0." << endl;
        goto errexit;
    }
    return new InputArgs(modelPath, mongodbIP, port, scenarioID,
                         calibrationID, numThread, layeringMethod);

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

InputArgs::InputArgs(string modelPath, char *host, uint16_t port, int scenarioID,
                     int calibrationID, int numThread, LayeringMethod lyrMethod)
        : m_model_path(modelPath), m_model_name(""), m_port(port), m_scenario_id(scenarioID),
          m_calibration_id(calibrationID), m_thread_num(numThread), m_layer_mtd(lyrMethod) {
    stringcpy(m_host_ip, host);
    /// Get model name
    size_t nameIdx = m_model_path.rfind(SEP);
    m_model_name = modelPath.substr(nameIdx + 1);
}

int MainMongoDB(InputArgs *in_args, int subbasin_id /* = 0 */) {
    /// Get module path
    string modulePath = GetAppPath();
    /// Create data center according to subbasin number,
    /// 0 means the whole basin which is default for omp version.
    DataCenterMongoDB *dataCenter = new DataCenterMongoDB(in_args->m_host_ip,
                                                          in_args->m_port,
                                                          in_args->m_model_path,
                                                          modulePath,
                                                          in_args->m_layer_mtd,
                                                          subbasin_id,
                                                          in_args->m_scenario_id,
                                                          in_args->m_calibration_id,
                                                          in_args->m_thread_num);
    /// Create module factory
    ModuleFactory *moduleFactory = new ModuleFactory(dataCenter);
    /// Create SEIMS model by dataCenter and moduleFactory
    unique_ptr<ModelMain> main(new ModelMain(dataCenter, moduleFactory));
    /// Execute model and write outputs
    main->Execute();
    main->Output();

    return 0;
}
