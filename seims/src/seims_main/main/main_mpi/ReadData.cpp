#include <cmath>
#include <sstream>
#include <fstream>
#include <iostream>
#include <set>

#include "ogrsf_frmts.h"

#include "ReadData.h"
#include "clsRasterData.cpp"
#include "ModelException.h"
#include "invoke.h"

#include "bson.h"

using namespace std;

//int ReadInputData(int index, string &layerFilePrefix, string &dirFilePrefix,
//                  string &slopeFilePrefix, string &streamFilePrefix,
//                  float **&rtLayers, float **&flowIn, float *&flowOut, float *&dir, float *&slope, float *&stream,
//                  int &layersCount, int &cellsCount)
//{
//    //input
//    ostringstream oss;
//    oss << layerFilePrefix << index << ".txt";
//    string layerFile = oss.str();
//
//    oss.str("");
//    oss << dirFilePrefix << index << ".txt";
//    string dirFile = oss.str();
//
//    oss.str("");
//    oss << slopeFilePrefix << index << ".txt";
//    string slopeFile = oss.str();
//
//    oss.str("");
//    oss << streamFilePrefix << index << ".txt";
//    string streamFile = oss.str();
//
//    // read data from files
//    ifstream fLayer(layerFile.c_str());
//    string tmp;
//    fLayer >> tmp >> layersCount;
//    rtLayers = new float *[layersCount];
//    for (int i = 0; i < layersCount; ++i)
//    {
//        int nCells;
//        fLayer >> nCells;
//        rtLayers[i] = new float[nCells + 1];
//        rtLayers[i][0] = (float) nCells;
//        for (int j = 1; j <= nCells; ++j)
//            fLayer >> rtLayers[i][j];
//    }
//    fLayer.close();
//
//    ifstream fFlowIn(dirFile.c_str());
//    fFlowIn >> tmp >> cellsCount;
//    flowIn = new float *[cellsCount];
//    flowOut = new float[cellsCount];
//    dir = new float[cellsCount];
//    for (int i = 0; i < cellsCount; ++i)
//    {
//        int inNum;
//        fFlowIn >> dir[i] >> flowOut[i] >> inNum;
//        flowIn[i] = new float[inNum + 1];
//        flowIn[i][0] = (float) inNum;
//        for (int j = 1; j <= inNum; ++j)
//        {
//            fFlowIn >> flowIn[i][j];
//        }
//    }
//    fFlowIn.close();
//
//    clsRasterData<float> slopeRaster(slopeFile.c_str());
//    if (slopeRaster.getCellNumber() != cellsCount)
//    {
//        throw ModelException("", "ReadInputData", "The slope file is not consistent with the layer file.");
//    }
//    slope = new float[cellsCount];
//    for (int i = 0; i < cellsCount; ++i)
//    {
//        slope[i] = 0.01f * slopeRaster.getValue(i);
//        if (abs(slope[i]) < 0.0001f)
//            slope[i] = 0.0001f;
//    }
//
//    clsRasterData<float> streamRaster(streamFile.c_str(), &slopeRaster);
//    stream = new float[cellsCount];
//    for (int i = 0; i < cellsCount; ++i)
//    {
//        stream[i] = streamRaster.getValue(i);
//        //    cout << int(stream[i]) << " ";
//    }
//    return 0;
//}


int ReadSubbasinOutlets(const char *outletFile, int nSubbasins, float **&outlets)
{
    ifstream ifs(outletFile);
    int id;
    for (int i = 0; i < nSubbasins; i++)
    {
        ifs >> id >> outlets[i][0] >> outlets[i][1];
    }
    ifs.close();

    return 0;
}

int BuildLayer(SubbasinStruct *pDownStream)
{
    vector<SubbasinStruct *> &ups = pDownStream->upStreams;
    for (size_t i = 0; i < ups.size(); ++i)
    {
        ups[i]->disToOutlet = pDownStream->disToOutlet + 1;
        BuildLayer(ups[i]);
    }
    return 1;
}

