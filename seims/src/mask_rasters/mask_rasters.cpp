/*!
 * \brief Get subset of a input raster according to mask
 *
 * \author Liang-Jun Zhu, Junzhi Liu
 * \date Feb. 2017
 * E-mail:  zlj@lreis.ac.cn
 * Copyright (c) 2017. Liang-Jun Zhu
 * 
 */

#if (defined _DEBUG) && (defined MSVC) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */
#include <iostream>
#include <string>
#include <fstream>
#include <time.h>
#include <sstream>
#include <vector>

#ifdef SUPPORT_OMP
#include <omp.h>
#endif /* SUPPORT_OMP */

#include "clsRasterData.cpp"

using namespace std;

int main(int argc, char *argv[]) {
    SetDefaultOpenMPThread();
    GDALAllRegister();

    if (argc < 2) {
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
    if (argc >= 3 && strcmp("-asc", argv[2]) == 0) {
        outputAsc = true;
    }

    const char *configFile = argv[1];
    string maskFile;
    int n = 0;
    vector <string> inputFiles;
    vector <string> outputFiles;
    vector<float> defaultValues;

    // read input information
    string tmp1, tmp2;
    float tmpVal;
    ifstream ifs(configFile);
    ifs >> maskFile;
    ifs >> n;
    for (int i = 0; i < n; i++) {
        ifs >> tmp1 >> tmpVal >> tmp2;
        inputFiles.push_back(tmp1);
        defaultValues.push_back(tmpVal);
        outputFiles.push_back(tmp2);
    }
    ifs.close();

    // read mask information
    clsRasterData<int> maskLayer(maskFile);

    // loop to mask each raster
    for (int i = 0; i < n; ++i) {
        cout << inputFiles[i] << endl;
        clsRasterData<float, int> inputLayer(inputFiles[i], true, &maskLayer, true, defaultValues[i]);

        inputLayer.outputToFile(outputFiles[i]);
    }

    return 0;
}
