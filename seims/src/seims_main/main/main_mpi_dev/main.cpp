#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#ifndef USE_MONGODB
#define USE_MONGODB
#endif /* USE_MONGODB */

#include "ManagementProcess.h"
#include "CalculateProcess.h"
#include "parallel.h"

int main(int argc, const char** argv) {
    /// Parse input arguments
    InputArgs* input_args = InputArgs::Init(argc, argv);
    if (nullptr == input_args) { exit(EXIT_FAILURE); }
    /// Register GDAL
    GDALAllRegister();
    /// Initialize of MPI environment
    int numprocs;
    int world_rank;

    MPI_Init(NULL, NULL);
    {
        MPI_Comm_size(MCW, &numprocs);
        MPI_Comm_rank(MCW, &world_rank);

        try {
            if (world_rank == MASTER_RANK) {
                /// Run management process on rank 0
                ManagementProcess(input_args);
            }
            // Run computing process on slave ranks
            CalculateProcess(world_rank, numprocs, input_args);
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
