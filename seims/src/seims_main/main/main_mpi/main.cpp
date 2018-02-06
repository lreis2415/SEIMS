#ifndef USE_MONGODB
#define USE_MONGODB
#endif /* USE_MONGODB */

#include "seims.h"
#include "invoke.h"

#include "parallel.h"
//#include <mpi.h>

//#include "ReadData.h"
//#include <cstdio>
//#include <cstdlib>
//#include <iostream>
//#include <sstream>
//#include <fstream>
//#include <ctime>
//#include <set>
////gdal
//#include "ogrsf_frmts.h"
//#include "gdal.h"
//#include "gdal_priv.h"
//#include "cpl_string.h"

//#include "utilities.h"
//#include "ModelException.h"
//#include "invoke.h"
//#include "mongoc.h"
////#include "mongo.h"
//#include "bson.h"
////#include "gridfs.h"


using namespace std;

///*
// * Deprecated
// */
//void WriteFileInOut(string &projectPath, string &startTime, string &endTime) {
//    string fileInName = projectPath + "file.in";
//    string fileOutName = projectPath + "file.out";
//
//    ofstream fin(fileInName.c_str());
//    fin << "MODE|Daily\nINTERVAL|1\n";
//    fin << "STARTTIME|" << startTime << " 00:00:00\n";
//    fin << "ENDTIME|" << endTime << " 00:00:00";
//    fin.close();
//
//    ofstream fout(fileOutName.c_str());
//
//    //fout << "OUTPUTID|SOER\nCOUNT | 1\nTYPE | SUM\n";
//    //fout << "STARTTIME|" << startTime << " 00:00:00\n";
//    //fout << "ENDTIME|" << endTime << " 00:00:00\n";
//    //fout << "FILENAME|SOER\n\n";
//
//    fout << "OUTPUTID|SURU\nCOUNT | 1\nTYPE | SUM\n";
//    fout << "STARTTIME|" << startTime << " 00:00:00\n";
//    fout << "ENDTIME|" << endTime << " 00:00:00\n";
//    fout << "FILENAME|SURU\n\n";
//
//    //fout << "OUTPUTID|SOET\nCOUNT | 1\nTYPE | SUM\n";
//    //fout << "STARTTIME|" << startTime << " 00:00:00\n";
//    //fout << "ENDTIME|" << endTime << " 00:00:00\n";
//    //fout << "FILENAME|SOET\n\n";
//
//    //fout << "OUTPUTID|INLO\nCOUNT | 1\nTYPE | SUM\n";
//    //fout << "STARTTIME|" << startTime << " 00:00:00\n";
//    //fout << "ENDTIME|" << endTime << " 00:00:00\n";
//    //fout << "FILENAME|INLO\n";
//
//    fout.close();
//}

int main(int argc, const char **argv) {
    /// Parse input arguments
    InputArgs *input_args = InputArgs::Init(argc, argv);
    if (nullptr == input_args) { exit(EXIT_FAILURE); }
    /// Register GDAL
    GDALAllRegister();
    /// Initialize of MPI environment
    int numprocs, rank, nameLen;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    MPI_Init(NULL, NULL);
    {
        MPI_Comm_size(MCW, &numprocs);
        MPI_Comm_rank(MCW, &rank);
        MPI_Get_processor_name(processor_name, &nameLen);
        MPI_Group MPI_GROUP_WORLD, slaveGroup;
        MPI_Comm slaveComm;
        MPI_Comm_group(MCW, &MPI_GROUP_WORLD);

        // create the master, transfer and calculation groups
        static int notSlaveRanks[] = { MASTER_RANK };
        // create slaveGroup which exclude the master rank (i.e., notSlaveRanks)
        MPI_Group_excl(MPI_GROUP_WORLD, 1, notSlaveRanks, &slaveGroup);
        MPI_Comm_create(MCW, slaveGroup, &slaveComm);

        if (numprocs < 2) {
            cout << "The number of processors must be greater than 1!" << endl;
            MPI_Abort(MCW, 1);
        }
        int nSlaves = numprocs - 1;
        // cout << "nSlaves:" << nSlaves << endl;
        try {
            if (rank == MASTER_RANK) {
                /// connect to mongodb, abort if failed.
                MongoClient *mclient = MongoClient::Init(input_args->m_host_ip, input_args->m_port);
                if (nullptr == mclient) {
                    cout << "Connect to MongoDB (" << input_args->m_host_ip
                        << ":" << input_args->m_port << ") failed!" << endl;
                    MPI_Abort(MCW, 2);
                }
                mongoc_client_t *conn = mclient->getConn();

                // read river topology data
                map<int, SubbasinStruct *> subbasinMap;
                set<int> groupSet;
                //ReadRiverTopology(reachFile, subbasinMap, groupSet);
                string groupMethod = "KMETIS";
                ReadReachTopologyFromMongoDB(conn, modelName.c_str(), subbasinMap, groupSet, nSlaves, REACH_KMETIS);
                if ((size_t)nSlaves != groupSet.size()) {
                    groupSet.clear();
                    groupMethod = "PMETIS";
                    ReadReachTopologyFromMongoDB(conn, modelName.c_str(), subbasinMap, groupSet, nSlaves, REACH_PMETIS);
                    if ((size_t)nSlaves != groupSet.size()) {
                        groupSet.clear();
                        groupMethod = "GROUP";
                        ReadReachTopologyFromMongoDB(conn, modelName.c_str(), subbasinMap, groupSet, nSlaves, REACH_GROUP);
                        if ((size_t)nSlaves != groupSet.size()) {
                            cout << "The number of slave processes (" << nSlaves << ") is not consist with the group number(" <<
                                groupSet.size() << ").\n";
                            exit(-1);
                        }
                    }
                }
                cout << groupMethod << ", Number of Groups:" << groupSet.size() << endl;
                //try {
                    MasterProcess(subbasinMap, groupSet, projectPath, outputFile);
                //}
                //catch (ModelException e) {
                //    cout << e.toString() << endl;
                //}
                mongoc_uri_destroy(uri);
                // mongo_destroy(conn);

                //MPI_Finalize();
                //return 0;
            }
            else {
                CalculateProcess(rank, numprocs, nSlaves, slaveComm, projectPath, modulePath, dbAddress.c_str(), dbPort,
                    modelName.c_str(), nThread, layeringMethod);
            }
        }
        catch (ModelException &e) {
            cout << e.toString() << endl;
            MPI_Abort(MCW, 3);
        }
        catch (exception &e) {
            cout << e.what() << endl;
            MPI_Abort(MCW, 4);
        }
        catch (...) {
            cout << "Unknown exception occurred!" << endl;
            MPI_Abort(MCW, 5);
        }
    }
    /// Finalize the MPI environment and exit with success
    MPI_Finalize();
    return 0;
}
