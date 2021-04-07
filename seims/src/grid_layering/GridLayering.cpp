#include "GridLayering.h"
#ifdef SUPPORT_OMP
#include "omp.h"
#endif

int find_flow_direction_index_ccw(const int fd) {
    for (int i = 1; i <= 8; i++) {
        if (fdccw[i] == fd) return i;
    }
    return -1;
}

int get_reversed_fdir(const int fd) {
    int fd_idx = find_flow_direction_index_ccw(fd);
    if (fd_idx < 0) return -1;
    int reversed_fd_idx = fd_idx > 4 ? fd_idx - 4 : fd_idx + 4;
    return fdccw[reversed_fd_idx];
}

vector<int> uncompress_flow_directions(const int compressed_fd) {
    vector<int> flow_dirs; // record multiple flow directions by counter-clockwise
    if (compressed_fd & 1) {
        flow_dirs.emplace_back(1);
    }
    if (compressed_fd & 128) {
        flow_dirs.emplace_back(128);
    }
    if (compressed_fd & 64) {
        flow_dirs.emplace_back(64);
    }
    if (compressed_fd & 32) {
        flow_dirs.emplace_back(32);
    }
    if (compressed_fd & 16) {
        flow_dirs.emplace_back(16);
    }
    if (compressed_fd & 8) {
        flow_dirs.emplace_back(8);
    }
    if (compressed_fd & 4) {
        flow_dirs.emplace_back(4);
    }
    if (compressed_fd & 2) {
        flow_dirs.emplace_back(2);
    }
    if (flow_dirs.size() >= 2 && flow_dirs[0] == 1 && flow_dirs.back() == 2) {
        flow_dirs[0] = 2;
        flow_dirs.back() = 1;
    }
    return flow_dirs;
}

#ifdef USE_MONGODB
GridLayering::GridLayering(const int id, MongoGridFs* gfs, const char* out_dir) :
    gfs_(gfs), use_mongo_(true), has_mask_(false), fdtype_(FD_D8), output_dir_(out_dir), subbasin_id_(id),
    n_rows_(-1), n_cols_(-1), out_nodata_(-9999.f),
    n_valid_cells_(-1), pos_index_(nullptr), pos_rowcol_(nullptr),
    mask_(nullptr), flowdir_(nullptr), flowdir_matrix_(nullptr), reverse_dir_(nullptr),
    flow_in_num_(nullptr), flow_in_count_(0), flow_in_cells_(nullptr),
    flow_out_num_(nullptr), flow_out_count_(0), flow_out_cells_(nullptr),
    layers_updown_(nullptr), layers_downup_(nullptr),
    layer_cells_updown_(nullptr), layer_cells_downup_(nullptr) {
}
#endif

GridLayering::GridLayering(const int id, const char* out_dir):
    gfs_(nullptr), use_mongo_(false), has_mask_(false), fdtype_(FD_D8), output_dir_(out_dir), subbasin_id_(id),
    n_rows_(-1), n_cols_(-1), out_nodata_(-9999.f),
    n_valid_cells_(-1), pos_index_(nullptr), pos_rowcol_(nullptr),
    mask_(nullptr), flowdir_(nullptr), flowdir_matrix_(nullptr), reverse_dir_(nullptr),
    flow_in_num_(nullptr), flow_in_count_(0), flow_in_cells_(nullptr),
    flow_out_num_(nullptr), flow_out_count_(0), flow_out_cells_(nullptr),
    layers_updown_(nullptr), layers_downup_(nullptr),
    layer_cells_updown_(nullptr), layer_cells_downup_(nullptr) {
}

GridLayering::~GridLayering() {
    delete flowdir_; // flowdir_matrix_ will be released too.
    if (has_mask_) delete mask_;
    if (nullptr != pos_index_) Release1DArray(pos_index_);
    if (nullptr != reverse_dir_) Release1DArray(reverse_dir_);
    if (nullptr != flow_in_num_) Release1DArray(flow_in_num_);
    if (nullptr != flow_in_cells_) Release1DArray(flow_in_cells_);
    if (nullptr != flow_out_num_) Release1DArray(flow_out_num_);
    if (nullptr != flow_out_cells_) Release1DArray(flow_out_cells_);
    if (nullptr != layers_updown_) Release1DArray(layers_updown_);
    if (nullptr != layers_downup_) Release1DArray(layers_downup_);
    if (nullptr != layer_cells_updown_) Release1DArray(layer_cells_updown_);
    if (nullptr != layer_cells_downup_) Release1DArray(layer_cells_downup_);
}

