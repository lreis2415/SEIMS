/***************************************************************************
*
* Purpose: Get subset of a input raster according to mask. 
*
*
* Author:  Junzhi Liu
* E-mail:  liujz@lreis.ac.cn
****************************************************************************
* Copyright (c) 2013. Junzhi Liu
* 
****************************************************************************/


#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
#include <sstream>
#include <vector>
#include <omp.h>

#include "Raster.cpp"

using namespace std;

template<typename T>
int ApplyMaskToRaster(Raster<int> &mask, Raster<T> &input, Raster<T> &output, bool hasDefault = false,
                      T defaultValue = -9999)
{
    T outputNoDataValue = -9999;
    if (!hasDefault)
        defaultValue = outputNoDataValue;

    int xSizeMask = mask.GetNumberofColumns();
    int ySizeMask = mask.GetNumberOfRows();
    int nMask = xSizeMask * ySizeMask;
    int *maskData = mask.GetData();
    int noDataMask = mask.GetNoDataValue();
    double xMinMask = mask.GetXMin();
    double yMaxMask = mask.GetYMax();
    double dxMask = mask.GetXCellSize();
    double dyMask = mask.GetYCellSize();

    int xSize = input.GetNumberofColumns();
    int ySize = input.GetNumberOfRows();
    double noDataValue = input.GetNoDataValue();
    double dx = input.GetXCellSize();
    double dy = input.GetYCellSize();
    double xMin = input.GetXMin();
    double xMax = xMin + xSize * dx;
    double yMax = input.GetYMax();
    double yMin = yMax - ySize * dy;

    T *inputData = input.GetData();
    T *outputData = output.GetData();

    int iRow, iCol, iRowInput, iColInput;
    double x, y;
    for (int i = 0; i < nMask; ++i)
    {
        // values outside the mask
        if (maskData[i] - noDataMask == 0)
        {
            outputData[i] = outputNoDataValue;
            continue;
        }

        iRow = i / xSizeMask;
        iCol = i % xSizeMask;

        x = xMinMask + (iCol + 0.5) * dxMask;
        y = yMaxMask - (iRow + 0.5) * dyMask;

        if (x < xMin || x > xMax || y < yMin || y > yMax)
        {
            outputData[i] = defaultValue;
            continue;
        }

        iColInput = int((x - xMin) / dx);
        iRowInput = int((yMax - y) / dy);

        outputData[i] = inputData[iRowInput * xSize + iColInput];

        if (int(outputData[i]) == int(noDataValue))
        {
            //cout << outputData[i] << "\t" << noDataValue << "\t" << abs(outputData[i] - noDataValue) << "\t" << RASTER_MINI_VALUE << endl;
            outputData[i] = defaultValue;
            bool done = false;
            int nbr = 1;
            while (!done)
            {
                for (int m = -1; m <= 1; m++)
                {
                    for (int n = -1; n <= 1; n++)
                    {
                        int ii = iRowInput + m * nbr;
                        int jj = iColInput + n * nbr;

                        if (ii < 0 || jj < 0 || ii >= ySize || jj >= xSize)
                            continue;

                        if (int(inputData[ii * xSize + jj]) != int(noDataValue))
                        {
                            outputData[i] = inputData[ii * xSize + jj];
                            done = true;
                            break;
                        }
                    }
                }
                ++nbr;
            }
        }

        //cout << iRow << "\t" <<  iCol << "\t" << outputData[i] << "\t" << noDataValue << endl;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    //omp_set_num_threads(threadNUM);
    GDALAllRegister();

    if (argc < 2)
    {
        cout << "A config file should be given as input.\n";
        cout << "Format of config file: \n\n";
        cout << "maskFile\n";
        cout << "nRasters\n";
        cout << "inputFile1\tdefaultValue1\toutput1\n";
        cout << "inputFile2\tdefaultValue2\toutput2\n";
        cout << "...\n\n";
        cout << "NOTE: No space is allowed in the filename currently.\n";
        exit(-1);
    }

    bool outputAsc = false;
    if (argc >= 3 && strcmp("-asc", argv[2]) == 0)
        outputAsc = true;

    const char *configFile = argv[1];
    string maskFile;
    int n = 0;
    vector<string> inputFiles;
    vector<string> outputFiles;
    vector<float> defaultValues;

    // read input information
    string tmp1, tmp2;
    float tmpVal;
    ifstream ifs(configFile);
    ifs >> maskFile;
    ifs >> n;
    for (int i = 0; i < n; i++)
    {
        ifs >> tmp1 >> tmpVal >> tmp2;
        inputFiles.push_back(tmp1);
        defaultValues.push_back(tmpVal);
        outputFiles.push_back(tmp2);
    }
    ifs.close();

    // read mask information
    Raster<int> maskLayer;
    maskLayer.ReadAsInt32(maskFile.c_str());

    // loop to mask each raster
    for (int i = 0; i < n; ++i)
    {
        cout << inputFiles[i] << endl;
        Raster<float> inputLayer;
        inputLayer.ReadAsFloat32(inputFiles[i].c_str());

        Raster<float> outputLayer;
        outputLayer.CopyMask(maskLayer);
        outputLayer.SetDataType(GDT_Float32);

        if (abs(defaultValues[i] + 9999) > RASTER_MINI_VALUE)
        {
            cout << defaultValues[i] << endl;
            ApplyMaskToRaster<float>(maskLayer, inputLayer, outputLayer, true, defaultValues[i]);
        }
        else
            ApplyMaskToRaster<float>(maskLayer, inputLayer, outputLayer);


        if (outputAsc)
            outputLayer.OutputAsc(outputFiles[i].c_str());
        else
            outputLayer.OutputGTiff(outputFiles[i].c_str());
    }

    return 0;
}