//int GetGroupCount(mongoc_client_t *conn, const char *dbName, int decompostionPlan, const char *groupField)
//{
//    bson b[1];
//    bson_init(b);
//    bson_append_int(b, "GROUP_DIVIDE", decompostionPlan);
//    bson_finish(b);
//
//    ostringstream oss;
//    oss << dbName << ".reaches";
//    mongo_cursor cursor[1];
//    mongo_cursor_init(cursor, conn, oss.str().c_str());
//    mongo_cursor_set_query(cursor, b);
//
//    // set subbasin
//    set<int> groupSet;
//    while (mongo_cursor_next(cursor) == MONGO_OK)
//    {
//        bson &rec = cursor->current;
//
//        int group;
//        bson_iterator iterator[1];
//        if (bson_find(iterator, &rec, groupField))
//            group = bson_iterator_int(iterator);
//        else if (bson_find(iterator, &rec, "GROUP"))
//            group = bson_iterator_int(iterator);
//        else
//            throw ModelException("ReadData", "ReadTopologyFromMongoDB", "Subbasin GROUP is not found in database.");
//
//
//        groupSet.insert(group);
//    }
//    mongo_cursor_destroy(cursor);
//
//    return groupSet.size();
//}

int ReadReachTopologyFromMongoDB(mongoc_client_t *conn, const char *dbName, map<int, SubbasinStruct *> &subbasins, set<int> &groupSet,
                            int decompostionPlan, const char *groupField)
{
	bson_t *b = bson_new();
	BSON_APPEND_INT32(b, REACH_GROUPDIVIDED, decompostionPlan);
	
    //bson b[1];
    //bson_init(b);
    //bson_append_int(b, "GROUP_DIVIDE", decompostionPlan);
    //bson_finish(b);

    //ostringstream oss;
    //oss << dbName << ".reaches";
    //mongo_cursor cursor[1];
    //mongo_cursor_init(cursor, conn, oss.str().c_str());
    //mongo_cursor_set_query(cursor, b);

	mongoc_cursor_t *cursor;
	const bson_t *rec;
	mongoc_collection_t *collection = NULL;
	collection = mongoc_client_get_collection(conn, dbName, DB_TAB_REACH);
	if (collection == NULL)
		throw ModelException("ReadData", "ReadReachesTopologyFromMongoDB", "Failed to get collection: " + string(DB_TAB_REACH) + ".\n");
	cursor = mongoc_collection_find(collection, MONGOC_QUERY_NONE, 0, 0, 0, b, NULL, NULL);

    // set subbasin
    map<int, int> downStreamMap;
    bool readFailed = true;
	while (mongoc_cursor_more(cursor) && mongoc_cursor_next(cursor, &rec))
	{
    //while (mongo_cursor_next(cursor) == MONGO_OK)
    //{
    //    bson &rec = cursor->current;

        int id, group;
        //bson_iterator iterator[1];
		bson_iter_t iterator;
        //if (bson_find(iterator, &rec, "SUBBASIN"))
        //    id = bson_iterator_int(iterator);
		if (bson_iter_init_find(&iterator, rec, REACH_SUBBASIN))
			id = GetIntFromBSONITER(&iterator);
        else
            throw ModelException("ReadData", "ReadReachesTopologyFromMongoDB", "Subbasin ID is not found in database.");

        //if (bson_find(iterator, &rec, groupField))
        //    group = bson_iterator_int(iterator);
		if (bson_iter_init_find(&iterator, rec, groupField))
			group = GetIntFromBSONITER(&iterator);
        else
        {
            return -1;
            //cout <<  "Subbasin GROUP_KMETIS or GROUP is not found in database.\n";
            //throw ModelException("ReadData", "ReadReachesTopologyFromMongoDB", "Subbasin GROUP is not found in database.");
        }
        //if (bson_find(iterator, &rec, "DOWNSTREAM"))
        //    downStreamMap[id] = bson_iterator_int(iterator);
		if (bson_iter_init_find(&iterator, rec, REACH_DOWNSTREAM))
			downStreamMap[id] = GetIntFromBSONITER(&iterator);
        else
            return -1;
        //throw ModelException("ReadData", "ReadReachesTopologyFromMongoDB", "Subbasin DOWNSTREAM is not found in database.");

        subbasins[id] = new SubbasinStruct(id, group);
        groupSet.insert(group);
        readFailed = false;
    }

	bson_destroy(b);
    // mongo_cursor_destroy(cursor);
	mongoc_cursor_destroy(cursor);
	mongoc_collection_destroy(collection);
    //cout << "Size of groupSet: " << groupSet.size() << endl;
    if (readFailed)
    {
        // bson_print(b);
		cout << bson_as_json(b,NULL) <<endl;
        cout << "The result of query in ReadReachTopologyFromMongoDB is null.\n";
        return -1;
    }

    // fill topology information
    SubbasinStruct *pOutlet = NULL;
    for (map<int, int>::iterator it = downStreamMap.begin(); it != downStreamMap.end(); it++)
    {
        int id = it->first;
        int to = it->second;

        if (to > 0)
        {
            subbasins[id]->downStream = subbasins[to];
            subbasins[to]->upStreams.push_back(subbasins[id]);
        }
        else
        {
            pOutlet = subbasins[id];
        }
    }

    //cout << "end river topology\n";

    // calculate distance to the outlet
    pOutlet->disToOutlet = 0;
    BuildLayer(pOutlet);

    // ranking
    bool finished = false;
    while (!finished)
    {
        finished = true;
        for (map<int, SubbasinStruct *>::iterator it = subbasins.begin(); it != subbasins.end(); ++it)
        {
            if (it->second->rank < 0)
            {
                vector<SubbasinStruct *> &ups = it->second->upStreams;
                // most upstream rivers
                if (ups.empty())
                {
                    it->second->rank = 1;
                }
                else
                    // max rank of upstream ranks + 1
                {
                    int maxUpStreamRank = -1;
                    bool childrenRanked = true;
                    for (size_t i = 0; i < ups.size(); i++)
                    {
                        if (ups[i]->rank < 0)
                        {
                            childrenRanked = false;
                            break;
                        }
                        if (ups[i]->rank > maxUpStreamRank)
                            maxUpStreamRank = ups[i]->rank;
                    }
                    if (childrenRanked)
                        it->second->rank = maxUpStreamRank + 1;
                }
                finished = false;
            }

        }
    }
    return 0;
}

