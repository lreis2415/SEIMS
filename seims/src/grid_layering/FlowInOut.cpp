// Author: Junzhi Liu
// Date: 2013-09-06
#include <iostream>
#include "GridLayering.h"

using namespace std;
void TauDEM2ArcGIS(int nRows, int nCols, int *&dirMatrix)
{
        /// Find out D8 coding system, TauDEM or ArcGIS
        int maxDirection = -1;
        for (int i = 0; i < nRows; ++i)
        {
                for (int j = 0; j < nCols; ++j)
                {
                        int index = i * nCols + j;
                        if(dirMatrix[index] >= maxDirection)
                                maxDirection = dirMatrix[index];
                }
        }
        ///cout<<maxDirection<<endl;
        bool isTauDEM = false;
        if(maxDirection == 8)
                isTauDEM = true;
        if (isTauDEM)
        {
                for (int i = 0; i < nRows; ++i)
                {
                        for (int j = 0; j < nCols; ++j)
                        {
                                int index = i * nCols + j;
                                if(dirMatrix[index] == 1)
                                        dirMatrix[index] = 1;
                                else if(dirMatrix[index] == 2)
                                        dirMatrix[index] = 128;
                                else if(dirMatrix[index] == 3)
                                        dirMatrix[index] = 64;
                                else if(dirMatrix[index] == 4)
                                        dirMatrix[index] = 32;
                                else if(dirMatrix[index] == 5)
                                        dirMatrix[index] = 16;
                                else if(dirMatrix[index] == 6)
                                        dirMatrix[index] = 8;
                                else if(dirMatrix[index] == 7)
                                        dirMatrix[index] = 4;
                                else if(dirMatrix[index] == 8)
                                        dirMatrix[index] = 2;
                                else
                                        dirMatrix[index] = -9999;
                        }
                }
        }
        //if(isTauDEM)
        // cout<<"TauDEM D8 to ArcGIS Done!"<<endl;
}
void OutputFlowOutD8(gridfs *gfs, int id, int nRows, int nCols, int validCount, const int *dirMatrix,
                     int dirNoDataValue, const int *compressedIndex)
{
        float *pOutput = new float[validCount];
        for (int i = 0; i < nRows; ++i)
        {
                for (int j = 0; j < nCols; ++j)
                {
                        int index = i * nCols + j;
                        if (dirMatrix[index] == dirNoDataValue)
                                continue;

                        int ci = compressedIndex[index];
                        int flow_dir = dirMatrix[index];

                        if ((flow_dir & 1) && (j != nCols - 1) && (dirMatrix[index + 1] != dirNoDataValue))
                                pOutput[ci] = compressedIndex[index + 1];
                        else if ((flow_dir & 2) && (i != nRows - 1) && (j != nCols - 1) &&
                                 (dirMatrix[index + nCols + 1] != dirNoDataValue))
                                pOutput[ci] = compressedIndex[index + nCols + 1];
                        else if ((flow_dir & 4) && (i != nRows - 1) && (dirMatrix[index + nCols] != dirNoDataValue))
                                pOutput[ci] = compressedIndex[index + nCols];
                        else if ((flow_dir & 8) && (i != nRows - 1) && (j != 0) && (dirMatrix[index + nCols - 1] != dirNoDataValue))
                                pOutput[ci] = compressedIndex[index + nCols - 1];
                        else if ((flow_dir & 16) && (j != 0) && (dirMatrix[index - 1] != dirNoDataValue))
                                pOutput[ci] = compressedIndex[index - 1];
                        else if ((flow_dir & 32) && (i != 0) && (j != 0) && (dirMatrix[index - nCols - 1] != dirNoDataValue))
                                pOutput[ci] = compressedIndex[index - nCols - 1];
                        else if ((flow_dir & 64) && (i != 0) && (dirMatrix[index - nCols] != dirNoDataValue))
                                pOutput[ci] = compressedIndex[index - nCols];
                        else if ((flow_dir & 128) && (i != 0) && (j != nCols - 1) &&
                                 (dirMatrix[index - nCols + 1] != dirNoDataValue))
                                pOutput[ci] = compressedIndex[index - nCols + 1];
                        else
                                pOutput[ci] = -1;
                }
        }

        WriteStringToMongoDB(gfs, id, "FLOWOUT_INDEX_D8", validCount, (const char *) pOutput);
        delete[] pOutput;
        pOutput = NULL;
        cout<< "OutputD8FlowOutIndex done, n:" << validCount <<endl;
}

