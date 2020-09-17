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

    /// Initialize MPI environment
    int size;
    int rank;
    int provided;
    MPI_Init_thread(NULL, NULL, MPI_THREAD_FUNNELED, &provided);
    if (provided < MPI_THREAD_FUNNELED) {
        cout << "Not a high enough level of thread support!" << endl;
        MPI_Abort(MCW, 1);
    }
    {
        MPI_Comm_size(MCW, &size);
        MPI_Comm_rank(MCW, &rank);

        /// Initialize easylogging++
        START_EASYLOGGINGPP(argc, argv);
        el::Helpers::setStorage(sharedLoggingRepository());
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

        try {
            CalculateProcess(input_args, rank, size);
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
    }
    /// clean up
    delete input_args;

    /// Finalize the MPI environment and exit with success
    MPI_Finalize();
    return 0;
}
