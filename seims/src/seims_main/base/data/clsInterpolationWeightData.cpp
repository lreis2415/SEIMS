#include "clsInterpolationWeightData.h"

using namespace std;

clsITPWeightData::clsITPWeightData(MongoGridFS *gfs, const char *remoteFilename) :
    m_fileName(remoteFilename), m_weightData(nullptr) {
    ReadFromMongoDB(gfs, remoteFilename);
}

clsITPWeightData::~clsITPWeightData() {
    if (nullptr != m_weightData) {
        Release1DArray(m_weightData);
    }
}

void clsITPWeightData::getWeightData(int *n, float **data) {
    *n = m_nRows;
    *data = m_weightData;
}

void clsITPWeightData::dump(ostream *fs) {
    if (fs == nullptr) return;

    int index = 0;
    for (int i = 0; i < m_nRows; ++i) {
        //rasterFile >> tmp >> tmp;
        for (int j = 0; j < m_nCols; ++j) {
            index = i * m_nCols + j;
            *fs << m_weightData[index] << "\t";
        }
        *fs << endl;
    }
}

void clsITPWeightData::dump(string fileName) {
    ofstream fs;
    fs.open(fileName.c_str(), ios::out);
    if (fs.is_open()) {
        dump(&fs);
        fs.close();
    }
}

void clsITPWeightData::ReadFromMongoDB(MongoGridFS *gfs, const char *remoteFilename) {
    string wfilename = string(remoteFilename);
    vector<string> gfilenames;
    gfs->getFileNames(gfilenames);
    string filename = string(remoteFilename);
    if (!ValueInVector(filename, gfilenames)) {
        size_t index = filename.find_last_of('_');
        string type = filename.substr(index + 1);
        if (StringMatch(type, DataType_PotentialEvapotranspiration) || StringMatch(type, DataType_SolarRadiation)
            || StringMatch(type, DataType_RelativeAirMoisture) || StringMatch(type, DataType_MeanTemperature)
            || StringMatch(type, DataType_MaximumTemperature) || StringMatch(type, DataType_MinimumTemperature)) {
            wfilename = filename.substr(0, index + 1) + DataType_Meteorology;
        }
    }
    char *databuf;
    size_t datalength;
    gfs->getStreamData(wfilename, databuf, datalength);
    m_weightData = (float *) databuf;
    /// Get metadata
    bson_t *md = gfs->getFileMetadata(wfilename);
    /// Get value of given keys
    GetNumericFromBson(md, MONG_GRIDFS_WEIGHT_CELLS, m_nRows);
    GetNumericFromBson(md, MONG_GRIDFS_WEIGHT_SITES, m_nCols);
}
