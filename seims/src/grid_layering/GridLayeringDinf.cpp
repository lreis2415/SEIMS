#include "GridLayering.h"

#ifdef USE_MONGODB
GridLayeringDinf::GridLayeringDinf(const int id, MongoGridFs* gfs, const char* out_dir) :
    GridLayering(id, gfs, out_dir),
    flow_fraction_(nullptr),
    flowfrac_matrix_(nullptr), flowin_fracs_(nullptr), flowout_fracs_(nullptr) {
    string prefix = ValueToString(subbasin_id_);
    // inputs
    flowdir_name_ = prefix + "_FLOW_DIR_DINF";
    flowfrac_name_ = prefix + "_WEIGHT_DINF";
    mask_name_ = prefix + "_MASK";
    // outputs
    flowin_index_name_ = prefix + "_FLOWIN_INDEX_DINF";
    flowin_frac_name_ = prefix + "_FLOWIN_FRACTION_DINF";
    flowout_index_name_ = prefix + "_FLOWOUT_INDEX_DINF";
    flowout_frac_name_ = prefix + "_FLOWOUT_FRACTION_DINF";
    layering_updown_name_ = prefix + "_ROUTING_LAYERS_UP_DOWN_DINF";
    layering_downup_name_ = prefix + "_ROUTING_LAYERS_DOWN_UP_DINF";
}
#endif

GridLayeringDinf::GridLayeringDinf(const int id, const char* fd_file, const char* fraction_file,
                                   const char* mask_file, const char* out_dir) :
    GridLayering(id, out_dir), flow_fraction_(nullptr), flowfrac_matrix_(nullptr), flowin_fracs_(nullptr),
    flowout_fracs_(nullptr) {
    string prefix = ValueToString(subbasin_id_);
    // inputs
    flowdir_name_ = fd_file;
    flowfrac_name_ = fraction_file;
    mask_name_ = mask_file;
    // outputs
    flowin_index_name_ = prefix + "_FLOWIN_INDEX_DINF";
    flowin_frac_name_ = prefix + "_FLOWIN_FRACTION_DINF";
    flowout_index_name_ = prefix + "_FLOWOUT_INDEX_DINF";
    flowout_frac_name_ = prefix + "_FLOWOUT_FRACTION_DINF";
    layering_updown_name_ = prefix + "_ROUTING_LAYERS_UP_DOWN_DINF";
    layering_downup_name_ = prefix + "_ROUTING_LAYERS_DOWN_UP_DINF";
}

GridLayeringDinf::~GridLayeringDinf() {
    delete flow_fraction_;
    if (nullptr != flowin_fracs_) Release1DArray(flowin_fracs_);
    if (nullptr != flowout_fracs_) Release1DArray(flowout_fracs_);
}

bool GridLayeringDinf::LoadData() {
    if (use_mongo_) {
#ifdef USE_MONGODB
        has_mask_ = true;
        mask_ = FloatRaster::Init(gfs_, mask_name_.c_str(), true);
        flowdir_ = FltMaskFltRaster::Init(gfs_, flowdir_name_.c_str(), true, mask_, true);
        flow_fraction_ = FltMaskFltRaster::Init(gfs_, flowfrac_name_.c_str(), true, mask_, true);
#else
        return false;
#endif
    } else {
        if (StringMatch(flowdir_name_, mask_name_)) {
            flowdir_ = FloatRaster::Init(flowdir_name_, true);
            mask_ = flowdir_;
        } else {
            has_mask_ = true;
            mask_ = FloatRaster::Init(mask_name_, true);
            flowdir_ = FltMaskFltRaster::Init(flowdir_name_, true, mask_, true);
        }
        flow_fraction_ = FltMaskFltRaster::Init(flowfrac_name_, true, mask_, true);
    }
    if (nullptr == flowdir_ || nullptr == flow_fraction_ || nullptr == mask_) return false;

    n_rows_ = mask_->GetRows();
    n_cols_ = mask_->GetCols();
    mask_->GetRasterPositionData(&n_valid_cells_, &pos_rowcol_);

    flowdir_matrix_ = flowdir_->GetRasterDataPointer();
    if (FloatEqual(flowdir_->GetNoDataValue(), out_nodata_)) flowdir_->ReplaceNoData(out_nodata_);
    flowfrac_matrix_ = flow_fraction_->GetRasterDataPointer();
    if (flowdir_->GetValidNumber() != flow_fraction_->GetValidNumber()) {
        cout << "The valid cell number must be the same between "
                "Dinf flow direction and flow fraction raster data!" << endl;
        return false;
    }
    return true;
}