int ReadRiverTopology(const char *reachFile, map<int, SubbasinStruct *> &subbasins, set<int> &groupSet)
{
    OGRDataSource *poDS;

    poDS = OGRSFDriverRegistrar::Open(reachFile);
    if (poDS == NULL)
    {
        printf("OGR Open failed in function ReadRiverTopology.\n");
        return -1;
    }

    OGRLayer *poLayer = poDS->GetLayer(0);
    OGRFeature *poFeature;

    // setup subbasins
    groupSet.clear();
    poLayer->ResetReading();
    while ((poFeature = poLayer->GetNextFeature()) != NULL)
    {
        int id = poFeature->GetFieldAsInteger("Subbasin");
        int group = poFeature->GetFieldAsInteger("GROUP");
        subbasins[id] = new SubbasinStruct(id, group);
        groupSet.insert(group);
    }
    //for(set<int>::iterator iter = groupSet.begin(); iter!=groupSet.end(); iter++)
    //	 cout << *iter<<" ";

    // fill topology information
    poLayer->ResetReading();
    SubbasinStruct *pOutlet = NULL;
    while ((poFeature = poLayer->GetNextFeature()) != NULL)
    {
        int id = poFeature->GetFieldAsInteger("Subbasin");
        int to = poFeature->GetFieldAsInteger("SubbasinR");

        if (to != 0)
        {
            subbasins[id]->downStream = subbasins[to];
            subbasins[to]->upStreams.push_back(subbasins[id]);
        }
        else
        {
            pOutlet = subbasins[id];
        }
    }

    // calculate distance to the outlet
    pOutlet->disToOutlet = 0;
    BuildLayer(pOutlet);

    // ranking
    bool finished = false;
    while (!finished)
    {
        finished = true;
        for (map<int, SubbasinStruct *>::iterator it = subbasins.begin(); it != subbasins.end(); ++it)
        {
            if (it->second->rank < 0)
            {
                vector<SubbasinStruct *> &ups = it->second->upStreams;
                // most upstream rivers
                if (ups.empty())
                {
                    it->second->rank = 1;
                }
                else
                    // max rank of upstream ranks + 1
                {
                    int maxUpStreamRank = -1;
                    bool childrenRanked = true;
                    for (size_t i = 0; i < ups.size(); i++)
                    {
                        if (ups[i]->rank < 0)
                        {
                            childrenRanked = false;
                            break;
                        }
                        if (ups[i]->rank > maxUpStreamRank)
                            maxUpStreamRank = ups[i]->rank;
                    }
                    if (childrenRanked)
                        it->second->rank = maxUpStreamRank + 1;
                }
                finished = false;
            }

        }
    }


    return 0;
}
