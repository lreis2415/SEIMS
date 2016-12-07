/*----------------------------------------------------------------------
*	Purpose:  Grid layering from outlet
*
*	Created:	Junzhi Liu
*	Date:		28-March-2013
*
*	Revision:
*   Date:
*---------------------------------------------------------------------*/
#include <iostream>
#include <ctime>
#include <fstream>
#include <sstream>
#include <vector>
#include "GridLayering.h"

using namespace std;


void LayeringFromOutlet(int nValidGrids, const int *dirMatrix, const int *compressedIndex, int nRows, int nCols,
                        RasterHeader &header, int outputNoDataValue, const char *outputTxtFile,
                        const char *outputAscFile)
{
        clock_t t1, t2;
        int dirNoDataValue = header.noDataValue;

        // preprocessing
        t1 = clock();
        int *outDegreeMatrix = GetOutDegreeMatrix(dirMatrix, nRows, nCols, dirNoDataValue);
        t2 = clock();
        //cout << "Preprocessing time: " << t2 - t1 << endl;


        // perform grid layering
        t1 = clock();
        int *layeringNum = NULL;
        string outputStr = GridLayeringFromOutlet(nValidGrids, dirMatrix, compressedIndex, outDegreeMatrix, nRows, nCols,
                                                  dirNoDataValue, outputNoDataValue, layeringNum);
        t2 = clock();
        //cout << "Layering time: " << t2 - t1 << endl;

        // output layering result
        OutputArcAscii(outputAscFile, header, layeringNum, outputNoDataValue);

        ofstream ofs(outputTxtFile);
        ofs << outputStr;
        ofs.close();

        delete[] outDegreeMatrix;
        delete[] layeringNum;
        outDegreeMatrix = NULL;
        layeringNum = NULL;
}

int *GetOutDegreeMatrix(const int *dirMatrix, int nRows, int nCols, int dirNoDataValue)
{
        int n = nRows * nCols;
        int *outDegreeMatrix = new int[n];
        int index = 0;
        for (int i = 0; i < nRows; i++)
        {
                for (int j = 0; j < nCols; j++)
                {
                        index = i * nCols + j;
                        outDegreeMatrix[index] = 0;

                        int flow_dir = dirMatrix[index];
                        if (flow_dir == dirNoDataValue)
                        {
                                outDegreeMatrix[index] = dirNoDataValue;
                                continue;
                        }

                        if ((flow_dir & 1) && (j != nCols - 1) && (dirMatrix[index + 1] != dirNoDataValue))
                                outDegreeMatrix[index]++;

                        if ((flow_dir & 2) && (i != nRows - 1) && (j != nCols - 1) &&
                            (dirMatrix[index + nCols + 1] != dirNoDataValue))
                                outDegreeMatrix[index]++;

                        if ((flow_dir & 4) && (i != nRows - 1) && (dirMatrix[index + nCols] != dirNoDataValue))
                                outDegreeMatrix[index]++;

                        if ((flow_dir & 8) && (i != nRows - 1) && (j != 0) && (dirMatrix[index + nCols - 1] != dirNoDataValue))
                                outDegreeMatrix[index]++;

                        if ((flow_dir & 16) && (j != 0) && (dirMatrix[index - 1] != dirNoDataValue))
                                outDegreeMatrix[index]++;

                        if ((flow_dir & 32) && (i != 0) && (j != 0) && (dirMatrix[index - nCols - 1] != dirNoDataValue))
                                outDegreeMatrix[index]++;

                        if ((flow_dir & 64) && (i != 0) && (dirMatrix[index - nCols] != dirNoDataValue))
                                outDegreeMatrix[index]++;

                        if ((flow_dir & 128) && (i != 0) && (j != nCols - 1) && (dirMatrix[index - nCols + 1] != dirNoDataValue))
                                outDegreeMatrix[index]++;
                }
        }
        return outDegreeMatrix;
}

