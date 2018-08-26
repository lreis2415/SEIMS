#include "GridLayering.h"
#ifdef SUPPORT_OMP
#include "omp.h"
#endif

GridLayering::GridLayering(const int id, MongoGridFs* gfs, const char* out_dir) :
    gfs_(gfs), output_dir_(out_dir), subbasin_id_(id),
    n_rows_(-1), n_cols_(-1), dir_nodata_(-9999), out_nodata_(-9999),
    n_valid_cells_(-1), pos_index_(nullptr),
    flowdir_(nullptr), flowdir_matrix_(nullptr), reverse_dir_(nullptr),
    flow_in_num_(nullptr), flow_in_count_(0), flow_in_cells_(nullptr),
    flow_out_num_(nullptr), flow_out_count_(0), flow_out_cells_(nullptr),
    layers_updown_(nullptr), layers_downup_(nullptr),
    layer_cells_updown_(nullptr), layer_cells_downup_(nullptr),
    flowdir_name_(""), flowin_index_name_(""), flowout_index_name_(""),
    layering_updown_name_(""), layering_downup_name_("") {
}

GridLayering::~GridLayering() {
    delete flowdir_; // m_flowdirMatrix will be released too.
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
    n_valid_cells_ = 0;
    for (int i = 0; i < n_rows_ * n_cols_; i++) {
        if (FloatEqual(flowdir_matrix_[i], dir_nodata_)) continue;
        pos_index_[i] = n_valid_cells_++;
    }
}

void GridLayering::GetReverseDirMatrix() {
    if (nullptr == reverse_dir_) Initialize1DArray(n_rows_ * n_cols_, reverse_dir_, 0);
#pragma omp parallel for
    for (int i = 0; i < n_rows_; i++) {
        for (int j = 0; j < n_cols_; j++) {
            int index = i * n_cols_ + j;
            int flow_dir = flowdir_matrix_[index];
            if (flow_dir == dir_nodata_ || flow_dir < 0) {
                reverse_dir_[index] = dir_nodata_;
                continue;
            }

            if (flow_dir & 1 && j != n_cols_ - 1 && flowdir_matrix_[index + 1] != dir_nodata_
                && flowdir_matrix_[index + 1] > 0) {
                reverse_dir_[index + 1] += 16;
            }

            if (flow_dir & 2 && i != n_rows_ - 1 && j != n_cols_ - 1 &&
                flowdir_matrix_[index + n_cols_ + 1] != dir_nodata_ &&
                flowdir_matrix_[index + n_cols_ + 1] > 0) {
                reverse_dir_[index + n_cols_ + 1] += 32;
            }

            if (flow_dir & 4 && i != n_rows_ - 1 && flowdir_matrix_[index + n_cols_] != dir_nodata_ &&
                flowdir_matrix_[index + n_cols_] > 0) {
                reverse_dir_[index + n_cols_] += 64;
            }

            if (flow_dir & 8 && i != n_rows_ - 1 && j != 0 && flowdir_matrix_[index + n_cols_ - 1] != dir_nodata_ &&
                flowdir_matrix_[index + n_cols_ - 1] > 0) {
                reverse_dir_[index + n_cols_ - 1] += 128;
            }

            if (flow_dir & 16 && j != 0 && flowdir_matrix_[index - 1] != dir_nodata_
                && flowdir_matrix_[index - 1] > 0) {
                reverse_dir_[index - 1] += 1;
            }

            if (flow_dir & 32 && i != 0 && j != 0 && flowdir_matrix_[index - n_cols_ - 1] != dir_nodata_ &&
                flowdir_matrix_[index - n_cols_ - 1] > 0) {
                reverse_dir_[index - n_cols_ - 1] += 2;
            }

            if (flow_dir & 64 && i != 0 && flowdir_matrix_[index - n_cols_] != dir_nodata_
                && flowdir_matrix_[index - n_cols_] > 0) {
                reverse_dir_[index - n_cols_] += 4;
            }

            if (flow_dir & 128 && i != 0 && j != n_cols_ - 1 && flowdir_matrix_[index - n_cols_ + 1] != dir_nodata_ &&
                flowdir_matrix_[index - n_cols_ + 1] > 0) {
                reverse_dir_[index - n_cols_ + 1] += 8;
            }
        }
    }
}

