#include "GridLayering.h"

GridLayeringD8::GridLayeringD8(const int id, MongoGridFs* gfs, const char* out_dir) :
    GridLayering(id, gfs, out_dir) {
    string prefix = ValueToString(subbasin_id_);
    flowdir_name_ = prefix + "_FLOW_DIR";
    flowin_index_name_ = prefix + "_FLOWIN_INDEX_D8";
    flowout_index_name_ = prefix + "_FLOWOUT_INDEX_D8";
    layering_updown_name_ = prefix + "_ROUTING_LAYERS_UP_DOWN";
    layering_downup_name_ = prefix + "_ROUTING_LAYERS_DOWN_UP";
}

GridLayeringD8::~GridLayeringD8() {
    // Nothing to do.
}


bool GridLayeringD8::LoadData() {
    flowdir_ = clsRasterData<int>::Init(gfs_, flowdir_name_.c_str(), false);
    if (nullptr == flowdir_) return false;
    n_rows_ = flowdir_->GetRows();
    n_cols_ = flowdir_->GetCols();
    dir_nodata_ = flowdir_->GetNoDataValue();
    flowdir_matrix_ = flowdir_->GetRasterDataPointer();
    if (dir_nodata_ != out_nodata_) flowdir_->ReplaceNoData(out_nodata_);
    return true;
}

bool GridLayeringD8::OutputFlowOut() {
    CountFlowOutCells();
    float* p_output = nullptr;
    Initialize1DArray(n_valid_cells_, p_output, -1.f);
#pragma omp parallel for
    for (int i = 0; i < n_rows_; ++i) {
        for (int j = 0; j < n_cols_; ++j) {
            int index = i * n_cols_ + j;
            if (flowdir_matrix_[index] == dir_nodata_ || flowdir_matrix_[index] < 0) {
                continue;
            }
            int ci = pos_index_[index];
            int flow_dir = flowdir_matrix_[index];

            if (flow_dir & 1 && j != n_cols_ - 1 && flowdir_matrix_[index + 1] != dir_nodata_) {
                p_output[ci] = CVT_FLT(pos_index_[index + 1]);
            } else if (flow_dir & 2 && i != n_rows_ - 1 && j != n_cols_ - 1 &&
                flowdir_matrix_[index + n_cols_ + 1] != dir_nodata_) {
                p_output[ci] = CVT_FLT(pos_index_[index + n_cols_ + 1]);
            } else if (flow_dir & 4 && i != n_rows_ - 1 && flowdir_matrix_[index + n_cols_] != dir_nodata_) {
                p_output[ci] = CVT_FLT(pos_index_[index + n_cols_]);
            } else if (flow_dir & 8 && i != n_rows_ - 1 && j != 0 &&
                flowdir_matrix_[index + n_cols_ - 1] != dir_nodata_) {
                p_output[ci] = CVT_FLT(pos_index_[index + n_cols_ - 1]);
            } else if (flow_dir & 16 && j != 0 && flowdir_matrix_[index - 1] != dir_nodata_) {
                p_output[ci] = CVT_FLT(pos_index_[index - 1]);
            } else if (flow_dir & 32 && i != 0 && j != 0
                && flowdir_matrix_[index - n_cols_ - 1] != dir_nodata_) {
                p_output[ci] = CVT_FLT(pos_index_[index - n_cols_ - 1]);
            } else if (flow_dir & 64 && i != 0 && flowdir_matrix_[index - n_cols_] != dir_nodata_) {
                p_output[ci] = CVT_FLT(pos_index_[index - n_cols_]);
            } else if (flow_dir & 128 && i != 0 && j != n_cols_ - 1
                && flowdir_matrix_[index - n_cols_ + 1] != dir_nodata_) {
                p_output[ci] = CVT_FLT(pos_index_[index - n_cols_ + 1]);
            } else { p_output[ci] = -1; }
        }
    }
    // output to txt
    string outpath = string(output_dir_) + "/" + flowout_index_name_ + ".txt";
    std::ofstream ofs(outpath.c_str());
    ofs << "ID\tDownstreamID" << endl;
    for (int i = 0; i < n_valid_cells_; i++) {
        ofs << i << "\t" << p_output[i] << endl;
    }
    ofs.close();

    bool flag = OutputArrayAsGfs(flowout_index_name_, n_valid_cells_, p_output);
    Release1DArray(p_output);
    return flag;
}
