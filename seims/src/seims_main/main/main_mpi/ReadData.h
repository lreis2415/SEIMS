#pragma once

#include <string>
#include <vector>
#include <map>
#include "mongoc.h"
// #include "mongo.h"

using namespace std;

/*
 * \TODO Should this SubbasinStruct be integrated into Subbasin class defined in clsSubbasin of data module. by LJ
 */
struct SubbasinStruct
{
    float qOutlet; // flow out the subbasin outlet
    bool calculated; // whether this subbasin is already calculated
    int rank;
    SubbasinStruct *downStream;
    int disToOutlet;
    int id;
    int group;
    vector<SubbasinStruct *> upStreams;

    SubbasinStruct(int id, int group) : qOutlet(0.f), calculated(false), rank(-1), downStream(NULL), disToOutlet(-1)
    {
        this->id = id;
        this->group = group;
    }
};
/*
 * Currently, not used, then commented by LJ.
 */
//int ReadInputData(int index, string &layerFilePrefix, string &dirFilePrefix,
//                  string &slopeFilePrefix, string &streamFilePre,
//                  float **&rtLayers, float **&flowIn, float *&flowOut, float *&dir, float *&slope, float *&stream,
//                  int &layersCount, int &cellsCount);

int ReadSubbasinOutlets(const char *outletFile, int nSubbasins, float **&outlets);

int ReadRiverTopology(const char *reachFile, map<int, SubbasinStruct *> &subbasins, set<int> &groupSet);
/*
 * \TODO, temporary, this function is not invoked, so I commented it. by LJ
 */
//int GetGroupCount(mongoc_client_t *conn, const char *dbName, int decompostionPlan);
/*
 * read subbasin topology information from MongoDB
 * \TODO, clean up the old commented code as soon as the development completed. by LJ
 */
int ReadReachTopologyFromMongoDB(mongoc_client_t *conn, const char *dbName, map<int, SubbasinStruct *> &subbasins, set<int> &groupSet,
	int decompostionPlan, const char *groupField);

//int GetGroupCount(mongo *conn, const char *dbName, int decompostionPlan);
//
//int ReadTopologyFromMongoDB(mongo *conn, const char *dbName, map<int, Subbasin *> &subbasins, set<int> &groupSet,
//                            int decompostionPlan, const char *groupField);
