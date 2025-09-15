#include "GridLayering.h"
#include "NormalizeFlowFractions.h"

#ifdef USE_MONGODB
GridLayeringDinf::GridLayeringDinf(const int id, MongoGridFs* gfs, const char* out_dir,
                                   const char* stream_file/*=nullptr*/,
                                   bool force_outlet/*=false*/, bool force_inbasin/*=true*/, int decimals/*=4*/) :
    GridLayering(id, gfs, out_dir) {
    // outputs
    OutputFilenames(FD_Dinf);
    // inputs
    string prefix = ValueToString(subbasin_id_);
    flowdir_name_ = prefix + "_FLOW_DIR_" + fdtype_str_;
    flowfrac_name_ = prefix + "_WEIGHT_" + fdtype_str_;
    mask_name_ = prefix + "_SUBBASIN";
    stream_file_ = stream_file;
    force_outlet_ = force_outlet;
    force_inbasin_ = force_inbasin;
    decimals_ = decimals;
}
#endif

GridLayeringDinf::GridLayeringDinf(const int id, const char* out_dir,
                                   const char* fd_file, const char* fraction_file,
                                   const char* mask_file/*=nullptr*/, const char* stream_file/*=nullptr*/,
                                   bool force_outlet/*=false*/, bool force_inbasin/*=true*/, int decimals/*=4*/) :
    GridLayering(id, out_dir) {
    string prefix = ValueToString(subbasin_id_);
    // inputs
    flowdir_name_ = fd_file;
    flowfrac_name_ = fraction_file;
    mask_name_ = mask_file;
    stream_file_ = stream_file;
    force_outlet_ = force_outlet;
    force_inbasin_ = force_inbasin;
    decimals_ = decimals;
    // outputs
    OutputFilenames(FD_Dinf);
}

GridLayeringDinf::~GridLayeringDinf() {
    // Nothing to do here!
}

// void GridLayeringDinf::OutputFilenames(flowDirTypes ftype) {
//     GridLayering::OutputFilenames(ftype);
//     string prefix = ValueToString(subbasin_id_);
//     flowin_frac_name_ = prefix + "_FLOWIN_FRACTION_" + fdtype_str_;
//     flowout_frac_name_ = prefix + "_FLOWOUT_FRACTION_" + fdtype_str_;
// }