void GridLayering::CountFlowInCells() {
    int n = n_rows_ * n_cols_;
    if (nullptr == flow_in_num_) Initialize1DArray(n, flow_in_num_, dir_nodata_);
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        int flow_dir = reverse_dir_[i];
        if (flow_dir == dir_nodata_ || flow_dir < 0) { continue; }

        flow_in_num_[i] = 0;
        if (flow_dir & 1) { flow_in_num_[i]++; }
        if (flow_dir & 2) { flow_in_num_[i]++; }
        if (flow_dir & 4) { flow_in_num_[i]++; }
        if (flow_dir & 8) { flow_in_num_[i]++; }
        if (flow_dir & 16) { flow_in_num_[i]++; }
        if (flow_dir & 32) { flow_in_num_[i]++; }
        if (flow_dir & 64) { flow_in_num_[i]++; }
        if (flow_dir & 128) { flow_in_num_[i]++; }
    }
    int total = 0;
#pragma omp parallel for reduction(+:total)
    for (int index = 0; index < n; index++) {
        if (flow_in_num_[index] <= 0 || flow_in_num_[index] == dir_nodata_) { continue; }
        total += flow_in_num_[index];
    }
    flow_in_count_ = total;
}

void GridLayering::BuildMultiFlowOutArray(int*& compressed_dir,
                                          int*& connect_count, float*& p_output) {
    p_output[0] = CVT_FLT(n_valid_cells_);
    int counter = 1;
    for (int i = 0; i < n_rows_; i++) {
        for (int j = 0; j < n_cols_; j++) {
            int index = i * n_cols_ + j;
            if (pos_index_[index] < 0) continue;
            /// flow in cell's number
            p_output[counter++] = CVT_FLT(connect_count[index]);
            /// accumulated flow in directions
            int acc_flowin_dir = compressed_dir[index];
            /// append the compressed index of flow in cells
            if (acc_flowin_dir & 1) {
                if (j != n_cols_ - 1 && compressed_dir[index + 1] != dir_nodata_) {
                    p_output[counter++] = CVT_FLT(pos_index_[index + 1]);
                }
            }
            if (acc_flowin_dir & 2) {
                if (i != n_rows_ - 1 && j != n_cols_ - 1 &&
                    compressed_dir[index + n_cols_ + 1] != dir_nodata_) {
                    p_output[counter++] = CVT_FLT(pos_index_[index + n_cols_ + 1]);
                }
            }
            if (acc_flowin_dir & 4) {
                if (i != n_rows_ - 1 && compressed_dir[index + n_cols_] != dir_nodata_) {
                    p_output[counter++] = CVT_FLT(pos_index_[index + n_cols_]);
                }
            }
            if (acc_flowin_dir & 8) {
                if (i != n_rows_ - 1 && j != 0 && compressed_dir[index + n_cols_ - 1] != dir_nodata_) {
                    p_output[counter++] = CVT_FLT(pos_index_[index + n_cols_ - 1]);
                }
            }
            if (acc_flowin_dir & 16) {
                if (j != 0 && compressed_dir[index - 1] != dir_nodata_) {
                    p_output[counter++] = CVT_FLT(pos_index_[index - 1]);
                }
            }
            if (acc_flowin_dir & 32) {
                if (i != 0 && j != 0 && compressed_dir[index - n_cols_ - 1] != dir_nodata_) {
                    p_output[counter++] = CVT_FLT(pos_index_[index - n_cols_ - 1]);
                }
            }
            if (acc_flowin_dir & 64) {
                if (i != 0 && compressed_dir[index - n_cols_] != dir_nodata_) {
                    p_output[counter++] = CVT_FLT(pos_index_[index - n_cols_]);
                }
            }
            if (acc_flowin_dir & 128) {
                if (i != 0 && j != n_cols_ - 1 && compressed_dir[index - n_cols_ + 1] != dir_nodata_) {
                    p_output[counter++] = CVT_FLT(pos_index_[index - n_cols_ + 1]);
                }
            }
        }
    }
}

void GridLayering::BuildFlowInCellsArray() {
    int nOutput = flow_in_count_ + n_valid_cells_ + 1;
    if (nullptr == flow_in_cells_) Initialize1DArray(nOutput, flow_in_cells_, 0.f);
    BuildMultiFlowOutArray(reverse_dir_, flow_in_num_, flow_in_cells_);
}

