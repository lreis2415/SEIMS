/***************************************************************************
*
* Purpose: ReClassify raster data. 
*
*
* Author:  Liang-Jun Zhu, Junzhi Liu
* E-mail:  zlj@lreis.ac.cn
****************************************************************************
* Copyright (c) 2017. Liang-Jun Zhu, Junzhi Liu
* 
****************************************************************************/


#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
#include <sstream>
#include <vector>
#include <map>

#ifdef SUPPORT_OMP
#include <omp.h>
#endif

#include "clsRasterData.cpp"

using namespace std;

bool ReadReclassMap(const char *filename, map<int, float> &_reclassMap) {
    ifstream ofs(filename);
    int n;
    ofs >> n;
    int k;
    float value;
    for (int i = 0; i < n; i++) {
        ofs >> k >> value;
        _reclassMap[k] = value;
    }
    return true;
}

int main(int argc, char *argv[]) {
    SetDefaultOpenMPThread();
    GDALAllRegister();
    char *configFile;

    if (argc < 2) {
        cout << "Usage of reclassify:\n\treclassify configFile\n";
        cout << "\nFormat of the config file: \n\n";
        cout << "typeLayer defaultType\n";
        cout << "lookupFolder\n";
        cout << "outputFolder\n";
        cout << "numOutputs\n";
        cout << "attribute1\n";
        cout << "attribute2\n";
        cout << "...\n\n";
        cout << "NOTE: No space is allowed in the filename currently.\n";

        exit(-1);
    } else {
        configFile = argv[1];
    }
    // read input information
    string typeFile, lookupFolder, outputFolder, tmp;
    int n = 0;
    int defaultType = -1;
    vector <string> attrNames;
    // read attribute list
    ifstream ifs(configFile);
    ifs >> typeFile >> defaultType >> lookupFolder >> outputFolder >> n;
    for (int i = 0; i < n; ++i) {
        ifs >> tmp;
        attrNames.push_back(tmp);
    }
    ifs.close();

    int nLen = lookupFolder.length();
    if (lookupFolder.substr(nLen - 1, 1) != SEP) {
        lookupFolder += SEP;
    }
    nLen = outputFolder.length();
    if (outputFolder.substr(nLen - 1, 1) != SEP) {
        outputFolder += SEP;
    }

    clsRasterData<float> typeRaster(typeFile);

    // loop to reclassify each attribute
    string lookupFile, outputFile;
    for (int i = 0; i < n; ++i) {
        lookupFile = lookupFolder + attrNames[i] + ".txt";
        outputFile = outputFolder + attrNames[i] + ".tif";
        clsRasterData<float> outputLayer;
        outputLayer.Copy(typeRaster);
        map<int, float> reclassMap;
        ReadReclassMap(lookupFile.c_str(), reclassMap);
        outputLayer.reclassify(reclassMap);
        outputLayer.outputToFile(outputFile);
    }
    return 0;
}