bool GridLayering::Execute() {
    if (!LoadData()) return false;
    CalPositionIndex();
    return OutputFlowOut() && OutputFlowIn() && GridLayeringFromSource() && GridLayeringFromOutlet();
}

void GridLayering::CalPositionIndex() {
    if (n_valid_cells_ > 0 && nullptr != pos_index_) return;
    Initialize1DArray(n_rows_ * n_cols_, pos_index_, -1);
    for (int i = 0; i < n_valid_cells_; i++) {
        pos_index_[pos_rowcol_[i][0] * n_cols_ + pos_rowcol_[i][1]] = i;
    }
}

void GridLayering::GetReverseDirMatrix() {
    if (nullptr == reverse_dir_) Initialize1DArray(n_valid_cells_, reverse_dir_, 0);
    for (int valid_idx = 0; valid_idx < n_valid_cells_; valid_idx++) {
        int i = pos_rowcol_[valid_idx][0]; // row
        int j = pos_rowcol_[valid_idx][1]; // col
        int flow_dir = CVT_INT(flowdir_matrix_[valid_idx]);
        if (flowdir_->IsNoData(i, j) || flow_dir < 0) {
            reverse_dir_[valid_idx] = CVT_INT(out_nodata_);
            continue;
        }
        vector<int> flow_dirs = uncompress_flow_directions(flow_dir);
        for (vector<int>::iterator it = flow_dirs.begin(); it != flow_dirs.end(); ++it) {
            int fd_idx = find_flow_direction_index_ccw(*it);
            int dst_row = i + drow[fd_idx];
            int dst_col = j + dcol[fd_idx];
            if (!mask_->ValidateRowCol(dst_row, dst_col) ||
                mask_->IsNoData(dst_row, dst_col)) {
                continue;
            }
            int dst_idx = pos_index_[dst_row * n_cols_ + dst_col];
            if (FloatEqual(reverse_dir_[dst_idx], out_nodata_) || reverse_dir_[dst_idx] < 0) {
                // Boundary cells may be NoData in flowdir_, but they can accept flow in
                reverse_dir_[dst_idx] = 0;
            }
            reverse_dir_[dst_idx] += get_reversed_fdir(fdccw[fd_idx]);
        }
    }
}

void GridLayering::CountFlowInCells() {
    if (nullptr == flow_in_num_) Initialize1DArray(n_valid_cells_, flow_in_num_, 0);
#pragma omp parallel for
    for (int i = 0; i < n_valid_cells_; i++) {
        int reverse_fdir = CVT_INT(reverse_dir_[i]);
        if (reverse_fdir < 0) {
            continue;
        }
        vector<int> reverse_fdirs = uncompress_flow_directions(reverse_fdir);
        flow_in_num_[i] = CVT_INT(reverse_fdirs.size());
    }
    int total = 0;
#pragma omp parallel for reduction(+:total)
    for (int index = 0; index < n_valid_cells_; index++) {
        total += flow_in_num_[index];
    }
    flow_in_count_ = total;
}

int GridLayering::BuildMultiFlowOutArray(float*& compressed_dir,
                                         int*& connect_count, float*& p_output) {
    p_output[0] = CVT_FLT(n_valid_cells_);
    int counter = 1;
    for (int valid_idx = 0; valid_idx < n_valid_cells_; valid_idx++) {
        int i = pos_rowcol_[valid_idx][0]; // row
        int j = pos_rowcol_[valid_idx][1]; // col
        /// flow out cell's number
        p_output[counter++] = CVT_FLT(connect_count[valid_idx]); // maybe 0
        if (connect_count[valid_idx] == 0) continue;
        /// accumulated flow in directions
        int acc_flowin_dir = CVT_INT(compressed_dir[valid_idx]);
        vector<int> flow_dirs = uncompress_flow_directions(acc_flowin_dir);
        for (vector<int>::iterator it = flow_dirs.begin(); it != flow_dirs.end(); ++it) {
            int fd_idx = find_flow_direction_index_ccw(*it);
            if (!mask_->ValidateRowCol(i + drow[fd_idx], j + dcol[fd_idx]) ||
                mask_->IsNoData(i + drow[fd_idx], j + dcol[fd_idx])) {
                continue;
            }
            p_output[counter++] = CVT_FLT(pos_index_[(i + drow[fd_idx]) * n_cols_ + j + dcol[fd_idx]]);
        }
    }
    return counter;
}

