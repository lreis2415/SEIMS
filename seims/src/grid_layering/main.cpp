/*----------------------------------------------------------------------
*	Purpose:  Grid layering functions
*
*	Created:	Junzhi Liu
*	Date:		28-March-2013
*
*	Revision:   Liangjun Zhu
*   Date:       21-July-2016
*               9-February-2017
*               4-July-2017  Check if success after import layering to MongoDB
*---------------------------------------------------------------------*/

#if (defined _DEBUG) && (defined MSVC) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */
#include <iostream>
#include <sstream>
#include <ctime>
#include "GridLayering.h"
#include "MongoUtil.h"
#include "clsRasterData.h"

using namespace std;

int main(int argc, char **argv) {
    if (argc < 6) {
        cout << "usage: grid_layering <hostIP> <port> <output_dir> <modelName> <gridFSName> <nsubbasin>\n";
        exit(-1);
    }
    /// set default OpenMP thread number to improve compute efficiency
    SetDefaultOpenMPThread();

    const char *hostName = argv[1];
    int port = atoi(argv[2]);
    const char *outputDir = argv[3];
    const char *modelName = argv[4];
    const char *gridFSName = argv[5];
    int nSubbasins = atoi(argv[6]);

    /// connect to MongoDB
    MongoClient client = MongoClient(hostName, port);
    mongoc_client_t *conn = client.getConn();
    mongoc_gridfs_t *gfs = client.getGridFS(string(modelName), string(gridFSName));

    int outputNoDataValue = (int) NODATA_VALUE;
    double t1 = TimeCounting();
    int subbasinStartID = 1;
    if (nSubbasins == 0) {
        subbasinStartID = 0;
    }
    for (int i = subbasinStartID; i <= nSubbasins; i++) {
        // read D8 flow direction
        ostringstream oss;
        oss << i << "_FLOW_DIR";
        RasterHeader header;
        int *dirMatrix;
        ReadFromMongo(gfs, oss.str().c_str(), header, dirMatrix);

        int nRows = header.nRows;
        int nCols = header.nCols;
        int dirNoDataValue = header.noDataValue;

        int n = nRows * nCols;
        int *compressedIndex = new int[n];
        int nValidGrids = CalCompressedIndex(n, dirMatrix, header.noDataValue, compressedIndex);
        // if it is TauDEM flow code, then convert it to ArcGIS
        TauDEM2ArcGIS(nRows, nCols, dirMatrix, dirNoDataValue);
        // Output flow out index to MongoDB (D8)
        int max_loop = 3;
        int cur_loop = 1;
        while (cur_loop < max_loop) {
            if (!OutputFlowOutD8(outputDir, gfs, i, nRows, nCols, nValidGrids, 
                                dirMatrix, header.noDataValue, compressedIndex)) {
                cur_loop++;
            }
            else
                break;
        }
        if (cur_loop == max_loop) {
            cout << "ERROR! Exceed the max. tries times, please contact the developers!" << endl;
            exit(EXIT_FAILURE);
        }
        
        // Output flow in indexes to MongoDB (D8), and write ROUTING_LAYERS from up to down
        string layeringFile = LayeringFromSourceD8(outputDir, gfs, i, nValidGrids, dirMatrix, compressedIndex, header,
                                                   outputNoDataValue);
        //cout << layeringFile << endl;
        // Output ROUTING_LAYERS_UP_DOWN (D8) to MongoDB
        cur_loop = 1;
        while (cur_loop < max_loop) {
            if (!OutputLayersToMongoDB(layeringFile.c_str(), "ROUTING_LAYERS_UP_DOWN", i, gfs)) {
                cur_loop++;
            }
            else
                break;
        }
        if (cur_loop == max_loop) {
            cout << "ERROR! Exceed the max. tries times, please contact the developers!" << endl;
            exit(EXIT_FAILURE);
        }

        // The following code is for D-infinite algorithm
        ostringstream ossDinf;
        ossDinf << i << "_FLOW_DIR_DINF";
        int *dirMatrixDinf;
        RasterHeader dinf_header;
        ReadFromMongo(gfs, ossDinf.str().c_str(), dinf_header, dirMatrixDinf);

        ostringstream ossAngle;
        ossAngle << i << "_FLOW_DIR_ANGLE_DINF";
        float *angle;
        RasterHeader dinfang_header;
        ReadFromMongoFloat(gfs, ossAngle.str().c_str(), dinfang_header, angle);

        float *flowOutDinf;
        int *outDegreeMatrixDinf = GetOutDegreeMatrix(dirMatrixDinf, nRows, nCols, dinf_header.noDataValue);
        int nOutputFlowOut = OutputMultiFlowOut(nRows, nCols, nValidGrids, outDegreeMatrixDinf, dirMatrixDinf,
                                                dinf_header.noDataValue, compressedIndex, flowOutDinf);
        WriteStringToMongoDB(gfs, i, "FLOWOUT_INDEX_DINF", nOutputFlowOut, (char *) flowOutDinf);

        string layeringFileDinf = LayeringFromSourceDinf(outputDir, gfs, i, nValidGrids, angle, dirMatrixDinf,
                                                         compressedIndex, dinfang_header, (int) NODATA_VALUE);
        // cout << layeringFileDinf << endl;
        OutputLayersToMongoDB(layeringFileDinf.c_str(), "ROUTING_LAYERS_DINF", i, gfs);

        delete[] dirMatrix;
        delete[] compressedIndex;
        delete[] dirMatrixDinf;
        delete[] outDegreeMatrixDinf;
        delete[] angle;
        delete[] flowOutDinf;
        dirMatrix = NULL;
        compressedIndex = NULL;
        dirMatrixDinf = NULL;
        outDegreeMatrixDinf = NULL;
        angle = NULL;
        flowOutDinf = NULL;
    }

    double t2 = TimeCounting();
    cout << "time-consuming: " << t2 - t1 << " seconds." << endl;
    return 0;
}