void GridLayering::CountFlowOutCells() {
    int n = n_rows_ * n_cols_;
    if (nullptr == flow_out_num_) Initialize1DArray(n, flow_out_num_, dir_nodata_);
#pragma omp parallel for
    for (int index = 0; index < n; index++) {
        if (pos_index_[index] < 0) continue;
        int i = index / n_cols_;
        int j = index % n_cols_;
        flow_out_num_[index] = 0;
        int flow_dir = flowdir_matrix_[index];

        if (flow_dir & 1 && j != n_cols_ - 1 && flowdir_matrix_[index + 1] != dir_nodata_) {
            flow_out_num_[index]++;
        }
        if (flow_dir & 2 && i != n_rows_ - 1 && j != n_cols_ - 1 &&
            flowdir_matrix_[index + n_cols_ + 1] != dir_nodata_) {
            flow_out_num_[index]++;
        }
        if (flow_dir & 4 && i != n_rows_ - 1 &&
            flowdir_matrix_[index + n_cols_] != dir_nodata_) {
            flow_out_num_[index]++;
        }
        if (flow_dir & 8 && i != n_rows_ - 1 && j != 0 &&
            flowdir_matrix_[index + n_cols_ - 1] != dir_nodata_) {
            flow_out_num_[index]++;
        }
        if (flow_dir & 16 && j != 0 &&
            flowdir_matrix_[index - 1] != dir_nodata_) {
            flow_out_num_[index]++;
        }
        if (flow_dir & 32 && i != 0 && j != 0 &&
            flowdir_matrix_[index - n_cols_ - 1] != dir_nodata_) {
            flow_out_num_[index]++;
        }
        if (flow_dir & 64 && i != 0 &&
            flowdir_matrix_[index - n_cols_] != dir_nodata_) {
            flow_out_num_[index]++;
        }
        if (flow_dir & 128 && i != 0 && j != n_cols_ - 1 &&
            flowdir_matrix_[index - n_cols_ + 1] != dir_nodata_) {
            flow_out_num_[index]++;
        }
    }
    int total = 0;
#pragma omp parallel for reduction(+:total)
    for (int index = 0; index < n; index++) {
        if (flow_out_num_[index] <= 0 || flow_out_num_[index] == dir_nodata_) { continue; }
        total += flow_out_num_[index];
    }
    flow_out_count_ = total;
}

void GridLayering::BuildFlowOutCellsArray() {
    int n_output = flow_out_count_ + n_valid_cells_ + 1;
    if (nullptr == flow_out_cells_) Initialize1DArray(n_output, flow_out_cells_, 0.f);
    BuildMultiFlowOutArray(flowdir_matrix_, flow_out_num_, flow_out_cells_);
}

bool GridLayering::Output2DimensionArrayTxt(const string& name, string& header, float* const matrix) {
    string outpath = string(output_dir_) + "/" + name + ".txt";
    std::ofstream ofs(outpath.c_str());
    ofs << matrix[0] << endl;
    ofs << header << endl;
    int tmp_count = 1;
    for (int i = 0; i < CVT_INT(matrix[0]); i++) {
        int count = CVT_INT(matrix[tmp_count++]);
        ofs << i << "\t" << count << "\t";
        for (int j = 0; j < count; j++) {
            ofs << matrix[tmp_count++] << ", ";
        }
        ofs << endl;
    }
    ofs.close();
    return true;
}

bool GridLayering::OutputArrayAsGfs(const string& name, const int length, float* const matrix) {
    bool flag = false;
    int max_loop = 3;
    int cur_loop = 1;
    while (cur_loop < max_loop) {
        if (!OutputToMongodb(name.c_str(), length, reinterpret_cast<char *>(matrix))) {
            cur_loop++;
        } else {
            cout << "Output " << name << " done!" << endl;
            flag = true;
            break;
        }
    }
    return flag;
}

bool GridLayering::OutputFlowIn() {
    GetReverseDirMatrix();
    CountFlowInCells();
    BuildFlowInCellsArray();
    string header = "ID\tUpstreamCount\tUpstreamID";
    int datalength = n_valid_cells_ + flow_in_count_ + 1;
    return Output2DimensionArrayTxt(flowin_index_name_, header, flow_in_cells_) &&
            OutputArrayAsGfs(flowin_index_name_, datalength, flow_in_cells_);
}