bool GridLayering::BuildFlowInCellsArray() {
    int n_output = flow_in_count_ + n_valid_cells_ + 1;
    if (nullptr == flow_in_cells_) Initialize1DArray(n_output, flow_in_cells_, 0.f);
    int n_output2 = BuildMultiFlowOutArray(reverse_dir_, flow_in_num_, flow_in_cells_);
    if (n_output2 != n_output) {
        cout << "BuildFlowInCellsArray failed!" << endl;
        return false;
    }
    return true;
}

void GridLayering::CountFlowOutCells() {
    if (nullptr == flow_out_num_) Initialize1DArray(n_valid_cells_, flow_out_num_, 0);
#pragma omp parallel for
    for (int index = 0; index < n_valid_cells_; index++) {
        int i = pos_rowcol_[index][0]; // row
        int j = pos_rowcol_[index][1]; // col
        if (flowdir_->IsNoData(i, j) || flowdir_matrix_[index] < 0) continue;

        int flow_dir = CVT_INT(flowdir_matrix_[index]);
        vector<int> flow_dirs = uncompress_flow_directions(flow_dir);
        for (vector<int>::iterator it = flow_dirs.begin(); it != flow_dirs.end(); ++it) {
            int fd_idx = find_flow_direction_index_ccw(*it);
            if (mask_->ValidateRowCol(i + drow[fd_idx], j + dcol[fd_idx]) &&
                !mask_->IsNoData(i + drow[fd_idx], j + dcol[fd_idx])) {
                flow_out_num_[index]++;
            }
        }
    }
    int total = 0;
#pragma omp parallel for reduction(+:total)
    for (int index = 0; index < n_valid_cells_; index++) {
        total += flow_out_num_[index];
    }
    flow_out_count_ = total;
}

bool GridLayering::BuildFlowOutCellsArray() {
    int n_output = flow_out_count_ + n_valid_cells_ + 1;
    if (nullptr == flow_out_cells_) Initialize1DArray(n_output, flow_out_cells_, 0.f);
    int n_output2 = BuildMultiFlowOutArray(flowdir_matrix_, flow_out_num_, flow_out_cells_);
    if (n_output2 != n_output) {
        cout << "BuildFlowOutCellsArray failed!" << endl;
        return false;
    }
    return true;
}

bool GridLayering::Output2DimensionArrayTxt(const string& name, string& header,
                                            float* const matrix, float* matrix2/* = nullptr */) {
    string outpath = string(output_dir_) + SEP + name + ".txt";
    std::ofstream ofs(outpath.c_str());
    ofs << matrix[0] << endl;
    ofs << header << endl;
    int tmp_count = 1;
    int tmp_count2 = 1;
    for (int i = 0; i < CVT_INT(matrix[0]); i++) {
        int count = CVT_INT(matrix[tmp_count++]);
        ofs << i << "\t" << count << "\t";
        for (int j = 0; j < count; j++) {
            if (j == count - 1)
                ofs << matrix[tmp_count++];
            else
                ofs << matrix[tmp_count++] << ",";
        }
        if (nullptr != matrix2) {
            ofs << "\t";
            tmp_count2 = tmp_count - count;
            for (int k = 0; k < count; k++) {
                if (k == count - 1)
                    ofs << matrix2[tmp_count2++];
                else
                    ofs << matrix2[tmp_count2++] << ",";
            }
        }
        ofs << "\n"; // std::endl vs \n? https://stackoverflow.com/a/213977/4837280
    }
    ofs.close();
    return true;
}

#ifdef USE_MONGODB
bool GridLayering::OutputArrayAsGfs(const string& name, float* const matrix) {
    bool flag = false;
    int max_loop = 3;
    int cur_loop = 1;
    while (cur_loop < max_loop) {
        if (!OutputToMongodb(name.c_str(), CVT_INT(matrix[0]), reinterpret_cast<char*>(matrix))) {
            cur_loop++;
        } else {
            cout << "Output " << name << " done!" << endl;
            flag = true;
            break;
        }
    }
    return flag;
}
#endif

bool GridLayering::OutputFlowIn() {
    GetReverseDirMatrix();
    CountFlowInCells();
    if (!BuildFlowInCellsArray()) return false;
    string header = "ID\tUpstreamCount\tUpstreamID";
    bool done = Output2DimensionArrayTxt(flowin_index_name_, header, flow_in_cells_);
    if (use_mongo_) {
#ifdef USE_MONGODB
        done = done && OutputArrayAsGfs(flowin_index_name_, flow_in_cells_);
#endif
    }
    return done;
}


