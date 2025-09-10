#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#ifndef USE_MONGODB
#define USE_MONGODB
#endif /* USE_MONGODB */

#include "basic.h"

#include "seims.h"
#include "text.h"
#include "invoke.h"
#include "ModelMain.h"
#include "Logging.h"

INITIALIZE_EASYLOGGINGPP

int main(const int argc, const char** argv) {
    /// Parse input arguments
    InputArgs* input_args = InputArgs::Init(argc, argv);
    if (nullptr == input_args) { exit(EXIT_FAILURE); }

    /// Initialize easylogging++
    START_EASYLOGGINGPP(argc, argv);
    Logging::init();
    Logging::setLoggingToFile(input_args->output_path + SEP + input_args->output_scene + ".log");
    Logging::setLogLevel(Logging::getLLfromString(input_args->log_level), nullptr);

    /// Register GDAL
    GDALAllRegister();

    /// Run model.
    try {
        double input_t = TimeCounting();
        /// Get module path
        string module_path = GetAppPath();
        /// Initialize the MongoDB connection client
        MongoClient* mongo_client = MongoClient::Init(input_args->host.c_str(), input_args->port);
        if (nullptr == mongo_client) {
            throw ModelException("MongoDBClient", "Constructor", "Failed to connect to MongoDB!");
        }
        MongoGridFs* spatial_gfs_in = new MongoGridFs(mongo_client->GetGridFs(input_args->model_name, DB_TAB_SPATIAL));
        MongoGridFs* spatial_gfs_out = new MongoGridFs(mongo_client->GetGridFs(input_args->model_name, DB_TAB_OUT_SPATIAL));
        /// Load SettingsInput from file.in. This is the default behavior, that is convenient for running single model.
        ///  But when running enormous models, e.g., for parameter sensitivity analysis, load from FILE_IN table
        ///  in MongoDB is more convenient. In this case, the argument '-filein_mongo' should be explicitly set to 1!
        ///
        /// Since ModuleFactory needs to know which simulation mode (DAILY or STORM) we want to initialize, if we set
        ///  '-filein_mongo 1', we MUST also set the mode explicitly: '-mode 1' or '-mode 0'.
        /// 
        SettingsInput* simu_settings_input = nullptr;
        SimulationMode mode = input_args->mode;
        if (!input_args->filein_mongo) {
            simu_settings_input = SettingsInput::Init(input_args);
            if (nullptr == simu_settings_input) {
                throw ModelException("SettingsInput", "Constructor", "Failed in parsing file.in!");
            }
            SimulationMode mode_fromfilein = simu_settings_input->isStormMode() ? STORM : DAILY;
            if (mode == UNDEFINED) { // User didn't input the argument -load_filein_frommongo
                mode = mode_fromfilein;
            } else {
                if (mode_fromfilein != mode) {
                    CLOG(TRACE, LOG_INIT) << "The simulation mode in file.in is not the same "
                        "with your input command!" << endl;
                }
            }
        } else {
            if (mode == UNDEFINED) {
                throw ModelException("SettingsInput", "Constructor", "You set -load_filein_frommongo 1, "
                                     "and you MUST also set the mode explicitly: '-mode 1' or '-mode 0'!");
            }
        }
        /// Create module factory
        ModuleFactory* module_factory = ModuleFactory::Init(module_path, input_args, mode);
        if (nullptr == module_factory) {
            throw ModelException("ModuleFactory", "Constructor", "Failed in constructing ModuleFactory!");
        }
        /// Create data center according to subbasin number, 0 means the whole basin which is default for omp version.
        DataCenterMongoDB* data_center = new DataCenterMongoDB(input_args, mongo_client, spatial_gfs_in, spatial_gfs_out,
                                                               simu_settings_input, module_factory, input_args->subbasin_id);
        /// Create SEIMS model by dataCenter and moduleFactory
        ModelMain* model_main = new ModelMain(data_center, module_factory);
        CLOG(INFO, LOG_TIMESPAN) << "[IO  ][Input] " << std::fixed << setprecision(3) << TimeCounting() - input_t;
        /// Execute model and write outputs
        model_main->Execute();
        model_main->Output();
        CLOG(INFO, LOG_TIMESPAN) << "[SIMU][ALL] " << std::fixed << setprecision(3) << TimeCounting() - input_t;
        /// Clean up
        delete model_main;
        delete data_center;
        delete module_factory;
        delete spatial_gfs_in;
        delete spatial_gfs_out;
        mongo_client->Destroy();
        delete mongo_client;
        delete input_args;
        /// Manually to flush all log files for all levels
        el::Loggers::flushAll();
    } catch (ModelException& e) {
        LOG(ERROR) << e.ToString();
        exit(EXIT_FAILURE);
    }
    catch (std::exception& e) {
        LOG(ERROR) << e.what();
        exit(EXIT_FAILURE);
    }
    catch (...) {
        LOG(ERROR) << "Unknown exception occurred!";
        exit(EXIT_FAILURE);
    }

    return 0;
}
