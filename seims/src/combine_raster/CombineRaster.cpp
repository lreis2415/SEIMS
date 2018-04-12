#include "CombineRaster.h"

FloatRaster* CombineRasters(map<int, FloatRaster *>& allRasterData) {
    if (allRasterData.empty()) return nullptr;
    double xMin = FLT_MAX;
    double xMax = FLT_MIN;
    double yMin = FLT_MAX;
    double yMax = FLT_MIN;

    double xll = 0.f, yll = 0.f, dx = 0.f, xur = 0.f, yur = 0.f;
    int nRows = 0, nCols = 0;
    int nSubbasins = allRasterData.size();
    string srs;
    // loop to get global extent
    for (int i = 1; i <= nSubbasins; i++) {
        FloatRaster* rs = allRasterData.at(i);
        nRows = rs->getRows();
        nCols = rs->getCols();
        dx = rs->getCellWidth();
        xll = rs->getXllCenter() - 0.5f * dx;
        yll = rs->getYllCenter() - 0.5f * dx;
        xur = xll + nCols * dx;
        yur = yll + nRows * dx;
        if (i == 1) {
            srs = rs->getSRSString();
            xMin = xll;
            yMin = yll;
            xMax = xur;
            yMax = yur;
            continue;
        }
        xMin = (xll < xMin) ? xll : xMin;
        xMax = (xur > xMax) ? xur : xMax;
        yMin = (yll < yMin) ? yll : yMin;
        yMax = (yur > yMax) ? yur : yMax;
    }

    int nColsTotal = int((xMax - xMin) / dx + 0.5);
    int nRowsTotal = int((yMax - yMin) / dx + 0.5);

    int nTotal = nRowsTotal * nColsTotal;
    // single layer or multi-layers raster data
    int nlayers = allRasterData.at(1)->getLayers();
    float* data = nullptr;
    float** data2d = nullptr;
    if (nlayers > 1) {
        Initialize2DArray(nTotal, nlayers, data2d, NODATA_VALUE);
    } else {
        Initialize1DArray(nTotal, data, NODATA_VALUE);
    }

    // loop to put data in the array
    for (int i = 1; i <= nSubbasins; i++) {
        FloatRaster* rs = allRasterData.at(i);
        nRows = rs->getRows();
        nCols = rs->getCols();
        dx = rs->getCellWidth();
        xll = rs->getXllCenter() - 0.5f * dx;
        yll = rs->getYllCenter() - 0.5f * dx;
        yur = yll + nRows * dx;

        int cellNums = 0;
        int** validPosition = nullptr;
        rs->getRasterPositionData(&cellNums, &validPosition);
        for (int idx = 0; idx < cellNums; idx++) {
            int k = validPosition[idx][0];
            int m = validPosition[idx][1];
            int wi = int((yMax - yur + (k + 0.5) * dx) / dx); /// row in whole extent
            int wj = int((xll + (m + 0.5) * dx - xMin) / dx); /// col in whole extent

            int index = wi * nColsTotal + wj; /// index in whole extent
            if (nlayers > 1) {
                for (int li = 1; li <= nlayers; li++) {
                    data2d[index][li - 1] = rs->getValueByIndex(idx, li);
                }
            } else {
                data[index] = rs->getValueByIndex(idx);
            }
        }
    }
    if (nlayers > 1) {
        return new FloatRaster(data2d, nColsTotal, nRowsTotal, nlayers, NODATA_VALUE, dx,
                               xMin + 0.5 * dx, yMin + 0.5 * dx, srs);
    } else {
        return new FloatRaster(data, nColsTotal, nRowsTotal, NODATA_VALUE, dx,
                               xMin + 0.5 * dx, yMin + 0.5 * dx, srs);
    }
}

void CombineRasterResults(const string& folder, const string& sVar, const string& fileType, int nSubbasins) {
    map<int, FloatRaster *> allRasterData;
    string filename = "";
    if (fileType.find('.') == string::npos) {
        filename = sVar + "." + fileType;
    } else {
        filename = sVar + fileType;
    }
    for (int i = 1; i <= nSubbasins; i++) {
        string curFileName = folder + SEP + ValueToString(i) + "_" + filename;
        FloatRaster* rs = FloatRaster::Init(curFileName, true);
        if (nullptr == rs) {
            exit(-1);
        }
        allRasterData.insert(make_pair(i, rs));
    }
    FloatRaster* combined_rs = CombineRasters(allRasterData);
    combined_rs->outputToFile(folder + SEP + filename);
    // clean up
    delete combined_rs;
    for (auto it = allRasterData.begin(); it != allRasterData.end();) {
        if (it->second != nullptr) {
            delete it->second;
            it->second = nullptr;
        }
        allRasterData.erase(it++);
    }
}

void CombineRasterResultsMongo(MongoGridFS* gfs, const string& sVar, int nSubbasins, const string& folder /* = "" */) {
    map<int, FloatRaster *> allRasterData;
    for (int i = 1; i <= nSubbasins; i++) {
        string curFileName = ValueToString(i) + "_" + sVar;
        FloatRaster* rs = FloatRaster::Init(gfs, curFileName.c_str(), true);
        if (nullptr == rs) {
            exit(-1);
        }
        allRasterData.insert(make_pair(i, rs));
    }
    FloatRaster* combined_rs = CombineRasters(allRasterData);
    gfs->removeFile(sVar);
    combined_rs->outputToMongoDB(sVar, gfs);
    if (folder != "") {
        combined_rs->outputToFile(folder + SEP + sVar + "." + GTiffExtension);
    }
    // clean up
    delete combined_rs;
    for (auto it = allRasterData.begin(); it != allRasterData.end();) {
        if (it->second != nullptr) {
            delete it->second;
            it->second = nullptr;
        }
        allRasterData.erase(it++);
    }
}
