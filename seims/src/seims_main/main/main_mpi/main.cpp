#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */
#ifndef USE_MONGODB
#define USE_MONGODB
#endif /* USE_MONGODB */

#include "parallel.h"
#include "CalculateProcess.h"
#include "Logging.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, const char** argv) {
    /// Parse input arguments
    InputArgs* input_args = InputArgs::Init(argc, argv);
    if (nullptr == input_args) { exit(EXIT_FAILURE); }

    /// Register GDAL
    GDALAllRegister();

    /// Connect to MongoDB. Two method are provided and the 2nd one used as default.
    int conn2mongo = 2;
    mongoc_uri_t* uri = nullptr;
    mongoc_client_pool_t *mongoc_pool = nullptr;
    if (conn2mongo == 1) {
        /// Method 1: test mongo host and port; create mongo client in each process. NOT RECOMMENDED!
        MongoClient* mongo_client = MongoClient::Init(input_args->host.c_str(), input_args->port);
        if (nullptr == mongo_client) {
            throw ModelException("MongoDBClient", "Constructor", "Failed to connect to MongoDB!");
        }
    } else if (conn2mongo == 2) {
        /// Method 2: create mongo client pool in the master process
        mongoc_init();
        uri = mongoc_uri_new_for_host_port(input_args->host.c_str(), input_args->port);
        mongoc_pool = mongoc_client_pool_new(uri);
        mongoc_client_pool_set_error_api(mongoc_pool, MONGOC_ERROR_API_VERSION_2);
    }
    /// Initialize MPI environment
    int size;
    int rank;
    int namelen;
    char hostname[MPI_MAX_PROCESSOR_NAME];

    int provided;
    MPI_Init_thread(NULL, NULL, MPI_THREAD_FUNNELED, &provided);
    if (provided < MPI_THREAD_FUNNELED) {
        cout << "Not a high enough level of thread support!" << endl;
        MPI_Abort(MCW, 1);
    }
    {
        MPI_Comm_size(MCW, &size);
        MPI_Comm_rank(MCW, &rank);
        MPI_Get_processor_name(hostname, &namelen);

        /// Initialize easylogging++
        START_EASYLOGGINGPP(argc, argv);
        Logging::init();
        if (rank == 0) {
            Logging::setLoggingToFile(input_args->output_path + SEP + 
                                      input_args->output_scene + ".log");
        } else {
            Logging::setLoggingToFile(input_args->output_path + SEP + 
                                      input_args->output_scene + "-mpi-rank" +
                                      ValueToString(rank) + ".log");
        }
        Logging::setLogLevel(Logging::getLLfromString(input_args->log_level), nullptr);

        LOG(INFO) << "Process " << rank << " out of " << size << " running on " << hostname;

        try {
            if (conn2mongo == 1) {
                CalculateProcess(input_args, rank, size);
            } else if (conn2mongo == 2) {
                CalculateProcess(input_args, rank, size, mongoc_pool);
            }
        } catch (ModelException& e) {
            LOG(ERROR) << e.what();
            MPI_Abort(MCW, 3);
        }
        catch (std::exception& e) {
            LOG(ERROR) << e.what();
            MPI_Abort(MCW, 4);
        }
        catch (...) {
            LOG(ERROR) << "Unknown exception occurred!";
            MPI_Abort(MCW, 5);
        }
        MPI_Barrier(MCW);
        el::Loggers::flushAll();
    }
    /// Finalize the MPI environment
    MPI_Finalize();

    /// clean up input arguments
    delete input_args;

    /// clean up mongoc
    if (conn2mongo == 2) {
        mongoc_client_pool_destroy(mongoc_pool);
        mongoc_uri_destroy(uri);
        mongoc_cleanup();
    }

    /// exit with success
    return 0;
}