bool GridLayeringDinf::OutputFlowIn() {
    GetReverseDirMatrix();
    CountFlowInCells();
    if (!BuildFlowInCellsArray()) return false;

    int datalength = n_valid_cells_ + flow_in_count_ + 1;
    if (nullptr == flowin_fracs_) Initialize1DArray(datalength, flowin_fracs_, 0.f);
    flowin_fracs_[0] = CVT_FLT(n_valid_cells_);
    int count = 1;
    for (int valid_idx = 0; valid_idx < n_valid_cells_; valid_idx++) {
        int i = pos_rowcol_[valid_idx][0]; // row
        int j = pos_rowcol_[valid_idx][1]; // col
        flowin_fracs_[count++] = CVT_FLT(flow_in_num_[valid_idx]);
        if (flow_in_num_[valid_idx] == 0) continue;

        int reversed_fdir = reverse_dir_[valid_idx];
        if (reversed_fdir < 0) continue; // This will not happen, just in case!

        vector<int> reversed_fdirs = uncompress_flow_directions(reversed_fdir);
        for (vector<int>::iterator it = reversed_fdirs.begin(); it != reversed_fdirs.end(); ++it) {
            int rfd_idx = find_flow_direction_index_ccw(*it);
            int source_row = i + drow[rfd_idx];
            int source_col = j + dcol[rfd_idx];
            if (!mask_->ValidateRowCol(source_row, source_col) ||
                mask_->IsNoData(source_row, source_col) ||
                flowdir_->IsNoData(source_row, source_col))
                continue;
            int source_index = pos_index_[source_row * n_cols_ + source_col];
            int source_fdir = CVT_INT(flowdir_matrix_[source_index]);
            vector<int> source_fdirs = uncompress_flow_directions(source_fdir);
            if (source_fdirs.size() == 1) {
                flowin_fracs_[count++] = 1.f;
                continue;
            }
            int source_1stfd_idx = find_flow_direction_index_ccw(source_fdirs[0]);
            if (i == source_row + drow[source_1stfd_idx] && j == source_col + dcol[source_1stfd_idx]) {
                flowin_fracs_[count++] = flowfrac_matrix_[source_index];
            } else {
                flowin_fracs_[count++] = 1.f - flowfrac_matrix_[source_index];
            }
        }
    }

    if (count != datalength) {
        cout << "Build flow in fraction array failed!" << endl;
        return false;
    }

    string header = "ID\tUpstreamCount\tUpstreamID\tFlowInFraction";
    bool done = Output2DimensionArrayTxt(flowin_index_name_, header, flow_in_cells_, flowin_fracs_);
    if (use_mongo_) {
#ifdef USE_MONGODB
        done = done && OutputArrayAsGfs(flowin_index_name_, flow_in_cells_) && 
                OutputArrayAsGfs(flowin_frac_name_, flowin_fracs_);
#endif
    }
    return done;
}

bool GridLayeringDinf::OutputFlowOut() {
    CountFlowOutCells();
    if (!BuildFlowOutCellsArray()) return false;

    if (nullptr == flowout_fracs_) Initialize1DArray(flow_out_count_ + n_valid_cells_ + 1, flowout_fracs_, 0.f);
    flowout_fracs_[0] = CVT_FLT(n_valid_cells_);
    int count = 1;
    for (int valid_idx = 0; valid_idx < n_valid_cells_; valid_idx++) {
        int i = pos_rowcol_[valid_idx][0];                           // row
        int j = pos_rowcol_[valid_idx][1];                           // col
        flowout_fracs_[count++] = CVT_FLT(flow_out_num_[valid_idx]); // maybe 0
        if (flow_out_num_[valid_idx] == 0) continue;

        int flow_dir = CVT_INT(flowdir_matrix_[valid_idx]);
        if (flow_dir < 0) continue; // This will not happen, just in case!

        vector<int> flow_dirs = uncompress_flow_directions(flow_dir);
        if (flow_dirs.empty()) continue;

        int fd_idx = find_flow_direction_index_ccw(flow_dirs[0]);
        if (mask_->ValidateRowCol(i + drow[fd_idx], j + dcol[fd_idx]) &&
            !mask_->IsNoData(i + drow[fd_idx], j + dcol[fd_idx])) {
            flowout_fracs_[count++] = flowfrac_matrix_[valid_idx];
        }
        if (flow_dirs.size() == 1) {
            continue;
        }
        fd_idx = find_flow_direction_index_ccw(flow_dirs[1]);
        if (mask_->ValidateRowCol(i + drow[fd_idx], j + dcol[fd_idx]) &&
            !mask_->IsNoData(i + drow[fd_idx], j + dcol[fd_idx])) {
            flowout_fracs_[count++] = 1.f - flowfrac_matrix_[valid_idx];
        }
    }

    if (count != flow_out_count_ + n_valid_cells_ + 1) {
        cout << "Build flow out fraction array failed!" << endl;
        return false;
    }

    string header = "ID\tDownstreamCount\tDownstreamID\tFlowOutFraction";
    bool done = Output2DimensionArrayTxt(flowout_index_name_, header, flow_out_cells_, flowout_fracs_);
    if (use_mongo_) {
#ifdef USE_MONGODB
        done = OutputArrayAsGfs(flowout_index_name_, flow_out_cells_) && 
                OutputArrayAsGfs(flowout_frac_name_, flowout_fracs_);
#endif
    }
    return done;
}
