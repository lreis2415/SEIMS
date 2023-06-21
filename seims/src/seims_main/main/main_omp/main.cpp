#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#ifndef USE_MONGODB
#define USE_MONGODB
#endif /* USE_MONGODB */

#include "basic.h"

#include "seims.h"
#include <text.h>
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

        //TODO: output all DLL xml in a standalone program -- wyj
//        vector<string> dll_paths;
//        FindFiles(module_path.c_str(), "*.dll", dll_paths);
//        vector<DLLINSTANCE> dllHandles; // dynamic library handles (.dll in Windows, .so in Linux, and .dylib in macOS)
//        map<string, InstanceFunction> instanceFuncs; // map of modules instance
//        map<string, MetadataFunction> metadataFuncs; // Metadata map of modules
//        for (auto dll_path : dll_paths) {
//            string dll_name = GetCoreFileName(dll_path);
//            cout << dll_name << endl;
//            ModuleFactory::ReadDLL(module_path,dll_name,dll_name,dllHandles,instanceFuncs,metadataFuncs);
//            MetadataFunction metadataInfo = metadataFuncs[dll_name];
//            const char* current_metadata = metadataInfo();
//            std::ostringstream oss;
//            oss << "C:/src/SEIMS/data/youwuzhen/demo_youwuzhen30m_conceptual_model/knowledge/" << dll_name << ".xml";
//
//            std::ofstream myfile;
//            myfile.open(oss.str().c_str());
//            myfile << current_metadata;
//            myfile.close();
//
//            // parse the metadata
//            //TiXmlDocument doc;
//            //doc.Parse(current_metadata);
//            //doc.Print();
//            //doc.SaveFile(oss.str().c_str());
//        }

        /// Initialize the MongoDB connection client
        MongoClient* mongo_client = MongoClient::Init(input_args->host.c_str(), input_args->port);
        if (nullptr == mongo_client) {
            throw ModelException("MongoDBClient", "Constructor", "Failed to connect to MongoDB!");
        }
        MongoGridFs* spatial_gfs_in = new MongoGridFs(mongo_client->GetGridFs(input_args->model_name, DB_TAB_SPATIAL));
        MongoGridFs* spatial_gfs_out = new MongoGridFs(mongo_client->GetGridFs(input_args->model_name, DB_TAB_OUT_SPATIAL));
        /// Create module factory
        ModuleFactory* module_factory = ModuleFactory::Init(module_path, input_args);
        if (nullptr == module_factory) {
            throw ModelException("ModuleFactory", "Constructor", "Failed in constructing ModuleFactory!");
        }
        /// Create data center according to subbasin number, 0 means the whole basin which is default for omp version.
        DataCenterMongoDB* data_center = new DataCenterMongoDB(input_args, mongo_client, spatial_gfs_in, spatial_gfs_out,
                                                               module_factory, input_args->subbasin_id);
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
