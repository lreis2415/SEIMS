#pragma once

#include <map>
#include <set>

#include "ReadData.h"
#include "mpi.h"
#include "util.h"

#define WORK_TAG 0
#define MASTER_RANK 0
#define SLAVE0_RANK 1
#define MAX_UPSTREAM 4
#define MSG_LEN 5

#ifndef linux
#define SEP "\\"
#else
#define SEP "/"
#endif

using namespace std;

//#define DEBUG_OUTPUT

int MasterProcess(map<int, Subbasin *> &subbasinMap, set<int> &groupSet, const char *outputFile);

void CalculateProcess(int rank, int nSlaves, MPI_Comm slaveComm,
                      string &projectPath, string &modulePath, const char *host, int port, const char *dbName,
                      int nThreads, LayeringMethod layeringMethod);
