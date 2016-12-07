/*----------------------------------------------------------------------
*	Purpose:  Grid layering functions
*
*	Created:	Junzhi Liu
*	Date:		28-March-2013
*
*	Revision:   Liangjun Zhu
*   Date:       21-July-2016
*---------------------------------------------------------------------*/


#include <iostream>
#include <sstream>
#include <ctime>
#include "GridLayering.h"
#include "gridfs.h"
#include "mongo.h"
#include "bson.h"

using namespace std;

int main(int argc, char **argv)
{
        if (argc < 6)
        {
                cout << "usage: grid_layering <hostIP> <port> <output_dir> <modelName> <gridFSName> <nsubbasin>\n";
                exit(-1);
        }

        const char *hostName = argv[1];
        int port = atoi(argv[2]);
        const char *outputDir = argv[3];
        const char *modelName = argv[4];
        const char *gridFSName = argv[5];
        int nSubbasins = atoi(argv[6]);

        mongo conn[1];
        gridfs gfs[1];
        int status = mongo_connect(conn, hostName, port);
        if (MONGO_OK != status)
        {
                cout << "can not connect to MongoDB.\n";
                exit(-1);
        }

        gridfs_init(conn, modelName, gridFSName, gfs);

        int outputNoDataValue = -9999;
        clock_t t1 = clock();
		int subbasinStartID = 1;
		if (nSubbasins == 0)
			subbasinStartID = 0;
        for (int i = subbasinStartID; i <= nSubbasins; i++)
        {
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
                TauDEM2ArcGIS(nRows, nCols, dirMatrix);
                // Output flow out index to MongoDB (D8)
                OutputFlowOutD8(gfs, i, nRows, nCols, nValidGrids, dirMatrix, header.noDataValue, compressedIndex);
                // Output flow in indexes to MongoDB (D8), and write ROUTING_LAYERS from up to down
                string layeringFile = LayeringFromSourceD8(outputDir, gfs, i, nValidGrids, dirMatrix, compressedIndex, header,
                                                           outputNoDataValue);
                //cout << layeringFile << endl;
                // Output ROUTING_LAYERS_UP_DOWN (D8) to MongoDB
                OutputLayersToMongoDB(layeringFile.c_str(), "ROUTING_LAYERS_UP_DOWN", i, gfs);

                // The following code is for D-infinite algorithm
                ostringstream ossDinf;
                ossDinf << i << "_FLOW_DIR_DINF";
                int *dirMatrixDinf;
                ReadFromMongo(gfs, ossDinf.str().c_str(), header, dirMatrixDinf);

                ostringstream ossAngle;
                ossAngle << i << "_FLOW_DIR_ANGLE_DINF";
                float *angle;
                ReadFromMongoFloat(gfs, ossAngle.str().c_str(), header, angle);

                float *flowOutDinf;
                int *outDegreeMatrixDinf = GetOutDegreeMatrix(dirMatrixDinf, nRows, nCols, dirNoDataValue);
                int nOutputFlowOut = OutputMultiFlowOut(nRows, nCols, nValidGrids, outDegreeMatrixDinf, dirMatrixDinf,
                                                        dirNoDataValue, compressedIndex, flowOutDinf);;
                WriteStringToMongoDB(gfs, i, "FLOWOUT_INDEX_DINF", nOutputFlowOut, (const char *) flowOutDinf);

                string layeringFileDinf = LayeringFromSourceDinf(outputDir, gfs, i, nValidGrids, angle, dirMatrixDinf,
                                                                 compressedIndex, header, outputNoDataValue);
                // cout << layeringFileDinf << endl;
                OutputLayersToMongoDB(layeringFileDinf.c_str(), "ROUTING_LAYERS_DINF", i, gfs);


                delete[] dirMatrix;
                delete[] compressedIndex;
                delete[] dirMatrixDinf;
                delete[] outDegreeMatrixDinf;
                delete[] angle;
                dirMatrix = NULL;
                compressedIndex = NULL;
                dirMatrixDinf = NULL;
                outDegreeMatrixDinf = NULL;
                angle = NULL;
        }

        clock_t t2 = clock();

        //cout << t2 - t1 << endl;
        return 0;
}
