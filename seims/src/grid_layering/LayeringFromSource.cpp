/*----------------------------------------------------------------------
*	Purpose:  Grid layering from source
*
*	Created:	Junzhi Liu
*	Date:		28-March-2013
*
*	Revision:
*   Date:
*---------------------------------------------------------------------*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include "GridLayering.h"

using namespace std;

string LayeringFromSourceD8(const char *outputDir, mongoc_gridfs_t *gfs, int id, int nValidGrids, const int *dirMatrix,
                            const int *compressedIndex, RasterHeader &header, int outputNoDataValue) {

    int nRows = header.nRows;
    int nCols = header.nCols;
    int dirNoDataValue = header.noDataValue;

    // preprocessing
    /* accumulate each cell's upstream direction
     * e.g. cell (i, j) has three upstream source: (i, j+1), (i-1, j), (i+1, j)
     * then the reverseDir value of cell (i, j) is 1 + 64 + 4 = 69
     */
    int *reverseDirMatrix = GetReverseDirMatrix(dirMatrix, nRows, nCols, dirNoDataValue);
    /* count each cell's upstream number by bitwise AND operator
     * e.g. cell (i, j) has a reversed direction value of 69, which stored as 1000101
     * 1000101 & 1 is True, and so as to 100, 1000000. So the upstream cell number is 3.
     */
    int *inDegreeMatrix = GetInDegreeMatrix(reverseDirMatrix, nRows, nCols, dirNoDataValue);

    float *flowInOutput;
    int nFlowInOutput = OutputMultiFlowOut(nRows, nCols, nValidGrids, inDegreeMatrix, reverseDirMatrix, dirNoDataValue,
                                           compressedIndex, flowInOutput);
    // output to txt
    ostringstream oss;
    oss << outputDir << "/" << id << "_FLOWIN_INDEX_D8.txt";
    ofstream ofs_flowInD8(oss.str().c_str());
    ofs_flowInD8 << flowInOutput[0] << endl;
    ofs_flowInD8 << "ID\tUpstreamCount\tUpstreamID" << endl;
    int tmpCount = 1;
    for (int i = 0; i < flowInOutput[0]; i++) {
        int upstreamcount = flowInOutput[tmpCount++];
        ofs_flowInD8 << i << "\t" << upstreamcount << "\t";
        for (int j = 0; j < upstreamcount; j++) {
            ofs_flowInD8 << flowInOutput[tmpCount++] << ", ";
        }
        ofs_flowInD8 << endl;
    }
    ofs_flowInD8.close();
    int max_loop = 3;
    int cur_loop = 1;
    while (cur_loop < max_loop) {
        if (!WriteStringToMongoDB(gfs, id, "FLOWIN_INDEX_D8", nFlowInOutput, (char *)flowInOutput)) {
            cur_loop++;
        }
        else
            break;
    }
    if (cur_loop == max_loop) {
        cout << "ERROR! Exceed the max. tries times, please contact the developers!" << endl;
        exit(EXIT_FAILURE);
    }
    // perform grid layering
    int *layeringGrid = NULL;
    string outputStr = GridLayeringFromSource(nValidGrids, dirMatrix, compressedIndex, inDegreeMatrix, nRows, nCols,
                                              dirNoDataValue, outputNoDataValue, layeringGrid);

    // output layering result
    // ostringstream oss;
    oss << outputDir << "/" << id << "_ROUTING_LAYERS_UP_DOWN.asc";
    //cout << oss.str().c_str() << endl;
    OutputArcAscii(oss.str().c_str(), header, layeringGrid, outputNoDataValue);

    oss.str("");
    oss << outputDir << "/" << id << "_ROUTING_LAYERS_UP_DOWN.txt";
    //cout << oss.str().c_str() << endl;
    ofstream ofs(oss.str().c_str());
    ofs << outputStr;
    ofs.close();

    delete[] reverseDirMatrix;
    delete[] inDegreeMatrix;
    delete[] layeringGrid;
    delete[] flowInOutput;
    reverseDirMatrix = NULL;
    inDegreeMatrix = NULL;
    layeringGrid = NULL;
    flowInOutput = NULL;

    cout << "OutputD8FlowInIndexes done!" << endl;
    return oss.str();
}

