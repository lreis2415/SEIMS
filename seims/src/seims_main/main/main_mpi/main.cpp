#include <mpi.h>
#include <cstdio> 
#include <cstdlib>
#include <iostream>
#include <sstream> 
#include <fstream>
#include <ctime>
#include <set>
//gdal
#include "ogrsf_frmts.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_string.h"

#include "ReadData.h"
#include "util.h"
#include "ModelException.h"
#include "invoke.h"
#include "mongoc.h"
//#include "mongo.h"
#include "bson.h"
//#include "gridfs.h"

#include "parallel.h"

using namespace std;
/*
 * Deprecated
 */
void WriteFileInOut(string &projectPath, string &startTime, string &endTime)
{
    string fileInName = projectPath + "file.in";
    string fileOutName = projectPath + "file.out";

    ofstream fin(fileInName.c_str());
    fin << "MODE|Daily\nINTERVAL|1\n";
    fin << "STARTTIME|" << startTime << " 00:00:00\n";
    fin << "ENDTIME|" << endTime << " 00:00:00";
    fin.close();

    ofstream fout(fileOutName.c_str());

    //fout << "OUTPUTID|SOER\nCOUNT | 1\nTYPE | SUM\n";
    //fout << "STARTTIME|" << startTime << " 00:00:00\n";
    //fout << "ENDTIME|" << endTime << " 00:00:00\n";
    //fout << "FILENAME|SOER\n\n";

    fout << "OUTPUTID|SURU\nCOUNT | 1\nTYPE | SUM\n";
    fout << "STARTTIME|" << startTime << " 00:00:00\n";
    fout << "ENDTIME|" << endTime << " 00:00:00\n";
    fout << "FILENAME|SURU\n\n";

    //fout << "OUTPUTID|SOET\nCOUNT | 1\nTYPE | SUM\n";
    //fout << "STARTTIME|" << startTime << " 00:00:00\n";
    //fout << "ENDTIME|" << endTime << " 00:00:00\n";
    //fout << "FILENAME|SOET\n\n";

    //fout << "OUTPUTID|INLO\nCOUNT | 1\nTYPE | SUM\n";
    //fout << "STARTTIME|" << startTime << " 00:00:00\n";
    //fout << "ENDTIME|" << endTime << " 00:00:00\n";
    //fout << "FILENAME|INLO\n";

    fout.close();
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: mpiexec -n [threadsNum] seims_mpi configFile [outputQFile]\n";
        exit(-1);
    }

    OGRRegisterAll();
    GDALAllRegister();


    const char *configFile = argv[1];
    string outputFile = "q.txt";
    if (argc >= 3)
        outputFile = argv[2];


    // read info from config file
    ifstream ifsConfig(configFile);
    string tmp, dbAddress, modelName, projectPath, modulePath;
    int dbPort = 27017;
    int nThread = 1;
    int layering = 0;
    ifsConfig >> tmp >> modelName;
    ifsConfig >> tmp >> projectPath;
    ifsConfig >> tmp >> modulePath;
    ifsConfig >> tmp >> dbAddress;
    ifsConfig >> tmp >> dbPort;
    ifsConfig >> tmp >> nThread;
    ifsConfig >> tmp >> layering;
    //ifsConfig >> outputFile;
    ifsConfig.close();

    if (argc >= 4)
    {
        nThread = atoi(argv[3]);
    }

    if (argc >= 5)
        layering = atoi(argv[4]);

    LayeringMethod layeringMethod = DOWN_UP;
    if (layering == 0)
        layeringMethod = UP_DOWN;
    else
        layeringMethod = DOWN_UP;

    int nLen = projectPath.length();
    if (projectPath.substr(nLen - 1, 1) != SEP)
    {
        projectPath = projectPath + SEP;
    }

    nLen = modulePath.length();
    if (modulePath.substr(nLen - 1, 1) != SEP)
    {
        modulePath = modulePath + SEP;
    }

    int numprocs, rank, nameLen;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    MPI_Get_processor_name(processor_name, &nameLen);

    MPI_Group MPI_GROUP_WORLD, slaveGroup;
    MPI_Comm slaveComm;
    MPI_Comm_group(MPI_COMM_WORLD, &MPI_GROUP_WORLD);

    // create the master, transfer and calculation groups
    static int notSlaveRanks[] = {0};
    MPI_Group_excl(MPI_GROUP_WORLD, 1, notSlaveRanks, &slaveGroup);
    MPI_Comm_create(MPI_COMM_WORLD, slaveGroup, &slaveComm);

    int nSlaves = numprocs - 1;
    //cout << "nSlaves:" << nSlaves << endl;

    if (rank == MASTER_RANK)
    {
        //// connect to mongodb
        //mongo conn[1];
        //int status = mongo_connect(conn, dbAddress.c_str(), dbPort);
        ////cout << "mongodb at " << dbAddress << ":" << dbPort << endl;

        //if (MONGO_OK != status)
        //{
        //    cout << "can not connect to mongodb at " << dbAddress << ":" << dbPort << endl;
        //    exit(-1);
        //}
		/// connect to mongoDB using mongo-c-driver-1.3.5 or later
		mongoc_client_t *conn;
		const char* host = dbAddress.c_str();
		if (!isIPAddress(host))
			throw ModelException("MainMPI", "Connect to MongoDB",
			"IP address: " + string(host) + "is invalid, Please check!\n");
		mongoc_init();
		mongoc_uri_t *uri = mongoc_uri_new_for_host_port(host, dbPort);
		conn = mongoc_client_new_from_uri(uri);
		/// Check the connection to MongoDB is success or not
		bson_t *reply = bson_new();
		bson_error_t *err = NULL;
		if (!mongoc_client_get_server_status(conn, NULL, reply, err))
			throw ModelException(MODEL_NAME, "MainMongoDB", "Failed to connect to MongoDB!\n");
		bson_destroy(reply);

        // read river topology data
        map<int, SubbasinStruct *> subbasinMap;
        set<int> groupSet;
        //ReadRiverTopology(reachFile, subbasinMap, groupSet);
        string groupMethod = "KMETIS";
        ReadReachTopologyFromMongoDB(conn, modelName.c_str(), subbasinMap, groupSet, nSlaves, REACH_KMETIS);
        if ((size_t) nSlaves != groupSet.size())
        {
            groupSet.clear();
            groupMethod = "PMETIS";
            ReadReachTopologyFromMongoDB(conn, modelName.c_str(), subbasinMap, groupSet, nSlaves, REACH_PMETIS);
            if ((size_t) nSlaves != groupSet.size())
            {
                groupSet.clear();
                groupMethod = "GROUP";
                ReadReachTopologyFromMongoDB(conn, modelName.c_str(), subbasinMap, groupSet, nSlaves, REACH_GROUP);
                if ((size_t) nSlaves != groupSet.size())
                {
                    cout << "The number of slave processes (" << nSlaves << ") is not consist with the group number(" <<
                    groupSet.size() << ").\n";
                    exit(-1);
                }
            }
        }
        cout << groupMethod << ", Number of Groups:" << groupSet.size() << endl;
        try
        {
            MasterProcess(subbasinMap, groupSet, projectPath, outputFile);
        }
        catch (ModelException e)
        {
            cout << e.toString() << endl;
        }
		mongoc_uri_destroy(uri);
        // mongo_destroy(conn);

        MPI_Finalize();
        return 0;
    }
    try
    {
        CalculateProcess(rank, numprocs, nSlaves, slaveComm, projectPath, modulePath, dbAddress.c_str(), dbPort,
                         modelName.c_str(), nThread, layeringMethod);
    }
    catch (ModelException e)
    {
        cout << e.toString() << endl;
    }

    MPI_Finalize();

    return 0;
}
