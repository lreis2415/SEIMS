#include "GridLayering.h"

GridLayeringDinf::GridLayeringDinf(const int id, MongoGridFs* gfs, const char* out_dir) :
    GridLayering(id, gfs, out_dir),
    flow_angle_(nullptr),
    angle_(nullptr), flow_in_angle_(nullptr),
    flow_angle_name_(""), flowin_angle_name_("") {
    string prefix = ValueToString(subbasin_id_);
    flowdir_name_ = prefix + "_FLOW_DIR_DINF";
    flow_angle_name_ = prefix + "_FLOW_DIR_ANGLE_DINF";
    flowin_index_name_ = prefix + "_FLOWIN_INDEX_DINF";
    flowin_angle_name_ = prefix + "_FLOWIN_PERCENTAGE_DINF";
    flowout_index_name_ = prefix + "_FLOWOUT_INDEX_DINF";
    layering_updown_name_ = prefix + "_ROUTING_LAYERS_UP_DOWN_DINF";
    layering_downup_name_ = prefix + "_ROUTING_LAYERS_DOWN_UP_DINF";
}

GridLayeringDinf::~GridLayeringDinf() {
    delete flow_angle_;
    if (nullptr != flow_in_angle_) Release1DArray(flow_in_angle_);
}

bool GridLayeringDinf::LoadData() {
    flowdir_ = clsRasterData<int>::Init(gfs_, flowdir_name_.c_str(), false);
    if (nullptr == flowdir_) return false;
    n_rows_ = flowdir_->GetRows();
    n_cols_ = flowdir_->GetCols();
    dir_nodata_ = flowdir_->GetNoDataValue();
    flowdir_matrix_ = flowdir_->GetRasterDataPointer();
    if (dir_nodata_ != out_nodata_) flowdir_->ReplaceNoData(out_nodata_);

    flow_angle_ = clsRasterData<float>::Init(gfs_, flow_angle_name_.c_str(), false);
    if (nullptr == flow_angle_) return false;
    angle_ = flow_angle_->GetRasterDataPointer();
    return true;
}

void GridLayeringDinf::BuildMultiFlowOutAngleArray(int*& compressed_dir,
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
                    p_output[counter++] = GetPercentage(angle_[index + 1], 0, 1);;
                }
            }
            if (acc_flowin_dir & 2) {
                if (i != n_rows_ - 1 && j != n_cols_ - 1 &&
                    compressed_dir[index + n_cols_ + 1] != dir_nodata_) {
                    p_output[counter++] = GetPercentage(angle_[index + n_cols_ + 1], 1, 1);
                }
            }
            if (acc_flowin_dir & 4) {
                if (i != n_rows_ - 1 && compressed_dir[index + n_cols_] != dir_nodata_) {
                    p_output[counter++] = GetPercentage(angle_[index + n_cols_], 1, 0);
                }
            }
            if (acc_flowin_dir & 8) {
                if (i != n_rows_ - 1 && j != 0 && compressed_dir[index + n_cols_ - 1] != dir_nodata_) {
                    p_output[counter++] = GetPercentage(angle_[index + n_cols_ - 1], 1, -1);
                }
            }
            if (acc_flowin_dir & 16) {
                if (j != 0 && compressed_dir[index - 1] != dir_nodata_) {
                    p_output[counter++] = GetPercentage(angle_[index - 1], 0, -1);
                }
            }
            if (acc_flowin_dir & 32) {
                if (i != 0 && j != 0 && compressed_dir[index - n_cols_ - 1] != dir_nodata_) {
                    p_output[counter++] = GetPercentage(angle_[index - n_cols_ - 1], -1, -1);
                }
            }
            if (acc_flowin_dir & 64) {
                if (i != 0 && compressed_dir[index - n_cols_] != dir_nodata_) {
                    p_output[counter++] = GetPercentage(angle_[index - n_cols_], -1, 0);
                }
            }
            if (acc_flowin_dir & 128) {
                if (i != 0 && j != n_cols_ - 1 && compressed_dir[index - n_cols_ + 1] != dir_nodata_) {
                    p_output[counter++] = GetPercentage(angle_[index - n_cols_ + 1], -1, 1);
                }
            }
        }
    }
}

bool GridLayeringDinf::OutputFlowIn() {
    GetReverseDirMatrix();
    CountFlowInCells();
    BuildFlowInCellsArray();
    string header = "ID\tUpstreamCount\tUpstreamID";
    int datalength = n_valid_cells_ + flow_in_count_ + 1;
    bool flag1 = Output2DimensionArrayTxt(flowin_index_name_, header, flow_in_cells_) &&
            OutputArrayAsGfs(flowin_index_name_, datalength, flow_in_cells_);

    if (nullptr == flow_in_angle_) Initialize1DArray(datalength, flow_in_angle_, 0.f);
    this->BuildMultiFlowOutAngleArray(flowdir_matrix_, flow_in_num_, flow_in_angle_);
    header = "ID\tUpstreamCount\tFlowInPartition";
    bool flag2 = Output2DimensionArrayTxt(flowin_angle_name_, header, flow_in_angle_) &&
            OutputArrayAsGfs(flowin_angle_name_, datalength, flow_in_angle_);
    return flag1 && flag2;
}

bool GridLayeringDinf::OutputFlowOut() {
    CountFlowOutCells();
    BuildFlowOutCellsArray();
    string header = "ID\tDownstreamCount\tDownstreamID";
    int datalength = n_valid_cells_ + flow_out_count_ + 1;
    return Output2DimensionArrayTxt(flowout_index_name_, header, flow_out_cells_) &&
            OutputArrayAsGfs(flowout_index_name_, datalength, flow_out_cells_);
}

float GridLayeringDinf::GetPercentage(const float angle, const int di, const int dj) {
    float a = 4.f * angle / PI;
    int n = int(a) % 7;
    float r = a - n;
    switch (n) {
        case 0: { return di != 0 ? r : 1 - r; }
        case 1: { return dj == 0 ? r : 1 - r; }
        case 2: { return dj != 0 ? r : 1 - r; }
        case 3: { return di == 0 ? r : 1 - r; }
        case 4: { return di != 0 ? r : 1 - r; }
        case 5: { return dj == 0 ? r : 1 - r; }
        case 6: { return dj != 0 ? r : 1 - r; }
        case 7: { return di == 0 ? r : 1 - r; }
        default: {
            cout << "Invalid angle value: " << angle << endl;
            exit(-1);
        }
    }
}
