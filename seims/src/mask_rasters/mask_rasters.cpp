/*!
 * \brief Get subset of a input raster according to mask
 *
 * \author Liang-Jun Zhu, Junzhi Liu
 * \date Feb. 2017
 * E-mail:  zlj@lreis.ac.cn
 * Copyright (c) 2017. Liang-Jun Zhu
 *
 */

#if (defined _DEBUG) && (defined _MSC_VER) && (defined VLD)
#include "vld.h"
#endif /* Run Visual Leak Detector during Debug */

#ifdef SUPPORT_OMP
#include <omp.h>
#endif /* SUPPORT_OMP */

#include "basic.h"
#include "data_raster.h"

using namespace ccgl;
using namespace data_raster;

int main(int argc, char* argv[]) {
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

    const char* config_file = argv[1];
    string mask_file;
    int n = 0;
    vector<string> input_files;
    vector<string> output_files;
    vector<float> default_values;

    // read input information
    string tmp1, tmp2;
    float tmp_val;
    std::ifstream ifs(config_file);
    ifs >> mask_file;
    ifs >> n;
    for (int i = 0; i < n; i++) {
        ifs >> tmp1 >> tmp_val >> tmp2;
        input_files.push_back(tmp1);
        default_values.push_back(tmp_val);
        output_files.push_back(tmp2);
    }
    ifs.close();

    // read mask information
    clsRasterData<int>* mask_layer = clsRasterData<int>::Init(mask_file);
    if (nullptr == mask_layer) exit(-1);

    // loop to mask each raster
    for (int i = 0; i < n; ++i) {
        cout << input_files[i] << endl;
        clsRasterData<float, int>* input_layer = clsRasterData<float, int>::Init(input_files[i], true,
                                                                                 mask_layer, true,
                                                                                 default_values[i]);
        input_layer->OutputToFile(output_files[i]);
        delete input_layer;
    }
    delete mask_layer;
    return 0;
}