bool GridLayering::GridLayeringFromSource() {
    int n = n_rows_ * n_cols_;
    Initialize1DArray(n, layers_updown_, out_nodata_);

    int* last_layer = nullptr;
    Initialize1DArray(n, last_layer, out_nodata_);
    int num_last_layer = 0; // the number of cells of last layer

    int index = 0;
    vector<vector<int> > layerCells_updown;
    // 1. find source grids, i.e., the first layer
    for (int i = 0; i < n; i++) {
        if (flow_in_num_[i] == 0) { last_layer[num_last_layer++] = i; }
    }
    // 2. loop and layer
    int num_next_layer = 0;
    int cur_num = 0; //the layering number of the current layer. In the end, it is the total number of layers.
    int* next_layer = nullptr;
    Initialize1DArray(n, next_layer, out_nodata_);
    int* tmp = nullptr;
    vector<int> lyr_cells;
    while (num_last_layer > 0) {
        cur_num++;
        num_next_layer = 0;
        for (int i_in_layer = 0; i_in_layer < num_last_layer; i_in_layer++) {
            index = last_layer[i_in_layer];
            lyr_cells.emplace_back(pos_index_[index]);
            layers_updown_[index] = cur_num;

            int i = index / n_cols_;
            int j = index % n_cols_;

            int dir = flowdir_matrix_[index];
            if (dir & 1 && j != n_cols_ - 1) {
                if (--flow_in_num_[index + 1] == 0) {
                    next_layer[num_next_layer++] = index + 1;
                }
            }
            if (dir & 2 && i != n_rows_ - 1 && j != n_cols_ - 1) {
                if (--flow_in_num_[index + n_cols_ + 1] == 0) {
                    next_layer[num_next_layer++] = index + n_cols_ + 1;
                }
            }
            if (dir & 4 && i != n_rows_ - 1) {
                if (--flow_in_num_[index + n_cols_] == 0) {
                    next_layer[num_next_layer++] = index + n_cols_;
                }
            }
            if (dir & 8 && i != n_rows_ - 1 && j != 0) {
                if (--flow_in_num_[index + n_cols_ - 1] == 0) {
                    next_layer[num_next_layer++] = index + n_cols_ - 1;
                }
            }
            if (dir & 16 && j != 0) {
                if (--flow_in_num_[index - 1] == 0) {
                    next_layer[num_next_layer++] = index - 1;
                }
            }
            if (dir & 32 && i != 0 && j != 0) {
                if (--flow_in_num_[index - n_cols_ - 1] == 0) {
                    next_layer[num_next_layer++] = index - n_cols_ - 1;
                }
            }
            if (dir & 64 && i != 0) {
                if (--flow_in_num_[index - n_cols_] == 0) {
                    next_layer[num_next_layer++] = index - n_cols_;
                }
            }
            if (dir & 128 && i != 0 && j != n_cols_ - 1) {
                if (--flow_in_num_[index - n_cols_ + 1] == 0) {
                    next_layer[num_next_layer++] = index - n_cols_ + 1;
                }
            }
        }
        layerCells_updown.emplace_back(vector<int>(lyr_cells));
        lyr_cells.clear();

        num_last_layer = num_next_layer;
        tmp = last_layer;
        last_layer = next_layer;
        next_layer = tmp;
    }
    Release1DArray(last_layer);
    Release1DArray(next_layer);

    int layer_num = CVT_INT(layerCells_updown.size());
    int length = n_valid_cells_ + layer_num + 1;
    Initialize1DArray(length, layer_cells_updown_, 0.f);
    layer_cells_updown_[0] = CVT_FLT(layer_num);
    index = 1;
    for (auto it = layerCells_updown.begin(); it != layerCells_updown.end(); ++it) {
        layer_cells_updown_[index++] = CVT_FLT((*it).size());
        for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
            layer_cells_updown_[index++] = CVT_FLT(*it2);
        }
    }
    return OutputGridLayering(layering_updown_name_, length,
                              layers_updown_, layer_cells_updown_);
}

