#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#ifndef USE_MONGODB
#define USE_MONGODB
#endif /* USE_MONGODB */

#include "basic.h"

#include "seims.h"
#include "invoke.h"
#include "ModelMain.h"

int main(const int argc, const char** argv) {
    /// Parse input arguments
    InputArgs* input_args = InputArgs::Init(argc, argv);
    if (nullptr == input_args) { exit(EXIT_FAILURE); }
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
        /// Create module factory
        ModuleFactory* module_factory = ModuleFactory::Init(module_path, input_args);
        if (nullptr == module_factory) {
            throw ModelException("ModuleFactory", "Constructor", "Failed in constructing ModuleFactory!");
        }
        /// Create data center according to subbasin number, 0 means the whole basin which is default for omp version.
        DataCenterMongoDB* data_center = new DataCenterMongoDB(input_args, mongo_client,
                                                               module_factory, input_args->subbasin_id);
        /// Create SEIMS model by dataCenter and moduleFactory
        ModelMain* model_main = new ModelMain(data_center, module_factory);
        cout << "[TIMESPAN][IO  ][Input] " << std::fixed << setprecision(3) << TimeCounting() - input_t << endl;
        /// Execute model and write outputs
        model_main->Execute();
        model_main->Output();
        cout << "[TIMESPAN][SIMU][ALL] " << std::fixed << setprecision(3) << TimeCounting() - input_t << endl;
        /// Clean up
        delete model_main;
        delete data_center;
        delete module_factory;
        delete mongo_client;
        delete input_args;
    } catch (ModelException& e) {
        cout << e.ToString() << endl;
        exit(EXIT_FAILURE);
    }
    catch (std::exception& e) {
        cout << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    catch (...) {
        cout << "Unknown exception occurred!" << endl;
        exit(EXIT_FAILURE);
    }
    return 0;
}