bool GridLayering::OutputFlowOut() {
    CountFlowOutCells();
    if (!BuildFlowOutCellsArray()) return false;
    string header = "ID\tDownstreamCount\tDownstreamID";
    bool done = Output2DimensionArrayTxt(flowout_index_name_, header, flow_out_cells_);
    if (use_mongo_) {
#ifdef USE_MONGODB
        done = OutputArrayAsGfs(flowout_index_name_, flow_out_cells_);
#endif
    }
    return done;
}

bool GridLayering::GridLayeringFromSource() {
    Initialize1DArray(n_valid_cells_, layers_updown_, out_nodata_);

    int* last_layer = nullptr; // indexes of cells in last layer, the length is cell numbers of last layer
    Initialize1DArray(n_valid_cells_, last_layer, out_nodata_);
    int num_last_layer = 0; // number of cells in last layer

    vector<vector<int> > layer_cells_updown; // layer number - indexes of cells

    // 1. find source grids, i.e., the first layer
    for (int i = 0; i < n_valid_cells_; i++) {
        if (flow_in_num_[i] == 0 && flowdir_matrix_[i] > 0) {
            last_layer[num_last_layer++] = i;
        }
    }
    // 2. loop and layer
    int num_next_layer = 0;
    int cur_num = 0; //the layering number of the current layer. In the end, it is the total number of layers.
    int* next_layer = nullptr;
    Initialize1DArray(n_valid_cells_, next_layer, out_nodata_);
    int* tmp = nullptr;
    int valid_idx = 0;
    vector<int> lyr_cells;
    while (num_last_layer > 0) {
        cur_num++;
        num_next_layer = 0;
        for (int i_in_layer = 0; i_in_layer < num_last_layer; i_in_layer++) {
            valid_idx = last_layer[i_in_layer];
            lyr_cells.emplace_back(valid_idx);
            layers_updown_[valid_idx] = cur_num;

            int i = pos_rowcol_[valid_idx][0]; // row
            int j = pos_rowcol_[valid_idx][1]; // col

            int dir = CVT_INT(flowdir_matrix_[valid_idx]);
            vector<int> dirs = uncompress_flow_directions(dir);
            for (vector<int>::iterator it = dirs.begin(); it != dirs.end(); ++it) {
                int fd_idx = find_flow_direction_index_ccw(*it);
                int dst_row = i + drow[fd_idx];
                int dst_col = j + dcol[fd_idx];
                if (!mask_->ValidateRowCol(dst_row, dst_col) ||
                    mask_->IsNoData(dst_row, dst_col))
                    continue;
                int dst_index = dst_row * n_cols_ + dst_col;
                if (--flow_in_num_[pos_index_[dst_index]] == 0) {
                    next_layer[num_next_layer++] = pos_index_[dst_index];
                }
            }
        }
        vector<int>(lyr_cells).swap(lyr_cells);
        layer_cells_updown.emplace_back(vector<int>(lyr_cells));
        lyr_cells.clear();

        num_last_layer = num_next_layer;
        tmp = last_layer;
        last_layer = next_layer;
        next_layer = tmp;
    }
    Release1DArray(last_layer);
    Release1DArray(next_layer);

    int layer_num = CVT_INT(layer_cells_updown.size());
    int length = n_valid_cells_ + layer_num + 1;
    Initialize1DArray(length, layer_cells_updown_, 0.f);
    layer_cells_updown_[0] = CVT_FLT(layer_num);
    valid_idx = 1;
    for (auto it = layer_cells_updown.begin(); it != layer_cells_updown.end(); ++it) {
        layer_cells_updown_[valid_idx++] = CVT_FLT((*it).size());
        for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
            layer_cells_updown_[valid_idx++] = CVT_FLT(*it2);
        }
    }
    return OutputGridLayering(layering_updown_name_, length,
                              layers_updown_, layer_cells_updown_);
}

