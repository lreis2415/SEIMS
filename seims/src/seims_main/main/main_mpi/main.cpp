#ifndef USE_MONGODB
#define USE_MONGODB
#endif /* USE_MONGODB */

#include "parallel.h"
#include "CalculateProcess.h"

int main(int argc, const char** argv) {
    /// Parse input arguments
    InputArgs* input_args = InputArgs::Init(argc, argv);
    if (nullptr == input_args) { exit(EXIT_FAILURE); }
    /// Register GDAL
    GDALAllRegister();
    /// Initialize of MPI environment
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
        try {
            CalculateProcess(input_args, rank, size);
        } catch (ModelException& e) {
            cout << e.what() << endl;
            MPI_Abort(MCW, 3);
        }
        catch (std::exception& e) {
            cout << e.what() << endl;
            MPI_Abort(MCW, 4);
        }
        catch (...) {
            cout << "Unknown exception occurred!" << endl;
            MPI_Abort(MCW, 5);
        }
    }
    /// clean up
    delete input_args;
    /// Finalize the MPI environment and exit with success
    MPI_Finalize();
    return 0;
}
