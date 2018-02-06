#ifndef SEIMS_MPI_H
#define SEIMS_MPI_H

#include "seims.h"
#include <map>
#include <set>
#include <vector>

#include "ReadData.h"
#include "mpi.h"
#include "utilities.h"

#include "mongoc.h"


#define WORK_TAG 0
#define MASTER_RANK 0
#define SLAVE0_RANK 1
#define MAX_UPSTREAM 4
#define MSG_LEN 5
#define MCW MPI_COMM_WORLD

using namespace std;

//#define DEBUG_OUTPUT

void CombineRasterResults(string &folder, string &sVar, string &fileType, int nSubbasins, string &outputRaster);

//void CombineRasterResultsMongo(gridfs *gfs, string &sVar, int nSubbasins, string &outputRaster);
void CombineRasterResultsMongo(mongoc_gridfs_t *gfs, string &sVar, int nSubbasins, string &outputRaster);

int MasterProcess(map < int, SubbasinStruct * > &subbasinMap, set < int > &groupSet, string & projectPath,
                  string & outputFile);

void CalculateProcess(int rank, int numprocs, int nSlaves, MPI_Comm slaveComm,
                      string &projectPath, string &modulePath, const char *host, int port, const char *dbName,
                      int nThreads, LayeringMethod layeringMethod);

void CalculateProcessMonolithic(int rank, int numprocs, int nSlaves, MPI_Comm slaveComm,
                                string &projectPath, string &modulePath, const char *host, int port, const char *dbName,
                                int nThreads, LayeringMethod layeringMethod);

#endif /* SEIMS_MPI_H */