int OutputMultiFlowOut(int nRows, int nCols, int validCount,
                       const int *degreeMatrix, const int *dirMatrix, int dirNoDataValue, const int *compressedIndex,
                       float *&pOutput)
{
        int nAllOut = 0;
        for (int i = 0; i < nRows; ++i)
        {
                for (int j = 0; j < nCols; ++j)
                {
                        if (dirMatrix[i * nCols + j] == dirNoDataValue)
                                continue;
                        nAllOut += degreeMatrix[i * nCols + j];
                }
        }

        int nOutput = nAllOut + validCount + 1;
        pOutput = new float[nOutput];
        pOutput[0] = validCount;
        int counter = 1;
        for (int i = 0; i < nRows; ++i)
        {
                for (int j = 0; j < nCols; ++j)
                {
                        int index = i * nCols + j;
                        if (dirMatrix[index] == dirNoDataValue)
                                continue;

                        pOutput[counter++] = degreeMatrix[index];

                        int flow_dir = dirMatrix[index];

                        if (flow_dir & 1)
                        {
                                if (j != nCols - 1 && dirMatrix[index + 1] != dirNoDataValue)
                                        pOutput[counter++] = compressedIndex[index + 1];
                        }
                        if (flow_dir & 2)
                        {
                                if (i != nRows - 1 && j != nCols - 1 && dirMatrix[index + nCols + 1] != dirNoDataValue)
                                        pOutput[counter++] = compressedIndex[index + nCols + 1];
                        }
                        if (flow_dir & 4)
                        {
                                if (i != nRows - 1 && dirMatrix[index + nCols] != dirNoDataValue)
                                        pOutput[counter++] = compressedIndex[index + nCols];
                        }
                        if (flow_dir & 8)
                        {
                                if (i != nRows - 1 && j != 0 && dirMatrix[index + nCols - 1] != dirNoDataValue)
                                        pOutput[counter++] = compressedIndex[index + nCols - 1];
                        }
                        if (flow_dir & 16)
                        {
                                if (j != 0 && dirMatrix[index - 1] != dirNoDataValue)
                                        pOutput[counter++] = compressedIndex[index - 1];
                        }
                        if (flow_dir & 32)
                        {
                                if (i != 0 && j != 0 && dirMatrix[index - nCols - 1] != dirNoDataValue)
                                        pOutput[counter++] = compressedIndex[index - nCols - 1];
                        }
                        if (flow_dir & 64)
                        {
                                if (i != 0 && dirMatrix[index - nCols] != dirNoDataValue)
                                        pOutput[counter++] = compressedIndex[index - nCols];
                        }
                        if (flow_dir & 128)
                        {
                                if (i != 0 && j != nCols - 1 && dirMatrix[index - nCols + 1] != dirNoDataValue)
                                        pOutput[counter++] = compressedIndex[index - nCols + 1];
                        }
                }
        }

        cout << "OutputMultiFlowOut n:" << nOutput << "\t" << nOutput - counter << endl;

        return nOutput;
}

#define PI 3.1415926f

