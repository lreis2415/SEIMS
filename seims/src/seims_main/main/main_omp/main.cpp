#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#ifndef USE_MONGODB
#define USE_MONGODB
#endif /* USE_MONGODB */

#include "seims.h"
#include "invoke.h"
#include "ModelMain.h"

int main(int argc, const char **argv) {
    /// Parse input arguments
    InputArgs *input_args = InputArgs::Init(argc, argv);
    if (nullptr == input_args) { exit(EXIT_FAILURE); }
    /// Register GDAL
    GDALAllRegister();
    /// Run model.
    try {
        /// Get module path
        string modulePath = GetAppPath();
        /// Initialize the MongoDB connection client
        MongoClient *mongoClient = MongoClient::Init(input_args->m_host_ip, input_args->m_port);
        if (nullptr == mongoClient) {
            throw ModelException("MongoDBClient", "Constructor", "Failed to connect to MongoDB!");
        }
        /// Create module factory
        ModuleFactory *moduleFactory = ModuleFactory::Init(modulePath, input_args);
        if (nullptr == moduleFactory) {
            throw ModelException("ModuleFactory", "Constructor", "Failed in constructing ModuleFactory!");
        }
        /// Create data center according to subbasin number, 0 means the whole basin which is default for omp version.
        DataCenterMongoDB *dataCenter = new DataCenterMongoDB(input_args, mongoClient, moduleFactory);
        /// Create SEIMS model by dataCenter and moduleFactory
        ModelMain *modelMain = new ModelMain(dataCenter, moduleFactory);
        /// Execute model and write outputs
        modelMain->Execute();
        modelMain->Output();
        /// Clean up
        delete modelMain;
        delete dataCenter;
        delete moduleFactory;
        delete mongoClient;
        delete input_args;
    }
    catch (ModelException &e) {
        cout << e.toString() << endl;
        exit(EXIT_FAILURE);
    }
    catch (exception &e) {
        cout << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    catch (...) {
        cout << "Unknown exception occurred!" << endl;
        exit(EXIT_FAILURE);
    }
    return 0;
}
