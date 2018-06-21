#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#ifndef USE_MONGODB
#define USE_MONGODB
#endif /* USE_MONGODB */

#include "parallel.h"
#include "text.h"

int main(int argc, const char** argv) {
    /// Parse input arguments
    InputArgs* input_args = InputArgs::Init(argc, argv);
    if (nullptr == input_args) { exit(EXIT_FAILURE); }
    /// Register GDAL
    GDALAllRegister();
    /// Initialize of MPI environment
    int numprocs;
    int world_rank;
    int name_len;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(NULL, NULL);
    {
        MPI_Comm_size(MCW, &numprocs);
        MPI_Comm_rank(MCW, &world_rank);
        MPI_Get_processor_name(processor_name, &name_len);
        MPI_Group MPI_GROUP_WORLD, slave_group;
        MPI_Comm slave_comm;
        MPI_Comm_group(MCW, &MPI_GROUP_WORLD);

        // create the master, transfer and calculation groups
        static int not_slave_ranks[] = {MASTER_RANK};
        // create slaveGroup which exclude the master rank (i.e., notSlaveRanks)
        MPI_Group_excl(MPI_GROUP_WORLD, 1, not_slave_ranks, &slave_group);
        MPI_Comm_create(MCW, slave_group, &slave_comm);

        if (numprocs < 2) {
            cout << "The number of processors must be greater than 1!" << endl;
            MPI_Abort(MCW, 1);
        }
        int nslaves = numprocs - 1;
        try {
            if (world_rank == MASTER_RANK) {
                /// connect to mongodb, abort if failed.
                MongoClient* mclient = MongoClient::Init(input_args->host.c_str(), input_args->port);
                if (nullptr == mclient) {
                    cout << "Connect to MongoDB (" << input_args->host
                            << ":" << input_args->port << ") failed!" << endl;
                    MPI_Abort(MCW, 2);
                }
                // read river topology data
                map<int, SubbasinStruct *> subbasin_map;
                set<int> group_set;
                string group_method = REACH_KMETIS; // by default
                if (CreateReachTopology(mclient, input_args->model_name, group_method,
                                        nslaves, subbasin_map, group_set) != 0) {
                    cout << "Read and create reaches topology information failed." << endl;
                    MPI_Abort(MCW, 1);
                }
                delete mclient;
                if (size_t(nslaves) != group_set.size()) {
                    group_set.clear();
                    cout << "The number of slave processes (" << nslaves << ") is not consist with the group number("
                            << group_set.size() << ")." << endl;
                    MPI_Abort(MCW, 1);
                }
                // Run management process on master rank
                MasterProcess(subbasin_map, group_set);
                // Release memory
                for (auto it = subbasin_map.begin(); it != subbasin_map.end();) {
                    delete it->second;
                    subbasin_map.erase(it++);
                }
            } else {
                // Run computing process on slave ranks
                CalculateProcess(world_rank, numprocs, nslaves, slave_comm, input_args);
            }
            MPI_Barrier(MCW);
            // free MPI sources
            MPI_Group_free(&MPI_GROUP_WORLD);
            MPI_Group_free(&slave_group);
            // VS2013: Fatal error in MPI_Comm_free: Invalid communicator, error stack.
            // I still think the communicator should be released. by lj.
            // MPI_Comm_free(&slaveComm);
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