float GetPercentage(float angle, int di, int dj)
{
        float a = 4.f * angle / PI;
        int n = int(a) % 7;
        float r = a - n;
        switch (n)
        {
        case 0:
        {
                return (di != 0 ? r : 1 - r);
                break;
        }
        case 1:
        {
                return (dj == 0 ? r : 1 - r);
                break;
        }
        case 2:
        {
                return (dj != 0 ? r : 1 - r);
                break;
        }
        case 3:
        {
                return (di == 0 ? r : 1 - r);
                break;
        }
        case 4:
        {
                return (di != 0 ? r : 1 - r);
                break;
        }
        case 5:
        {
                return (dj == 0 ? r : 1 - r);
                break;
        }
        case 6:
        {
                return (dj != 0 ? r : 1 - r);
                break;
        }
        case 7:
        {
                return (di == 0 ? r : 1 - r);
                break;
        }
        default:
        {
                cout << "Invalid angle value: " << angle << endl;
                exit(-1);
                break;
        }
        }
}

int OutputFlowOutPercentageDinf(int nRows, int nCols, int validCount, const float *angle,
                                const int *degreeMatrix, const int *reverseDirMatrix, int dirNoDataValue,
                                const int *compressedIndex, float *&pOutput)
{
        int nAllOut = 0;
        for (int i = 0; i < nRows; ++i)
        {
                for (int j = 0; j < nCols; ++j)
                {
                        if (reverseDirMatrix[i * nCols + j] == dirNoDataValue)
                                continue;
                        nAllOut += degreeMatrix[i * nCols + j];
                }
        }

        int nOutput = nAllOut + validCount + 1;
        pOutput = new float[nOutput];
        pOutput[0] = validCount;
        int counter = 1;
        int fromRow, fromCol;
        for (int i = 0; i < nRows; ++i)
        {
                for (int j = 0; j < nCols; ++j)
                {
                        int index = i * nCols + j;
                        if (reverseDirMatrix[index] == dirNoDataValue)
                                continue;

                        pOutput[counter++] = degreeMatrix[index];

                        int flow_dir = reverseDirMatrix[index];

                        if ((flow_dir & 1) && (j != nCols - 1) && (reverseDirMatrix[index + 1] != dirNoDataValue))
                        {
                                pOutput[counter++] = GetPercentage(angle[index + 1], 0, 1);
                        }
                        if (flow_dir & 2)
                        {
                                if (i != nRows - 1 && j != nCols - 1 && reverseDirMatrix[index + nCols + 1] != dirNoDataValue)
                                        pOutput[counter++] = GetPercentage(angle[index + nCols + 1], 1, 1);
                        }
                        if (flow_dir & 4)
                        {
                                if (i != nRows - 1 && reverseDirMatrix[index + nCols] != dirNoDataValue)
                                        pOutput[counter++] = GetPercentage(angle[index + nCols], 1, 0);
                        }
                        if (flow_dir & 8)
                        {
                                if (i != nRows - 1 && j != 0 && reverseDirMatrix[index + nCols - 1] != dirNoDataValue)
                                        pOutput[counter++] = GetPercentage(angle[index + nCols - 1], 1, -1);
                        }
                        if (flow_dir & 16)
                        {
                                if (j != 0 && reverseDirMatrix[index - 1] != dirNoDataValue)
                                        pOutput[counter++] = GetPercentage(angle[index - 1], 0, -1);
                        }
                        if (flow_dir & 32)
                        {
                                if (i != 0 && j != 0 && reverseDirMatrix[index - nCols - 1] != dirNoDataValue)
                                        pOutput[counter++] = GetPercentage(angle[index - nCols - 1], -1, -1);
                        }
                        if (flow_dir & 64)
                        {
                                if (i != 0 && reverseDirMatrix[index - nCols] != dirNoDataValue)
                                        pOutput[counter++] = GetPercentage(angle[index - nCols], -1, 0);
                        }
                        if (flow_dir & 128)
                        {
                                if (i != 0 && j != nCols - 1 && reverseDirMatrix[index - nCols + 1] != dirNoDataValue)
                                        pOutput[counter++] = GetPercentage(angle[index - nCols + 1], -1, 1);
                        }
                }
        }

        cout << "OutputFlowOutPercentageDinf n:" << nOutput << "\t" << nOutput - counter << endl;

        return nOutput;
}
