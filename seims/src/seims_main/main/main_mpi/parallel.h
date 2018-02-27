#ifndef SEIMS_MPI_H
#define SEIMS_MPI_H

#include "ReadData.h"

#include "seims.h"
#include "utilities.h"
#include "invoke.h"

#include "mpi.h"

#include <map>
#include <set>
#include <vector>

#define WORK_TAG 0
#define MASTER_RANK 0
#define SLAVE0_RANK 1 ///< Rank of this slave processor in SlaveGroup is 0
#define MAX_UPSTREAM 4
#define MSG_LEN 5
#define MCW MPI_COMM_WORLD

using namespace std;

//void CombineRasterResults(string &folder, string &sVar, string &fileType, int nSubbasins, string &outputRaster);
//
////void CombineRasterResultsMongo(gridfs *gfs, string &sVar, int nSubbasins, string &outputRaster);
//void CombineRasterResultsMongo(mongoc_gridfs_t *gfs, string &sVar, int nSubbasins, string &outputRaster);

int MasterProcess(map<int, SubbasinStruct *> &subbasinMap, set<int> &groupSet, InputArgs *input_args);

void CalculateProcess(int world_rank, int numprocs, int nSlaves, MPI_Comm slaveComm, InputArgs *input_args);
//string &projectPath, string &modulePath, const char *host, int port, const char *dbName,
//int nThreads, LayeringMethod layeringMethod);

//void CalculateProcessMonolithic(int rank, int numprocs, int nSlaves, MPI_Comm slaveComm,
//                                string &projectPath, string &modulePath, const char *host, int port, const char *dbName,
//                                int nThreads, LayeringMethod layeringMethod);

#endif /* SEIMS_MPI_H */