string LayeringFromSourceDinf(const char *outputDir, mongoc_gridfs_t *gfs, int id, int nValidGrids, const float *angle,
                              const int *dirMatrix, const int *compressedIndex, RasterHeader &header,
                              int outputNoDataValue) {
    clock_t t1, t2;
    int nRows = header.nRows;
    int nCols = header.nCols;
    int dirNoDataValue = header.noDataValue;

    // preprocessing
    t1 = clock();
    int *reverseDirMatrix = GetReverseDirMatrix(dirMatrix, nRows, nCols, dirNoDataValue);
    int *inDegreeMatrix = GetInDegreeMatrix(reverseDirMatrix, nRows, nCols, dirNoDataValue);

    float *flowInOutput;
    // reverse dir means flow in relationships
    int nFlowInOutput = OutputMultiFlowOut(nRows, nCols, nValidGrids, inDegreeMatrix, reverseDirMatrix, dirNoDataValue,
                                           compressedIndex, flowInOutput);
    WriteStringToMongoDB(gfs, id, "FLOWIN_INDEX_DINF", nFlowInOutput, (char *) flowInOutput);

    float *percentage;
    OutputFlowOutPercentageDinf(nRows, nCols, nValidGrids, angle, inDegreeMatrix, reverseDirMatrix, dirNoDataValue,
                                compressedIndex, percentage);
    WriteStringToMongoDB(gfs, id, "FLOWIN_PERCENTAGE_DINF", nFlowInOutput, (char *) percentage);
    t2 = clock();

    // perform grid layering
    t1 = clock();
    int *layeringGrid = NULL;
    string outputStr = GridLayeringFromSource(nValidGrids, dirMatrix, compressedIndex, inDegreeMatrix, nRows, nCols,
                                              dirNoDataValue, outputNoDataValue, layeringGrid);
    t2 = clock();
    //cout << "Layering time: " << t2 - t1 << endl;

    // output layering result
    ostringstream oss;
    oss << outputDir << "/" << id << "_ROUTING_LAYERS_DINF.asc";
    //cout << oss.str().c_str() << endl;
    OutputArcAscii(oss.str().c_str(), header, layeringGrid, outputNoDataValue);

    oss.str("");
    oss << outputDir << "/" << id << "_ROUTING_LAYERS_DINF.txt";
    //cout << oss.str().c_str() << endl;
    ofstream ofs(oss.str().c_str());
    ofs << outputStr;

    delete[] reverseDirMatrix;
    delete[] inDegreeMatrix;
    delete[] layeringGrid;
    delete[] flowInOutput;
    delete[] percentage;
    reverseDirMatrix = NULL;
    inDegreeMatrix = NULL;
    layeringGrid = NULL;
    flowInOutput = NULL;
    percentage = NULL;

    return oss.str();
}

// Get the reverse flow direction matrix
// Modified from Zhan Li-Jun 2012
int *GetReverseDirMatrix(const int *dirMatrix, int nRows, int nCols, int dirNoDataValue) {
    int n = nRows * nCols;
    int *reverseDirMatrix = new int[n];
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        reverseDirMatrix[i] = 0;
    }
#pragma omp parallel for
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            int index = i * nCols + j;
            int flow_dir = dirMatrix[index];
            if (flow_dir == dirNoDataValue || flow_dir < 0) {
                reverseDirMatrix[index] = dirNoDataValue;
                continue;
            }

            if ((flow_dir & 1) && j != nCols - 1 && dirMatrix[index + 1] != dirNoDataValue
                && dirMatrix[index + 1] > 0) {
                    reverseDirMatrix[index + 1] += 16;
            }

            if ((flow_dir & 2) && i != nRows - 1 && j != nCols - 1 && dirMatrix[index + nCols + 1] != dirNoDataValue &&
                dirMatrix[index + nCols + 1] > 0) {
                    reverseDirMatrix[index + nCols + 1] += 32;
            }

            if ((flow_dir & 4) && i != nRows - 1 && dirMatrix[index + nCols] != dirNoDataValue &&
                dirMatrix[index + nCols] > 0) {
                    reverseDirMatrix[index + nCols] += 64;
            }

            if ((flow_dir & 8) && i != nRows - 1 && j != 0 && dirMatrix[index + nCols - 1] != dirNoDataValue &&
                dirMatrix[index + nCols - 1] > 0) {
                    reverseDirMatrix[index + nCols - 1] += 128;
            }

            if ((flow_dir & 16) && j != 0 && dirMatrix[index - 1] != dirNoDataValue && dirMatrix[index - 1] > 0) {
                reverseDirMatrix[index - 1] += 1;
            }

            if ((flow_dir & 32) && i != 0 && j != 0 && dirMatrix[index - nCols - 1] != dirNoDataValue &&
                dirMatrix[index - nCols - 1] > 0) {
                    reverseDirMatrix[index - nCols - 1] += 2;
            }

            if ((flow_dir & 64) && i != 0 && dirMatrix[index - nCols] != dirNoDataValue
                && dirMatrix[index - nCols] > 0) {
                    reverseDirMatrix[index - nCols] += 4;
            }

            if ((flow_dir & 128) && i != 0 && j != nCols - 1 && dirMatrix[index - nCols + 1] != dirNoDataValue &&
                dirMatrix[index - nCols + 1] > 0) {
                    reverseDirMatrix[index - nCols + 1] += 8;
            }
        }
    }

    return reverseDirMatrix;
}

