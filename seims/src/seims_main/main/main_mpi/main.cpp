#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#ifndef USE_MONGODB
#define USE_MONGODB
#endif /* USE_MONGODB */

#include "parallel.h"

using namespace std;

int main(int argc, const char** argv) {
    /// Parse input arguments
    InputArgs* inputArgs = InputArgs::Init(argc, argv);
    if (nullptr == inputArgs) { exit(EXIT_FAILURE); }
    /// Register GDAL
    GDALAllRegister();
    /// Initialize of MPI environment
    int numprocs;
    int worldRank;
    int nameLen;
    char processorName[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(NULL, NULL);
    {
        MPI_Comm_size(MCW, &numprocs);
        MPI_Comm_rank(MCW, &worldRank);
        MPI_Get_processor_name(processorName, &nameLen);
        MPI_Group MPI_GROUP_WORLD, slaveGroup;
        MPI_Comm slaveComm;
        MPI_Comm_group(MCW, &MPI_GROUP_WORLD);

        // create the master, transfer and calculation groups
        static int notSlaveRanks[] = {MASTER_RANK};
        // create slaveGroup which exclude the master rank (i.e., notSlaveRanks)
        MPI_Group_excl(MPI_GROUP_WORLD, 1, notSlaveRanks, &slaveGroup);
        MPI_Comm_create(MCW, slaveGroup, &slaveComm);

        if (numprocs < 2) {
            cout << "The number of processors must be greater than 1!" << endl;
            MPI_Abort(MCW, 1);
        }
        int nSlaves = numprocs - 1;
        try {
            if (worldRank == MASTER_RANK) {
                /// connect to mongodb, abort if failed.
                MongoClient* mclient = MongoClient::Init(inputArgs->m_host_ip, inputArgs->m_port);
                if (nullptr == mclient) {
                    cout << "Connect to MongoDB (" << inputArgs->m_host_ip
                            << ":" << inputArgs->m_port << ") failed!" << endl;
                    MPI_Abort(MCW, 2);
                }
                // read river topology data
                map<int, SubbasinStruct *> subbasinMap;
                set<int> groupSet;
                string groupMethod = REACH_KMETIS; // by default
                if (CreateReachTopology(mclient, inputArgs->m_model_name, groupMethod,
                                        nSlaves, subbasinMap, groupSet) != 0) {
                    cout << "Read and create reaches topology information failed." << endl;
                    MPI_Abort(MCW, 1);
                }
                delete mclient;
                if (size_t(nSlaves) != groupSet.size()) {
                    groupSet.clear();
                    cout << "The number of slave processes (" << nSlaves << ") is not consist with the group number("
                            << groupSet.size() << ")." << endl;
                    MPI_Abort(MCW, 1);
                }
                // Run management process on master rank
                MasterProcess(subbasinMap, groupSet);
                // Release memory
                for (auto it = subbasinMap.begin(); it != subbasinMap.end();) {
                    delete it->second;
                    subbasinMap.erase(it++);
                }
            } else {
                // Run computing process on slave ranks
                CalculateProcess(worldRank, numprocs, nSlaves, slaveComm, inputArgs);
            }
            MPI_Barrier(MCW);
            // free MPI sources
            MPI_Group_free(&MPI_GROUP_WORLD);
            MPI_Group_free(&slaveGroup);
            // VS2013: Fatal error in MPI_Comm_free: Invalid communicator, error stack.
            // I still think the communicator should be released. by lj.
            // MPI_Comm_free(&slaveComm);
        } catch (ModelException& e) {
            cout << e.what() << endl;
            MPI_Abort(MCW, 3);
        }
        catch (exception& e) {
            cout << e.what() << endl;
            MPI_Abort(MCW, 4);
        }
        catch (...) {
            cout << "Unknown exception occurred!" << endl;
            MPI_Abort(MCW, 5);
        }
    }
    /// clean up
    delete inputArgs;
    /// Finalize the MPI environment and exit with success
    MPI_Finalize();
    return 0;
}