void UpdateUpStream(int i, int j, int nRows, int nCols, const int *dirMatrix, int *outDegreeMatrix,
                    int &numNextLayer, int *nextLayer)
{
        int index = i * nCols + j + 1;
        if (j != nCols - 1 && dirMatrix[index] & 16)
        {
                if ((--outDegreeMatrix[index]) == 0)
                {
                        nextLayer[numNextLayer++] = index;
                }
        }

        index = (i + 1) * nCols + j + 1;
        if (i != nRows - 1 && j != nCols - 1 && dirMatrix[index] & 32)
        {
                if ((--outDegreeMatrix[index]) == 0)
                {
                        nextLayer[numNextLayer++] = index;
                }
        }

        index = (i + 1) * nCols + j;
        if (i != nRows - 1 && dirMatrix[index] & 64)
        {
                if ((--outDegreeMatrix[index]) == 0)
                {
                        nextLayer[numNextLayer++] = index;
                }
        }

        index = (i + 1) * nCols + j - 1;
        if (i != nRows - 1 && j != 0 && dirMatrix[index] & 128)
        {
                if ((--outDegreeMatrix[index]) == 0)
                {
                        nextLayer[numNextLayer++] = index;
                }
        }

        index = i * nCols + j - 1;
        if (j != 0 && dirMatrix[index] & 1)
        {
                if ((--outDegreeMatrix[index]) == 0)
                {
                        nextLayer[numNextLayer++] = index;
                }
        }

        index = (i - 1) * nCols + j - 1;
        if (i != 0 && j != 0 && dirMatrix[index] & 2)
        {
                if ((--outDegreeMatrix[index]) == 0)
                {
                        nextLayer[numNextLayer++] = index;
                }
        }

        index = (i - 1) * nCols + j;
        if (i != 0 && dirMatrix[index] & 4)
        {
                if ((--outDegreeMatrix[index]) == 0)
                {
                        nextLayer[numNextLayer++] = index;
                }
        }

        index = (i - 1) * nCols + j + 1;
        if (i != 0 && j != nCols - 1 && dirMatrix[index] & 8)
        {
                if ((--outDegreeMatrix[index]) == 0)
                {
                        nextLayer[numNextLayer++] = index;
                }
        }
}

string GridLayeringFromOutlet(int nValidGrids, const int *dirMatrix, const int *compressedIndex, int *outDegreeMatrix,
                              int nRows, int nCols, int dirNoDataValue, int outputNoDataValue, int *&layeringNum)
{
        int n = nRows * nCols;
        layeringNum = new int[n];

        int *lastLayer = new int[n];
        int numLastLayer = 0;

        bool flag = true;
        int curNum = 0;
        int index = 0;


        // 1. find outlet grids
        for (int i = 0; i < nRows; i++)
        {
                for (int j = 0; j < nCols; j++)
                {
                        index = i * nCols + j;

                        if (dirMatrix[index] == dirNoDataValue)
                        {
                                layeringNum[index] = outputNoDataValue;
                        }
                        else
                        {
                                if (outDegreeMatrix[index] == 0)
                                {
                                        lastLayer[numLastLayer++] = index;
                                }
                        }
                }
        }

        string strLayers = "";
        //cout << "nubmer of outlets: " << numLastLayer << endl;

        // 2. loop and layer
        int numNextLayer = 0;
        int *nextLayer = new int[n];
        int *tmp;

        while (numLastLayer > 0)
        {
                curNum++;

                ostringstream oss;
                oss << "\n" << numLastLayer << "\t";

                numNextLayer = 0;
                for (int iInLayer = 0; iInLayer < numLastLayer; iInLayer++)
                {
                        index = lastLayer[iInLayer];
                        oss << compressedIndex[index] << "\t";

                        layeringNum[index] = curNum;

                        int i = index / nCols;
                        int j = index % nCols;

                        UpdateUpStream(i, j, nRows, nCols, dirMatrix, outDegreeMatrix, numNextLayer, nextLayer);
                }
                strLayers = oss.str() + strLayers;

                numLastLayer = numNextLayer;

                tmp = lastLayer;
                lastLayer = nextLayer;
                nextLayer = tmp;
        }

        ostringstream outputOss;
        outputOss << nValidGrids << "\t" << curNum << strLayers;
        // 3. reverse the layer number
        for (int i = 0; i < n; i++)
        {
                if (layeringNum[i] != outputNoDataValue)
                        layeringNum[i] = curNum - layeringNum[i] + 1;
        }

        delete[] lastLayer;
        delete[] nextLayer;
        lastLayer = NULL;
        nextLayer = NULL;

        return outputOss.str();
}