bool GridLayering::GridLayeringFromOutlet() {
    Initialize1DArray(n_valid_cells_, layers_downup_, out_nodata_);

    int* last_layer = nullptr; // indexes of cells in last layer, the length is cell numbers of last layer
    Initialize1DArray(n_valid_cells_, last_layer, out_nodata_);
    int num_last_layer = 0; // number of cells in last layer

    vector<vector<int> > layer_cells_downup; // layer number - indexes of cells

    // 1. find outlet grids
    for (int i = 0; i < n_valid_cells_; i++) {
        int irow = pos_rowcol_[i][0]; // row
        int icol = pos_rowcol_[i][1]; // col
        if (!flowdir_->IsNoData(irow, icol) && flow_out_num_[i] == 0) {
            last_layer[num_last_layer++] = i;
        }
    }
    // 2. loop and layer
    int num_next_layer = 0;
    int* next_layer = nullptr;
    Initialize1DArray(n_valid_cells_, next_layer, out_nodata_);
    int* tmp = nullptr;
    int cur_num = 0;
    vector<int> lyr_cells;
    while (num_last_layer > 0) {
        cur_num++;
        num_next_layer = 0;
        for (int i_in_layer = 0; i_in_layer < num_last_layer; i_in_layer++) {
            int valid_idx = last_layer[i_in_layer];
            lyr_cells.emplace_back(valid_idx);
            layers_downup_[valid_idx] = cur_num;

            int jrow = pos_rowcol_[valid_idx][0]; // row
            int jcol = pos_rowcol_[valid_idx][1]; // col

            for (int ccwidx = 1; ccwidx <= 8; ccwidx++) {
                int src_row = jrow + drow[ccwidx];
                int src_col = jcol + dcol[ccwidx];
                if (!mask_->ValidateRowCol(src_row, src_col) ||
                    mask_->IsNoData(src_row, src_col))
                    continue;
                int src_idx = src_row * n_cols_ + src_col;
                if (!(CVT_INT(flowdir_matrix_[pos_index_[src_idx]]) & get_reversed_fdir(fdccw[ccwidx])))
                    continue;
                if (--flow_out_num_[pos_index_[src_idx]] == 0) {
                    next_layer[num_next_layer++] = pos_index_[src_idx];
                }
            }
        }
        vector<int>(lyr_cells).swap(lyr_cells);
        layer_cells_downup.emplace_back(vector<int>(lyr_cells));
        lyr_cells.clear();

        num_last_layer = num_next_layer;
        tmp = last_layer;
        last_layer = next_layer;
        next_layer = tmp;
    }

    // 3. reverse the layer number
#pragma omp parallel for
    for (int i = 0; i < n_valid_cells_; i++) {
        if (!FloatEqual(layers_downup_[i], out_nodata_)) {
            layers_downup_[i] = cur_num - layers_downup_[i] + 1;
        }
    }
    Release1DArray(last_layer);
    Release1DArray(next_layer);

    int layer_num = CVT_INT(layer_cells_downup.size());
    int length = n_valid_cells_ + layer_num + 1;
    Initialize1DArray(length, layer_cells_downup_, 0.f);
    layer_cells_downup_[0] = CVT_FLT(layer_num);
    int index = 1;
    for (auto it = layer_cells_downup.rbegin(); 
         it != layer_cells_downup.rend(); ++it) {
        layer_cells_downup_[index++] = CVT_FLT((*it).size());
        for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
            layer_cells_downup_[index++] = CVT_FLT(*it2);
        }
    }
    return OutputGridLayering(layering_downup_name_, length,
                              layers_downup_, layer_cells_downup_);
}

#ifdef USE_MONGODB
bool GridLayering::OutputToMongodb(const char* name, const int number, char* s) {
    bson_t p = BSON_INITIALIZER;
    BSON_APPEND_INT32(&p, "SUBBASIN", subbasin_id_);
    BSON_APPEND_UTF8(&p, "TYPE", name);
    BSON_APPEND_UTF8(&p, "ID", name);
    BSON_APPEND_UTF8(&p, "DESCRIPTION", name);
    BSON_APPEND_DOUBLE(&p, "NUMBER", number);

    gfs_->RemoveFile(string(name));
    size_t n = number * sizeof(float);
    gfs_->WriteStreamData(string(name), s, n, &p);
    bson_destroy(&p);
    if (NULL == gfs_->GetFile(name)) {
        return false;
    }
    return true;
}
#endif

bool GridLayering::OutputGridLayering(const string& name, const int datalength,
                                      float* const layer_grid, float* const layer_cells) {
    string outpath = string(output_dir_) + "/" + name + ".tif";
    FloatRaster(mask_, layer_grid).OutputFileByGdal(outpath);

    string header = "LayerID\tCellCount\tCellIDs";
    bool done = Output2DimensionArrayTxt(name, header, layer_cells);
    if (use_mongo_) {
#ifdef USE_MONGODB
        done = done && OutputArrayAsGfs(name, layer_cells);
    }
#endif
    return done;
}