bool GridLayering::GridLayeringFromOutlet() {
    int n = n_rows_ * n_cols_;
    Initialize1DArray(n, layers_downup_, out_nodata_);

    int* last_layer = nullptr;
    Initialize1DArray(n, last_layer, out_nodata_);
    int num_last_layer = 0;

    int cur_num = 0;
    vector<vector<int> > layer_cells_downup;
    // 1. find outlet grids
    for (int i = 0; i < n; i++) {
        if (flowdir_matrix_[i] != dir_nodata_ && flow_out_num_[i] == 0) {
            last_layer[num_last_layer++] = i;
        }
    }
    // 2. loop and layer
    int num_next_layer = 0;
    int* next_layer = nullptr;
    Initialize1DArray(n, next_layer, out_nodata_);
    int* tmp = nullptr;
    vector<int> lyr_cells;
    while (num_last_layer > 0) {
        cur_num++;
        num_next_layer = 0;
        for (int i_in_layer = 0; i_in_layer < num_last_layer; i_in_layer++) {
            int index = last_layer[i_in_layer];
            lyr_cells.emplace_back(pos_index_[index]);
            layers_downup_[index] = cur_num;

            int i = index / n_cols_;
            int j = index % n_cols_;

            int index2 = i * n_cols_ + j + 1;
            if (j != n_cols_ - 1 && flowdir_matrix_[index2] & 16) {
                if (--flow_out_num_[index2] == 0) {
                    next_layer[num_next_layer++] = index2;
                }
            }
            index2 = (i + 1) * n_cols_ + j + 1;
            if (i != n_rows_ - 1 && j != n_cols_ - 1 && flowdir_matrix_[index2] & 32) {
                if (--flow_out_num_[index2] == 0) {
                    next_layer[num_next_layer++] = index2;
                }
            }
            index2 = (i + 1) * n_cols_ + j;
            if (i != n_rows_ - 1 && flowdir_matrix_[index2] & 64) {
                if (--flow_out_num_[index2] == 0) {
                    next_layer[num_next_layer++] = index2;
                }
            }
            index2 = (i + 1) * n_cols_ + j - 1;
            if (i != n_rows_ - 1 && j != 0 && flowdir_matrix_[index2] & 128) {
                if (--flow_out_num_[index2] == 0) {
                    next_layer[num_next_layer++] = index2;
                }
            }
            index2 = i * n_cols_ + j - 1;
            if (j != 0 && flowdir_matrix_[index2] & 1) {
                if (--flow_out_num_[index2] == 0) {
                    next_layer[num_next_layer++] = index2;
                }
            }
            index2 = (i - 1) * n_cols_ + j - 1;
            if (i != 0 && j != 0 && flowdir_matrix_[index2] & 2) {
                if (--flow_out_num_[index2] == 0) {
                    next_layer[num_next_layer++] = index2;
                }
            }
            index2 = (i - 1) * n_cols_ + j;
            if (i != 0 && flowdir_matrix_[index2] & 4) {
                if (--flow_out_num_[index2] == 0) {
                    next_layer[num_next_layer++] = index2;
                }
            }
            index2 = (i - 1) * n_cols_ + j + 1;
            if (i != 0 && j != n_cols_ - 1 && flowdir_matrix_[index2] & 8) {
                if (--flow_out_num_[index2] == 0) {
                    next_layer[num_next_layer++] = index2;
                }
            }
        }
        layer_cells_downup.emplace_back(vector<int>(lyr_cells));
        lyr_cells.clear();

        num_last_layer = num_next_layer;
        tmp = last_layer;
        last_layer = next_layer;
        next_layer = tmp;
    }
    // 3. reverse the layer number
#pragma omp parallel for
    for (int i = 0; i < n; i++) {
        if (layers_downup_[i] != out_nodata_) {
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
    for (auto it = layer_cells_downup.end() - 1; it != layer_cells_downup.begin(); --it) {
        layer_cells_downup_[index++] = CVT_FLT((*it).size());
        for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
            layer_cells_downup_[index++] = CVT_FLT(*it2);
        }
    }
    return OutputGridLayering(layering_downup_name_, length,
                              layers_downup_, layer_cells_downup_);
}

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
    if (NULL == gfs_->GetFile(name)) { return false; }
    return true;
}

bool GridLayering::OutputGridLayering(const string& name, const int datalength,
                                      int* const layer_grid, float* const layer_cells) {
    string outpath = string(output_dir_) + "/" + name + ".tif";
    IntRaster(flowdir_, layer_grid).OutputFileByGdal(outpath);

    string header = "LayerID\tCellCount\tCellIDs";
    return Output2DimensionArrayTxt(name, header, layer_cells) &&
            OutputArrayAsGfs(name, datalength, layer_cells);
}
