#include "clsInterpolationWeightData.h"

#include <fstream>

#include "utils_array.h"
#include "utils_string.h"
#include "text.h"

using namespace utils_array;
using namespace utils_string;

ItpWeightData::ItpWeightData(MongoGridFs* gfs, const string& filename) :
filename_(filename), itp_weight_data_(nullptr), n_rows_(-1), n_cols_(-1), initialized_(false) {
    initialized_ = ReadFromMongoDB(gfs, filename_);
}

ItpWeightData::~ItpWeightData() {
    if (nullptr != itp_weight_data_) { Release1DArray(itp_weight_data_); }
}

void ItpWeightData::GetWeightData(int* n, float** data) {
    *n = n_rows_;
    *data = itp_weight_data_;
}

void ItpWeightData::Dump(std::ostream* fs) {
    if (fs == nullptr) return;
    int index = 0;
    for (int i = 0; i < n_rows_; i++) {
        for (int j = 0; j < n_cols_; j++) {
            index = i * n_cols_ + j;
            *fs << itp_weight_data_[index] << "\t";
        }
        *fs << endl;
    }
}

void ItpWeightData::Dump(const string& filename) {
    std::ofstream fs;
    fs.open(filename.c_str(), std::ios::out);
    if (fs.is_open()) {
        Dump(&fs);
        fs.close();
    }
}

bool ItpWeightData::ReadFromMongoDB(MongoGridFs* gfs, const string& filename) {
    string wfilename = filename;
    vector<string> gfilenames;
    gfs->GetFileNames(gfilenames);
    if (!ValueInVector(filename, gfilenames)) {
        size_t index = filename.find_last_of('_');
        string type = filename.substr(index + 1);
        if (StringMatch(type, DataType_PotentialEvapotranspiration) || StringMatch(type, DataType_SolarRadiation)
            || StringMatch(type, DataType_RelativeAirMoisture) || StringMatch(type, DataType_MeanTemperature)
            || StringMatch(type, DataType_MaximumTemperature) || StringMatch(type, DataType_MinimumTemperature)) {
            wfilename = filename.substr(0, index + 1) + DataType_Meteorology;
        }
    }
    char* databuf = nullptr;
    size_t datalength;
    gfs->GetStreamData(wfilename, databuf, datalength);
    if (nullptr == databuf) return false;

    itp_weight_data_ = reinterpret_cast<float *>(databuf); // deprecate C-style: (float *) databuf
    /// Get metadata
    bson_t* md = gfs->GetFileMetadata(wfilename);
    /// Get value of given keys
    GetNumericFromBson(md, MONG_GRIDFS_WEIGHT_CELLS, n_rows_);
    GetNumericFromBson(md, MONG_GRIDFS_WEIGHT_SITES, n_cols_);
    return true;
}