// Get the in-degree matrix
int *GetInDegreeMatrix(const int *reverseDirMatrix, int nRows, int nCols, int noDataValue) {
    int n = nRows * nCols;
    int *inDegreeMatrix = new int[n];
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        inDegreeMatrix[i] = 0;
        int flow_dir = reverseDirMatrix[i];
        if (flow_dir == noDataValue || flow_dir < 0) {
            inDegreeMatrix[i] = noDataValue;
            continue;
        }

        if (flow_dir & 1) {
            inDegreeMatrix[i]++;
        }
        if (flow_dir & 2) {
            inDegreeMatrix[i]++;
        }
        if (flow_dir & 4) {
            inDegreeMatrix[i]++;
        }
        if (flow_dir & 8) {
            inDegreeMatrix[i]++;
        }
        if (flow_dir & 16) {
            inDegreeMatrix[i]++;
        }
        if (flow_dir & 32) {
            inDegreeMatrix[i]++;
        }
        if (flow_dir & 64) {
            inDegreeMatrix[i]++;
        }
        if (flow_dir & 128) {
            inDegreeMatrix[i]++;
        }
    }

    return inDegreeMatrix;
}

void UpdateDownStream(int i, int j, int nRows, int nCols, const int *dirMatrix, int *inDegreeMatrix,
                      int &numNextLayer, int *nextLayer) {
    int index = i * nCols + j;
    int dir = dirMatrix[index];

    if ((dir & 1) && (j != nCols - 1)) {
        if ((--inDegreeMatrix[index + 1]) == 0) {
            nextLayer[numNextLayer++] = index + 1;
        }
    }

    if ((dir & 2) && i != nRows - 1 && j != nCols - 1) {
        if ((--inDegreeMatrix[index + nCols + 1]) == 0) {
            nextLayer[numNextLayer++] = index + nCols + 1;
        }
    }

    if ((dir & 4) && i != nRows - 1) {
        if ((--inDegreeMatrix[index + nCols]) == 0) {
            nextLayer[numNextLayer++] = index + nCols;
        }
    }

    if ((dir & 8) && i != nRows - 1 && j != 0) {
        if ((--inDegreeMatrix[index + nCols - 1]) == 0) {
            nextLayer[numNextLayer++] = index + nCols - 1;
        }
    }

    if ((dir & 16) && j != 0) {
        if ((--inDegreeMatrix[index - 1]) == 0) {
            nextLayer[numNextLayer++] = index - 1;
        }
    }

    if ((dir & 32) && i != 0 && j != 0) {
        if ((--inDegreeMatrix[index - nCols - 1]) == 0) {
            nextLayer[numNextLayer++] = index - nCols - 1;
        }
    }

    if ((dir & 64) && i != 0) {
        if ((--inDegreeMatrix[index - nCols]) == 0) {
            nextLayer[numNextLayer++] = index - nCols;
        }
    }

    if ((dir & 128) && i != 0 && j != nCols - 1) {
        if ((--inDegreeMatrix[index - nCols + 1]) == 0) {
            nextLayer[numNextLayer++] = index - nCols + 1;
        }
    }
}

string GridLayeringFromSource(int nValidGrids, const int *dirMatrix, const int *compressedIndex, int *inDegreeMatrix,
                              int nRows, int nCols, int dirNoDataValue, int outputNoDataValue, int *&layeringGrid) {
    int n = nRows * nCols;
    layeringGrid = new int[n]; //the value in this grid is the layering number
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        layeringGrid[i] = outputNoDataValue;
    }

    int *lastLayer = new int[n];
    int numLastLayer = 0; // the number of cells of last layer

    int index = 0;

    // 1. find source grids
    for (int i = 0; i < nRows; i++) {
        for (int j = 0; j < nCols; j++) {
            index = i * nCols + j;

            if (dirMatrix[index] == dirNoDataValue || dirMatrix[index] < 0) {
                continue;
            } else {
                if (inDegreeMatrix[index] == 0) {
                    lastLayer[numLastLayer++] = index;
                }
            }
        }
    }

    // 2. loop and layer
    int numNextLayer = 0;
    int curNum = 0; //the layering number of the current layer. In the end, it is the total number of layers.
    int *nextLayer = new int[n];
    int *tmp;
    ostringstream oss;
    while (numLastLayer > 0) {
        oss << "\n" << numLastLayer << "\t";
        curNum++;

        numNextLayer = 0;
        for (int iInLayer = 0; iInLayer < numLastLayer; iInLayer++) {
            index = lastLayer[iInLayer];
            oss << compressedIndex[index] << "\t";

            layeringGrid[index] = curNum;

            int i = index / nCols;
            int j = index % nCols;

            UpdateDownStream(i, j, nRows, nCols, dirMatrix, inDegreeMatrix, numNextLayer, nextLayer);
        }

        numLastLayer = numNextLayer;

        tmp = lastLayer;
        lastLayer = nextLayer;
        nextLayer = tmp;
    }

    delete[] lastLayer;
    delete[] nextLayer;
    lastLayer = NULL;
    nextLayer = NULL;

    ostringstream outputOss;
    outputOss << nValidGrids << "\t" << curNum << oss.str();

    return outputOss.str();
}