bool GridLayeringDinf::LoadData() {
    if (use_mongo_) {
#ifdef USE_MONGODB
        has_mask_ = true;
        mask_ = IntRaster::Init(gfs_, mask_name_.c_str(), true);
        STRING_MAP opts;
        UpdateStringMap(opts, HEADER_INC_NODATA, "FALSE");
        flowdir_ = IntRaster::Init(gfs_, flowdir_name_.c_str(), true,
                                   mask_, true, NODATA_VALUE, opts);
        flow_fraction_ = FloatRaster::Init(gfs_, flowfrac_name_.c_str(), true,
                                                mask_, true, NODATA_VALUE, opts);
#else
        return false;
#endif
    } else {
        if (StringMatch(flowdir_name_, mask_name_)) {
            flowdir_ = IntRaster::Init(flowdir_name_, true);
            mask_ = flowdir_;
        } else {
            has_mask_ = true;
            mask_ = IntRaster::Init(mask_name_, true);
            flowdir_ = IntRaster::Init(flowdir_name_, true, mask_, true);
        }
        flow_fraction_ = FloatRaster::Init(flowfrac_name_, true, mask_, true);
    }
    if (nullptr == flowdir_ || nullptr == flow_fraction_ || nullptr == mask_) return false;

    n_rows_ = mask_->GetRows();
    n_cols_ = mask_->GetCols();
    mask_->GetRasterPositionData(&n_valid_cells_, &pos_rowcol_);

    flowdir_matrix_ = flowdir_->GetRasterDataPointer();
    if (FloatEqual(flowdir_->GetNoDataValue(), out_nodata_)) flowdir_->ReplaceNoData(out_nodata_);
    FLTPT* flowfrac_matrix_org = flow_fraction_->GetRasterDataPointer();
    if (flowdir_->GetValidNumber() != flow_fraction_->GetValidNumber()) {
        cout << "The valid cell number must be the same between "
                "Dinf flow direction and flow fraction raster data!" << endl;
        return false;
    }
    // Preprocessing flow fraction matrix to 2D array
    Initialize2DArray(n_valid_cells_, 8, flowfrac_matrix_, out_nodata_);
    for (int valid_idx = 0; valid_idx < n_valid_cells_; valid_idx++) {
        int flow_dir = flowdir_matrix_[valid_idx];
        if (flow_dir < 0) {
            continue; // This will not happen, just in case!
        }
        vector<int> flow_dirs = uncompress_flow_directions(flow_dir);
        if (flow_dirs.empty()) {
            continue; // This will not happen, just in case!
        }
        if (CVT_INT(flow_dirs.size()) == 1) { // Only one downslope cell
            int fd_idx = find_flow_direction_index_ccw(flow_dirs[0]);
            flowfrac_matrix_[valid_idx][fd_idx - 1] = 1.;
            flowfrac_matrix_org[valid_idx] = 1.; // currently, no further used
            continue;
        }
        int fd_idx = find_flow_direction_index_ccw(flow_dirs[0]);
        int fd_idx2 = find_flow_direction_index_ccw(flow_dirs[1]);
        // We don't need to consider the downslope cell is a valid cell or nodata here!
        vector<FLTPT> fracin(2);
        fracin[0] = flowfrac_matrix_org[valid_idx];
        fracin[1] = 1. - flowfrac_matrix_org[valid_idx];
        vector<FLTPT> fracout;
        normalize_flow_fraction(fracin, decimals_, fracout);
        flowfrac_matrix_[valid_idx][fd_idx - 1] = fracout[0];
        flowfrac_matrix_[valid_idx][fd_idx2 - 1] = fracout[1];
        flowfrac_matrix_org[valid_idx] = fracout[0]; // write back, although, currently, no further used
    }
    return LoadStreamData();
    /*
    // Force stream grid to flow into single downstream cell
    if (stream_file_.empty())
        return true;
    if (nullptr == stream_matrix_) Initialize1DArray(n_valid_cells_, stream_matrix_, mask_->GetNoDataValue());
    vector<vector<ROW_COL> > stream_rc;
    bool flag = read_stream_vertexes(stream_file_, mask_, stream_rc, stream_matrix_);
    if (!flag) return false;

    for(vector<vector<ROW_COL> >::iterator it = stream_rc.begin(); it != stream_rc.end(); ++it) {
        if (it->size() < 2) continue; // A line should have at least two points!
        // cout << it->size() << endl;
        for (vector<ROW_COL>::reverse_iterator it2 = it->rbegin(); it2 != it->rend() - 1; ++it2) {
            // FOR TEST ONLY
            // cout << "(" << it2->first << ", " << it2->second << "), ";
            // XY_COOR xy = mask_->GetCoordinateByRowCol(it2->first, it2->second);
            // printf("(%.1f, %.1f), ", xy.first, xy.second);
            // cout << "(" << it2->first << ", " << it2->second << ") -> (" << (it2+1)->first << ", " << (it2+1)->second << "), ";
            int delta_row = (it2 + 1)->first - it2->first;
            int delta_col = (it2 + 1)->second - it2->second;
            int fd_index = find_flow_direction_index_ccw(delta_row, delta_col);
            //cout << flowdir_->GetValue(it2->first, it2->second) << ": " <<
            //        flow_fraction_->GetValue(it2->first, it2->second);
            if (fd_index < 0) {
                flowdir_->SetValue(it2->first, it2->second, -1.f);
                flow_fraction_->SetValue(it2->first, it2->second, flow_fraction_->GetNoDataValue());
            } else {
                flowdir_->SetValue(it2->first, it2->second, CVT_FLT(fdccw[fd_index]));
                flow_fraction_->SetValue(it2->first, it2->second, 1.f);
            }
            //cout << " --> " << flowdir_->GetValue(it2->first, it2->second) << ": " <<
            //        flow_fraction_->GetValue(it2->first, it2->second) << endl;
        }
        // cout << endl;
    }
    */
    return true;
}
/*
bool GridLayeringDinf::OutputFlowIn() {
    GetReverseDirMatrix();
    if (!BuildFlowInCellsArray()) return false;

    int datalength = n_valid_cells_ + flow_in_count_ + 1;
    if (nullptr == flowin_fracs_) Initialize1DArray(datalength, flowin_fracs_, 0.f);
    flowin_fracs_[0] = CVT_FLT(n_valid_cells_);
    int count = 1;
    for (int valid_idx = 0; valid_idx < n_valid_cells_; valid_idx++) {
        int i = pos_rowcol_[valid_idx][0]; // row
        int j = pos_rowcol_[valid_idx][1]; // col
        flowin_fracs_[count++] = CVT_FLT(flow_in_num_[valid_idx]);
        if (flow_in_num_[valid_idx] == 0) {
            continue;
        }

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
            int source_fdir = flowdir_matrix_[source_index];
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
        done = done && OutputArrayAsGfs(flowin_index_name_, count, flow_in_cells_) &&
                OutputArrayAsGfs(flowin_frac_name_, count, flowin_fracs_);
#endif
    }
    return done;
}

bool GridLayeringDinf::OutputFlowOut() {
    CountFlowOutCells();
    if (!BuildFlowOutCellsArray()) return false;

    if (nullptr == flowout_fracs_) {
        Initialize1DArray(flow_out_count_ + n_valid_cells_ + 1, flowout_fracs_, 0.f);
    }
    flowout_fracs_[0] = CVT_FLT(n_valid_cells_);
    int count = 1;
    for (int valid_idx = 0; valid_idx < n_valid_cells_; valid_idx++) {
        int i = pos_rowcol_[valid_idx][0];                           // row
        int j = pos_rowcol_[valid_idx][1];                           // col
        flowout_fracs_[count++] = CVT_FLT(flow_out_num_[valid_idx]); // maybe 0
        if (flow_out_num_[valid_idx] <= 0) {
            flowout_fracs_[count - 1] = 0.f;
            continue;
        }
        int flow_dir = flowdir_matrix_[valid_idx];
        if (flow_dir < 0) {
            flowout_fracs_[count - 1] = 0.f;
            continue; // This will not happen, just in case!
        }
        vector<int> flow_dirs = uncompress_flow_directions(flow_dir);
        if (flow_dirs.empty()) {
            flowout_fracs_[count - 1] = 0.f;
            continue;
        }
        int fd_idx = find_flow_direction_index_ccw(flow_dirs[0]);
        if (mask_->ValidateRowCol(i + drow[fd_idx], j + dcol[fd_idx]) &&
            !mask_->IsNoData(i + drow[fd_idx], j + dcol[fd_idx])) {
            flowout_fracs_[count++] = flowfrac_matrix_[valid_idx];
        } else { // This will not happen, just in case!
            flowout_fracs_[count - 1] = 0.f;
            continue;
        }
        if (flow_dirs.size() == 1) {
            flowout_fracs_[count - 1] = 1.f; // Make sure the only flow direction has all flow fraction
            flowfrac_matrix_[valid_idx] = 1.f;
            continue;
        }
        fd_idx = find_flow_direction_index_ccw(flow_dirs[1]);
        if (mask_->ValidateRowCol(i + drow[fd_idx], j + dcol[fd_idx]) &&
            !mask_->IsNoData(i + drow[fd_idx], j + dcol[fd_idx])) {
            // Make sure the sum of the two flow fractions EXACTLY equals to 1!
            vector<FLTPT> fracin(2);
            fracin[0] = flowfrac_matrix_[valid_idx];
            fracin[1] = 1. - flowfrac_matrix_[valid_idx];
            vector<FLTPT> fracout;
            normalize_flow_fraction(fracin, decimals_, fracout);
            flowout_fracs_[count - 1] = fracout[0];
            flowout_fracs_[count++] = fracout[1];
            flowfrac_matrix_[valid_idx] = fracout[0]; // write back
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
        done = OutputArrayAsGfs(flowout_index_name_, count, flow_out_cells_) &&
                OutputArrayAsGfs(flowout_frac_name_, count, flowout_fracs_);
#endif
    }
    return done;
}
*/