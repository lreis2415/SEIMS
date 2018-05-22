/***************************************************************************
*
* Purpose: ReClassify raster data.
*
*
* Author:  Liang-Jun Zhu, Junzhi Liu
* E-mail:  zlj@lreis.ac.cn
****************************************************************************
* Copyright (c) 2017. Liang-Jun Zhu, Junzhi Liu
****************************************************************************/

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

bool ReadReclassMap(const char* filename, map<int, float>& reclass_map) {
    std::ifstream ofs(filename);
    int n;
    ofs >> n;
    int k;
    float value;
    for (int i = 0; i < n; i++) {
        ofs >> k >> value;
        reclass_map[k] = value;
    }
    return true;
}

int main(int argc, char* argv[]) {
    SetDefaultOpenMPThread();
    GDALAllRegister();

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
    }
    char* config_file = argv[1];

    // read input information
    string type_file, lookup_folder, output_folder, tmp;
    int n = 0;
    int default_type = -1;
    vector<string> attr_names;
    // read attribute list
    std::ifstream ifs(config_file);
    ifs >> type_file >> default_type >> lookup_folder >> output_folder >> n;
    for (int i = 0; i < n; ++i) {
        ifs >> tmp;
        attr_names.push_back(tmp);
    }
    ifs.close();

    int n_len = CVT_INT(lookup_folder.length());
    if (lookup_folder.substr(n_len - 1, 1) != SEP) {
        lookup_folder += SEP;
    }
    n_len = CVT_INT(output_folder.length());
    if (output_folder.substr(n_len - 1, 1) != SEP) {
        output_folder += SEP;
    }

    clsRasterData<float>* type_raster = clsRasterData<float>::Init(type_file);

    for (int i = 0; i < n; ++i) {
        string lookup_file = lookup_folder + attr_names[i] + ".txt";
        string output_file = output_folder + attr_names[i] + ".tif";
        clsRasterData<float>* output_layer = new clsRasterData<float>(type_raster);
        map<int, float> reclass_map;
        ReadReclassMap(lookup_file.c_str(), reclass_map);
        output_layer->Reclassify(reclass_map);
        output_layer->OutputToFile(output_file);
        delete output_layer;
    }
    // release memory
    delete type_raster;
    return 0;
}
