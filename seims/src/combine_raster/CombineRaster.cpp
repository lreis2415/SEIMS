#include "CombineRaster.h"

#include <cfloat>

#include "utils_array.h"
#include "utils_string.h"

using namespace utils_array;
using namespace utils_string;

FloatRaster* CombineRasters(map<int, FloatRaster *>& all_raster_data) {
    if (all_raster_data.empty()) return nullptr;
    double x_min = FLT_MAX;
    double x_max = FLT_MIN;
    double y_min = FLT_MAX;
    double y_max = FLT_MIN;

    double xll = 0.f, yll = 0.f, dx = 0.f, xur = 0.f, yur = 0.f;
    int n_rows = 0, n_cols = 0;
    int n_subbasins = CVT_INT(all_raster_data.size());
    string srs;
    // loop to get global extent
    for (int i = 1; i <= n_subbasins; i++) {
        FloatRaster* rs = all_raster_data.at(i);
        n_rows = rs->GetRows();
        n_cols = rs->GetCols();
        dx = rs->GetCellWidth();
        xll = rs->GetXllCenter() - 0.5f * dx;
        yll = rs->GetYllCenter() - 0.5f * dx;
        xur = xll + n_cols * dx;
        yur = yll + n_rows * dx;
        if (i == 1) {
            srs = rs->GetSrsString();
            x_min = xll;
            y_min = yll;
            x_max = xur;
            y_max = yur;
            continue;
        }
        x_min = xll < x_min ? xll : x_min;
        x_max = xur > x_max ? xur : x_max;
        y_min = yll < y_min ? yll : y_min;
        y_max = yur > y_max ? yur : y_max;
    }

    int n_cols_total = CVT_INT(ceil((x_max - x_min) / dx)); // int((x_max - x_min) / dx + 0.5)
    int n_rows_total = CVT_INT(ceil((y_max - y_min) / dx)); // int((x_max - x_min) / dx + 0.5)

    int n_total = n_rows_total * n_cols_total;
    // single layer or multi-layers raster data
    int nlayers = all_raster_data.at(1)->GetLayers();
    float* data = nullptr;
    float** data2d = nullptr;
    if (nlayers > 1) {
        Initialize2DArray(n_total, nlayers, data2d, NODATA_VALUE);
    } else {
        Initialize1DArray(n_total, data, NODATA_VALUE);
    }

    // loop to put data in the array
    for (int i = 1; i <= n_subbasins; i++) {
        FloatRaster* rs = all_raster_data.at(i);
        n_rows = rs->GetRows();
        dx = rs->GetCellWidth();
        xll = rs->GetXllCenter() - 0.5f * dx;
        yll = rs->GetYllCenter() - 0.5f * dx;
        yur = yll + n_rows * dx;

        int cell_nums = 0;
        int** valid_position = nullptr;
        rs->GetRasterPositionData(&cell_nums, &valid_position);
        for (int idx = 0; idx < cell_nums; idx++) {
            int k = valid_position[idx][0];
            int m = valid_position[idx][1];
            int wi = CVT_INT((y_max - yur + (k + 0.5) * dx) / dx); /// row in whole extent
            int wj = CVT_INT((xll + (m + 0.5) * dx - x_min) / dx); /// col in whole extent

            int index = wi * n_cols_total + wj; /// index in whole extent
            if (nlayers > 1) {
                for (int li = 1; li <= nlayers; li++) {
                    data2d[index][li - 1] = rs->GetValueByIndex(idx, li);
                }
            } else {
                data[index] = rs->GetValueByIndex(idx);
            }
        }
    }
    if (nlayers > 1) {
        return new FloatRaster(data2d, n_cols_total, n_rows_total, nlayers, NODATA_VALUE, dx,
                               x_min + 0.5 * dx, y_min + 0.5 * dx, srs);
    }
    return new FloatRaster(data, n_cols_total, n_rows_total, NODATA_VALUE, dx,
                           x_min + 0.5 * dx, y_min + 0.5 * dx, srs);
}

void CombineRasterResults(const string& folder, const string& s_var,
                          const string& file_type, const int n_subbasins) {
    map<int, FloatRaster *> all_raster_data;
    string filename;
    if (file_type.find('.') == string::npos) {
        filename = s_var + "." + file_type;
    } else {
        filename = s_var + file_type;
    }
    for (int i = 1; i <= n_subbasins; i++) {
        string cur_file_name = folder + SEP + ValueToString(i) + "_" + filename;
        FloatRaster* rs = FloatRaster::Init(cur_file_name, true);
        if (nullptr == rs) {
            exit(-1);
        }
        all_raster_data.insert(make_pair(i, rs));
    }
    FloatRaster* combined_rs = CombineRasters(all_raster_data);
    combined_rs->OutputToFile(folder + SEP + filename);
    // clean up
    delete combined_rs;
    for (auto it = all_raster_data.begin(); it != all_raster_data.end();) {
        if (it->second != nullptr) {
            delete it->second;
            it->second = nullptr;
        }
        all_raster_data.erase(it++);
    }
}

#ifdef USE_MONGODB
void CombineRasterResultsMongo(MongoGridFs* gfs, const string& s_var,
                               const int n_subbasins, const string& folder /* = "" */) {
    map<int, FloatRaster *> all_raster_data;
    for (int i = 1; i <= n_subbasins; i++) {
        string cur_file_name = ValueToString(i) + "_" + s_var;
        FloatRaster* rs = FloatRaster::Init(gfs, cur_file_name.c_str(), true);
        if (nullptr == rs) {
            exit(-1);
        }
        all_raster_data.insert(make_pair(i, rs));
    }
    FloatRaster* combined_rs = CombineRasters(all_raster_data);
    gfs->RemoveFile(s_var);
    combined_rs->OutputToMongoDB(s_var, gfs);
    if (!folder.empty()) {
        combined_rs->OutputToFile(folder + SEP + s_var + "." + GTiffExtension);
    }
    // clean up
    delete combined_rs;
    for (auto it = all_raster_data.begin(); it != all_raster_data.end();) {
        if (it->second != nullptr) {
            delete it->second;
            it->second = nullptr;
        }
        all_raster_data.erase(it++);
    }
}
#endif /* USE_MONGODB */
