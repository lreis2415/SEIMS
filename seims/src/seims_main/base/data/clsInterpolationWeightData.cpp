#include "clsInterpolationWeightData.h"

using namespace std;

clsInterpolationWeightData::clsInterpolationWeightData(string weightFileName) {
    m_fileName = weightFileName;
    m_weightData = NULL;
}

clsInterpolationWeightData::clsInterpolationWeightData(mongoc_gridfs_t *gfs, const char *remoteFilename) {
    m_fileName = remoteFilename;
    m_weightData = NULL;
    ReadFromMongoDB(gfs, remoteFilename);
}

clsInterpolationWeightData::~clsInterpolationWeightData(void) {
    if (m_weightData != NULL) {
        delete[] m_weightData;
    }
}

void clsInterpolationWeightData::getWeightData(int *n, float **data) {
    *n = m_nRows;
    *data = m_weightData;
}

void clsInterpolationWeightData::dump(ostream *fs) {
    if (fs == NULL) return;

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

void clsInterpolationWeightData::dump(string fileName) {
    ofstream fs;
    fs.open(fileName.c_str(), ios::out);
    if (fs.is_open()) {
        dump(&fs);
        fs.close();
    }
}

void clsInterpolationWeightData::ReadFromMongoDB(mongoc_gridfs_t *gfs, const char *remoteFilename) {
    string wfilename = string(remoteFilename);
    MongoGridFS mongogfs = MongoGridFS();
    vector <string> gfilenames = mongogfs.getFileNames(gfs);
    string filename = remoteFilename;
    if (!ValueInVector(filename, gfilenames)) {
        int index = filename.find_last_of('_');
        string type = filename.substr(index + 1);
        if (StringMatch(type, DataType_PotentialEvapotranspiration) || StringMatch(type, DataType_SolarRadiation)
            || StringMatch(type, DataType_RelativeAirMoisture) || StringMatch(type, DataType_MeanTemperature)
            || StringMatch(type, DataType_MaximumTemperature) || StringMatch(type, DataType_MinimumTemperature)) {
            wfilename = filename.substr(0, index + 1) + DataType_Meteorology;
        }
    }
    char *databuf;
    int datalength;
    mongogfs.getStreamData(wfilename, databuf, datalength, gfs);
    mongoc_gridfs_file_t *gfile = NULL;
    bson_error_t *err = NULL;
    gfile = mongoc_gridfs_find_one_by_filename(gfs, remoteFilename, err);
    m_weightData = (float *) databuf;
    /// Get metadata
    bson_t *md = mongogfs.getFileMetadata(wfilename, gfs);
    /// Get value of given keys
    GetNumericFromBson(md, MONG_GRIDFS_WEIGHT_CELLS, m_nRows);
    GetNumericFromBson(md, MONG_GRIDFS_WEIGHT_SITES, m_nCols);
}
